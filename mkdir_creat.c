#include "type.h"

// helper functions and such
extern int imap, ninodes;

int tst_bit(char *buf, int bit); // in Chapter 11.3.1

int set_bit(char *buf, int bit); // in Chapter 11.3.1

int ialloc(int dev)  // allocate an inode number from inode_bitmap
{
  int  i;
  char buf[BLKSIZE];

  // read inode_bitmap block
  get_block(dev, imap, buf);

  for (i=0; i < ninodes; i++){
    if (tst_bit(buf, i)==0){
        set_bit(buf, i);
        put_block(dev, imap, buf);
        printf("allocated ino = %d\n", i+1); // bits count from 0; ino from 1
        return i+1;
    }
  }
  return 0;
}

// meat of the bones here

int mkdir(int dev) {
    // (1). divide pathname into dirname and basename, e.g. pathname=/a/b/c, then dirname=/a/b; basename=c;

    // (2). // dirname must exist and is a DIR:
    int pino = getino(dirname);
    int pmip = iget(dev, pino);
    //check pmip->INODE is a DIR

    // (3). // basename must not exist in parent DIR:
    search(pmip, basename); //must return 0;

    // (4). call kmkdir(pmip, basename) to create a DIR;
    kmkdir(pmip, basename, dev);
    
    // (5). increment parent INODE’s links_count by 1 and mark pmip dirty;
    iput(pmip);
}

int kmkdir(int pmip, int basename, int dev) {
  // kmkdir() consists of 4 major steps:

  // (4).1. Allocate an INODE and a disk block:
  int ino = ialloc(dev);
  int blk = balloc(dev);

  // (4).2. mip = iget(dev, ino) // load INODE into a minode
  //initialize mip->INODE as a DIR INODE;
  // mip->INODE.i_block[0] = blk; other i_block[ ] = 0;
  //mark minode modified (dirty);
  //iput(mip); // write INODE back to disk

  //(4).3. make data block 0 of INODE to contain . and .. entries;
  // write to disk block blk.
    
  // (4).4. enter_child(pmip, ino, basename); which enters (ino, basename) as a dir_entry to the parent INODE;
}

int creat() {

}

int enter_name() {

}