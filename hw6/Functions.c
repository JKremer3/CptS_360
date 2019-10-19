#include "func.h"
#include "assert.h"

int get_block(int fd, int blk, char *buf)
{
    if (-1 == lseek(fd, (long)(blk*BLKSIZE), 0))
    {
        assert(0);
    }
    read(fd, buf, BLKSIZE);
}

void get_inode_table(int fd)
{
    GD* gp;
    char buf[BLKSIZE];
    get_gd(fd, buf);
    gp = (GD*)buf;
    inodeBegin = gp->bg_inode_table;
}

void printBin(char a)
{
    int bin = 0, i = 0;
    for(i = 0; i < 8; i++)
    {
        bin = a;
        printf("%u \n", (bin>>i)&1);
    }
}

int get_super(int fd, char * output)
{
  // read SUPER block (block 1)
  get_block(fd, 1, output);
}

int print_super(char * input)
{
  SUPER* sp = (SUPER *)input;
  
  printf("\n***Super Block***\n");
  // check EXT2 FS magic number:
  // Magic number is 0xEF53
  printf("magic = %x\n", sp->s_magic);

  // print other fields of SUPER block
  printf("inodes_count = %u\n", sp->s_inodes_count);
  printf("blocks_count = %u\n", sp->s_blocks_count);
  printf("free_inodes_count = %u\n", sp->s_free_inodes_count);
  printf("free_blocks_count = %u\n", sp->s_free_blocks_count);
  printf("log_block_size = %u\n", sp->s_log_block_size);
  printf("blocks_per_group = %u\n", sp->s_blocks_per_group);
  printf("inodes_per_group = %u\n", sp->s_inodes_per_group);
  printf("mnt_count = %hu\n", sp->s_mnt_count);
  printf("max_mnt_count = %hu\n", sp->s_max_mnt_count);
  printf("mtime = %s\n", ctime((time_t*)&(sp->s_mtime)));
  printf("inode_size = %hu\n", sp->s_inode_size);
}

int is_ext2(char *input) // checks if the input name is a path to EXt2 file system
{
    SUPER *sp = (SUPER*)input;
    if (EXT2_NUM == sp->s_magic)
    {
        printf("magic = %x\n", sp->s_magic);
        return 1;
    }
    return 0;
}

// Prints group descriptor information
int get_gd(int fd, char *output)
{
    //read GD in block 2.
    get_block(fd, 2, output);
}

int print_gd(char *input) //Prints group descriptor of input
{
    GD* gp = (GD *)input;
        
    //print fields of GP block
    printf("\n***Group Block***\n");
    printf("bg_block_bitmap = %u\n", gp->bg_block_bitmap);
    printf("bg_inode_bitmap = %u\n", gp->bg_inode_bitmap);
    printf("bg_inode_table = %u\n", gp->bg_inode_table);
    printf("bg_free_blocks_count = %hu\n", gp->bg_free_blocks_count);
    printf("bg_free_inodes_count = %hu\n", gp->bg_free_inodes_count);
    printf("bg_used_dirs_count = %hu\n", gp->bg_used_dirs_count);
}

//Print Bitmap
int print_bmap(int fd)
{
    char buf[BLKSIZE];
    char buf1[BLKSIZE];
    int i = 0, j = 0;
    GD* gp;
    
    //read GD block in block 2.
    get_gd(fd, buf1);
    gp = (GD *)buf1;

    //read Bitmap into buffer (bitmap block # taken from group block)
    get_block(fd, gp->bg_block_bitmap, buf);

    //Print bitmap
    printf("\n***Bmap***\n");
    while(i < BLKSIZE)
    {
       for(j = 0; (j < 10) && (i < BLKSIZE); j++)
       {
            printBin(buf[i]);
            printf(" ");
            i++;
        }
        printf("\n");
    }
}

//Print Imap
int print_imap(int fd)
{
    char buf[BLKSIZE];
    char buf1[BLKSIZE];
    int i = 0, j = 0;
    GD* gp;
    
    //read GP block at byte offset 2048
    get_gd(fd, buf1);
    gp = (GD *)buf1;

    //read Bitmap into buffer (bitmap block # taken from group block)
    get_block(fd, gp->bg_inode_bitmap, buf);

    
    //Print bitmap
    printf("\n***Imap***\n");
    while(i < BLKSIZE)
    {
       for(j = 0; (j < 10) && (i < BLKSIZE); j++)
       {
            printBin(buf[i]);
            printf(" ");
            i++;
        }
        printf("\n");
    }
}

