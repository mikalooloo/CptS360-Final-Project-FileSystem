/****************************************************************************
*                   KCW: mount root file system                             *
*****************************************************************************/
#include "header.h"

extern MINODE *iget();

MINODE minode[NMINODE];
MINODE *root;
PROC   proc[NPROC], *running;
MOUNT mountTable[NMOUNT]; // in init() all dev are set to 0

char gpath[128]; // global for tokenized components
char *name[64];  // assume at most 64 components in pathname
int   n;         // number of component strings

int fd, dev;
int nblocks, ninodes, bmap, imap, iblk;
char line[128], cmd[32], pathname[128], pathname2[128];
char *disk = "diskimage";

int init()
{
  int i, j;
  MINODE *mip;
  PROC   *p;
  MOUNT * mntptr;

  printf("init()\n");

  for (i=0; i<NMINODE; i++){ // setting minode[] to 0
    mip = &minode[i];
    mip->dev = mip->ino = 0;
    mip->refCount = 0;
    mip->mounted = 0;
    mip->mptr = 0;
  }
  for (i=0; i<NPROC; i++){ // setting proc[] to 0
    p = &proc[i];
    p->pid = i;
    p->uid = p->gid = 0;
    p->cwd = 0;
    for (j = 0; j<NFD; j++){
      p->fd[j] = 0;
    }
    p->next = &proc[i+1];
  }
  proc[NPROC-1].next = &proc[0]; // circular list
  for (i=0; i < NMOUNT; i++){ // setting mountTable[] dev to 0
     mntptr = &mountTable[i];
     mntptr->dev = 0;
  }
}

// load root INODE and set root pointer to it
// TO DO: Modification: mountTable[0] has been used to record dev, ninodes, nblocks, bmap, impap, iblk, of root device
int mount_root()
{  
  printf("mount_root()\n");
  root = iget(dev, 2);
  MOUNT * mptr = &mountTable[0];
  mptr->dev = dev;
  mptr->ninodes = ninodes;
  mptr->nblocks = nblocks;
  mptr->bmap = bmap;
  mptr->imap = imap;
  mptr->blk = iblk;
  mptr->mounted_inode = root;
  root->mptr = mptr;
  root->mounted = 1;
  strcpy(mptr->name,"/");
  strncpy(mptr->mount_name, disk, NLENGTH); 
  // set proc's CWD
  for (int i = 0; i < NPROC; i++) 
      proc[i].cwd = iget(dev, 2);
}

//TO DO: write a MOUNT *getmptr(int dev) function, which returns a pointer to dev's mountTable[] entry
MOUNT * getmptr(int dev) {
   for (int i = 0; i < NMOUNT; i++) 
   {
      if (mountTable[i].dev == dev)
      {
         //printf("\nfound dev %d's pointer to mountTable[%d] entry %s\n", dev, i, mountTable[i].mount_name);
         return &mountTable[i];
      }
   }
}

