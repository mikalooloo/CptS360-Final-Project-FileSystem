#include "header.h"


// helper functions and such
extern int imap, bmap, ninodes, dev;
extern char * name[64];
extern MINODE minode[NMINODE];
extern PROC   proc[NPROC], *running;
extern MINODE *root;

int tst_bit(char *buf, int bit) // in Chapter 11.3.1 
{
  return buf[bit/8] & (1 << (bit % 8));
}

int set_bit(char *buf, int bit) // in Chapter 11.3.1
{
  buf[bit/8] |= (1 << (bit % 8));
}

int decFreeInodes(int dev)
{
  char buf[BLKSIZE];
  // dec free inodes count in SUPER and GD
  get_block(dev, 1, buf);
  sp = (SUPER *)buf;
  sp->s_free_inodes_count--;
  put_block(dev, 1, buf);
  get_block(dev, 2, buf);
  gp = (GD *)buf;
  gp->bg_free_inodes_count--;
  put_block(dev, 2, buf);
}

int decFreeBlocks(int dev)
{
  char buf[BLKSIZE];
  get_block(dev, 1, buf);
  sp = (SUPER *)buf;
  sp->s_free_blocks_count--;
  put_block(dev, 1, buf);
  get_block(dev, 2, buf);
  gp = (GD *)buf;
  gp->bg_free_blocks_count--;
  put_block(dev, 2, buf);
}

int ialloc(int dev)  // allocate an inode number from inode_bitmap
{
  int  i;
  char buf[BLKSIZE];

  // read inode_bitmap block
  //MTABLE *mp = (MTABLE *)get_mtable(dev);
  get_block(dev, imap, buf);

  for (i=0; i < ninodes; i++){
    if (tst_bit(buf, i)==0){
        set_bit(buf, i);
        put_block(dev, imap, buf);
        printf("allocated ino = %d\n", i+1); // bits count from 0; ino from 1
        decFreeInodes(dev);
        return i+1;
    }
  }
  return 0;
}

int balloc(int dev) 
{
  char buf[BLKSIZE];

  get_block(dev, bmap, buf);

  for (int i = 0; i < 12; ++i) {
    if (tst_bit(buf, i) == 0) {
      set_bit(buf, i);
      decFreeBlocks(dev);
      put_block(dev, bmap, buf);
      return i + 1;
    }
  }
}
// meat of the bones here

int enter_name(MINODE * pip, int ino, char * name) {
  DIR * dp;
  char * cp;
  char buf[BLKSIZE];
  int blk;

  int ideal_length = 0, need_length = 0;
  
  // assume only 12 direct blocks,
  // for each data block of parent DIR
  for (int i = 0; i < 12; ++i) {
    if (pip->INODE.i_block[i] == 0)
      break; // go to step 5
    else
      blk = pip->INODE.i_block[i];

    // (1). Get parent’s data block into a buf[ ];
    get_block(pip->dev, pip->INODE.i_block[i], buf);
    dp = (DIR *)buf;
    cp = buf;


    // (2). In a data block of the parent directory, each dir_entry has an ideal length
    ideal_length = 4 * ((8 + dp->name_len + 3)/4); // a multiple of 4
    // All dir_entires rec_len = ideal_length except the last entry
    // The rec_len of the last entry is to the end of the block, which may be larger than ideal_length

    // (3). In order to enter a new entry of name with n_len, the needed length is
    need_length = 4* ((8 + strlen(name) + 3)/4); // a multiple of 4

    // (4). Step to the last entry in the data block:
    get_block(pip->dev, pip->INODE.i_block[i], buf);
    dp = (DIR *)buf;
    cp = buf;
    while (cp + dp->rec_len < buf + BLKSIZE){
      cp += dp->rec_len;
      dp = (DIR *)cp;
    }

    // dp NOW points at last entry in block
    // remain = LAST entry’s rec_len - its ideal_length;
    ideal_length = 4 * ((8 + dp->name_len + 3)/4);
    int remain = dp->rec_len - ideal_length;
    if (remain >= need_length) {
    // enter the new entry as the LAST entry and
    // trim the previous entry rec_len to its ideal_length;
      dp->rec_len = ideal_length;
      cp += dp->rec_len;
      dp = (DIR *)cp;

      dp->inode = ino;
      strcpy(dp->name, name);
      dp->name_len = strlen(name);
      dp->rec_len = remain;
  
      put_block(dev, blk, buf);
      return 0;
    }
    else { // no space in existing data blocks
      // Allocate a new data block; increment parent size by BLKSIZE;
      // Enter new entry as the first entry in the new data block with rec_len¼BLKSIZE.
      pip->INODE.i_size = BLKSIZE;
      blk = balloc(dev);
      pip->INODE.i_block[i] = blk;
      pip->dirty = 1;

      get_block(dev, blk, buf);
      dp = (DIR *)buf;
      cp = buf;

      dp->name_len = strlen(name);
      strcpy(dp->name, name);
      dp->inode = ino;
      dp->rec_len = BLKSIZE;

      put_block(dev, blk, buf);
      return 1;
    }
  }

}

