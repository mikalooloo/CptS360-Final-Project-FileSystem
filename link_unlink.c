//link_unlink.c

#include "header.h"

extern int dev;
extern char * totalPath();
extern PROC   proc[NPROC], *running;
extern char * name[64];

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
int my_link(char *old_file, char *new_file)
{
	//var dec/init
	char * parent = (char *)malloc(sizeof(new_file)), * child = (char *)malloc(sizeof(new_file)), buf[BLKSIZE];
	int oino, oino2, bnum, needLen, bestLen, newRec;
	MINODE *omip, *omip2;
	char *cp;
	DIR *dp;

	 // check for valid pathname
    if (validPathname(old_file) == -1) {
      printf("\nfirst pathname is not valid: link failed\n");
      return -1;
    }
	if (validPathname(new_file) == -1) {
	printf("\nsecond pathname is not valid: link failed\n");
      return -1;
	}

	//1 -- verify that file exists and is not a DIR
	oino = getino(old_file);

	if(oino == -1)
	{
		printf("\nfirst file %s does not exist: link failed\n", old_file);
		return -1;
	}
	else printf("first file %s exists: file check passed\n", old_file);

	omip = iget(dev, oino);

	//check if reg
	if(!S_ISREG(omip->INODE.i_mode))
	{
		printf("\nfirst file %s is not regular: link failed\n", old_file);
		iput(omip);
		return -1;
	}
	else printf("first file %s is regular: reg check passed\n", old_file);

	//check second file
	// see if valid
	if (strcmp(new_file, "")==0) {
		printf("\nsecond filename is not valid: link failed\n");
		return -1;
	}
	oino2 = getino(new_file);

	// if valid does it exist
	if(oino2 != -1)
	{
		printf("\nsecond file %s already exists: link failed\n", new_file);
		iput(omip);
		return -1;
	}
	else printf("second file %s does not exist yet: file check passed\n", new_file);

	// tokenizing filename to check new_file
    separatePathname(new_file, &parent, &child, "link");

	// checking new_file
	oino2 = getino(parent);

	if (oino2 == -1) 
	{
		printf("parent %s does not exist: link failed\n", parent);
		return -1;
	}
	else printf("parent %s exists: parent check passed\n", parent);

	omip2 = iget(omip->dev, oino2);

	//check dir parent
	if(!S_ISDIR(omip2->INODE.i_mode))
	{
		printf("parent %s not a directory: link failed\n", parent);
		iput(omip);
		iput(omip2);
		return -1;
	}
	else printf("parent %s is a DIR: DIR check passed\n", parent);

	// make new_file
	enter_name(omip2, oino, child);
	
	omip->INODE.i_links_count++;//inc INODE's links_count by 1

	//for write back by iput(omip)
	omip->dirty = 1;
	memset(buf, 0, BLKSIZE);
	iput(omip);
	iput(omip2);

	printf("\nlink successful\n");
	return 1;
}


//my_unlink function to unlink a file, command: unlink filename
//decrements file's link_count by 1 and deletes the file name from its parent DIR
//when file's links_count reaches 0, the file is truly removed by deallocating its data blocks and inode
int my_unlink(char *filename)
{
	// check for valid pathname
    if (validPathname(filename) == -1) {
      printf("\npathname is not valid: unlink failed\n");
      return -1;
    }

	//var initialization
	char buf[1024];
	//int dev = 0;
	
	char * parent = (char *)malloc(sizeof(filename)), * child = (char *)malloc(sizeof(filename));
    separatePathname(filename, &parent, &child, "unlink");

	int ino = getino(filename);
	if (ino == -1) 
	{
		printf("\nc%s does not exist: unlink failed\n", filename);
		return -1;
	}
	else printf("%s exists: existing file check passed\n", filename);

	MINODE * mip = iget(dev, ino);

	//check if a REG or symbolic LNK file; can not be a DIR
	if(S_ISDIR(mip->INODE.i_mode))
	{
		printf("%s is a DIR: unlink failed\n", child);
		return -1;
	}
	else printf("%s is not a DIR: DIR check passed\n", child);

	int pino = getino(parent);
	//check if it exists
	if(pino == -1)
	{
		printf("%s does not exist: unlink failed\n", parent);
		return -1;
	}
	else printf("file %s exists: existing file check passed\n", parent);
	
	MINODE * pmip = iget(dev, pino);

	int blk = (ino - 1)/8; //CHECK IF RIGHT 
	int offset = (ino - 1) % 8;

	//get_block(dev, blk, buf);

	//mip = (INODE*)buf + offset;

	//(2). remove name entry from parent DIR's data block
	rm_child(pmip, child);
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
		for (int i = 0; i < 12; ++i) {
			if (mip->INODE.i_block[i] == 0) break;
			bdalloc(mip->dev, mip->INODE.i_block[i]);
			mip->INODE.i_block[i] = 0;
		}

		idalloc(dev, ino);
		mip->INODE.i_blocks = 0;
		mip->INODE.i_size = 0;
		mip->dirty = 1;
	}
	//release mip
	iput(mip);

	printf("\nunlink successful\n");
}









