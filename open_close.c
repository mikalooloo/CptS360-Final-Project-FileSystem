// open_close

#include "header.h"

extern int dev;
extern PROC   proc[NPROC], *running;
extern char * name[64];

int truncate(MINODE *mip)
{
  //1. release mip->INODE's data blocks;
     //a file may have 12 direct blocks, 256 indirect blocks and 256*256
  for (int i = 0; i < 12; ++i) // direct
  {
    if (mip->INODE.i_block[i] == 0) break;
    bdalloc(dev, mip->INODE.i_block[i]);
    mip->INODE.i_block[i] = 0;
  }
  // indirect
  char buf[BLKSIZE];
  if (mip->INODE.i_block[12] != 0) 
  {
    get_block(dev, mip->INODE.i_block[12], buf);
    int j;
    for (int i = 0; i < 256 + 12; ++i) 
    {
      j = i * 4;
      if (buf[j] == 0) break;
      bdalloc(dev, buf[j]);
    }
    mip->INODE.i_block[12] = 0;
  }
     //double indirect data blocks. release them all.
  if (mip->INODE.i_block[13] != 0) 
  {
    get_block(dev, mip->INODE.i_block[13], buf);
    int j;
    for (int i = 0; i < 256 + 12; ++i) 
    {
      j = i * 4;
      if (buf[j] == 0) break;
      bdalloc(dev, buf[j]);
    }
    mip->INODE.i_block[13] = 0;
  }

  //2. update INODE's time field
  mip->INODE.i_atime = time(0L);
  mip->INODE.i_mtime = time(0L);
  //3. set INODE's size to 0 and mark Minode[ ] dirty
  mip->INODE.i_blocks = 0;
  mip->INODE.i_size = 0;
  mip->dirty = 1;
  iput(mip);
}

int mylseek(int fd, int position)
{
  // From fd, find the OFT entry. 

  // change OFT entry's offset to position but make sure NOT to over run either end of the file.

  // return originalPosition
}

int my_pfd() // returns number of files open too
{
  /* This function displays the currently opened files as follows:

        fd     mode    offset    INODE
       ----    ----    ------   --------
         0     READ    1234   [dev, ino]  
         1     WRITE      0   [dev, ino]
      --------------------------------------
  to help the user know what files has been opened. */
  // mode = 0|1|2|3 for R|W|RW|APPEND
  char * modes[4] = { "RD", "WR", "RW", "APPEND" };
  int openCount = 0;

  printf("\nfd\tmode\toffset\tINODE\n");
  for (int i = 0; i < NFD; ++i) 
  {
    if (running->fd[i] != NULL) 
    {
      printf("%d\t%s\t%d\t[%d, %d]\n", i, modes[running->fd[i]->mode], running->fd[i]->offset, running->fd[i]->minodePtr->dev, running->fd[i]->minodePtr->ino);
      ++openCount;
    }
  }

  if (openCount == 0) printf("\nno files currently open\n");
  return openCount;
}

int dup(int fd)
{
  // verify fd is an opened descriptor;
  // duplicates (copy) fd[fd] into FIRST empty fd[ ] slot;
  // increment OFT's refCount by 1;
}

int dup2(int fd, int gd)
{
  // CLOSE gd fisrt if it's already opened;
  // duplicates fd[fd] into fd[gd]; 
}

