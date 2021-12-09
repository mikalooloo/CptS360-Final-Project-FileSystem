/*********** util.c file ****************/
#include "header.h"

#include <fcntl.h>

/**** globals defined in main.c file ****/
extern MINODE minode[NMINODE];
extern MINODE *root;
extern PROC   proc[NPROC], *running;

extern char gpath[128];
extern char *name[64];
extern int n;

extern int fd, dev;
extern int nblocks, ninodes, bmap, imap, iblk;

extern char line[128], cmd[32], pathname[128];

off_t lseek(int fildes, off_t offset, int whence);
ssize_t read(int fildes, void *buf, size_t nybte);
ssize_t write(int fildes, const void * buf, size_t nbyte);

int get_block(int dev, int blk, char *buf)
{
   lseek(dev, (long)blk*BLKSIZE, 0);
   read(dev, buf, BLKSIZE);
}   

int put_block(int dev, int blk, char *buf)
{
   lseek(dev, (long)blk*BLKSIZE, 0);
   write(dev, buf, BLKSIZE);
}   

int tokenize(char *pathname)
{
  int i;
  char *s;
  //printf("tokenize %s\n", pathname);

  strcpy(gpath, pathname);   // tokens are in global gpath[ ]
  n = 0;

  s = strtok(gpath, "/");
  while(s){
    name[n] = s;
    n++;
    s = strtok(0, "/");
  }
  name[n] = 0;
  
  //for (i= 0; i<n; i++)
  //  printf("%s  ", name[i]);
  //printf("\n");

  return n;
}

MINODE *mialloc() // allocate a FREE minode for use
{
   int i;
   for (i=0; i<NMINODE; i++){
      MINODE *mp = &minode[i];
      if (mp->refCount == 0){
         mp->refCount = 1;
         return mp;
      }
   }
   printf("FS panic: out of minodes\n");
   return 0;
}

int midalloc(MINODE *mip) // release a used minode
{
   mip->refCount = 0;
}

// return minode pointer to loaded INODE
MINODE *iget(int dev, int ino)
{
  int i;
  MINODE *mip; INODE *ip;
  char buf[BLKSIZE];
  int blk, iblk2 = iblk, offset;
  MOUNT * mptr = getmptr(dev);
  if (mptr != NULL) iblk2 = mptr->blk;

  for (i=0; i<NMINODE; i++){
    mip = &minode[i];
    if (mip->refCount && mip->dev == dev && mip->ino == ino){
       mip->refCount++;
       //printf("found [%d %d] as minode[%d] in core\n", dev, ino, i);
       return mip;
    }
  }
    
   //printf("allocating NEW minode[%d] for [%d %d]\n", i, dev, ino);
   mip = mialloc();
   mip->dev = dev;
   mip->ino = ino;
   // get INODE of ino to buf    
   blk    = (ino-1) / 8 + iblk2; // iblk2 is either global iblk if initializing mount_root()
   offset = (ino-1) % 8;         // or the current dev's iblk

   //printf("iget: ino=%d blk=%d offset=%d\n", ino, blk, offset);

   get_block(dev, blk, buf);
   ip = (INODE *)buf + offset;
   // copy INODE to mp->INODE
   mip->INODE = *ip;
   // initialize minode
   mip->refCount = 1;
   mip->mounted = 0;
   mip->dirty = 0;
   mip->mptr = 0;
   return mip;
}

void iput(MINODE *mip)
{
 int i, block, offset;
 char buf[BLKSIZE];
 INODE * ip;
 MOUNT * mptr = getmptr(dev);

 if (mip==0) return;
 mip->refCount--;
 if (mip->refCount > 0) return;
 if (!mip->dirty)       return;
 
 /* write INODE back to disk */
 /**************** NOTE ******************************
  For mountroot, we never MODIFY any loaded INODE
                 so no need to write it back
  FOR LATER WROK: MUST write INODE back to disk if refCount==0 && DIRTY

  Write YOUR code here to write INODE back to disk
 *****************************************************/
   block  = (mip->ino - 1) / 8 + mptr->blk;
   offset = (mip->ino - 1) % 8;
   get_block(mip->dev, block, buf);
   ip = (INODE *)buf + offset; // ip points at INODE
   *ip = mip->INODE; // copy INODE to inode in block
   put_block(mip->dev, block, buf); // write back to disk
   midalloc(mip); // mip->refCount = 0

} 

int search(MINODE *mip, char *name)
{
   int i; 
   char *cp, c, sbuf[BLKSIZE], temp[256];
   DIR *dp;
   INODE *ip;

   printf("search for %s in MINODE = [%d, %d]\n", name,mip->dev,mip->ino);
   ip = &(mip->INODE);

   /*** search for name in mip's data blocks: ***/
   // searching in direct blocks only
   for (i = 0; i < 12; i++) {
      if (mip->INODE.i_block[i] == 0) return -1;
      get_block(dev, ip->i_block[i], sbuf);
      dp = (DIR *)sbuf;
      cp = sbuf;
      printf("  ino   rlen  nlen  name\n");
      while (cp < sbuf + BLKSIZE){
         strncpy(temp, dp->name, dp->name_len);
         temp[dp->name_len] = 0;
         printf("%4d  %4d  %4d    %.*s\n", 
            dp->inode, dp->rec_len, dp->name_len, dp->name_len, dp->name); // add .* to the print so it only prints how long the name is supposed to be (sometimes has extra chars)
         if (strcmp(temp, name)==0){
            //printf("found %s : ino = %d\n", temp, dp->inode);
            return dp->inode;
         }
         cp += dp->rec_len;
         dp = (DIR *)cp;
      }
   }
   return -1;
}

