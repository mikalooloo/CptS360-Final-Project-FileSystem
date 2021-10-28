#include "header.h"

extern int dev;

char *t1 = "xwrxwrxwr-------";
char *t2 = "----------------";

/************* cd_ls_pwd.c file **************/

// changes cwd to pathname
// returns 1 if successful, 0 if failed
int cd(char * pathname)
{
  // (1). int ino = getino(pathname); // return error if ino=0
  int ino = getino(pathname);
  if (!ino) {
    printf("ino is zero as it cannot be found\n");
    return 0;
  }

  // (2). MINODE *mip = iget(dev, ino);
  MINODE *mip = iget(dev, ino);

  // (3). Verify mip->INODE is a DIR // return error if not DIR
  if (!S_ISDIR(mip->INODE.i_mode)) {
    printf("mip->INODE is not a DIR\n");
    return 0;
  }
  // (4). iput(running->cwd); // release old cwd
  iput(running->cwd);

  // (5). running->cwd = mip; // change cwd to mip
  running->cwd = mip;
  return 1;
}

int ls_file(MINODE *mip, char *name)
{
  //vars initialized
  int r, i;
  char ftime[64];

  //check inode, output accordingly
  if((mip->INODE.i_mode & 0xF000) == 0x8000)
  {
	  printf("%c", '-');
  }
  if((mip->INODE.i_mode & 0xF000) == 0x4000)
  {
	  printf("%c", 'd');
  }
  if((mip->INODE.i_mode & 0xF000) == 0xA000)
  {
	  printf("%c", 'l');
  }

  //iterate through
  for(i = 8; i >= 0; i--)
  {
	  if(mip->INODE.i_mode & (1 << i))
	  {
		  printf("%c", t1[i]);
	  }
	  else
	  {
		  printf("%c", t2[i]);
	  }
  }

  //output 
  printf("%4d ", mip->INODE.i_links_count);
  printf("%4d ", mip->INODE.i_gid);
  printf("%4d ", mip->INODE.i_uid);
  printf("%8d ", mip->INODE.i_size);

  //copy string, then print
  strcpy(ftime, ctime(&mip->INODE.i_ctime));
  ftime[strlen(ftime) - 1] = 0;
  printf("%s ", ftime);

  //output name
  //basename incl
  printf("%s", basename(name));

  //print->linkname
  if((mip->INODE.i_mode & 0xF000) == 0xA000)
  {
	  char buffer[256];
	  strcpy(buffer,(char *)mip->INODE.i_block);
	  printf("->");
	  printf("%s", buffer);
  }
  printf("\n");
}

int ls_dir(MINODE *mip)
{
  //printf("ls_dir: list CWD's file names; YOU FINISH IT as ls -l\n");

  char buf[BLKSIZE], temp[256];
  DIR *dp;
  char *cp;

  get_block(dev, mip->INODE.i_block[0], buf);
  dp = (DIR *)buf;
  cp = buf;
  
  while (cp < buf + BLKSIZE){
     strncpy(temp, dp->name, dp->name_len);
     temp[dp->name_len] = 0;
	
     printf("%s  ", temp);

     cp += dp->rec_len;
     dp = (DIR *)cp;
  }
  printf("\n");
}

int ls()
{
  printf("ls: list CWD only! YOU FINISH IT for ls pathname\n");
  ls_dir(running->cwd);
}

// recursive function for pwd
void rpwd(MINODE *wd) {
  // (1). if (wd==root) return;
  if (wd == root) {
    return;
  }

  // (2). from wd->INODE.i_block[0], get my_ino and parent_ino
  char buf[BLKSIZE];
  get_block(dev, wd->INODE.i_block[0], buf);

  int my_ino;
  int parent_ino;
  parent_ino = findino(wd, &my_ino);

  // (3). pip = iget(dev, parent_ino);
  MINODE * pip = iget(dev, parent_ino);

  // (4). from pip->INODE.i_block[ ]: get my_name string by my_ino as LOCAL
  char my_name[256];
  findmyname(pip, my_ino, my_name);

  // (5). rpwd(pip); // recursive call rpwd(pip) with parent minode
  rpwd(pip);
  pip->dirty = 1;
  iput(pip);

  // (6). print "/%s", my_name;
  printf("/%s", my_name);
}


char *pwd(MINODE *wd)
{
  if (wd == root) {
    printf("/\n");
  }
  else {
    rpwd(wd);
    printf("\n");
  }
}

int quit()
{
  int i;
  MINODE *mip;
  for (i=0; i<NMINODE; i++){
    mip = &minode[i];
    if (mip->refCount > 0)
      iput(mip);
  }
  exit(0);
}

