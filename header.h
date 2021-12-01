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
int mymkdir(char *pathname);
int mycreat(char *pathname);
int myrmdir(char *pathname);
int my_link(char *old_file, char *new_file);
int my_unlink(char *filename);
int symlink(char *old_file, char *new_file);

int enter_name(MINODE * pip, int ino, char * basename);
int rm_child(MINODE * pmip, char *rname);
int balloc(int dev);
int idalloc(int dev, int ino);
int bdalloc(int dev, int bno);
char * rpwd(MINODE *wd, int print);

int open_file(char * pathname);
int close_file(int fd);

#endif