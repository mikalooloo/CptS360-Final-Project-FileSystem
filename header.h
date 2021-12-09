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
void separatePathname(char * pathname, char ** dname, char ** bname, char * command);
int validPathname(char * pathname);

// my_command functions
// LEVEL 1
int my_ls(char * pathname);
int my_cd(char * pathname);
int my_pwd(MINODE *wd);
int my_mkdir(char *pathname);
int my_creat(char *pathname);
int my_rmdir(char *pathname);
int my_link(char *old_file, char *new_file);
int my_unlink(char *filename);
int my_symlink(char *old_file, char *new_file);
// LEVEL 2
int my_cat(char *pathname);
int my_cp(char * src, char * dest);
// LEVEL 3
int my_mount(char * pathname, char * filesys);
int my_umount(char *filesys);
// MISC
int menu();
int quit();
// MISC LEVEL 2
int open_file(char * pathname, int mode);
int close_file(int fd);
int mypfd();
int read_file(int fd, int nbytes);
int write_file(int fd, int nbytes);
int mymv(char * src, char * dest);
// MISC LEVEL 3


// command helper functions
int enter_name(MINODE * pip, int ino, char * basename);
int rm_child(MINODE * pmip, char *rname);
int balloc(int dev);
int idalloc(int dev, int ino);
int bdalloc(int dev, int bno);
char * rpwd(MINODE *wd, int print);
int myread(int fd, char buf[], int nbytes);

// debug
int printMinnodes(int m);

#endif