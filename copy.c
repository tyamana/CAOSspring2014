#include <sys/types.h>
#include <dirent.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <stdio.h>
#include <malloc.h>
#include <string.h>
#include <stdlib.h>

int ToFile(char* from, char* where)
{
	int source_fd = open(from, O_RDONLY); 
	if(source_fd == -1)
	{
		perror("open() error ");
		exit(1);
	}
	struct stat fromStat;
	if(stat(from, &fromStat))
	{
		perror("stat() error");
		exit(4);
	}
	
	int dest_fd = open(where, O_WRONLY | O_TRUNC | O_CREAT, fromStat.st_mode);
	if(dest_fd == -1)
	{
		perror("open() error ");
		exit(1);
	}
	char buf[1000];
	int n, byte_counter = 0;
	do
	{
		if(byte_counter == 1000)
		{
			n = write(dest_fd, buf, 1000);
			if(n < 0)
			{
				perror("write() error ");	
				exit(2);
			}
			byte_counter = 0;
		}
		n = read(source_fd, buf + byte_counter, 1);
		if(n != 0)
			byte_counter++;
		if(n < 0)
		{
			perror("read() error ");
			exit(3);
		}
	}
	while(n > 0);
	if(byte_counter > 0)  
	{
		n = write(dest_fd, buf, byte_counter);
		if (n < 0)
		{
			perror("write() error ");
			exit(4);
		}
	}
	
	close(source_fd);
	close(dest_fd);
	return 0;
}

int FileToDir(char* from, char* where)
{
	char* fileName = strrchr(from, '/');
	if(fileName)
		fileName++;
	else 
		fileName = NULL;
	char* path = (char*)malloc((strlen(where) + strlen(fileName) + 2)*sizeof(char));
	if(!path)
	{
		write(1, "malloc() error\n", 15);
		exit(1);
	}
	strcpy(path, where);
	strcat(path, "/");
	strcat(path, fileName);
	int n = ToFile(where, path);
	free(path);
	return n;
}

int DirToDir(char* from, char* where)
{
	struct stat fromStat;
	if(stat(from, &fromStat))
	{
		perror("stat() error()");
		_exit(1);
	}
	if(mkdir(where, fromStat.st_mode) != 0 && errno != EEXIST) 
	{
		perror("making directory error");
		_exit(2);
	}
	if(chmod(where, fromStat.st_mode))
	{
		perror("chmod() error");
		_exit(3);
	}
	DIR* fromDir = opendir(from);
	if(!fromDir)
	{
		perror("directory opening error");
		_exit(4);
	}
	struct dirent* ent;
	ent = readdir(fromDir);
	while(ent)
	{
		if(ent->d_name[0] != '.')
		{
			char* newFrom = (char*)malloc((2+strlen(from)+strlen(ent->d_name))*sizeof(char));
			char* newWhere = (char*)malloc((2+strlen(where)+strlen(ent->d_name))*sizeof(char));
			
			strcpy(newFrom, from);
			strcat(newFrom, "/");
			strcat(newFrom, ent->d_name);
			
			strcpy(newWhere, where);
			strcat(newWhere, "/");
			strcat(newWhere, ent->d_name);
			
			struct stat newFromStat;
			if(stat(newFrom, &newFromStat))
			{
				perror("stat() error()");
				_exit(1);
			}
			if(newFromStat.st_mode & S_IFREG)
			{
				if(ToFile(newFrom, newWhere))
				{
					write(1, "File to file copying error\n", 27);
				}
			}
			else if(newFromStat.st_mode & S_IFDIR)
			{
				if(DirToDir(newFrom, newWhere))
				{
					write(1, "Directory to directory copying error\n", 37);
				}
			}
			
			free(newFrom);
			free(newWhere);
		}
		ent = readdir(fromDir);
	}
	
	closedir(fromDir);
	return 0;
}

int copy(int argc, char** argv)
{
	struct stat whereStat;
	if(stat(argv[argc - 1], &whereStat))
	{
		perror("stat() error");
		_exit(2);
	}
	
	if(argc == 3)
	{
		struct stat fromStat;
		if(stat(argv[1], &fromStat))
		{
			perror("stat() error");
			_exit(2);
		}
		if(fromStat.st_mode & S_IFDIR)
		{
			write(1, "attempt to copy a directory without -R\n", 39);
			_exit(1); 
		}
		if(whereStat.st_mode & S_IFDIR)
			return FileToDir(argv[1], argv[2]);
		return ToFile(argv[1], argv[2]);
	}
	
	//recursive copying
	if(!strcmp(argv[1], "-R"));
	{
		if(!(whereStat.st_mode & S_IFDIR))
		{
			write(1, "error:recursive copying to file\n", 39);
			_exit(1);
		}
		int i;
		for(i = 2; i < argc - 1; i++)
		{
			struct stat fromStat;
			if(stat(argv[i], &fromStat))
			{
				perror("stat() error");
				_exit(2);
			}
			if(!(fromStat.st_mode & S_IFDIR))
				write(1, "error:recursive copying of file\n", 45);
			else
				DirToDir(argv[i], argv[argc - 1]);
		}
		return 0;				
	}
	
	if(whereStat.st_mode & S_IFREG)
	{
		write(1, "error:copying 2 or more files into another\n", 43);
		_exit(3);
	}
	int i;
	for(i = 1; i < argc - 1; i++)
	{
		struct stat fromStat;
		if(stat(argv[i], &fromStat))
		{
			perror("stat() error");
			_exit(2);
		}
		if(!(fromStat.st_mode & S_IFREG))
		{
			write(1, "error:attempt to copy a directory without -R\n", 45);
			_exit(3);
		}
		FileToDir(argv[i], argv[argc - 1]);
	}
	
	return 0;
}

int main(int argc, char** argv)
{
	if(argc < 3)
	{
		write(1, "Not enough parameters\n", 22);
		return -1;
	}
	copy(argc, argv);
	
	return 0;
}


