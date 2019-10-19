#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <ext2fs/ext2_fs.h>
#include <time.h>
#include <string.h>

#include "func.h"

// FS stuff
#define BLKSIZE 1024
// Fragment size 1024
// 184 inodes
// 1440 blocks
// First data block = 1
// 1 block group
// 8192 fragments, blocks per group


main(int argc, char *argv[])
{ 
  int i = 0, j = 0, k = 0, ino = 0, offset = 0;
  int fd;
  char buf[BLKSIZE];
  char *device = "mydisk";
  char *path;
  char *name[256];

  if (argc !=3)
  {
    printf("Usage: showblock <disk> <path>\n");
    return -1;
  }
  // Assign args
  device = argv[1];
  path = argv[2];

  fd = open(device, O_RDONLY);
  if (fd < 0){
     printf("open %s failed\n", device);
     exit(1);
  }

  get_super(fd, buf);
  get_inode_table(fd);
  if (is_ext2(buf) > 0)
  {
    GD* gp;
    INODE* ip;
    DIR* dp;
    int found = 0;

    // Print super block information
    print_super(buf);

    // Find and get the root inode
    get_gd(fd, buf);
    gp = (GD*)buf;
    get_block(fd, gp->bg_inode_table, buf);
    ip = (INODE*)buf;
    ip++;

    // Tokenize the input
    printf("\nPathname = %s\n", path);
    name[0] = strtok(path, "/");
    i = 1;
    while ((name[i] = strtok(NULL, "/")) != NULL)
    {
        i++;
    }
    name[i] = 0;
    i = 0;
    ino = 1;
    while(ino != 0 && name[i]) //Search for each segment of path until final segment found or 0 is returned (path does not exist)
    {
        printf("Searching for path segment: %s\n", name[i]);
        ino = search(fd, ip, name[i]); 
        if(ino)
        {
            offset = iget(fd, ino, buf);
            printf("\nFound: ino # = %d, Offset = %d\n", ino, offset);
            ip = (INODE *)buf + offset;
        }
        else
        {
            printf("Path not found\n");
        }
        i++;
    }
    //TODO: Print disk blocks of file (direct, indirect, double indirect)
    if(ino) //Path exists and ip has been set to inode at end of path so print block info
    { 
       printInoBlock(fd, ip); 
    }
  }
  else
  {
    printf("Error: Not EX2 file system\n"); 
  }
}