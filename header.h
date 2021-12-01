#ifndef HEADER_H
#define HEADER_H

#include "type.h"

// util functions
int get_block(int dev, int blk, char *buf);
int put_block(int dev, int blk, char *buf);
int tokenize(char *pathname);
MINODE *iget(int dev, int ino);
void iput(MINODE *mip);
int search(MINODE *mip, char *name);
int getino(char *pathname);
int findmyname(MINODE *parent, u32 myino, char myname[ ]) ;
int findino(MINODE *mip, u32 *myino);

// my_command functions
int my_cd(char * pathname);
int my_ls(char * pathname);
int my_pwd(MINODE *wd);
int quit();
int my_mkdir(char *pathname);
int my_creat(char *pathname);
int my_rmdir(char *pathname);
int my_link(char *old_file, char *new_file);
int my_unlink(char *filename);
int my_symlink(char *old_file, char *new_file);
int my_cat(char *pathname);
int my_cp(char * src, char * dest);

// command helper functions
int enter_name(MINODE * pip, int ino, char * basename);
int rm_child(MINODE * pmip, char *rname);
int balloc(int dev);
int idalloc(int dev, int ino);
int bdalloc(int dev, int bno);
char * rpwd(MINODE *wd, int print);

int open_file(char * pathname);
int close_file(int fd);

#endif