#include "header.h"

extern int dev, ninodes, imap, bmap, nblocks;

int clr_bit(char *buf, int bit) // clear bit in char buf[BLKSIZE]
{ buf[bit/8] &= ~(1 << (bit%8)); }

int incFreeInodes(int dev)
{
    char buf[BLKSIZE];
    // inc free inodes count in SUPER and GD
    get_block(dev, 1, buf);
    sp = (SUPER *)buf;
    sp->s_free_inodes_count++;
    put_block(dev, 1, buf);
    get_block(dev, 2, buf);
    gp = (GD *)buf;
    gp->bg_free_inodes_count++;
    put_block(dev, 2, buf);
}

int incFreeBlocks(int dev)
{
    char buf[BLKSIZE];
    // inc free inodes count in SUPER and GD
    get_block(dev, 1, buf);
    sp = (SUPER *)buf;
    sp->s_free_blocks_count++;
    put_block(dev, 1, buf);
    get_block(dev, 2, buf);
    gp = (GD *)buf;
    gp->bg_free_blocks_count++;
    put_block(dev, 2, buf);
}
int idalloc(int dev, int ino)
{
    int i;  
    char buf[BLKSIZE];
        // MTABLE *mp = (MTABLE *)get_mtable(dev);
    if (ino > ninodes){  
        printf("inumber %d out of range\n", ino);
        return -1;
    }
    
    get_block(dev, imap, buf);  // get inode bitmap block into buf[]
    
    clr_bit(buf, ino-1);        // clear bit ino-1 to 0

    put_block(dev, imap, buf);  // write buf back
    // update free inode count in SUPER and GD
    incFreeInodes(dev);

    return 0;
}

int bdalloc(int dev, int bno) 
{
    int i;  
    char buf[BLKSIZE];
    // MTABLE *mp = (MTABLE *)get_mtable(dev);
    if (bno > nblocks){  
        printf("inumber %d out of range\n", bno);
        return -1; 
    }
    
    get_block(dev, bmap, buf);  // get inode bitmap block into buf[]
    
    clr_bit(buf, bno-1);        // clear bit ino-1 to 0

    put_block(dev, bmap, buf);  // write buf back
    // update free inode count in SUPER and GD
    incFreeBlocks(dev);

    return 0;
}

int rm_child(MINODE * pmip, char *name) 
{
    // (1). Search parent INODE’s data block(s) for the entry of name

    // (2). Delete name entry from parent directory by
    // (2).1. if (first and only entry in a data block)
        // deallocate the data block; reduce parent’s file size by BLKSIZE;
        // compact parent’s i_block[ ] array to eliminate the deleted entry if it’s
        // between nonzero entries.
    // (2).2. else if LAST entry in block{
        // Absorb its rec_len to the predecessor entry
    // (2).3. else: entry is first but not the only entry or in the middle of a block:
        // move all trailing entries LEFT to overlay the deleted entry;
        // add deleted rec_len to the LAST entry; do not change parent’s file
        // size;
        // How to move trailing entries LEFT? Hint: memcpy(dp, cp, size);
}


int myrmdir(char pathname[128]) {
    char * cp, buf[BLKSIZE];
    
    // (1). get in-memory INODE of pathname:
    int ino = getino(pathname);
    if (ino == -1) {
        printf("ino %d does not exist\n", ino);
        return -1;
    }
    MINODE * mip = iget(dev, ino);
    // (2). verify INODE is a DIR (by INODE.i_mode field);
    if (!S_ISDIR(mip->INODE.i_mode)) {
        printf("INODE is not a DIR\n");
        return -1;
    }
    // minode is not BUSY (refCount = 1);
    if (mip->refCount != 1) {
        printf("minode is busy as refCount is %d\n", mip->refCount);
        return -1;
    }
    // verify DIR is empty (traverse data blocks for number of entries = 2);
    if (mip->INODE.i_links_count > 2) {
        printf("DIR %s has other dirs inside; cannot rmdir %s\n", pathname, pathname);
        return -1;
    }
    else if (mip->INODE.i_links_count == 2) { // only has . and ..
        get_block(mip->dev, mip->INODE.i_block[0], buf);
        dp = (DIR *)buf;
        cp = buf;
        int count = 0;
        while (cp < buf + BLKSIZE){
            ++count;
            cp += dp->rec_len;
            dp = (DIR *)cp;
        }
        if (count > 2) {
            printf("DIR %s has files inside; cannot rmdir %s\n", pathname, pathname);
            return -1;
        }
    }
    // (3). /* get parent’s ino and inode */
    int pino = findino(mip, &ino); //get pino from .. entry in INODE.i_block[0]
    MINODE * pmip = iget(mip->dev, pino);
    // (4). /* get name from parent DIR’s data block */
    findmyname(pmip, ino, pathname); //find name from parent DIR
    // (5). remove name from parent directory */
    rm_child(pmip, pathname);
    // (6). dec parent links_count by 1; mark parent pimp dirty;
    iput(pmip);
    // (7). /* deallocate its data blocks and inode */
    bdalloc(mip->dev, mip->INODE.i_block[0]);
    idalloc(mip->dev, mip->ino);
    iput(mip);
}