int kmkdir(MINODE * pmip, char basename[128], int dev) {
  // kmkdir() consists of 4 major steps:

  // (4).1. Allocate an INODE and a disk block:
  int ino = ialloc(dev);
  int bno = balloc(dev);

  // (4).2. mip = iget(dev, ino) // load INODE into a minode
  //initialize mip->INODE as a DIR INODE;
  MINODE *mip = iget(dev, ino);
  INODE *ip = &mip->INODE;
  /**ip = (INODE){
    ip->i_mode = 0x41ED, // 040755: DIR type and permissions
    ip->i_uid = running->uid, // owner uid
    ip->i_gid = running->gid, // group Id
    ip->i_size = BLKSIZE, // size in bytes
    ip->i_links_count = 2, // links count=2 because of . and ..
    ip->i_atime = ip->i_ctime = ip->i_mtime = time(0L),
    ip->i_blocks = 2, // LINUX: Blocks count in 512-byte chunks
    ip->i_block[0] = bno // new DIR has one data block
  };*/
  ip->i_mode = 0x41ED; // 040755: DIR type and permissions
  ip->i_uid = running->uid; // owner uid
  ip->i_gid = running->gid; // group Id
  ip->i_size = BLKSIZE;// size in bytes
  ip->i_links_count = 2; // links count=2 because of . and ..
  ip->i_atime = ip->i_ctime = ip->i_mtime = time(0L);
  ip->i_blocks = 2; // LINUX: Blocks count in 512-byte chunks
  ip->i_block[0] = bno;
  for (int i = 1; i < 15; ++i) {
    ip->i_block[i] = 0;
  }
  mip->dirty = 1; // mark minode dirty
  iput(mip); // write INODE to disk
  
  //(4).3. make data block 0 of INODE to contain . and .. entries;
  char buf[BLKSIZE];
  bzero(buf, BLKSIZE); // optional: clear buf[ ] to 0
  get_block(dev, bno, buf);
  DIR *dp = (DIR *)buf;
  // make . entry
  dp->inode = ino;
  dp->rec_len = 12;
  dp->name_len = 1;
  dp->name[0] = '.';
  // make .. entry: pino=parent DIR ino, blk=allocated block
  dp = (DIR *)((char *)dp + 12);
  dp->inode = pmip->ino;
  dp->rec_len = BLKSIZE-12; // rec_len spans block
  dp->name_len = 2;
  dp->name[0] = dp->name[1] = '.';
  put_block(dev, bno, buf); // write to blk on diks
  // write to disk block blk.
  
  // (4).4. enter_child(pmip, ino, basename); which enters (ino, basename) as a dir_entry to the parent INODE;
  // since book uses enter_name() i'll be using that instead
  enter_name(pmip, ino, basename);
}

int mymkdir(char pathname[128]) {
    // (1). divide pathname into dirname and basename, e.g. pathname=/a/b/c, then dirname=/a/b; basename=c;
    char dname[128] = "", bname[128];
    int n = tokenize(pathname);
    int i;

    for (i = 0; i < (n-1); ++i) {
      strcat(dname, "/");
      strcat(dname, name[i]);
    }
    if (i == 0) strcat(dname, "/");
    strcpy(bname, name[i]);

    // (2). // dirname must exist and is a DIR:
    int pino = getino(dname);
    if (pino == 0) {
      printf("parent %s doesn't exist", dname);
      return 0;
    }

    MINODE * pmip = iget(dev, pino);
    //check pmip->INODE is a DIR
    if (!S_ISDIR(pmip->INODE.i_mode)) {
      printf("mip->INODE %s is not a DIR\n", dname);
      return 0;
    }
    // (3). // basename must not exist in parent DIR:
    int s = search(pmip, bname); //must return 0;
    if (s != 0) {
      printf("search for %s returned non-zero number %d, so it already exists underneath %s\n", bname, s, dname);
      return 0;
    }

    // (4). call kmkdir(pmip, basename) to create a DIR;
    kmkdir(pmip, bname, dev);
    
    // (5). increment parent INODE’s links_count by 1 and mark pmip dirty;
    pmip->dirty = 1;
    iput(pmip);
}

int mycreat() {

}