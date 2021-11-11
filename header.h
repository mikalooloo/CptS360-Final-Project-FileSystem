#ifndef HEADER_H
#define HEADER_H

#include "type.h"

int get_block(int dev, int blk, char *buf);
int put_block(int dev, int blk, char *buf);
int tokenize(char *pathname);
MINODE *iget(int dev, int ino);
void iput(MINODE *mip);
int search(MINODE *mip, char *name);
int getino(char *pathname);
int findmyname(MINODE *parent, u32 myino, char myname[ ]) ;
int findino(MINODE *mip, u32 *myino);

int cd(char * pathname);
int ls(char * pathname);
char *pwd(MINODE *wd);
int quit();
int mymkdir(char pathname[128]);
int mycreat(char pathname[128]);

#endif