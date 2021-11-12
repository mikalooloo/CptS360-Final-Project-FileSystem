//symlink.c --> includes symlink, readLink functions

#include "header.h"

extern int dev;

//function symlink creates a symbolic link from new_file to old_file
//unlike hard links, bc symlink can link to anything, including DIRs, or even files not on the same
//device.
//symlink command: symlink old_file new_file
int symlink(char *old_file, char *new_file)
{
	//initialize buf
	char buf[BLKSIZE];

	int old_ino = getino(old_file);

	//(1). check: old_file must exist and new_file does not exist
	if(old_ino == -1)
	{
		printf("\nAttention: %s does not exist: symlink failed\n", old_file);
		return -1;
	}
	else printf("%s does exist: passed existence check\n", old_file);

	//MINODE * old_mip = iget(dev, old_ino);

	/* //check if file type is valid
	if(!(S_ISDIR(old_mip->INODE.i_mode) || S_ISREG(old_mip->INODE.i_mode)))
	{
		printf("Attention: file type invalid: symlink failed\n");
		return -1;
	}
 */
	int ino = getino(new_file);

	if (ino != -1) 
	{
		printf("\nAttention: %s already exists: symlink failed\n", new_file);
		return -1;
	}
	else printf("%s does not yet exist: passed existence check\n", new_file);

	
	//(2). creat new_file: change new_file to LNK type
	mycreat(new_file);
	int ino2 = getino(new_file);
	if (ino2 == -1) 
	{
		printf("\nAttention: %s creat failed: symlink failed\n", new_file);
		return -1;
	}
	else printf("%s exists: passed creat check\n", new_file);

	MINODE * mip = iget(dev, ino2);
	mip->INODE.i_mode = 0120000; // setting to lnk type
	mip->dirty = 1;
	//copy name
	strcpy((char*) mip->INODE.i_block, old_file);
	mip->INODE.i_size = strlen(old_file); 
	iput(mip);

	return 0;
}

//readLink function reads the target file name of a symbolic file and returns the length of the target 
//file name
int readLink(char *file, char *buffer)
{
	char buf[BLKSIZE];
	//(1). get file's INODE in memory; verify its a link file
	//(2). copy target filename from INODE.i_block() into buffer
	//(3). return file size
}