int getino(char *pathname)
{
  int i, ino;
  char buf[BLKSIZE];
  MINODE *mip;

  if (strcmp(pathname, "/")==0)
      return 2;
  
  // starting mip = root OR CWD
  if (pathname[0]=='/') { 
     dev = root->dev; 
     ino = root->ino; }
  else {
     dev = running->cwd->dev;
     ino = running->cwd->ino; }
  mip = iget(dev, ino);
   
  mip->refCount++;         // because we iput(mip) later
  
  tokenize(pathname);

  for (i=0; i<n; i++){
      printf("===========================================\n");
      printf("getino: i=%d name[%d]=%s\n", i, i, name[i]);
 
      if (!S_ISDIR(mip->INODE.i_mode)) {
         printf("%s is not a directory\n", name[i]);
         iput(mip);
         return -1;
      }

      ino = search(mip, name[i]);
   
      if (ino==-1) // couldn't find INODE
      {
         iput(mip);
         printf("\nname %s does not exist: getino failed\n", name[i]);
         return -1;
      }
      else if (ino == 2 && dev != mip->dev) // found root INODE but its dev number differs from real root
      {
         // **upwards traversal**
         // using its dev number, locate mount table entry, which points to mounted minode
         MOUNT * mptr = getmptr(dev);
         // switch to minode and continue
         iput(mip);
         mip = mptr->mounted_inode;       // switching to mounted minode
         dev = mptr->mounted_inode->dev;  // and mounted dev
      }
      else // INODE exists and is not root INODE of the mounted file system
      {
         iput(mip);
         mip = iget(dev, ino); // get new mip so we can check if it's mounted

         // **downwards traversal**
         if (mip->mounted == 1) 
         {
            // follow minode's mountTable pointer to locate the mountTable entry
            MOUNT * mptr = mip->mptr;
            // use the mountTable entry to get the new dev number, and set ino to 2 since it's at root
            dev = mptr->dev;
            ino = 2;
            // then iget() its root INODE into memory
            // and continue to search under the root INODE of the virtual disk
            iput(mip);
            mip = iget(dev, ino); // switching to root INODE of new mount
         }
      }
   }
   
   iput(mip);
   return ino;
}

// These 2 functions are needed for pwd()
// this looks for myname in the data blocks of parent - like the search() function except just copies into myname instead of returning
int findmyname(MINODE *parent, u32 myino, char myname[ ]) 
{
  // WRITE YOUR code here
  // search parent's data block for myino; SAME as search() but by myino
  // copy its name STRING to myname[ ]

   int i; 
   char *cp, c, sbuf[BLKSIZE], temp[256];
   DIR *dp;
   INODE *ip;

   printf("search for %d in MINODE = [%d, %d]\n", myino, parent->dev, parent->ino);
   ip = &(parent->INODE);

   /*** search for name in mip's data blocks:  ***/

   // going to 11 because direct blocks go from i_block[0] to i_block[11]
   for (i = 0; i < 12; ++i) {
      if (!ip->i_block[i]) return -1;

      get_block(dev, ip->i_block[0], sbuf);
      dp = (DIR *)sbuf;
      cp = sbuf;

      while (cp < sbuf + BLKSIZE){
         strncpy(temp, dp->name, dp->name_len);
         temp[dp->name_len] = 0;
         //printf("%4d  %4d  %4d    %s\n", 
            //dp->inode, dp->rec_len, dp->name_len, dp->name);
         if (dp->inode == myino){
         //if (!strcmp(temp, myname)) {
            //printf("found %s : ino = %d\n", temp, dp->inode);
            strncpy(myname, dp->name, dp->name_len);
             myname[dp->name_len] = 0;
            return 1;
         }
         cp += dp->rec_len;
         dp = (DIR *)cp;
      }
   }
   
   return -1;
}

int findino(MINODE *mip, u32 *myino) // myino = i# of . return i# of ..
{
  // mip points at a DIR minode
  char buf[BLKSIZE];
  get_block(dev, mip->INODE.i_block[0], buf);

  // WRITE your code here: myino = ino of .  return ino of ..
  // all in i_block[0] of this DIR INODE.
   DIR * dp = (DIR *) buf;
   * myino = mip->ino;
   dp = (DIR *) (buf + dp->rec_len);
   return dp->inode;
}

void separatePathname(char * pathname, char ** dname, char ** bname, char * command) 
{
   char temp_dname[128] = "", temp_bname[128] = "";
   int n = (tokenize(pathname) - 1);
   int i;

   if (pathname[0] == '/') { // if absolute
      printf("%s pathname %s is absolute\n", command, pathname);
      temp_dname[0] = '/';
   }

   for (i = 0; i < n; ++i) {
      if (i > 0) strcat(temp_dname, "/");
      strcat(temp_dname, name[i]);
   }
   strcpy(temp_bname, name[i]);

   if (pathname[0] != '/') { // if relative
      printf("%s pathname %s is relative\n", command, pathname);
      // getting cwd
      char path[128];
      strcpy(path, rpwd(running->cwd, 0));
      if (strcmp(path, "/")!=0) {
		  strcat(path, temp_dname);
		  strcpy(temp_dname, path);
	   }
      else if (strcmp(temp_dname, "")==0) temp_dname[0] = '/'; // if filename is one character
    }
    
    strcpy(*dname, temp_dname);
    strcpy(*bname, temp_bname);
}

int validPathname(char * pathname) {
   if (pathname != NULL && strcmp(pathname, "") != 0) return 1;
   else return -1;
}

// debug command only
int printMinnodes(int m) {
   printf("\n%d minnodes\n", m);
   for (int i=0; i<m; i++){
      MINODE *mp = &minode[i];
      printf("mp->ino: %d\nmp->refCount: %d\n", mp->ino, mp->refCount);
   }
}