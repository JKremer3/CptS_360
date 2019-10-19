#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <ext2fs/ext2_fs.h>

#define BLKSIZE 1024

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
int iblock;
char *disk = "mydisk";

int get_block(int fd, int blk, char buf[ ])
{
  lseek(fd, (long)blk*BLKSIZE, 0);
   read(fd, buf, BLKSIZE);
}

unsigned int search(INODE *ip, char *target, int fd)
{
    char buf[1024], *cp;
    unsigned int firstBlock = ip->i_block[0];
    DIR * dip;

    get_block(fd, firstBlock, buf);
    cp = buf;
    dip = (DIR*)buf;

    while(cp - buf < 1024)
    {
        char name[EXT2_NAME_LEN];
        strncpy(name, dip->name, dip->name_len);
        name[dip->name_len] = '\0';

        if(strcmp(name, target) == 0)
        {
            return dip->inode;
        }

        cp += dip->rec_len;
        dip = (DIR *) cp;
    }

    return 0;
}

int main(int argc, char *argv[])
{
    if(argc < 2)
    {
        printf("Usage: dir [disk] or dir [disk] [pathToSearch] ");
        return 1;
    }

    char buffer[BLKSIZE];
    int fd = open (argv[1], O_RDONLY);
    
    get_block(fd,2,buffer);
    
    GD* gp = (GD* ) buffer;

    get_block(fd, gp->bg_inode_table, buffer);

    INODE* ip = (INODE*) buffer + 1;

    //if argc == 2, print all entries under /
    if (argc == 2)
    {
        unsigned int baseBlock = ip->i_block[0];
        
        get_block(fd, baseBlock, buffer);

        char *cp = buffer;
        DIR * dp = (DIR*)buffer;

        while(cp - buffer < 1024)
        {
            char name[EXT2_NAME_LEN];

            strncpy(name, dp->name, dp->name_len);
            name[dp->name_len] = '\0';
            printf("%s \n", name);

            cp += dp->rec_len;
            dp = (DIR *)cp;
        }

        return 0;
    }
    else if (argc == 3) // call search using argv[2]
    {
        int isFound = search(ip, argv[2], fd);

        if(isFound)
            printf ("inode number = %d\n", isFound);
        else
            printf("%s not found\n", argv[2]);
    }

    return 0 ;
}