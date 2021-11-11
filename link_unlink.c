//link_unlink.c

#include "header.h"

extern char * totalPath();

//link function, cmd: link old_fle new_file
int my_link(char *pathname, char *parameter)
{
	//var dec/init
	char buf[1024];
	char buf2[1024] = {0};
	char temp[200] = {0};
	int ino = getino(pathname);

	//copy temp, name
	strcpy(temp, parameter);
	int ino_base = getino(dirname(temp));

	//check if ino, or ino_base is 0, then return 0
	if(ino == 0)
	{
		return 0;
	}
	if(ino_base == 0)
	{
		return 0;
	}

	//copy minode
	MINODE *mip_cpy = iget(dev, ino);
	MINODE *mip_base = iget(dev, ino_base);

	//output pathname, and parameter
	printf("pathname: %s, parameter: %s\n", pathname, parameter);

	//verify 
	

	//COMPLETE LINK
}



