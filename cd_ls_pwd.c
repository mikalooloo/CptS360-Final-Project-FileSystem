#include "header.h"

extern int dev;
extern MINODE minode[NMINODE];
extern PROC   proc[NPROC], *running;
extern MINODE *root;
char *t1 = "xwrxwrxwr-------";
char *t2 = "----------------";

/************* cd_ls_pwd.c file **************/

// changes cwd to pathname
// returns 0 if successful, -1 if failed
int cd(char * pathname)
{
  // (1). int ino = getino(pathname); // return error if ino==-1
  int ino = getino(pathname);
  if (ino == -1) {
    printf("ino for pathname %s cannot be found\n", pathname);
    return -1;
  }

  // (2). MINODE *mip = iget(dev, ino);
  MINODE *mip = iget(dev, ino);

  // (3). Verify mip->INODE is a DIR // return error if not DIR
  if (!S_ISDIR(mip->INODE.i_mode)) {
    printf("mip->INODE is not a DIR\n");
    return -1;
  }
  else printf("mip->INODE is a DIR\n");

  // (4). iput(running->cwd); // release old cwd
  iput(running->cwd);

  // (5). running->cwd = mip; // change cwd to mip
  running->cwd = mip;
  return 0;
}

int ls_file(MINODE *mip, char *name)
{
  //vars initialized
  int r, i;
  char ftime[256];
  
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
  strcpy(ftime, ctime((time_t *)&mip->INODE.i_mtime));
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
  char buf[BLKSIZE], temp[256];
  DIR *dp;
  char *cp;
  MINODE * mip2;

  get_block(dev, mip->INODE.i_block[0], buf);
  dp = (DIR *)buf;
  cp = buf;

  while (cp < buf + BLKSIZE){
     strncpy(temp, dp->name, dp->name_len);
     temp[dp->name_len] = 0;
	
     //printf("%s  ", temp);
     mip2 = iget(dev, dp->inode);
     
     ls_file(mip2, temp);
     mip2->dirty = 1;
     iput(mip2);
      
     cp += dp->rec_len;
     dp = (DIR *)cp;
  }
  printf("\n");
}

int ls(char * pathname)
{
  if (strcmp(pathname, "") != 0) printf("\nls %s\n", pathname);
  else printf("\nls cwd\n");

  if (!strcmp(pathname, "")) ls_dir(running->cwd); // if ls the cwd
  else { // otherwise
    dev = root->dev;
    int ino = getino(pathname);
    MINODE * mip = iget(dev, ino);

    if ((mip->INODE.i_mode & 0xF000) == 0x4000) ls_dir(mip);
    else ls_file(mip, basename(pathname));
    iput(mip);
  }
}

// recursive function for pwd
// if print is 1, it prints like expected for pwd. if print is 0, it returns the cwd
char * rpwd(MINODE *wd, int print) {
  // (1). if (wd==root) return;
  if (wd == root) {
    return "/";
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
  char * temp = rpwd(pip, print);
  char temp2[128];
  strcpy(temp2, temp);
  printf("my_name: %s\n", my_name);
  printf("temp: %s\n", temp);
  pip->dirty = 1;
  iput(pip);

  // (6). print "/%s", my_name;
  if (print) printf("/%s", my_name);
  else return strcat(temp2, my_name);
}


char *pwd(MINODE *wd)
{
  if (wd == root) {
    printf("\n/\n\n");
  }
  else {
    rpwd(wd, 1);
    printf("\n\n");
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

