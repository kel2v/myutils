#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <string.h>
#include <errno.h>
#include <limits.h>
#include <unistd.h>

char files[1024][PATH_MAX];
int count = 0;

int skipWhiteSpace(FILE *ptr)
{
	char ch;

	do
	{
		if((ch = fgetc(ptr)) == EOF)
		{
			if(feof(ptr) != 0)
			{
				return 0;
			}
			else
			{
				return -1;
			}
		}
	}
	while(ch == ' ' || ch == '\t' || ch == '\\' || ch == '\n');

	if(fseek(ptr, -1, SEEK_CUR) == -1)
	{
		return -1;
	}

	return 1;
}

int scanfile(FILE *ptr, char filename[])
{
	char ch;

	while(1)
	{
		if((ch = fgetc(ptr)) == EOF)
		{
			if(feof(ptr) != 0)
			{
				return 0;
			}
			else
			{
				return -1;
			}
		}
		else if(ch == '#')
		{
			int res;

			if((res = skipWhiteSpace(ptr)) == 0)
			{
				return 0;
			}
			else if(res == -1)
			{
				return -1;
			}

			char token[8];
			fgets(token, 8, ptr);

			if(strcmp(token, "include") == 0)
			{
				if((res = skipWhiteSpace(ptr)) == 0)
				{
					return 0;
				}
				else if(res == -1)
				{
					return -1;
				}

				if(fgetc(ptr) == '"')
				{
					return -2;
				}
			}
		}
	}
}

int scandirectory(DIR *dir_ptr, char dir_path[])
{
	struct dirent *dir_entry = NULL;

	errno = 0;
	if((dir_entry = readdir(dir_ptr)) == NULL)
	{
		if(errno == 0)
		{
			return 0;
		}
		else
		{
			perror(dir_path);
			return -1;
		}
	}
	else if(strcmp(dir_entry->d_name, ".") == 0 || strcmp(dir_entry->d_name, "..") == 0)
	{
		scandirectory(dir_ptr, dir_path);
		return -2;
	}
	else if(dir_entry->d_type == DT_REG)
	{
		char ent_fpath[PATH_MAX];
		strcpy(ent_fpath, dir_path);
		strcat(ent_fpath, "/");
		strcat(ent_fpath, dir_entry->d_name);

		int nlen = strlen(ent_fpath);
		char extension[3] = {ent_fpath[nlen-2], ent_fpath[nlen-1], '\0'};

		if(strcmp(extension, ".h") == 0 || strcmp(extension, ".c") == 0)
		{
			FILE *ptr = NULL;
			if((ptr = fopen(ent_fpath, "r")) == NULL)
			{
				perror(ent_fpath);
			}
			
			int result;
			if((result = scanfile(ptr, ent_fpath)) == 0)
			{
				strcpy(files[count++], ent_fpath);
			}
			else if(result == -1)
			{
				perror(ent_fpath);
			}

			if(ptr != NULL)
			{
				fclose(ptr);
			}
		}

		scandirectory(dir_ptr, dir_path);
		return 2;
	}
	else if(dir_entry->d_type == DT_DIR)
	{
		char entdir_fpath[PATH_MAX];
		strcpy(entdir_fpath, dir_path);
		strcat(entdir_fpath, "/");
		strcat(entdir_fpath, dir_entry->d_name);

		DIR *subdir_ptr = NULL;
		if((subdir_ptr = opendir(entdir_fpath)) == NULL)
		{
			perror(entdir_fpath);
		}
		else
		{
			if(scandirectory(subdir_ptr, entdir_fpath) == -1)
			{
				perror(entdir_fpath);
			}
		}

		if(subdir_ptr != NULL)
		{
			closedir(subdir_ptr);
		}

		scandirectory(dir_ptr, dir_path);
		return 3;
	}
}

int main(int argc, char *args[])
{
	fprintf(stderr, "directory name: %s\n\n", args[1]);

	DIR *dir_ptr = NULL;
	if((dir_ptr = opendir(args[1])) == NULL)
	{
		fprintf(stderr, "failed opening directory: %s\n", args[1]);
		return EXIT_FAILURE;
	}

	int result = scandirectory(dir_ptr, args[1]);
	if(result == -1)
	{
		fprintf(stderr, "error reading directory: %s\n\n", args[1]);
	}

	printf("count = %d\n", count);

	for(int i=0; i<count; i++)
	{
		fprintf(stdout, "%s\n", files[i]);
	}

	closedir(dir_ptr);

	return EXIT_SUCCESS;
}