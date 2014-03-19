#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>
#include <pwd.h>
#include <grp.h>
#include <time.h>
#include <locale.h>
#include <langinfo.h>
#include <string.h>
#include <errno.h>
#include <stdio.h>

void PrintRight(struct stat inf, int dirFlag)
{
	char right[11] = "----------\0";
	
	if(dirFlag)
		right[0] = 'd';
		
	if(inf.st_mode & S_IRUSR)
		right[1] = 'r';
	if(inf.st_mode & S_IWUSR)
		right[2] = 'w';
	if(inf.st_mode & S_IXUSR)
		right[3] = 'x';
		
	if(inf.st_mode & S_IRGRP)
		right[4] = 'r';
	if(inf.st_mode & S_IWGRP)
		right[5] = 'w';
	if(inf.st_mode & S_IXGRP)
		right[6] = 'x';
		
	if(inf.st_mode & S_IROTH)
		right[7] = 'r';
	if(inf.st_mode & S_IWOTH)
		right[8] = 'w';
	if(inf.st_mode & S_IXOTH)
		right[9] = 'x';	
		
	printf("%s ", right);			
}

void GetPath(char* path, char* dirName, struct dirent* ent)
{
	strcpy(path, dirName);
	strcat(path, ent->d_name);
	strcat(path, "\0");
}

void PrintInf(char* dirName, struct dirent* ent, int dirFlag)
{
	char* path;
	struct stat inf;
	int size = strlen(dirName) + strlen(ent->d_name) + 1;
	path = malloc(size*sizeof(char));
	GetPath(path, dirName, ent);
	stat(path, &inf);
	free(path);
	
	PrintRight(inf, dirFlag);
	//Hard-links number
	printf("%ld ", inf.st_nlink);
	//User
	printf("%s ", getpwuid(inf.st_uid)->pw_name);
	//Group
	printf("%s ", getgrgid(inf.st_uid)->gr_name);
	//Size
	printf("%ld ", inf.st_size);
	//Date
	char datestring[256];
	struct tm *tm = localtime(&inf.st_mtime); 
	strftime(datestring, sizeof(datestring), nl_langinfo(D_T_FMT), tm);
	printf("%s ", datestring);
}

int FilesNumber(char* dirName)
{
	DIR* dir = opendir(dirName);
	if(!dir)
	{
		perror("Can not open the directory");
		return -1;
	}
	
	struct dirent* ent;
	ent = readdir(dir);
	int number = 0;
	while(ent)
	{
		number++;
		ent = readdir(dir);
	}
	
	closedir(dir);
	return number;
}

int PrintDir(char* dirName, int flag[2])
{
	DIR* dir = opendir(dirName);
	if(!dir)
	{
		perror("Can not open the directory");
		return -1;
	}
	
	struct dirent* ent;
	
	if(flag[0])
		printf("Total %d\n", FilesNumber(dirName) - 2);
		
	ent = readdir(dir);
	while(ent)
	{
		if(ent->d_name[0] != '.')
		{
			if(flag[0])
				PrintInf(dirName, ent, ent->d_type & DT_DIR);
			printf("%s\n", ent->d_name);
		}
		ent = readdir(dir);
	}
	printf("\n");
	if(errno)
	{
		perror("Can not read the directory");
		return -2;
	}
	
	closedir(dir);
	return 0;
}

int ls(char* dirName, int flag[2])
{
	PrintDir(dirName, flag);
	if(flag[1])
	{
		DIR* dir = opendir(dirName);
		if(!dir)
		{
			perror("Can not open the directory");
			return -1;
		}
		
		struct dirent* ent;
		ent = readdir(dir);
		while(ent)
		{
			if(ent->d_name[0] != '.')
				if(ent->d_type & DT_DIR)
				{
					int size = strlen(dirName) + strlen(ent->d_name) + 2;
					char* newDir = malloc(size*sizeof(char));
					strcpy(newDir, dirName);
					strcat(newDir, ent->d_name);
					strcat(newDir, "/");
					ls(newDir, flag);
					free(newDir);
				}
			ent = readdir(dir);	
		}
		
		if(errno)
		{
			perror("Can not read the directory");
			return -2;
		}
		
		closedir(dir);
	}
	
	return 0;
}

int main(int argc, char** argv)
{
	if (argc > 3)
	{
		write(2, "Wrong number of parameters\n", 27);
		return 0; 
	}
	
	int i, flag[2] = {0, 0};
	for(i = 1; i < argc; i++)
	{
		if (!strcmp(argv[i], "-l"))
			flag[0] = 1;
		if (!strcmp(argv[i], "-R"))
			flag[1] = 1;
	}
	ls("./", flag);
	
	return 0;
}
	
