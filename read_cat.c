//read/cat

#include "header.h"

int read_file()
{
	//assume file is opened for RD or RW
	//ask for a fd and nbytes to read
	//verify that fd is indeed opened for RD and RW
	//return (myread(fd, buf, nbytes))
	
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
