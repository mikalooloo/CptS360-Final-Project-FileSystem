//read/cat

#include "header.h"

extern PROC   proc[NPROC], *running;

int myread(int fd, char buf[], int nbytes)
{
	//(1) int count = 0
	//avil = fileSize - OFT's offset (number of bytes still available in file)
	//char *cq = buf
	//
	//(2) while(nbytes && avil)
	//{
	//	compute LOGICAL BLOCK number and startByte in that block from offset
	//	lbk = oftp -> offset/BLKSIZE;
	//	startByte = oftp->offset % BLKSIZE;
	//
	//attn: only shown how to read DIRECT BLOCKS, must do INDIRECT and D_INDIRECT
	//
	//if(lbk < 12) //lbk is a direct block
	//{
	//	blk = mip->INODE.I_block[lbk]; //map LOGICAL lbk to PHYSICAL lbk
	//}
	//else if(lbk >= 12 && lbk < 256 + 12)
	//{
	//	//indirect blocks
	//}
	//else
	//{
	//	//double indirect blocks
	//}
	//
	//get the data block into readbuf[BLKSIZE]
	//get_block(mip->dev, blk, readbuf);
	//
	//copy from startByte to buf[], at most remain bytes in this block
	//char *cp = readbuf + startByte;
	//remain = BLKSIZE - startByte; //number of bytes remain in readbuf[]
	//
	//while(remain > 0)
	//{
	//	*cq++ = *cp++; //copy byte from readbuf[] into buf[]
	//	oftp->offset++; //advance offset
	//	count++; //inc count as number of bytes read
	//	avil--: nbytes--; remain--;
	//	if(nbytes < 0 || avil <= 0)
	//		break;
	//}
	//
	//if one data block is not enough, loop back to OUTER while for more
	//}
	//printf("myread: read %d char from file descriptor %d\n", count, fd);
	//return count; //count is the actual number of bytes read
	//
	//ATTN: OPTIMIZATION OF THE READ CODE
	//instead of reading one byte at a time and updating the counters on each byte,
	//try to make the read loops more efficent by reading a max chunk of data at a
	//time (and adjust the counters accordingly)
	//
	//REQUIRED: optimize the read algorithm in final project!!
	
	//declare required vars
	MINODE *mip;
	OFT *oftp = (OFT *)malloc(sizeof(OFT));;
	int count = 0, lbk, blk, startByte, remain, ino, avil, *ip;

	int indirect_blk, indirect_off;
	int buf2[BLKSIZE];

	char readbuf[1024], temp[1024];

	//set oftp
	oftp = running->fd[fd];
	
	//check oftp
	if(!oftp)
	{
		printf("Attention: file for write!\n");
		return -1;
	}

	//set mip
	mip = oftp->minodePtr;

	//calc bytes to read, bytes still available in file
	avil = mip->INODE.i_size - oftp->offset;
	//set cq to buf, cq points at buf
	char *cq = buf;

	//start of outer while loop
	while(nbytes && avil)
	{
		//compute LOGICAL BLOCK number lbk and startByte in that block from offset
		lbk = oftp->offset / BLKSIZE;
		startByte = oftp->offset % BLKSIZE;

		//for direct block
		if(lbk < 12)
		{
			//map LOGICAL lbk to PHYSICAL blk
			blk = mip -> INODE.i_block[lbk];
		}
		//for indirect block
		else if(lbk >= 12 && lbk < 256+12)
		{
			//get block
			get_block(mip->dev, mip->INODE.i_block[12], readbuf);

			//set ip
			ip = (int*)readbuf + lbk - 12;
			//set blk to ip
			blk = *ip;
		}
		//for double indirect
		else
		{
			//get block
			get_block(mip->dev, mip->INODE.i_block[13], readbuf);

			//set indirect_blk, and indirect_off
			indirect_blk = (lbk - 256 - 12) / 256;
			indirect_off = (lbk - 256 - 12) % 256;

			//output blk, and offset
			printf("blk = %d, offset = %d\n", indirect_blk, indirect_off);
			getchar();

			//set ip with indirect_blk
			ip = (int*)readbuf + indirect_blk;
			getchar();
			
			//get block
			get_block(mip->dev, *ip, readbuf);
			getchar();
			
			//set ip with indirect_off
			ip = (int*)readbuf + indirect_off;

			//set blk to ip pointer
			blk = *ip;
			getchar();
		}

		//get data block into readbuf
		get_block(mip->dev, blk, readbuf);

		//copy from startByte to buf[], at most remain bytes in this block
		char *cp = readbuf + startByte;

		//number of bytes that reamin in readbuf
		remain = BLKSIZE - startByte;

		//start of read optimization
		//set and initialize two temp vars to track remianing bytes and availiable, using bitwise XOR operator
		int temp = remain ^ ((avil ^ remain) & -(avil < remain));
		int temp2 = nbytes ^ ((temp ^ nbytes) & -(temp < nbytes));

		//check available and remianing bytes
		while(remain > 0)
		{
			//copy cp into cq with size of temp2 bytes
			strncpy(cq, cp, temp2);

			//add to temp2
			oftp->offset += temp2;
			count += temp2;
			
			//subtract from temp2
			avil -= temp2;
			nbytes -= temp2;
			remain -= temp2;

			//check if bytes are <= 0, and available <= 0, if so then break
			if(nbytes <= 0 || avil <= 0)
			{
				break;
			}
		}
	}
	//return count
	return count;

}

int read_file(char *pathname)
{
	//preparations:
	//assume file is opened for RD or RW
	//ask for a fd and nbytes to read
	//verify that fd is indeed opened for RD and RW
	//return (myread(fd, buf, nbytes))
	
	//declare path, and second path chars
	char path[256], second_path[256];
	//split_paths(pathname, path, second_path);

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


	int fd = open_file(pathname, 0); // open_file takes file name then an int depending on what mode, R = 0

	printf("\n");
	while(n = myread(fd, mybuf, 1024))
	{
		mybuf[n] = 0;
		printf("%s", mybuf);
	}
	close_file(fd);

	printf("\ncat successful\n");
}
