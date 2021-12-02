//read/cat

#include "header.h"

int read_file(char *pathname)
{
	//preparations:
	//assume file is opened for RD or RW
	//ask for a fd and nbytes to read
	//verify that fd is indeed opened for RD and RW
	//return (myread(fd, buf, nbytes))
	
	//declare path, and second path chars
	char path[256], second_path[256];
	split_paths(pathname, path, second_path);

	//from bytes to int
	int nbytes = atoi(second_path), actual = 0;
	int fd = 0;
	OFT *oftp;
	INODE *pip;
	MINODE *pmip;
	int i;
	char buf[nbytes + 1];
	MINODE *mip;
	INODE *ip;

	//copy buf
	strcpy(buf, "");

	//check fd
	if(!strcmp(pathname, ""))
	{
		printf("Attebtion: no fd!\n");
		return 0;
	}

	//convert fd into int
	fd = atoi(pathname);

	if(!strcmp(second_path, ""))
	{
		printf("Attention: no bytes!\n");
		return 0;
	}

	//return bytes
	actual = myread(fd, buf, nbytes);

	//check actual
	if(actual == -1)
	{
		strcpy(second_path, "");
		return 0;
	}

	//null
	buf[actual] = '\0';

	//output result of actual and buf
	printf("actual = %d buf = %s\n", actual, buf);
	return actual;
	
}

int myread(int fd, char buf[], int nbytes)
{
	//int count = 0
	//avil = fileSize - OFT's offset (number of bytes still available in file)
	//char *cq = buf
	//
	//while(nbytes && avil)
	//	compute LOGICAL BLOCK number and startByte in that block from offset
	//	...
}

//cat
int my_cat(char *pathname)
{
	//cat filename:
	//	char mybuf[1024], dummy = 0; (null char at end of mybuf[]
	//	int n;
	//
	//(1) int fd = open filename for READ
	//(2) while(n = read(fd, mybuf[1024], 1024))
	//	{
	//		mybuf[n] = 0; //as a terminated null string
	//		//printf("%s", mybuf);
	//		spit out chars from mybuf[]
	//	}
	//(3) close fd
	
	char mybuf[1024], dummy = 0;
	int n;

	char* temp;
	strcpy(temp, " R");

	int fd = open_file(temp);

	while(n = myread(fd, mybuf, 1024))
	{
		mybuf[n] = 0;
		printf("%s", mybuf);
	}
	close_file(fd);
}
