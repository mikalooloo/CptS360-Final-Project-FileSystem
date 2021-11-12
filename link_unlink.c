//link_unlink.c

#include "header.h"

extern int dev;
extern char * totalPath();

//trunctate function
INODE *truncate_ino(INODE *i)
{
	for(int j = 0; j < 15; j++)
	{
		i->i_block[j] = 0;
	}
	return i;
}


//link function, cmd: link old_fle new_file
int my_link(char *pathname)
{
	//var dec/init
	char old_file[1024], new_file[1024], parent[1024], child[1024], buf[BLKSIZE];
	int oino, oino2, bnum, needLen, bestLen, newRec;
	MINODE *omip, *omip2;
	char *cp;
	DIR *dp;


	//1 -- verify that file exists and is not a DIR
	oino = getino(old_file);

	if(oino <= 0)
	{
		printf("Attention: file does not exist!\n");
		return -1;
	}

	omip = iget(dev, oino);

	//check if reg
	if(!S_ISREG(omip->INODE.i_mode))
	{
		printf("Attention: not regular!\n");
		iput(omip);
		return -1;
	}

	//check second file
	oino2 = getino(parent);

	if(oino2 <= 0)
	{
		printf("Attention: file does not exist!\n");
		iput(omip);
		return -1;
	}

	omip2 = iget(omip->dev, oino2);

	//check dir parent
	if(!S_ISDIR(omip2->INODE.i_mode))
	{
		printf("Attention: not a directory!\n");
		iput(omip);
		iput(omip2);
		return -1;
	}

	//check if filename already used
	oino2 = search(omip2, child);

	if(oino2 > 0)
	{
		printf("File exits\n");
		iput(omip);
		iput(omip2);
		return -1;
	}

	enter_name(omip2, oino, child);
	/* memset(buf, 0, BLKSIZE);
	needLen = 4*((8+strlen(child)+3)/4);
	bnum = findLastBlock(omip2);
	//check if there is enough room
	get_block(omip2->dev, bnum, buf);
	dp = (DIR*)buf;
	cp = buf;
	while((dp->rec_len + cp)< buf+BLKSIZE)
	{
		cp += dp->rec_len;
		dp = (DIR*)cp;
	}
	bestLen = 4*((8+dp->name_len+3)/4);
	if(dp->rec_len - bestLen >= needLen)
	{
		newRec = dp->rec_len - bestLen;
		dp->rec_len = bestLen;
		cp += dp->rec_len;
		dp = (DIR*)cp;
		dp->inode = oino;
		dp->name_len = strlen(child);
		strncpy(dp->name, child, dp->name_len);
		dp->rec_len = newRec;
	}
	//allocate
	else
	{
		bnum = balloc(omip2->dev);
		dp = (DIR*)buf;
		dp->inode = oino;
		dp->name_len = strlen(child);
		strncpy(dp->name, child, dp->name_len);
		dp->rec_len = BLKSIZE;
	} */


	//put_block(omip2->dev, bnum, buf);
	
	omip->INODE.i_links_count++;//inc INODE's links_count by 1

	//for write back by iput(omip)
	omip->dirty = 1;
	memset(buf, 0, BLKSIZE);
	iput(omip);
	iput(omip2);

	return 1;
}


//my_unlink function to unlink a file, command: unlink filename
//decrements file's link_count by 1 and deletes the file name from its parent DIR
//when file's links_count reaches 0, the file is truly removed by deallocating its data blocks and inode
int my_unlink(char *filename)
{
	//check if filename is empty, if so return 0
	if(strlen(filename) == 0)
	{
		return 0;
	}

	//var initialization
	char buf[1024];
	//int dev = 0;
	
	//(1). get filename's minode
	int ino = getino(filename);
	int mip = iget(dev, ino);

	//check if it exists
	if(ino == 0)
	{
		printf("Attention: filename, %s, does not exist!\n", filename);
		return;
	}
	
	int blk = (ino - 1)/8; //CHECK IF RIGHT 
	int offset = (ino - 1) % 8;

	get_block(dev, blk, buf);

	INODE *mip = (INODE*)buf + offset;

	//check if a REG or symbolic LNK file; can not be a DIR
	if(S_ISDIR(mip->i_mode))
	{
		printf("Attention: Cannot unlink!\n");
		return;
	}

	//(2). remove name entry from parent DIR's data block
	char parent = dirname(filename);
	char child = basename(filename);

	int pino = getino(parent);
	int pmip = iget(dev, pino);

	rm_child(pmip, ino, child);
	pmip->dirty = 1; //ERROR: CHECK TYPE

	iput(pmip);

	//(3). decrement INODE's link_count by 1
	mip->INODE.i_links_count--;
	
	//(4).
	if(mip->INODE.i_links_count > 0)
	{
		mip->dirty = 1; //for write INODE back to disk
	}

	//(5).
	else
	{
		//if links_count = 0: remove filename
		//deallocate all data blocks in INODE
		//deallocate INODE;
		
	}
	//release mip
	iput(mip);
}









