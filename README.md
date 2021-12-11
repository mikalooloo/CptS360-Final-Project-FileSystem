cpts360_project

# CPTS360 (Systems Programming) at WSU
## KC Wang Fall 2021

### This project was to make our own filesystem in C, using similar structures and processes to that of Unix/Linux and MTX Operating Systems, with 3 levels of completion. 
This project was given credit for all 3 levels.
This includes programming the following functions to work in our filesystem:
  `ls -l, mkdir, rmdir, creat, cd, pwd, link, unlink, symlink, cat, cp, mount, umount`
and more. 

### Some other targets and learning points of this project include the following:
  **Unix/Linux and MTC Operating Systems**
     Files, directories, special files, logical organization of 
     Unix file system; user account, login process and command
     execution.

  **Program development**
     Source files;  compiler, assembler and object files; 
     linker, library and executable files; loader and execution 
     images. Symbolic debugger and run-time support.

  **Execution image of C programs**
     Code, data and stack segments; function calling convention, 
     stack frames and parameter passing; long jumps.

  **File I/O**
     System calls and low-level file I/O; open, close, read, write, 
     lseek, file descriptors and file sharing. Execution of User 
     mode and Kernel mode images, implementation and implications 
     of system calls.

  **File Control**
     Permissions and access control, fcntl, chown, chmod, hard 
     and soft links, file status and statistics.
     I/O redirection, pipes, filters and applications.

  **Standard I/O Library**
     Streams and high-level file I/O; user space buffering, 
     relationship with low-level I/O,  char and line mode I/O. 
     Formatted I/O.

 **File system implementation**
     Inodes and file representation; mkfs and physical file 
     system layout; traversal of the file system tree; booting 
     system images.

  **Processes**
     Concept and implementation of processes, process execution 
     environment, user mode and kernel mode images, process
     states transitions. Processes in the Unix system; init, 
     login and user processes.
  
  **Git**
    Git commands such as pull and push, meaningful commit
    comments, and repo, project, folder management.

### Future changes to be added:
- [ ] add permission checking
- [ ] finish write_file() and read_file(); both user-focused versions of cp() and cat()
- [ ] finish mv()
- [ ] add more uniformity to the code; create standard naming conventions, comment style, and more 
In retrospect, creating standards to be used and referenced throughout the code before starting the project can ensure less work on both the programmers and anyone reviewing the code. It will be easier to build off of previous commits and will be easier to understand. 
