//mount_umount

#include "header.h"

extern MOUNT mountTable[NMOUNT];
extern MINODE minode[NMINODE];
extern PROC   proc[NPROC], *running;
extern GD    *gp;

int my_mount(char * pathname, char * filesys)
{
	//variable declaration
	int i, fd, dev, ino;
	MINODE *mip;
	MOUNT *mntptr;
	int mnti;
	char buf[BLKSIZE];
	SUPER *ext;

	//1. ask for a filesys (a virtual disk) amd mount_point (a DIR pathname).
	//   if no parameters: display current mounted filesystems
	if(strcmp(pathname, "") == 0)
	{
		//display current mounted filesystems
		//NMOUNT is number of systems mounted
		printf("File systems currently mounted: \n");
		for(i = 0; i < NMOUNT; i++)
		{
			if(mountTable[i].dev)
			{
				printf("/t%s\t%s\n", mountTable[i].name, mountTable[i].mount_name);
			}
		}
		return 0;
	}	

	//2. check whether filesys is already mounted:
	//   (you may store mounted filesys name in the MOUNT entry)
	//   if already mounted, reject:
	//   else: allocate a free MOUNT table entry (dev = 0 means FREE).
	
	//searching through mount array
	for(i = 0; i < NMOUNT; i++)
	{
		//if already mounted reject
		if(mountTable[i].dev && !strcmp(mountTable[i].name, pathname))
		{
			printf("Attention: the filesystem is already mounted!\n");
			return -1;
		}
		//else, allocate a free MOUNT table entry
		else if(mountTable[i].dev == 0)
		{
			mnti = i;
			mntptr = &(mountTable[mnti]);
		}
	}

	if (mnti == -1) {
		printf("\ndid not allocate a free MOUNT table entry: mount failed\n");
		return -1;
	}

	//3. LINUX open filesys for RW; use its fd number as the new DEV;
	//   check whether its an EXT2 file system: if not reject.
	
	fd = open(pathname, O_RDWR);
	if(fd == -1) {
		printf("Attention: Invalid file descriptor!\n");
		return -1;
	}

	dev = fd;
    get_block(fd, 1, buf);
    ext = (SUPER*)buf;

	//check if EXT2 file system, if not then reject
    if(ext->s_magic != 0xEF53) {
		printf("Attention: Not an EXT2 Filesystem!!\n");
		return -1;
	}
	

	//4. for mount_point: find its ino, then get its minode:
	//	ino = getino(pathnname); //get ino
	//	mip = iget(dev, ino); //get minode in memory;
	
	ino = getino(pathname); //get ino
	mip = iget(dev, ino); //get minode into memory

	//5. check mount_point is a DIR
	//   check mount_point is NOT busy (e.g. can't be someone's CWD)
	
	//check if mount_point is DIR
	if(!S_ISDIR(mip->INODE.i_mode))
	{
		printf("Attention: Invalid mount point!\n");
		return 0;
	}
	
	//check if mount_point is not busy
	if(running->cwd->dev == mip->dev)
	{
		printf("Attention: mount point is not busy, directory is busy\n");
		return 0;
	}

	//
	//6. allocate a FREE (dev = 0) mountTable[] for newdev;
	//   record new DEV, ninodes, nblocks, bmap, imap, iblk in mountTable[]
	
	mntptr->dev = dev;
	strcpy(mntptr->name, pathname);
	strcpy(mntptr->mount_name, filesys);
	mntptr->ninodes = ext->s_inodes_count;
	mntptr->nblocks = ext->s_blocks_count;
	get_block(mntptr->dev, 2, buf);
	// global var
	gp = (GD *)buf;
	mntptr->bmap = gp->bg_block_bitmap;
	mntptr->imap = gp->bg_inode_bitmap;
	mntptr->blk = gp->bg_inode_table;

	//7. mark mount_point's as being mounted on and let it point at the MOUNT table entry,
	//   which points back to the mount_point minode.
	mip->mounted = 1;
	mip->mptr = mntptr;
	mntptr->mounted_inode = mip;
	// mntptr->mounted_inode = iget(dev, ROOT_INODE);
	//return 0 for success
	//
	
	return 0;
	
}

int my_umount(char *filesys)
{
	int i, j, count = 0;

	MOUNT* umnt = 0;

	//1. search the MOUNT table to check filesys is indeed mounted
	
	for(i = 0; i < NMOUNT; i++)
	{
		if(strcmp(mountTable[i].name, filesys) == 0)
		{
			count++;
			break;
		}
	}
	//not mounted
	if(count == 0)
	{
		printf("Attention: %s is not mounted!\n", filesys);
		return -1;
	}

	//2. check whether any file is still active in the mounted filesys;
	//	e.g. someone's CWD or opened files are still there,
	//   if so, the mounted filesys is BUSY ==> cannot be umounted yet.
	//   HOW to check? Ans: by checking all minode[].dev
	//
	//compare cwd->dev to MINODE->devs
	for(j = 0; j < NMINODE; j++)
	{
		if(minode[j].refCount && minode[j].mounted && (minode[j].mptr->dev == umnt->dev))
		{
			//close(umnt->dev);
			minode[j].mptr = 0;
			iput(&minode[j]);
			umnt->mounted_inode = 0;
			umnt->dev = 0;
			break;
		}
	}
		
	//3. find the mount_point's inode (which should be in mem while its mounted on)
	//   reset it to "not mounted"
	//   then iput() the minode (bc it was iget()ed during mounting)
	//
	//4. return 0 for success
	return 0;

}	
 

