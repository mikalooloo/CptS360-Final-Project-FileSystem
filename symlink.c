//symlink.c --> includes symlink, readLink functions

#include "header.h"

int my_symlink(char *pathname) {
	// separates pathname into old file and new file and calls symlink
}

//function symlink creates a symbolic link from new_file to old_file
//unlike hard links, bc symlink can link to anything, including DIRs, or even files not on the same
//device.
//symlink command: symlink old_file new_file
int symlink(char *old_file, char *new_file)
{
	//initialize buf
	char buf[1024];

	int dev;
	int old_ino = getino(old_file);

	//(1). check: old_file must exist and new_file does not exist
	if(old_ino == 0)
	{
		printf("Attention: %s does not exist!\n", old_file);
		return 0;
	}

	int n_blk = (old_ino - 1) / 8;
	int off = (old_ino - 1) % 8;
	get_block(dev, n_blk, buf);

	INODE *minode = (INODE*)buf + off;

	//check if file type is valid
	if(!(S_ISDIR(minode->i_mode) || S_ISREG(minode->i_mode)))
	{
		printf("Attention: file type invalid: symlink failed\n");
		return -1;
	}

	//(2). creat new_file: change new_file to LNK type
	mycreat(new_file);

	int ino = getino(new_file);
	int blk = (ino -1) / 8;
	int offset = (ino - 1) % 8;
	get_block(dev, blk, buf);

	INODE *new_ino = (INODE*)buf + offset;

	//copy name
	strcpy((char*) new_ino->i_block, new_file);

	put_block(dev, blk, buf);

	return 0;
}

//readLink function reads the target file name of a symbolic file and returns the length of the target 
//file name
int readLink(char *file, char *buffer)
{
	char buf[1024];
	//(1). get file's INODE in memory; verify its a link file
	//(2). copy target filename from INODE.i_block() into buffer
	//(3). return file size
}
