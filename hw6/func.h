#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <ext2fs/ext2_fs.h>
#include <time.h>

// define shorter TYPES
typedef struct ext2_group_desc  GD;
typedef struct ext2_super_block SUPER;
typedef struct ext2_inode       INODE;
typedef struct ext2_dir_entry_2 DIR; 

// FS stuff
#define BLKSIZE 1024
// Fragment size 1024
// 184 inodes
// 1440 blocks
// First data block = 1
// 1 block group
// 8192 fragments, blocks per group
// Magic number 0xEF53
#define EXT2_NUM 0xEF53

// Global Variables
unsigned int inodeBegin; //Block number of inode table

// Gets a block
int get_block(int fd, int blk, char *buf);

// Sets global variable inode_table to the block number of inode table
void get_inode_table(int fd);

void printBin(char a);

//
// Get super block information.
int get_super(int fd, char *output);

// Prints superblock information
int print_super(char *input);

// 1 if yes, 0 if no.
int is_ext2(char *input);

//
// Gets GD information
int get_gd(int fd, char* output);

// Prints group descriptor information
int print_gd(char *input);

//
//Print Bitmap
int print_bmap(int fd);

//Print Imap
int print_imap(int fd);

//
// Find and print the root inode.
int inode(int fd);

// Prints dir_entries of the root directory
void dir(int fd);

DIR* next_dir(DIR *dp);

//Sets buf to block containing inode and returns offset of inode within block
int iget(int fd, unsigned int ino, char buf[BLKSIZE]);

//Searches through inodes to find pathname segment, returns inode number if found or 0 if not found
int search(int fd, INODE *ip, char *str);

void printInoBlock(int fd, INODE *ip);