int open_file(char * pathname, int mode) 
{
    // 1. ask for a pathname and mode to open:
    // mode = 0|1|2|3 for R|W|RW|APPEND
    if (mode < 0 || mode > 3) {
      printf("\nmode %d is not valid: open_file failed\n", mode);
      return -1;
    }
    else printf("mode %d is valid: passed mode check\n", mode);

    char * dname = (char *)malloc(sizeof(pathname)), * bname = (char *)malloc(sizeof(pathname));
    separatePathname(pathname, &dname, &bname, "open_file");

    // 2. get pathname's inumber, minode pointer:
    int ino = getino(pathname); 
    if (ino == -1) 
    {
      printf("file %s does not exist, creating file\n", bname);
      // make new file here!
      my_creat(pathname);
      ino = getino(pathname);
      if (ino == -1) {
        printf("file %s could not be created: open_file failed\n", bname);
        return -1;
      }
      else printf("file %s successfully created: passed existence check\n", bname);
    }
    else printf("file %s already exists: passed existence check\n", bname);

    MINODE * mip = iget(dev, ino);  

    // 3. check mip->INODE.i_mode to verify it's a REGULAR file and permission OK.
    if (!S_ISREG(mip->INODE.i_mode)) {
      printf("\nfile %s is not a regular file: open_file failed\n", bname);
      return -1;
    }
    else printf("file %s is a regular file: passed file check\n", bname);

    // check permissions here

    // Check whether the file is ALREADY opened with INCOMPATIBLE mode:
           // If it's already opened for W, RW, APPEND : reject.
           // (that is, only multiple R are OK)
    // also does part 7 here for efficiency reasons
    int smallest_i = -1;
    for (int i = 0; i < NFD; ++i) 
    {
      if (running->fd[i] == NULL) 
      {
        smallest_i = i;
        break;
      }
      if (running->fd[i]->minodePtr == mip) // if the file
      { 
        printf("minodePtr[%d]: %d and mip: %d\n", i, running->fd[i]->minodePtr->ino, mip->ino);
        if (mode != 0) { // only R is okay!! and R = 0 
          printf("\nfile %s is already opened with incompatible mode %d: open_file failed\n", bname, mode);
          return -1;
        }
        else printf("file %s is already opened but with compatible mode %d\n", bname, mode);
      }
    }


    // 4. allocate a FREE OpenFileTable (OFT) and fill in values:
    OFT * oftp = (OFT *)malloc(sizeof(OFT));
    oftp->mode = mode;      // mode = 0|1|2|3 for R|W|RW|APPEND 
    oftp->refCount = 1;
    oftp->minodePtr = mip;  // point at the file's minode[]

    // 5. Depending on the open mode 0|1|2|3, set the OFT's offset accordingly:

    switch(mode){
        case 0 : 
            oftp->offset = 0;     // R: offset = 0
            break;
        case 1 : 
            truncate(mip);        // W: truncate file to 0 size
            oftp->offset = 0;
            break;
        case 2 : 
            oftp->offset = 0;     // RW: do NOT truncate file
            break;
        case 3 : 
            oftp->offset =  mip->INODE.i_size;  // APPEND mode
            break;
        default: 
            printf("\ninvalid mode %d: open_file failed\n", mode);
            return(-1);
      }

    // 7. find the SMALLEST i in running PROC's fd[ ] such that fd[i] is NULL
      // Let running->fd[i] point at the OFT entry
    if (smallest_i != -1) 
    {
      running->fd[smallest_i] = oftp;
    }

    // 8. update INODE's time field
         // for R: touch atime. 
    mip->INODE.i_atime = time(0L); // for all touch atime
         // for W|RW|APPEND mode : touch atime and mtime
    if (mode !=0) mip->INODE.i_mtime = time(0L); // for all but R touch mtime

      // mark Minode[ ] dirty
      mip->dirty = 1;
      //iput(mip);

    printf("open_file successful\n");
    // 9. return i as the file descriptor
    return smallest_i;
}

int close_file(int fd)
{
    // 1. verify fd is within range.

    // 2. verify running->fd[fd] is pointing at a OFT entry
    if (running->fd[fd] == NULL) {
      printf("\nrunning->fd[%d] is not pointing at a OFT entry: close_file failed\n", fd);
      return -1;
    }
    else printf("running->fd[%d] is pointing at an OFT entry: use check passed\n", fd);

    // 3. The following code segments should be fairly obvious:
    OFT * oftp = (OFT *)malloc(sizeof(OFT));
    oftp = running->fd[fd];
    running->fd[fd] = 0;
    oftp->refCount--;
    if (oftp->refCount > 0) return 0;

    // last user of this OFT entry ==> dispose of the Minode[]
    MINODE * mip = oftp->minodePtr;
    mip->dirty = 1;
    iput(mip);

    free(oftp);
    
    printf("close_file successful\n");
    return 0;
}