#include "header.h"


// helper functions and such
extern int imap, bmap, nblocks, ninodes, dev;
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
  MOUNT * mptr = getmptr(dev); // using current dev's instead

  // read inode_bitmap block
  get_block(dev, mptr->imap, buf);
  for (i=0; i < mptr->ninodes; i++){
    if (tst_bit(buf, i)==0){
        set_bit(buf, i);
        put_block(dev, mptr->imap, buf);
        decFreeInodes(dev);
        //printf("allocated ino = %d\n", i+1); // bits count from 0; ino from 1
        return i+1;
    }
  }
  printf("did not allocate an inode number: ialloc failed\n");
  return -1;
}

int balloc(int dev) 
{
  char buf[BLKSIZE];
  MOUNT * mptr = getmptr(dev); // using current dev's instead

  get_block(dev, mptr->bmap, buf);

  for (int i = 0; i < mptr->nblocks; ++i) {
    if (tst_bit(buf, i) == 0) {
      set_bit(buf, i);
      put_block(dev, mptr->bmap, buf);
      decFreeBlocks(dev);
      //printf("allocated blk = %d\n", i+1);
      return i + 1;
    }
  }

  printf("did not allocate a block number: balloc failed\n");
  return -1;
}
// meat of the bones here

int enter_name(MINODE * pip, int ino, char * name) {
  DIR * dp;
  char * cp;
  char buf[BLKSIZE];
  int blk;

  int ideal_length = 0, need_length = 4* ((8 + strlen(name) + 3)/4);

  // assume only 12 direct blocks,
  // for each data block of parent DIR
  for (int i = 0; i < 12; ++i) {
    if (pip->INODE.i_block[i] == 0)
      break; // go to step 5
    else
      blk = pip->INODE.i_block[i];

    // (1). Get parent’s data block into a buf[ ];
    get_block(pip->dev, blk, buf);
    dp = (DIR *)buf;
    cp = buf;

    // (2). In a data block of the parent directory, each dir_entry has an ideal length
    //ideal_length = 4 * ((8 + dp->name_len + 3)/4); // a multiple of 4
    // All dir_entires rec_len = ideal_length except the last entry
    // The rec_len of the last entry is to the end of the block, which may be larger than ideal_length

    // (3). In order to enter a new entry of name with n_len, the needed length is
    //need_length = 4* ((8 + strlen(name) + 3)/4); // a multiple of 4

    // (4). Step to the last entry in the data block:
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

      dp->inode = ino; // setting ino
      dp->rec_len = remain; // setting rec_len
      dp->name_len = strlen(name); // setting name_len
      strncpy(dp->name, name, dp->name_len); // setting name

      put_block(dev, blk, buf);
      return 1;
    }
    else { // no space in existing data blocks
      // Allocate a new data block; increment parent size by BLKSIZE;
      // Enter new entry as the first entry in the new data block with rec_len¼BLKSIZE.
      pip->INODE.i_size = BLKSIZE;
      pip->INODE.i_block[i] = blk = balloc(dev);
      pip->dirty = 1;

      get_block(dev, blk, buf);
      dp = (DIR *)buf;
      cp = buf;

      dp->inode = ino; // setting ino
      dp->rec_len = BLKSIZE; // setting rec_len
      dp->name_len = strlen(name); // setting name_len
      strncpy(dp->name, name, dp->name_len); // setting name

      put_block(dev, blk, buf);
      return 1;
    }
  }

  printf("enter_name appears to have failed\n");
  return -1;
}

int kmkdir(MINODE * pmip, char * basename) {
  // kmkdir() consists of 4 major steps:
  // (4).1. Allocate an INODE and a disk block:
  int ino = ialloc(dev);
  int blk = balloc(dev);
  
  // (4).2. mip = iget(dev, ino) // load INODE into a minode
  //initialize mip->INODE as a DIR INODE;
  MINODE *mip = iget(dev, ino);
  INODE * ip = &mip->INODE;
  ip->i_mode = 0x41ED; // 040755: DIR type and permissions
  ip->i_uid = running->uid; // owner uid
  ip->i_gid = running->gid; // group Id
  ip->i_size = BLKSIZE;// size in bytes
  ip->i_links_count = 2; // links count=2 because of . and ..
  ip->i_atime = ip->i_ctime = ip->i_mtime = time(0L);
  ip->i_blocks = 2; // LINUX: Blocks count in 512-byte chunks
  ip->i_block[0] = blk;
  for (int i = 1; i < 15; ++i) {
    ip->i_block[i] = 0;
  }
  mip->dirty = 1; // mark minode dirty
  iput(mip); // write INODE to disk

  //(4).3. make data block 0 of INODE to contain . and .. entries;
  char buf[BLKSIZE];
  bzero(buf, BLKSIZE); // optional: clear buf[ ] to 0
  get_block(dev, blk, buf);
  DIR *dp = (DIR *)buf;
  char *cp = buf;
  // make . entry
  dp->inode = ino;
  dp->rec_len = 12;
  dp->name_len = 1;
  dp->name[0] = '.';
  // move to next entry
  cp += dp->rec_len;
  dp = (DIR *)cp;
  // make .. entry: pino=parent DIR ino, blk=allocated block
  dp->inode = pmip->ino;
  dp->rec_len = (BLKSIZE-12); // rec_len spans block
  dp->name_len = 2;
  dp->name[0] = dp->name[1] = '.';
  put_block(dev, blk, buf); // write to blk on diks
  // write to disk block blk.
  // (4).4. enter_child(pmip, ino, basename); which enters (ino, basename) as a dir_entry to the parent INODE;
  // since book uses enter_name() i'll be using that instead
  enter_name(pmip, ino, basename);
}