// Find and print the root inode.
int inode(int fd)
{
    char buf[BLKSIZE];
    char buf1[BLKSIZE];
    int i = 0, j = 0;
    INODE* ip;
    GD* gp;
    
    //read GD block at block 2.
    get_block(fd, 2, buf1);
    gp = (GD *)buf1;

    //read inode table block into buffer (inode start from gp).
    get_block(fd, gp->bg_inode_table, buf);
    ip = (INODE *)buf;
    ip++; // Move to second inode in block.

    //Print root inode information
    printf("\n***INODE***\n");
    printf("inode_block = %u\n", gp->bg_inode_table);
    printf("mode = %x\n", ip->i_mode);
    printf("uid = %hu\n", ip->i_uid);
    printf("gid = %hu\n", ip->i_gid);
    printf("size = %u\n", ip->i_size);
    printf("time = %s\n", ctime((time_t *)&ip->i_mtime));
    printf("link = %u\n", ip->i_links_count);
    printf("i_block[0] = %u\n", ip->i_block[0]);
}

// Prints dir_entries of the root directory
void dir(int fd)
{
    // First read i_block[0]
    int root_inode_block = 0;
    int i_block_0 = 0;
    char buf[BLKSIZE];
    INODE* ip;
    GD* gp;
    DIR* dp;
    
    // Find the root inode through group descriptor
    get_block(fd, 2, buf);
    gp = (GD*)buf;
    root_inode_block = gp->bg_inode_table;

    // Now load the root inode.
    get_block(fd, root_inode_block, buf);
    ip = (INODE*)buf;
    ip++; // Move to 2nd inode
    i_block_0 = ip->i_block[0];

    // Now load i_block[0]
    get_block(fd, i_block_0, buf);
    dp = (DIR*)buf;

    while ((dp->inode != 0) && (((char*)dp - buf) < 1024))
    {
        char *next = (char*)dp;
        next += dp->rec_len;
        printf("\n***DIR***\n");
        printf("inode number = %u\n", dp->inode);
        printf("record length = %u\n", dp->rec_len);
        printf("name length = %u\n", dp->name_len);
        printf("file type = %u\n", dp->file_type);
        printf("File name = %s\n", dp->name);
        dp = (DIR*)next;
    }
}

DIR* next_dir(DIR *dp) //Moves the dir pointer to next dir
{
    char *next = (char*)dp;
    next += dp->rec_len;
    dp = (DIR*)next;
    return dp;
}

//Sets buf to block containing inode, and returns offset of inode within block
int iget(int fd, unsigned int ino, char buf[BLKSIZE])
{ 
    int blk, offset;

    blk = (ino-1)/8 + inodeBegin;
    offset = (ino-1)%8;

    get_block(fd, blk, buf);
    
    return offset;
}

int search(int fd, INODE *ip, char *str)
{
    char *cp;
    DIR *dp;
    char buf[BLKSIZE];
    char temp[256];

    for(int i = 0; i < 12; i++)
    { 
        printf("iblock[%d] = %d\n", i, ip->i_block[i]);
        if(ip->i_block[i] == 0) // not allocated
        {
            break;
        }

        get_block(fd, ip->i_block[i], buf); //get the dir at node
        dp = (DIR *)buf; 
        //printf("dp->name = %s\n", dp->name);
        cp = buf;

        while(cp < buf+BLKSIZE) 
        {
            memset(temp, 0, 256); // resets temp to all 0
            strncpy(temp, dp->name, dp->name_len); // copies dir name to temp
            printf("%4d %4d %4d %s\n", dp->inode, dp->rec_len, dp->name_len, temp);// prints values and string names
            if(strcmp(str, temp) == 0)
            {
                //printf("Ino = %d\n", dp->inode);
                return dp->inode;
            }
            cp += dp->rec_len;
            dp = (DIR*)cp;
        }
    }
    return 0;
}

void printInoBlock(int fd, INODE *ip)
{
    unsigned int buf[256];
    unsigned int buf2[256];
    printf("\n*********Disk Blocks**********\n");
    for(int i = 0; i < 15; i++)
    {
        if(ip->i_block[i] != 0)
        {
            if(i == 9)
            {
                printf("\n");
            }
            printf("%d ", ip->i_block[i]);
        }
    }
    printf("\n");
    if(ip->i_block[12]) //Indirect block exists
    {
        printf("\n--------------Indirect Blocks--------------\n");
        //Print indirect blocks 
        get_block(fd, ip->i_block[12], (char*)buf);
        for(int i = 0; i < 256; i++)
        {
            if(buf[i])
            {
                if((i != 0) && (i%10 == 0))
                {
                    printf("\n");
                }
                printf("%d ", buf[i]);
            }
        }
        printf("\n");
    }
    if(ip->i_block[13]) //Double indirect block exists
    {
        printf("\n--------------Double Indirect Blocks---------------\n");
        //TODO: Print Double indirect blocks
        get_block(fd, ip->i_block[13], (char*)buf);
        for(int i = 0; i < 256; i++)
        {
            if(buf[i])
            {
                get_block(fd, buf[i], (char*)buf2);
                for(int j = 0; j < 256; j++)
                {
                    if(buf2[j])
                    {
                        if((j != 0) && (j%10 == 0))
                        {
                            printf("\n");
                        }
                        printf("%d ", buf2[j]);
                    }
                }
            }
        }

        printf("\n");
    }
}