int main(int argc, char *argv[ ])
{
  int ino;
  int debug = 0;
  char buf[BLKSIZE];
  if (argc > 1) disk = argv[1];

  printf("checking EXT2 FS ....");
  if ((fd = open(disk, O_RDWR)) < 0){
    printf("open %s failed\n", disk);
    exit(1);
  }

  dev = fd;    // global dev same as this fd   

  /********** read super block  ****************/
  get_block(dev, 1, buf);
  sp = (SUPER *)buf;

  /* verify it's an ext2 file system ***********/
  if (sp->s_magic != 0xEF53){
      printf("magic = %x is not an ext2 filesystem\n", sp->s_magic);
      exit(1);
  }     
  printf("EXT2 FS OK\n");
  ninodes = sp->s_inodes_count;
  nblocks = sp->s_blocks_count;

  get_block(dev, 2, buf); 
  gp = (GD *)buf;

  bmap = gp->bg_block_bitmap;
  imap = gp->bg_inode_bitmap;
  iblk = gp->bg_inode_table;
  printf("bmp=%d imap=%d inode_start = %d\n", bmap, imap, iblk);

  init();  
  mount_root();
  printf("root refCount = %d\n", root->refCount);

  printf("creating P0 as running process\n");
  running = &proc[0];
  running->status = READY;
  running->cwd = iget(dev, 2);
  printf("root refCount = %d\n", root->refCount);

  // WRTIE code here to create P1 as a USER process
  printf("\n************************\n");
  while(1){
    printf("\nLevel 1: [ls|cd|pwd|mkdir|creat|rmdir|link|unlink|symlink]\nLevel 2: [cat|cp]\nLevel 3: [mount|umount]\nMisc:    [menu|open|close|pfd|read|write|mv|rename|debug|quit]\ninput command :  ");
    fgets(line, 128, stdin);
    line[strlen(line)-1] = 0;

    if (line[0]==0)
       continue;
    pathname[0] = 0;
    pathname2[0] = 0;

    sscanf(line, "%s %s %s", cmd, pathname, pathname2);
    printf("\ncmd=%s pathname=%s %s\n", cmd, pathname, pathname2);

   // MAIN COMMANDS
    if (strcmp(cmd, "ls")==0)           // LEVEL 1: LS
       my_ls(pathname);
    else if (strcmp(cmd, "cd")==0)      // LEVEL 1: CD
       my_cd(pathname);
    else if (strcmp(cmd, "pwd")==0)     // LEVEL 1: PWD
       my_pwd(running->cwd);
    else if (strcmp(cmd, "mkdir")==0)   // LEVEL 1: MKDIR
       my_mkdir(pathname);
    else if (strcmp(cmd, "creat")==0)   // LEVEL 1: CREAT
       my_creat(pathname);
    else if (strcmp(cmd, "rmdir")==0)   // LEVEL 1: RMDIR
       my_rmdir(pathname);
    else if (strcmp(cmd, "link")==0)    // LEVEL 1: LINK
       my_link(pathname, pathname2); 
    else if (strcmp(cmd, "unlink")==0)  // LEVEL 1: UNLINK
       my_unlink(pathname);
    else if (strcmp(cmd, "symlink")==0) // LEVEL 1: SYMLINK
       my_symlink(pathname, pathname2);
    else if (strcmp(cmd, "cat")==0)     // LEVEL 2: CAT
       my_cat(pathname);
    else if (strcmp(cmd, "cp")==0)      // LEVEL 2: CP
       my_cp(pathname, pathname2); 
    else if (strcmp(cmd, "mount")==0)   // LEVEL 3: MOUNT
       my_mount(pathname, pathname2);
    else if (strcmp(cmd, "umount")==0)  // LEVEL 3: UMOUNT
       my_umount(pathname);
   // MISC COMMANDS
    else if (strcmp(cmd, "mv")==0 || strcmp(cmd, "rename")==0)  // MISC [LEVEL 2]: MV, RENAME
       mymv(pathname, pathname2); 
    else if (strcmp(cmd, "open")==0) // MISC [LEVEL 2]: OPEN
       if (strcmp(pathname2,"") != 0) open_file(pathname, atoi(pathname2));
       else printf("\nnot valid arguments\n");
    else if (strcmp(cmd, "close")==0)  // MISC [LEVEL 2]: CLOSE
       if (strcmp(pathname,"") != 0) close_file(atoi(pathname));
       else printf("\nnot valid arguments\n");
    else if (strcmp(cmd, "pfd")==0)  // MISC [LEVEL 2]: PFD
       mypfd(); 
    else if (strcmp(cmd, "read")==0) // MISC [LEVEL 2]: READ
       if (strcmp(pathname, "") != 0 && strcmp(pathname2,"") != 0) read_file(atoi(pathname), atoi(pathname2));
       else printf("\nnot valid arguments\n");
    else if (strcmp(cmd, "write")==0) // MISC [LEVEL 2]: WRITE
       if (strcmp(pathname, "") != 0 && strcmp(pathname2,"") != 0) write_file(atoi(pathname), atoi(pathname2));
       else printf("\nnot valid arguments\n");
    else if (strcmp(cmd, "quit")==0)  // MISC: QUIT
       quit();
    else if (strcmp(cmd, "debug")==0) { // MISC: DEBUG
       debug = !debug;
       printf("\ndebug is now %s\n", (debug ? "ON" : "OFF"));
    }
    else if (debug) { // DEBUG COMMANDS
         if (strcmp(cmd, "print")==0) // print (int) -> prints out (int) minnodes
            if (strcmp(pathname,"") != 0) printMinnodes(atoi(pathname));
            else printf("\nnot valid arguments\n");
         else if (strcmp(cmd, "close")==0)  // close file
            if (strcmp(pathname,"") != 0) close_file(atoi(pathname));
            else printf("\nnot valid arguments\n");
         else if (strcmp(cmd, "getmptr")==0) // get mptr of dev inputted
            if (strcmp(pathname,"") != 0) getmptr(atoi(pathname));
            else printf("\nnot valid arguments\n");
         else if (strcmp(cmd, "menu")==0) // get debug menu
            debug_menu();
         else  
            printf("\nnot valid command\n");
    }
    else if (strcmp(cmd, "menu")==0) // MISC: MENU
       menu(); // put after debug commands, so if not in debug, give regular menu
    else
      printf("\nnot valid command\n");
  }
}

int menu() {
   printf("\n************************\n");
   printf("\n[menu]\nprints out all possible commands and their descriptions\n");
   printf("\n[ls (optional directory name)]\nprints files and directories in the current working directory by default in a long list format\n");
   printf("\n[cd (directory name)]\nchanges the current working directory to the directory listed after this command. use ../ to go to the previous directory\n");
   printf("\n[pwd]\nprints the current working directory\n");
   printf("\n[mkdir (new directory name)]\nmakes a new directory\n");
   printf("\n[creat (new file name)]\ncreates a new file in the current working directory\n");
   printf("\n[rmdir (directory name to remove)]\nremoves an empty directory\n");
   printf("\n[link (filename1) (filename2)]\nhard links filename2 to filename1\n");
   printf("\n[unlink (filename)]\nunlinks filename\n");
   printf("\n[symlink (name1) (name2)]\nsoft links name2 to name1\n");
   printf("\n[cat (filename)]\nprints contents of filename\n");
   printf("\n[cp (src) (dest)]\ncopies src to dest\n");
   printf("\n[mv (src) (dest)]\nmoves src to dest\n");
   printf("\n[rename (src) (dest)]\renames src to dest\n");
   printf("\n[pfd]\nprints out currently open files\n");
   printf("\n[debug]\nuse commands like open, close\n");
   printf("\n[quit]\nquits application\n");
   printf("\n************************\n");
}

int debug_menu() {
   printf("\n************************\n");
   printf("\n[menu]\nprints out all possible debug commands and their descriptions\n");
   printf("\n[print (int)]\nprints out int number of minnodes\n");
   
   printf("\n************************\n");
}