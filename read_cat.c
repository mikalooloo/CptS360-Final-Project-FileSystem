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
		printf("oftp is not valid: myread failed\n");
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
			get_block(mip->dev, mip->INODE.i_block[12], (char *)buf2);

			//set ip
			ip = buf2 + lbk - 12;
			//set blk to ip
			blk = *ip;
		}
		//for double indirect
		else
		{
			//get block
			get_block(mip->dev, mip->INODE.i_block[13], (char *)buf2);

			//set indirect_blk, and indirect_off
			indirect_blk = (lbk - 256 - 12) / 256;
			indirect_off = (lbk - 256 - 12) % 256;

			//output blk, and offset
			//printf("blk = %d, offset = %d\n", indirect_blk, indirect_off);
			//getchar();

			//set ip with indirect_blk
			ip = buf2 + indirect_blk;
			//getchar();
			
			//get block
			get_block(mip->dev, *ip, (char *)buf2);
			//getchar();
			
			//set ip with indirect_off
			ip = buf2 + indirect_off;

			//set blk to ip pointer
			blk = *ip;
			//getchar();
		}


		//start of read optimization
		//set and initialize two temp vars to track remianing bytes and availiable, using bitwise XOR operator
		/*int temp = remain ^ ((avil ^ remain) & -(avil < remain));
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
		}*/

		//get data block into readbuf
		get_block(mip->dev, blk, readbuf);

		//copy from startByte to buf[], at most remain bytes in this block
		char *cp = readbuf + startByte;

		//number of bytes that reamin in readbuf
		remain = BLKSIZE - startByte;

		// this is sorta what I did for cat, and it's basically what you've done but
		// I think this is a little more concise/clearer to read
		if (remain < nbytes) nbytes = remain;
		
		memmove(cq, cp, nbytes);
        cp += nbytes; cq += nbytes;
        oftp->offset += nbytes;
		count += nbytes;
		avil -= nbytes; remain -= nbytes;
		nbytes -= nbytes;

        if (oftp->offset > mip->INODE.i_size)
            mip->INODE.i_size = oftp->offset;

	}
	//return count
	return count;

}

int read_file(int fd, int nbytes)
{
	//preparations:
	//assume file is opened for RD or RW
	//ask for a fd and nbytes to read
	//verify that fd is indeed opened for RD and RW
	//return (myread(fd, buf, nbytes))

	//from bytes to int
	int actual = 0;
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
	if(fd == 0)
	{
		printf("Attebtion: no fd!\n");
		return 0;
	}

	if(nbytes == 0)
	{
		printf("Attention: no bytes!\n");
		return 0;
	}

	//return bytes
	actual = myread(fd, buf, nbytes);

	//check actual
	if(actual == -1)
	{
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

	 // check for valid pathname
    if (validPathname(pathname) == -1) {
      printf("\npathname is not valid: cat failed\n");
      return -1;
    }

	int fd = open_file(pathname, 0); // open_file takes file name then an int depending on what mode, R = 0

	if (fd == -1) {
        printf("open failure\n");
        close_file(fd); 
        printf("\nfile %s failed to open with fd %d: cat failed\n", pathname, fd);
        return -1;
    }
    else printf("file %s opened correctly with fd %d: open check passed\n", pathname, fd);

	printf("\n");
	while(n = myread(fd, mybuf, 1024))
	{
		mybuf[n] = 0;
		printf("%s", mybuf);
	}
	printf("\n");
	close_file(fd);

	printf("\ncat successful\n");
}
