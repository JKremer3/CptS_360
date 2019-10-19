#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <ext2fs/ext2_fs.h>
#include <assert.h>
#include <errno.h>
#include <string.h>


typedef unsigned char  u8;
typedef unsigned short u16;
typedef unsigned int   u32;

// define shorter TYPES, save typing efforts
typedef struct ext2_group_desc  GD;
typedef struct ext2_super_block SUPER;
typedef struct ext2_inode       INODE;
typedef struct ext2_dir_entry_2 DIR;    // need this for new version of e2fs

GD    *gp;
SUPER *sp;
INODE *ip;
DIR   *dp; 
int fd;

#define BLKSIZE 1024
char buf[BLKSIZE];

int get_gd(int dev, char *buf);
int get_block(int dev, int blk, char *buf);
int bmap();
unsigned int search(INODE* ip, char* looking_for, int fd);