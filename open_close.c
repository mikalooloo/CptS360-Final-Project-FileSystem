// open_close

#include "header.h"

extern int dev;
extern PROC   proc[NPROC], *running;
extern char * name[64];

int truncate(MINODE *mip)
{
  //1. release mip->INODE's data blocks;
     //a file may have 12 direct blocks, 256 indirect blocks and 256*256
     //double indirect data blocks. release them all.
  //2. update INODE's time field

  //3. set INODE's size to 0 and mark Minode[ ] dirty
}

int mylseek(int fd, int position)
{
  // From fd, find the OFT entry. 

  // change OFT entry's offset to position but make sure NOT to over run either end of the file.

  // return originalPosition
}

int pfd()
{
  /* This function displays the currently opened files as follows:

        fd     mode    offset    INODE
       ----    ----    ------   --------
         0     READ    1234   [dev, ino]  
         1     WRITE      0   [dev, ino]
      --------------------------------------
  to help the user know what files has been opened. */

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

    char * dname = (char *)malloc(sizeof(pathname)), * bname = (char *)malloc(sizeof(pathname));
    separatePathname(pathname, &dname, &bname, "open_file");
    printf("\nDNAME: %s\nBNAME: %s\n", dname, bname);

    // 2. get pathname's inumber, minode pointer:
    //int ino = getino(pathname); 
    //MINODE * mip = iget(dev, ino);  

    // 3. check mip->INODE.i_mode to verify it's a REGULAR file and permission OK.
      
    // Check whether the file is ALREADY opened with INCOMPATIBLE mode:
           // If it's already opened for W, RW, APPEND : reject.
           // (that is, only multiple R are OK)

    // 4. allocate a FREE OpenFileTable (OFT) and fill in values:
    /*OFT * oftp = (OFT *)malloc(sizeof(OFT));
    oftp->mode = mode;      // mode = 0|1|2|3 for R|W|RW|APPEND 
    oftp->refCount = 1;
    oftp->minodePtr = mip;*/  // point at the file's minode[]

    // 5. Depending on the open mode 0|1|2|3, set the OFT's offset accordingly:

    /*switch(mode){
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
            printf("invalid mode\n");
            return(-1);
      }*/

    // 7. find the SMALLEST i in running PROC's fd[ ] such that fd[i] is NULL
      // Let running->fd[i] point at the OFT entry

    // 8. update INODE's time field
         // for R: touch atime. 
         // for W|RW|APPEND mode : touch atime and mtime
      // mark Minode[ ] dirty

    // 9. return i as the file descriptor
}

int close_file(int fd)
{
    // 1. verify fd is within range.

    // 2. verify running->fd[fd] is pointing at a OFT entry

    // 3. The following code segments should be fairly obvious:
    OFT * oftp = (OFT *)malloc(sizeof(OFT));
    oftp = running->fd[fd];
    running->fd[fd] = 0;
    oftp->refCount--;
    if (oftp->refCount > 0) return 0;

    // last user of this OFT entry ==> dispose of the Minode[]
    MINODE * mip = oftp->minodePtr;
    iput(mip);

    return 0;
}