int my_mkdir(char * pathname) {
    // check for valid pathname
    if (validPathname(pathname) == -1) {
      printf("\npathname is not valid: mkdir failed\n");
      return -1;
    }

    // (1). divide pathname into dirname and basename, e.g. pathname=/a/b/c, then dirname=/a/b; basename=c;
    char * dname = (char *)malloc(sizeof(pathname)), * bname = (char *)malloc(sizeof(pathname));
    separatePathname(pathname, &dname, &bname, "mkdir");

    // (2). // dirname must exist and is a DIR:
    int pino = getino(dname);
    if (pino == -1) {
      printf("\nparent %s does not exist: mkdir failed\n", dname);
      return -1;
    }
    else printf("parent %s exists: parent check passed\n", dname);
  
    MINODE * pmip = iget(dev, pino);
    //check pmip->INODE is a DIR
    if (!S_ISDIR(pmip->INODE.i_mode)) {
      printf("\npmip->INODE %s is not a DIR: mkdir failed\n", dname);
      return -1;
    }
    else printf("%s is a DIR: DIR check passed\n", dname);

    // (3). // basename must not exist in parent DIR:
    int s = search(pmip, bname); //must return -1;
    if (s != -1) {
      printf("\n%s already exists under %s: mkdir failed\n", bname, dname);
      return -1;
    }
    else printf("%s does not exist under %s yet: new DIR check passed\n", bname, dname);

    // (4). call kmkdir(pmip, basename) to create a DIR;
    kmkdir(pmip, bname);
    // (5). increment parent INODE’s links_count by 1 and mark pmip dirty;
    pmip->INODE.i_links_count++;
    pmip->dirty = 1;
    iput(pmip);

    printf("\nmkdir successful\n");
}

int kcreat(MINODE * pmip, char * basename) {
  // (4).1. Allocate an INODE:
  int ino = ialloc(dev);

  // (4).2. mip = iget(dev, ino) // load INODE into a minode
  //initialize mip->INODE as a file INODE;
  MINODE *mip = iget(dev, ino);
  INODE *ip = &mip->INODE;
  ip->i_mode = 0100644; // file type and permissions
  ip->i_uid = running->uid; // owner uid
  ip->i_gid = running->gid; // group Id
  ip->i_size = 0;// no data block allocated for it
  ip->i_links_count = 1; // links count=1
  ip->i_atime = ip->i_ctime = ip->i_mtime = time(0L);
  ip->i_blocks = 0; // LINUX: Blocks count in 512-byte chunks
  for (int i = 0; i < 15; ++i) {
    ip->i_block[i] = 0;
  }
  mip->dirty = 1; // mark minode dirty
  iput(mip); // write INODE to disk
  
  // (4).4. enter_child(pmip, ino, basename); which enters (ino, basename) as a dir_entry to the parent INODE;
  // since book uses enter_name() i'll be using that instead
  enter_name(pmip, ino, basename);
}

int my_creat(char * pathname) {

    // check for valid pathname
    if (validPathname(pathname) == -1) {
      printf("\npathname is not valid: creat failed\n");
      return -1;
    }

  // (1). divide pathname into dirname and basename, e.g. pathname=/a/b/c, then dirname=/a/b; basename=c;
    char * dname = (char *)malloc(sizeof(pathname)), * bname = (char *)malloc(sizeof(pathname));
    separatePathname(pathname, &dname, &bname, "creat");

    // (2). // dirname must exist and is a DIR:
    int pino = getino(dname);
    if (pino == -1) {
      printf("\nparent %s does not exist: creat failed\n", dname);
      return 0;
    }
    else printf("parent %s exists: parent check passed\n", dname);

    MINODE * pmip = iget(dev, pino);
    //check pmip->INODE is a DIR
    if (!S_ISDIR(pmip->INODE.i_mode)) {
      printf("\nmip->INODE %s is not a DIR: creat failed\n", dname);
      return 0;
    }
    else printf("%s is a DIR: DIR check passed\n", dname);

    // (3). // basename must not exist in parent DIR:
    int s = search(pmip, bname); //must return -1;
    if (s != -1) {
      printf("\n%s already exists under %s: creat failed\n", bname, dname);
      return -1;
    }
    else printf("%s does not exist under %s yet: new file check passed\n", bname, dname);

    // (4). call kmkdir(pmip, basename) to create a DIR;
    kcreat(pmip, bname);
    
    // (5). mark pmip dirty;
    pmip->dirty = 1;
    iput(pmip);

    printf("\ncreat successful\n");
}