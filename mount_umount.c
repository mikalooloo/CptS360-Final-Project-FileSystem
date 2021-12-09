//mount_umount

#include "header.h"

extern MOUNT mountTable[NMOUNT];
extern MINODE minode[NMINODE];
extern PROC   proc[NPROC], *running;
extern GD    *gp;

int my_mount(char * filesys, char * pathname)
{
	//variable declaration
	int i, fd, dev, ino;
	MINODE *mip;
	MOUNT *mptr; int mi;
	char buf[BLKSIZE];
	SUPER *ext;

	//1. ask for a filesys (a virtual disk) amd mount_point (a DIR pathname).
	//   if no parameters: display current mounted filesystems
	if(strcmp(filesys, "") == 0 || strcmp(pathname, "") == 0)
	{
		//display current mounted filesystems
		//NMOUNT is number of systems mounted
		printf("\ncurrently mounted filesystems:\n");
		for(i = 0; i < NMOUNT; i++)
		{
			if(mountTable[i].dev)
			{
				printf("%s mounted on %s wth dev %d\n", mountTable[i].mount_name, mountTable[i].name, mountTable[i].dev);
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
		if(mountTable[i].dev && !strcmp(mountTable[i].mount_name, filesys))
		{
			printf("\n%s is already mounted: mount failed\n", filesys);
			return -1;
		}
		//else, allocate a free MOUNT table entry
		else if(mountTable[i].dev == 0)
		{
			mi = i;
			mptr = &(mountTable[mi]);
		}
	}
	printf("%s was not already mounted: mount check passed\n", filesys);

	if (mi == -1) {
		printf("\ndid not allocate a free MOUNT table entry: mount failed\n");
		return -1;
	}
	else printf("successfully allocated a free MOUNT table entry: free check passed\n");

	//3. LINUX open filesys for RW; use its fd number as the new DEV;
	//   check whether its an EXT2 file system: if not reject.
	
	fd = open(filesys, O_RDWR);
	if(fd == -1) {
		printf("\n%s failed to open with fd %d: mount failed\n", filesys, fd);
		return -1;
	}
	else printf("%s opened correctly with fd %d: open check passed\n", filesys, fd);

	printf("\nrunning->cwd->dev: %d, new dev: %d\n", running->cwd->dev, fd);
	dev = fd;
    get_block(fd, 1, buf);
    ext = (SUPER*)buf;

	//check if EXT2 file system, if not then reject
    if(ext->s_magic != 0xEF53) {
		printf("\next is not an EXT2 Filesystem: mount failed\n");
		return -1;
	}
	else printf("ext is an EXT2 Filesystem: system check passed\n");
	

	//4. for mount_point: find its ino, then get its minode:
	//	ino = getino(pathnname); //get ino
	//	mip = iget(dev, ino); //get minode in memory;
	
	ino = getino(pathname); //get ino

	if (ino == -1) {
		printf("\n%s does not exist: mount failed\n", pathname);
		return -1;
	}
	else printf("%s does exist: existence check passed\n", pathname);

	mip = iget(running->cwd->dev, ino); //get minode into memory

	//5. check mount_point is a DIR
	//   check mount_point is NOT busy (e.g. can't be someone's CWD)
	
	//check if mount_point is DIR
	if(!S_ISDIR(mip->INODE.i_mode))
	{
		printf("\n%s is not a DIR: mount failed\n", pathname);
		return -1;
	}
	else printf("%s is a DIR: DIR check passed\n", pathname);
	
	//check if mount_point is not busy
	//if (mip->refCount > 2) // if(running->cwd->dev == mip->dev)
	for (i = 0; i < NPROC; i++)
	{
		if (proc[i].cwd == mip) { // making sure it isn't someone else's CWD
			printf("\nDIR %s is busy: mount failed\n", pathname);
			return -1;
		}
	}
	printf("DIR %s is not busy: busy check passed\n", pathname);

	//
	//6. allocate a FREE (dev = 0) mountTable[] for newdev;
	//   record new DEV, ninodes, nblocks, bmap, imap, iblk in mountTable[]
	
	mptr->dev = dev;
	strncpy(mptr->name, pathname, NLENGTH);
	strncpy(mptr->mount_name, filesys, NLENGTH);
	mptr->ninodes = ext->s_inodes_count;
	mptr->nblocks = ext->s_blocks_count;
	get_block(mptr->dev, 2, buf);
	// global var
	gp = (GD *)buf;
	mptr->bmap = gp->bg_block_bitmap;
	mptr->imap = gp->bg_inode_bitmap;
	mptr->blk = gp->bg_inode_table;

	//7. mark mount_point's as being mounted on and let it point at the MOUNT table entry,
	//   which points back to the mount_point minode.
	mip->mounted = 1;
	mip->dirty = 1;
	mip->mptr = mptr;
	mptr->mounted_inode = mip;
	
	// return 1 for success
	printf("\nmount successful\n");
	return 1;
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

	//4. return 1 for success
	printf("\numount successful\n");
	return 1;
}	
 

