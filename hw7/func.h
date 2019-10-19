#include "type.h"
#include "assert.h"


int dev;
PROC *running;
PROC *readQueue;
PROC proc[NPROC];
MINODE minode[NMINODES];
MOUNT MountTable[NMOUNT];
int inodeBegin;
char pathname[128];
int bmap;
int imap;
int ninodes;
char *cmd[13] = {"ls", "cd", "pwd","quit", "mkdir", "rmdir", "creat", "rm", "link", "unlink", "symlink" , "chmod",NULL};

int init()
{
    proc[0].uid = 0;
    proc[0].cwd = 0;
    proc[1].uid = 1;
    proc[1].cwd = 0;

    running = &(proc[0]);
    readQueue = &(proc[1]);
    int i = 0;
    for(i = 0; i < 100; i++)
    {
        minode[i].refCount = 0;
        minode[i].ino = 0;
    }
    for(i = 0; i < 10; i++) { MountTable[i].dev = 0;}
    root = 0;
}

MINODE* iget(int dev1, unsigned int ino)
{
    int i = 0, blk, offset;
    char buf[BLKSIZE]; 
    MINODE *mip = NULL;
    //search minode[100] to see if inode already exists in array
    for(i = 0; i < 100; i++)
    {
        // If inode is already in array, set mip to point to MINODE in array, increment MINODE's refCount by 1.
        if(minode[i].refCount > 0 && minode[i].ino == ino)
        {
            //printf("MINODE for inode %d already exists, just copying\n", minode[i].ino); //FOR TESTING
            mip = &minode[i];
            minode[i].refCount++;
            return mip;
        }
    }
    
    //If you have reached here then the inode does not currently exist in minode[100]. Put inode from disk into free MINODE in array's INODE field.
    i = 0;
    while(minode[i].refCount > 0 && i < 100) { i++;}
    if(i == 100)
    {
        printf("Error: no space in INODE array\n");
        return 0;
    }
    blk = (ino-1)/8 + inodeBegin;
    offset = (ino-1)%8;
    get_block(dev1, blk, buf);
    ip = (INODE *)buf + offset;
    memcpy(&(minode[i].INODE), ip, sizeof(INODE)); //Copy inode from disk into minode array
    minode[i].dev = dev1;
    minode[i].ino = ino;
    minode[i].refCount = 1;
    minode[i].dirty = 0;
    minode[i].mounted = 0;
    minode[i].mptr = NULL; 
    return &minode[i];
}

int mount_root(char *devName)
{
    char buf[BLKSIZE];
    
    dev = open(devName, O_RDWR); //Open for read/write
    if (dev < 0){ //Could not open device
        printf("open %s failed\n", devName);
        exit(1);
    }
    get_super(dev, buf);
    sp = (SUPER*)buf;
    if(is_ext2(buf) <= 0) {exit(0);} //Check is ext2 filesystem, if not exit program

    get_inode_table(dev); //Sets global variable inodeBegin to start of inode table
    ninodes = sp->s_inodes_count;
    root = iget(dev, ROOT_INODE); //Set root inode
    proc[0].cwd = iget(dev, ROOT_INODE); // Set cwd for procedure 1 & 2 to root inode
    proc[1].cwd = iget(dev, ROOT_INODE);
    MountTable[0].mounted_inode = root;
    MountTable[0].ninodes = ninodes;
    MountTable[0].nblocks = sp->s_blocks_count;
    MountTable[0].dev = dev;
    strncpy(MountTable[0].name, devName, 256);
    return dev;
}

int findCmd(char *command)
{
    int i = 0;
    if (command == NULL)
    {
         printf("command failed to enter\n");
         return -1;
    }
    while (cmd[i])
    {
         //printf("comparing to cmd\n");
        if (!strcmp(command, cmd[i]))
            return i;
        
        i++;
    }
    printf("cmd not found\n");
    return -1;
}

//Tokenizes given pathname into an array of strings terminated with a null string.
char** tokenPath(char* pathname)
{
    int i = 0;
    char** name;
    char* tmp;
    name = (char**)malloc(sizeof(char*)*256);
    name[0] = strtok(pathname, "/");
    i = 1;
    while ((name[i] = strtok(NULL, "/")) != NULL) { i++;}
    name[i] = 0;
    i = 0;
    while(name[i])
    {
        tmp = (char*)malloc(sizeof(char)*strlen(name[i]));
        strcpy(tmp, name[i]);
        name[i] = tmp;
        i++;
    }
    return name;
}

//searches through 12 blocks to look for str. If found returns ino number, else returns 0 (indicating filepath does not exist)
int search(int dev, char *str, INODE *ip)
{
    int i;
    char *cp;
    DIR *dp;
    char buf[BLKSIZE], temp[256];
   
    for(i = 0; i < 12; i++)
    {
        if(ip->i_block[i] == 0){break;}
        get_block(dev, ip->i_block[i], buf);
        dp = (DIR *)buf;
        cp = buf;

        while(cp < buf+BLKSIZE)
        {
            memset(temp, 0, 256);
            strncpy(temp, dp->name, dp->name_len);

            if(strcmp(str, temp) == 0)
            { return dp->inode;}

            cp += dp->rec_len;
            dp = (DIR*)cp;
        }
    }

    return 0;
}

int searchByIno(int dev, int ino, INODE *ip, char* temp)
{
    int i;
    char *cp;
    DIR *dp;
    char buf[BLKSIZE];
   
    for(i = 0; i < 12; i++)
    {
        if(ip->i_block[i] == 0){ break;}
        get_block(dev, ip->i_block[i], buf);
        dp = (DIR *)buf;
        cp = buf;

        while(cp < buf+BLKSIZE)
        {
            if(ino == dp->inode) //Found the right inode
            { 
               strncpy(temp, dp->name, dp->name_len);
               return 1;
            }
            cp += dp->rec_len;
            dp = (DIR*)cp;
        }
    }
    return 0;
}

int get_block(int dev1, int blk, char *buf)
{
    // if the file offset change is not possible, print error
    if (-1 == lseek(dev1, (long)(blk*BLKSIZE), 0))
    {
        printf("%s\n", strerror(errno));
        assert(0);
    }
    read(dev1, buf, BLKSIZE);
}

int put_block(int dev, int blk, char *buf)
{
    if (-1 == lseek(dev, (long)(blk*BLKSIZE), 0))
    { assert(0);}
 
    write(dev, buf, BLKSIZE);
    return 1;
}

int is_ext2(char *buf)
{
    sp = (SUPER *)buf;
    if (SUPER_MAGIC != sp->s_magic)
    {
        printf("Error: Not an EXT2 file sytem\n");
        return -1;
    }
    return 1;
}
 
void get_inode_table(int dev1)
{
    char buf[BLKSIZE];
    get_gd(dev1, buf);
    gp = (GD*)buf;
    inodeBegin = gp->bg_inode_table;
    bmap = gp->bg_block_bitmap;
    imap = gp->bg_inode_bitmap;
}

int get_gd(int dev1, char *buf)
{
    get_block(dev1, GDBLOCK, buf);
}

int get_super(int dev1, char *buf)
{
   get_block(dev1, SUPERBLOCK, buf);
}

//Gets the ino number from a given pathname
unsigned int getino(int dev, char *path)
{
    int ino = 0, i = 0;
    char **tokens;
    MINODE *mip = NULL;
 
    if(path && path[0])
    {
        tokens = tokenPath(path);
    }
    else //No pathname given so set ino to cwd
    {
        ino = running->cwd->ino;
        return ino;
    }

    if(path[0]=='/') //start at root dir
    {
        ip = &(root->INODE);
        ino = root->ino;
    }
    else //start at cwd
    {
        ip = &(running->cwd->INODE);
    }

    while(tokens[i])
    {
        ino = search(dev, tokens[i], ip);
        if(0 >= ino)
        {
            if(mip){ iput(mip->dev, mip);}
            return -1;
        }
        if(mip) { iput(mip->dev, mip);}
        i++;
        if(tokens[i])
        {
            mip = iget(dev, ino);
            ip = &(mip->INODE);
        }
    }
    i = 0;
 
    while(tokens[i])
    {
        free(tokens[i]);
        i++;
    }
 
    if(mip) { iput(mip->dev, mip);}
 
    return ino;
}

int ls(char* path)
{
    int ino;
    MINODE *mip;
    
    if(!path || !pathname[0]) 
        ino = running->cwd->ino;
    else if(pathname[0] == '/' && !pathname[1]) 
        ino = root->ino;
    else 
        ino = getino(dev, path);
    
    if(0 >= ino)
    {
        printf("Invalid pathname\n");
        return -1;
    }
    mip = iget(dev, ino);
    
    findBlocks(&(mip->INODE), 0); 
    iput(mip->dev, mip);
}

//finds all the data blocks from a pointer to that inode and prints the dir names in those data blocks.
int findBlocks(INODE *ip, int printStat)
{
    int i, j , k;
    unsigned int buf[256], buf2[256];

    //Print dirs in direct blocks
    for(i = 0; i < 12; i++)
    {
        if(ip->i_block[i] != 0)
        {
            printDirs(ip->i_block[i], printStat);
        }
    }
    //Print dirs in indirect blocks
    if(ip->i_block[12]) //Indirect block exists
    {
        get_block(dev, ip->i_block[12], (char*)buf);
        for(i = 0; i < 256; i++)
        {
            if(buf[i]) { printDirs(buf[i], printStat); }
        }
    }
    //Print dirs in double indirect blocks
    if(ip->i_block[13]) //Double indirect block exists
    {
        get_block(dev, ip->i_block[13], (char*)buf);
        for(i = 0; i < 256; i++)
        {
            if(buf[i])
            {
                get_block(dev, buf[i], (char*)buf2);
                for(j = 0; j < 256; j++)
                {
                    if(buf2[j]) { printDirs(buf2[j], printStat);}
                }
            }
        }
    }
}

int printDirs(int block, int printStat)
{
    int i;
    char *cp;
    DIR *dp;
    char buf[BLKSIZE], temp[256];

    get_block(dev, block, buf);
    dp = (DIR *)buf;
    cp = buf;

    while(cp < buf+BLKSIZE)
    {
        if(printStat) 
            printStat1(dp);
        else
        {
            memset(temp, 0, 256);
            strncpy(temp, dp->name, dp->name_len);
            printf("%4d %4d %4d %s\n", dp->inode, dp->rec_len, dp->name_len, temp);
        }
        cp += dp->rec_len;
        dp = (DIR*)cp;
    }
    return 0;
}

int printStat1(DIR* dp)
{
    struct stat s;
    char temp[256], lnk[256];
    MINODE *mip;
    
    //Stat File
    myStat(dp, &s);

    //Print info
    if (S_ISLNK(s.st_mode)) { printf("%s ", "LNK"); }
    else if (S_ISREG(s.st_mode)) { printf("%s ", "REG"); }
    else if(S_ISDIR(s.st_mode)) { printf("%s ", "DIR"); }
    else { printf("%s ", "N/A"); }
    printf("%hu ", s.st_nlink);
    if (test_perm(&s, USER_READ) != 0) {printf("r");} 
    else {printf("-");}
    if (test_perm(&s, USER_WRITE) != 0) {printf("w");}
    else {printf("-");}
    if (test_perm(&s, USER_EXEC) != 0) {printf("x ");}
    else {printf("- ");}
    if (test_perm(&s, GROUP_READ) != 0) {printf("r");}
    else {printf("-");}
    if (test_perm(&s, GROUP_WRITE) != 0) {printf("w");}
    else {printf("-");} 
    if (test_perm(&s, GROUP_EXEC) != 0) {printf("x ");}
    else {printf("- ");}
    if (test_perm(&s, OTHER_READ) != 0) {printf("r");}
    else {printf("-");}
    if (test_perm(&s, OTHER_WRITE) != 0) {printf("w");}
    else {printf("-");}
    if (test_perm(&s, OTHER_EXEC) != 0) {printf("x ");}
    else {printf("- ");}
    
    // Print UID
    printf("%hu ", (unsigned short)s.st_uid);

    // Print size
    printf("%lu ", (unsigned long)s.st_size);

    // Print ctime in calendar format
    strncpy(temp,(char*)ctime(&(s.st_ctime)), 256);
    temp[strlen(temp)-1] = 0;
    printf("%s ", temp);

    //Print filename
    memset(temp, 0, 256);
    strncpy(temp, dp->name, dp->name_len);
    printf("%s ", temp);
    if(S_ISLNK(s.st_mode))
    {
        mip = iget(dev, dp->inode);
        printf("-> %s", (char*)mip->INODE.i_block);
        iput(mip->dev, mip);
    }
    printf("\n");
}

int myStat(DIR *dp, struct stat *s)
{
    int ino;
    MINODE *mip;
    
    if(!dp)
    {
        printf("Error: No directory entry\n");
        return -1;
    }
        //Get inode
    ino = dp->inode;
    mip = iget(dev, ino);
    ip = &(mip->INODE);

    s->st_dev = dev;
    s->st_ino = ino;
    s->st_mode = ip->i_mode; 
    s->st_nlink = ip->i_links_count;
    s->st_uid = ip->i_uid;
    s->st_gid = ip->i_gid;
    s->st_size = ip->i_size;
    s->st_blksize = BLKSIZE;
    s->st_blocks = ip->i_blocks;
    s->st_atime = ip->i_atime;
    s->st_mtime = ip->i_mtime;
    s->st_ctime = ip->i_ctime;

    iput(mip->dev, mip);
    return 1;
}

int test_mode(struct stat *input, enum stat_mode mode)
{
   if (((input->st_mode >> 12) & mode) == mode) { return 1; }
   return 0;
}

int test_perm(struct stat *input, enum perm_mode mode)
{
   if (((input->st_mode) & mode) != 0) { return 1; }
   return 0;
}

int pwd(char *pathname)
{
    char cwd[1024];
    
    printf("cwd = ");
    do_pwd(running->cwd);
    printf("\n");
}

int do_pwd(MINODE *wd)
{
    int ino = 0;
    MINODE *next = NULL;
    char temp[256];
    
    if(wd == root)
    {
        printf("/");
        return 1;
    }
    
    //Get parent's MINODE
    ino = search(dev, "..", &(wd->INODE));
    
    if(ino <= 0)
    {
        printf("ERROR: BAD INODE NUMBER\n");
        return -1;
    }
    
    next = iget(dev, ino);
    
    if(!next)
    {
        printf("ERROR: COULD NOT FIND INODE\n");
        return -1;
    }

    do_pwd(next);
    memset(temp, 0, 256);
    searchByIno(next->dev, wd->ino, &(next->INODE), temp);
    printf("%s/", temp);
    iput(next->dev, next);
    
    return;
}

int cd(char* pathname)
{
    MINODE *mip;
    unsigned int ino;
    if(!pathname || !pathname[0] || (pathname[0] == '/' && !pathname[1])){ ino = root->ino;}
    else { ino = getino(dev, pathname);}
    if(!ino)
    {
        printf("Error: Invalid Pathname\n");
        return 0;
    }
    mip = iget(dev, ino);
    //Verify inode is a dir
    if(!S_ISDIR(mip->INODE.i_mode))
    {
        printf("Error: End of path is not a directory\n");
        iput(dev, mip);
        return 0;
    }
    iput(dev, running->cwd);
    running->cwd = mip;
}

int iput(int dev, MINODE *mip)
{
    char buf[BLKSIZE];
    int blk, offset;
    INODE *tip;

    mip->refCount--;
 
    if(mip->refCount > 0) {return 1;}
    if(mip->dirty == 0) {return 1;}

    //Must write INODE back to disk
    blk = (mip->ino-1)/8 + inodeBegin;
    offset = (mip->ino-1)%8;
    get_block(dev, blk, buf);
    tip = (INODE*)buf + offset;
    memcpy(tip, &(mip->INODE), sizeof(INODE));
    put_block(mip->dev, blk, buf);
 
    return 1;
}

int close_file(char *pathname)
{
    OFT *oftp;
    MINODE *mip;
    int fd;
    if(!pathname[0])
    {
        printf("ERROR: NO FILE DESCRIPTOR GIVEN\n");
        return -1;
    }
    fd = atoi(pathname);

    if (fd > 9 || fd < 0)
    {
        printf("ERROR: FILE DESCRIPTOR OUT OF RANGE\n");
        return -1;
    }
    if(running->fd[fd] == NULL)
    {
        printf("ERROR: FILE DESCRIPTOR NOT FOUND\n");
        return -1;
    }
    oftp = running->fd[fd];
    running->fd[fd] = 0;
    oftp->refCount--;
    if(oftp->refCount > 0) {return 1;}
    mip = oftp->mptr;
    iput(mip->dev, mip);
    free(oftp);
    return 1;
}

//Quit program iputs all dirty MINODEs
int quit(char* pathname)
{
    int i = 0;
    char str[256];
    for(i = 0; i < 10; i++)
    {
        if(running->fd[i] != NULL)
        {
            snprintf(str, 10, "%d", i);
            close_file(str);
        }
    }
    for(i = 0; i < 100; i++)
    {
        if(minode[i].refCount > 0)
        {
            if(minode[i].dirty != 0)
            {
                minode[i].refCount = 1;
                iput(dev, &minode[i]);
            }
        }
    }
    printf("Exiting Program\n");
    exit(0);
}

int makedir(char *pathname)
{
    char *pathHolder = malloc(strlen(pathname) + 1), *dirHolder = malloc(strlen(pathname)), *baseHolder = malloc(strlen(pathname));
    int pino = 0; 
    MINODE *pmip = NULL;

    // Copy strings around
    strcpy(pathHolder, pathname);
    strcpy(dirHolder, dirname(pathname));
    strcpy(baseHolder, basename(pathname));

    pino = getino(dev, dirHolder);
    pmip = iget(dev, pino);

    if(S_ISDIR (pmip->INODE.i_mode))
    {
        if(search(dev, baseHolder, pmip) == 0)
        {
            int check = my_mkdir(pmip, baseHolder);
            
            if (check == 0)
            {
                printf("Error: Directory creation failed\n");
            }
        }
        else
        {
            printf("Error: Directory already exists\n");
            iput(pmip->dev, pmip);
            return -1;
        }
    } 
    else
    {
        printf("Error: Not a directory\n");
        iput(dev, pmip);
        return -1;
    }

    free(pathHolder);
    free (dirHolder);
    free(baseHolder);
}

int my_mkdir(MINODE *pip, char* basename)
{
    int inumber, bnumber, idealLen, needLen, newRec, i, j;
    MINODE *mip;
    char *cp;
    DIR *dpPrev;
    char buf[BLKSIZE];
    char buf2[BLKSIZE];
    int blk[256];

    inumber = ialloc(pip->dev);
    bnumber = balloc(pip->dev);
     
    mip = iget(pip->dev, inumber);

    //Write contents into inode for Directory entry
    mip->INODE.i_mode = 0x41ED;
    mip->INODE.i_uid = running->uid;
    mip->INODE.i_gid = running->gid;
    mip->INODE.i_size = BLKSIZE;
    mip->INODE.i_links_count = 2;
    mip->INODE.i_atime = mip->INODE.i_ctime = mip->INODE.i_mtime = time(0L);
    mip->INODE.i_blocks = 2;
    mip->dirty = 1;
    for(i = 0; i <15; i++) { mip->INODE.i_block[i] = 0; }
    mip->INODE.i_block[0] = bnumber;

    iput(mip->dev, mip);

    //Write . and .. entries
    dp = (DIR*)buf;
    dp->inode = inumber;
    strncpy(dp->name, ".", 1);
    dp->name_len = 1;
    dp->rec_len = 12;
    //printf("Wrote \".\" entry\n"); //FOR TESTING

    cp = buf + 12;
    dp = (DIR*)cp;
    dp->inode = pip->ino;
    dp->name_len = 2;
    strncpy(dp->name, "..", 2);
    dp->rec_len = BLKSIZE - 12;

    put_block(pip->dev, bnumber, buf);

    // Put name into parents directory
    memset(buf, 0, BLKSIZE);
    needLen = 4*((8+strlen(basename)+3)/4);
    bnumber = findLastBlock(pip);
    //Check if room in last block in parents directory
    get_block(pip->dev, bnumber, buf);
    
    cp = buf;
    dp = (DIR*)cp;
   
    while((dp->rec_len + cp) < buf+BLKSIZE)
    {
        cp += dp->rec_len;
        dp = (DIR*)cp;
    }
   
    idealLen = 4*((8+dp->name_len+3)/4);
   
    //There is room in this block
    if(dp->rec_len - idealLen >= needLen) 
    {
        //printf("There is room in this block\n"); //FOR TESTING
        newRec = dp->rec_len - idealLen;
        dp->rec_len = idealLen;
        cp += dp->rec_len;
        dp = (DIR*)cp;
        dp->inode = inumber;
        dp->name_len = strlen(basename);
        strncpy(dp->name, basename, dp->name_len);
        dp->rec_len = newRec;
    }
    else // Allocate new data block
    {
        bnumber = balloc(pip->dev);
        dp = (DIR*)buf;
        dp->inode = inumber;
        dp->name_len = strlen(basename);
        strncpy(dp->name, basename, dp->name_len);
        dp->rec_len = BLKSIZE;
        addLastBlock(pip, bnumber);
    }

    put_block(pip->dev, bnumber, buf);
    pip->dirty = 1;
    pip->INODE.i_links_count++;
    memset(buf, 0, BLKSIZE);
    //printf("Finding parent's ino\n"); //FOR TESTING
    searchByIno(pip->dev, pip->ino, &running->cwd->INODE, buf); 
    touch(buf);
    return 1;
} 

int tst_bit(char *buf, int bit)
{
    return buf[bit/8] << (1 << (bit % 8));
}

int set_bit(char *buf, int bit)
{
    buf[bit/8] |= (1 << (bit % 8));
}

//clears bit in buf
int clr_bit(char *buf, int bit)
{
    buf[bit/8] &= -(1 << (bit % 8));
}

int decFreeInodes(int dev)
{
    char buf[BLKSIZE];
    // dec free inodes count in SUPER and GD
    get_block(dev, 1, buf);
    sp = (SUPER *)buf;
    sp->s_free_inodes_count--;
    put_block(dev, 1, buf);
    get_block(dev, 2, buf);
    gp = (GD *)buf;
    gp->bg_free_inodes_count--;
    put_block(dev, 2, buf);
}

int incFreeInodes(int dev)
{
    char buf[BLKSIZE];
    //increment free inode count in super
    get_block(dev, 1, buf); //blk 1 is the super block
    sp = (SUPER *) buf;
    sp -> s_free_inodes_count++;
    put_block(dev, 1, buf);

    // increment free inode count in GD
    get_block(dev, 2 , buf); //2 is the GD block
    gp = (GD *) buf;
    gp -> bg_free_inodes_count++;
    put_block(dev, 2 , buf);
}

//allocates an inode
int ialloc(int dev)
{
    int i;
    char buf[BLKSIZE];

    get_block(dev, imap, buf);

    for (i = 0; i < ninodes; i++)
    {
        if(tst_bit(buf, i) == 0)
        {
            set_bit(buf, i);
            put_block(dev, imap, buf);
            // update free inodes in super and gd
            decFreeInodes(dev);
            return i+1;
        }
    }

    return 0; // no more free inodes
}

//deallocates an inode
int idalloc(int dev, int ino)
{
    int i;
    char buf[BLKSIZE];

    //get inode bitmap block
    get_block(dev, IBITMAP, buf);
    clr_bit(buf, ino-1);
    //write buf back
    put_block(dev, IBITMAP, buf);
    //update free inode count in SUPER and GD
    incFreeInodes(dev);
}

int decFreeBlocks(int dev1)
{
    char buf[BLKSIZE];
    //Pull down super to decrement free block count
    get_super(dev, buf);
    sp = (SUPER*)buf;
    sp->s_free_blocks_count -= 1; 
    put_block(dev1, SUPERBLOCK, buf); // put back updated super
    
    //pull down GD to decrement block count in GD
    get_gd(dev1, buf);
    gp = (GD*)buf;
    gp->bg_free_blocks_count -=1;
    put_block(dev1, GDBLOCK, buf); // put back updated GD
    return 1;
}

int incFreeBlocks(int dev)
{
    char buf[BLKSIZE];
    //increment free block count in super
    get_block(dev, 1, buf); //blk 1 is the super block
    sp = (SUPER *) buf;
    sp -> s_free_blocks_count++;
    put_block(dev, 1, buf);

    // increment free block count in GD
    get_block(dev, 2 , buf); //2 is the GD block
    gp = (GD *) buf;
    gp -> bg_free_blocks_count++;
    put_block(dev, 2 , buf);
    
}

int balloc(int dev)
{
    int i;
    char buf[BLKSIZE];            // BLKSIZE=block size in bytes

    get_block(dev, bmap, buf);   
    
    for (i=0; i < BLKSIZE; i++){  
        if (tst_bit(buf, i)==0){    
        set_bit(buf, i);          
        put_block(dev, bmap, buf);   // write bmap block back to disk

        // update free blocks count in SUPER and GD on dev
        decFreeBlocks(dev);       // assume you write this function  
        memset(buf, 0, BLKSIZE);
        put_block(dev, i+1, buf);
        return (i+1);
        }
    }

    return 0;                     // no more FREE blocks
}

int bdalloc(int dev, int ino)
{
    int i;
    char buf[BLKSIZE];

    // get block bitmap block
    get_block(dev, BBITMAP, buf);
    clr_bit(buf, ino-1);
    //write back the buffer
    put_block(dev, BBITMAP, buf);
    //update free block count in SUPER and GD
    incFreeBlocks(dev);
}

int findLastBlock(MINODE *pip)
{
    int buf[256];
    int buf2[256];
    int bnumber, i, j;
    
     //Find last used block in parents directory
    if(pip->INODE.i_block[0] == 0) {return 0;}
    for(i = 0; i < 12; i++) //Check direct blocks
    {
        if(pip->INODE.i_block[i] == 0) 
        {
            return (pip->INODE.i_block[i-1]);
        }
    }
    if(pip->INODE.i_block[12] == 0) {return pip->INODE.i_block[i-1];}
   
    get_block(dev, pip->INODE.i_block[12], (char*)buf);
   
    for(i = 0; i < 256; i++)
    {
            if(buf[i] == 0) {return buf[i-1];}
    }
   
    if(pip->INODE.i_block[13] == 0) {return buf[i-1];}
    //Print dirs in double indirect blocks
    memset(buf, 0, 256);
    get_block(pip->dev, pip->INODE.i_block[13], (char*)buf);
   
    for(i = 0; i < 256; i++)
    {
            if(buf[i] == 0) {return buf2[j-1];}
            if(buf[i])
            {
                    get_block(pip->dev, buf[i], (char*)buf2);
                    for(j = 0; j < 256; j++)
                    {
                        if(buf2[j] == 0) {return buf2[j-1];}
                    }
            }
    }
}

int addLastBlock(MINODE *pip, int bnumber)
{
    int buf[256];
    int buf2[256];
    int i, j, newBlk, newBlk2;
     //Find last used block in parents directory
    for(i = 0; i < 12; i++) //Check direct blocks
    {
        if(pip->INODE.i_block[i] == 0) {pip->INODE.i_block[i] = bnumber; return 1;}
    }
    
    if(pip->INODE.i_block[12] == 0) //Have to make indirect block
    {
        newBlk = balloc(pip->dev);
        pip->INODE.i_block[12] = newBlk;
        memset(buf, 0, 256);
        get_block(pip->dev, newBlk, (char*)buf);
        buf[0] = bnumber;
        put_block(pip->dev, newBlk, (char*)buf);
        return 1;
    }
    
    memset(buf, 0, 256);
    get_block(pip->dev, pip->INODE.i_block[12], (char*)buf);
    
    for(i = 0; i < 256; i++)
    {
            if(buf[i] == 0) {buf[i] = bnumber; return 1;}
    }
    
    if(pip->INODE.i_block[13] == 0) //Make double indirect block
    {   
        newBlk = balloc(pip->dev);
        pip->INODE.i_block[13] = newBlk;
        memset(buf, 0, 256);
        get_block(pip->dev, newBlk, (char*)buf);
        newBlk2 = balloc(pip->dev);
        buf[0] = newBlk2;
        put_block(pip->dev, newBlk, (char*)buf);
        memset(buf2, 0, 256);
        get_block(pip->dev, newBlk2, (char*)buf2);
        buf2[0] = bnumber;
        put_block(pip->dev, newBlk2, (char*)buf2);
        return 1;
    }
    
    memset(buf, 0, 256);
    get_block(pip->dev, pip->INODE.i_block[13], (char*)buf);
    
    for(i = 0; i < 256; i++)
    {
            if(buf[i] == 0) 
            {
                newBlk2 = balloc(pip->dev);
                buf[i] = newBlk2;
                put_block(pip->dev, pip->INODE.i_block[13], (char*)buf);
                memset(buf2, 0, 256);
                get_block(pip->dev, newBlk2, (char*)buf2);
                buf2[0] = bnumber;
                put_block(pip->dev, newBlk2, (char*)buf2);
                return 1;
            }
            memset(buf2, 0, 256);
            get_block(pip->dev, buf[i], (char*)buf2);
            for(j = 0; j < 256; j++)
            {
                if(buf2[j] == 0) {buf2[j] = bnumber; return 1;}
            }
    }
    
    printf("Error: could not add new block\n");
    return -1;
}

int touch (char* name)
{
    char buf[1024];
    int ino;
    MINODE *mip;
    
    ino = getino(dev, name);
    if(ino <= 0)
    {
        creat_file(name);
        return 1;
    }
    mip = iget(dev, ino);
    mip->INODE.i_atime = mip->INODE.i_mtime = mip->INODE.i_ctime = time(0L);
    mip->dirty = 1;
    iput(mip->dev, mip);
    return 1;
}

int rmdir(char *pathname)
{
    int ino, pino;
    MINODE* mip, *pmip;
    //   parent        child          path
    char dirName[256], baseName[256], origPathName[512];
    

    if(!pathname || !pathname[0])
    {
        printf("Error: No Pathname\n");
        return -1;
    }
    printf("%s\n", pathname);
    ino = getino(dev, pathname);
    mip = iget(dev, ino);
    //check if file is  a dir
    if(!S_ISDIR(mip->INODE.i_mode))
    {
        printf(" Error: Not a DIR\n");
        iput(mip->dev, mip);
        return -1;
    }
    // check if dir is in use
    if(mip ->refCount > 1)
    {
        printf("Error: Directory in use \n");
        return -1;
    }
    //check if Dir is empty
    if(mip ->INODE.i_links_count > 2)
    {
        printf("Error: Dir not empty\n");
        iput(mip->dev, mip);
        return -1;
    }

    //deallocate blocks in mip
    for(int i = 0; i < 12; i++)
    {
        if(mip->INODE.i_block[i] != 0)
        {
            bdalloc(mip->dev, mip->INODE.i_block[i]);
        }
    }
    //dealloc inode of mip
    idalloc(mip->dev, mip->ino);

    strcpy(origPathName, pathname);
    strcpy(dirName, dirname(origPathName));
    strcpy(baseName, basename(origPathName));

    //grab parent ino and MINODE
    pino = getino(mip->dev, dirName);
    pmip = iget(mip->dev, ino);
    iput(mip->dev, mip);

    //remove the dir at base
    rm_child(pmip, baseName);
    //decrement the number of links to parent
    pmip->INODE.i_links_count--;
    touch(dirName);
    //mark parent as dirty
    pmip->dirty = 1;
    iput(pmip->dev, pmip);
    return 1;
}

int rm_child(MINODE *pip, char *child)
{
    int i, size, found = 0;
    char *cp, *cp2;
    DIR *dp, *dp2, *dpPrev;
    char buf[BLKSIZE], buf2[BLKSIZE], temp[256];
    
    memset(buf2, 0, BLKSIZE);

    for(i = 0; i < 12; i++)
    {
        if(pip->INODE.i_block[i] == 0) { return 0; }

        get_block(pip->dev, pip->INODE.i_block[i], buf);
        dp = (DIR *)buf;
        dp2 = (DIR *)buf;
        dpPrev = (DIR *)buf;
        cp = buf;
        cp2 = buf;

        while(cp < buf+BLKSIZE && !found)
        {
            memset(temp, 0, 256);
            strncpy(temp, dp->name, dp->name_len);
            if(strcmp(child, temp) == 0)
            {
                //if child is only entry in block
                if(cp == buf && dp->rec_len == BLKSIZE)
                {
                    bdalloc(pip->dev, pip->INODE.i_block[i]);
                    pip->INODE.i_block[i] = 0;
                    pip->INODE.i_blocks--;
                    found = 1;
                }
                //else delete child and move entries over left
                else
                {
                    while((dp2->rec_len + cp2) < buf+BLKSIZE)
                    {
                        dpPrev = dp2;
                        cp2 += dp2->rec_len;
                        dp2 = (DIR*)cp2;
                    }
                    if(dp2 == dp) //Child is last entry
                    {
                        //printf("Child is last entry\n"); //FOR TESTING
                        dpPrev->rec_len += dp->rec_len;
                        found = 1;
                    }
                    else
                    {
                        //printf("Child is not the last entry\n"); //FOR TESTING
                        size = ((buf + BLKSIZE) - (cp + dp->rec_len));
                        //printf("Size to end = %d\n", size);
                        dp2->rec_len += dp->rec_len;
                       // printf("dp2 len = %d\n", dp2->rec_len);
                        memmove(cp, (cp + dp->rec_len), size);
                        dpPrev = (DIR*)cp;
                        memset(temp, 0, 256);
                        strncpy(temp, dpPrev->name, dpPrev->name_len);
                        //printf("new dp name = %s\n", temp);
                        found = 1;
                    }
                }
            }
            cp += dp->rec_len;
            dp = (DIR*)cp;
        }
        if(found)
        {
            put_block(pip->dev, pip->INODE.i_block[i], buf);
            return 1;
        }
    }
    printf("Error: Child Not Found\n");
    return -1;
}

int creat_file(char* pathname)
{
    int dev1, ino, r;
    char parent[256];
    char child[256];
    MINODE *mip;
    memset(parent, 0, 256);
    memset(child, 0, 256);

    if(pathname[0] == '/') { dev1 = root->dev;}
    else { dev1 = running->cwd->dev; }
    
    strcpy(parent, dirname(pathname));
    strcpy(child, basename(pathname));

    ino = getino(dev1, parent);
    if(ino <= 0)
    {
        printf("Error: invalid pathname\n");
        return -1;
    }

    mip = iget(dev1, ino);
    if(!S_ISDIR(mip->INODE.i_mode))
    {
        printf("Error: not a directory\n");
        iput(dev1, mip);
        return -1;
    }
    
    ino = search(dev1, child, &(mip->INODE));
    if(ino > 0)
    {
        printf("Error: file already exists\n");
        iput(mip->dev, mip);
        return -1;
    }
    
    r = my_creat(mip, child);
    iput(mip->dev, mip);
    return r;   
}

int my_creat(MINODE *pip, char child[256])
{
    int inumber, bnumber, idealLen, needLen, newRec, i, j;
    MINODE *mip;
    char *cp;
    DIR *dpPrev;
    char buf[BLKSIZE];
    char buf2[BLKSIZE];
    int blk[256];

    inumber = ialloc(pip->dev);
    mip = iget(pip->dev, inumber);

    //Write contents into inode for Directory entry
    mip->INODE.i_mode = 0x81A4;
    mip->INODE.i_uid = running->uid;
    mip->INODE.i_gid = running->gid;
    mip->INODE.i_size = 0;
    mip->INODE.i_links_count = 1;
    mip->INODE.i_atime = mip->INODE.i_ctime = mip->INODE.i_mtime = time(0L);
    mip->INODE.i_blocks = 0;
    mip->dirty = 1;
    for(i = 0; i <15; i++)
    {
        mip->INODE.i_block[i] = 0;
    }
    
    iput(mip->dev, mip);
    // Put name into parents directory
    memset(buf, 0, BLKSIZE);
    needLen = 4*((8+strlen(child)+3)/4);
    bnumber = findLastBlock(pip);
    //Check if rooom in last block in parents directory
    get_block(pip->dev, bnumber, buf);
    dp = (DIR*)buf;
    cp = buf;

    while((dp->rec_len + cp) < buf+BLKSIZE)
    {
        cp += dp->rec_len;
        dp = (DIR*)cp;
    }
    
    idealLen = 4*((8+dp->name_len+3)/4);
    if(dp->rec_len - idealLen >= needLen) //There is room in this block
    {
        newRec = dp->rec_len - idealLen;
        dp->rec_len = idealLen;
        cp += dp->rec_len;
        dp = (DIR*)cp;
        dp->inode = inumber;
        dp->name_len = strlen(child);
        strncpy(dp->name, child, dp->name_len);
        dp->rec_len = newRec;
    }
    else // Allocate new data block
    {
        bnumber = balloc(pip->dev);
        dp = (DIR*)buf;
        dp->inode = inumber;
        dp->name_len = strlen(child);
        strncpy(dp->name, child, dp->name_len);
        dp->rec_len = BLKSIZE;
        addLastBlock(pip, bnumber);
    }
    //printf("Putting parent block back\n"); //FOR TESTING

    put_block(pip->dev, bnumber, buf);
    pip->dirty = 1;
    memset(buf, 0, BLKSIZE);
    searchByIno(pip->dev, pip->ino, &running->cwd->INODE, buf); 
    touch(buf);
    
    return 1;
}

int rm(MINODE *mip)
{
    int i, j;
    int buf[256], buf2[256];
    
    if(!S_ISLNK(mip->INODE.i_mode))
    {
        for(i = 0; i < 12; i++)
        {
            if(mip->INODE.i_block[i] != 0)
            {
                bdalloc(mip->dev, mip->INODE.i_block[i]);
            }
        }
        if(mip->INODE.i_block[12] != 0)
        {
            memset(buf, 0, 256);
            get_block(mip->dev, mip->INODE.i_block[12], (char*)buf);
            for(i = 0; i < 256; i++)
            {
                if(buf[i] != 0) {bdalloc(mip->dev, buf[i]);}
            }
            bdalloc(mip->dev, mip->INODE.i_block[12]);
        }
        if(mip->INODE.i_block[13] != 0)
        {
            memset(buf, 0, 256);
            get_block(mip->dev, mip->INODE.i_block[13], (char*)buf);
            for(i = 0; i < 256; i++)
            {
                if(buf[i] != 0)
                {
                    memset(buf2, 0, 256);
                    get_block(mip->dev, buf[i], (char*)buf2);
                    for(j = 0; j < 256; j++)
                    {
                        if(buf2[j] != 0) {bdalloc(mip->dev, buf2[j]);}
                    }
                    bdalloc(mip->dev, buf[i]);
                }
            }
            bdalloc(mip->dev, mip->INODE.i_block[13]);
        }
    }    

    idalloc(mip->dev, mip->ino);
    return 1;
}

int split_paths(char *original, char *path1, char *path2)
{
    char *temp;
    //printf("Original = %s\n", original);
    temp = strtok(original, " ");
    strcpy(path1, temp);
    //printf("Path1 = %s\n", path1);
    temp = strtok(NULL, " ");
    if(temp == NULL)
    {
        printf("ERROR: NO SECOND PATH GIVEN\n");
        return -1;
    }
    strcpy(path2, temp);
    return 1;
}

int link(char* pathname)
{
    char oldFile[256], newFile[256], parent[256], child[256], buf[BLKSIZE];
    int ino, ino2, bnumber, needLen, idealLen, newRec;
    MINODE *mip, *mip2;
    char *cp;
    DIR *dp;

    if(0 >= split_paths(pathname, oldFile, newFile)) { return -1; }
    
    ino = getino(dev, oldFile);
    
    if(ino <= 0) { return -1; }
    
    mip = iget(dev, ino);

    if(!S_ISREG(mip->INODE.i_mode))
    {
        printf("ERROR: path is not a regular file\n");
        iput(mip->dev, mip);
        return -1;
    }
    
    strcpy(parent, dirname(newFile));
    strcpy(child, basename(newFile));

    if(0 >= (ino2 = getino(mip->dev, parent)))
    {
        iput(mip->dev, mip);
        return -1;
    }

    mip2 = iget(mip->dev, ino2);
    
    if(!S_ISDIR(mip2->INODE.i_mode))
    {
        printf("ERROR: not a directory\n");
        iput(mip->dev, mip);
        iput(mip2->dev, mip2);
        return -1;
    }
    //printf("Child = %s\n", child);
    
    ino2 = search(mip2->dev, child, &(mip2->INODE));
    
    if(ino2 > 0)
    {
        printf("ERROR: File Already Exists\n");
        iput(mip->dev, mip);
        iput(mip2->dev, mip2);
        return -1;
    }

    //Put name in parents block
    memset(buf, 0, BLKSIZE);
    needLen = 4*((8+strlen(child)+3)/4);
    bnumber = findLastBlock(mip2);
    
    //Check if room in last block in parents directory
    get_block(mip2->dev, bnumber, buf);
    dp = (DIR*)buf;
    cp = buf;

    while((dp->rec_len + cp) < buf+BLKSIZE)
    {
        cp += dp->rec_len;
        dp = (DIR*)cp;
    }
    
    idealLen = 4*((8+dp->name_len+3)/4);
    
    if(dp->rec_len - idealLen >= needLen) //There is room in this block
    {
        newRec = dp->rec_len - idealLen;
        dp->rec_len = idealLen;
        cp += dp->rec_len;
        dp = (DIR*)cp;
        dp->inode = ino;
        dp->name_len = strlen(child);
        strncpy(dp->name, child, dp->name_len);
        dp->rec_len = newRec;
    }
    else // Allocate new data block
    {
        bnumber = balloc(mip2->dev);
        dp = (DIR*)buf;
        dp->inode = ino;
        dp->name_len = strlen(child);
        strncpy(dp->name, child, dp->name_len);
        dp->rec_len = BLKSIZE;
        addLastBlock(mip2, bnumber);
    }

    put_block(mip2->dev, bnumber, buf);
    mip->dirty = 1;
    mip->INODE.i_links_count++;
    memset(buf, 0, BLKSIZE);
    searchByIno(mip2->dev, mip2->ino, &running->cwd->INODE, buf); 

    iput(mip->dev, mip);
    iput(mip2->dev, mip2);
    return 1;
}

int unlink(char *pathname)
{
    char oldFile[256], parent[256], child[256], buf[BLKSIZE];
    int ino, ino2, bnumber, needLen, idealLen, newRec;
    MINODE *mip, *mip2;
    char *cp;
    DIR *dp;
    
    ino = getino(dev, pathname);
    
    if(ino <= 0)
    {
        printf("ERROR: FILE DOES NOT EXIST\n");
        return -1;
    }
    
    mip = iget(dev, ino);
    
    if(!S_ISREG(mip->INODE.i_mode) && !S_ISLNK(mip->INODE.i_mode))
    {
        printf("Error: not a file\n");
        iput(mip->dev, mip);
        return -1;
    }
    
    mip->INODE.i_links_count--;
    
    if(mip->INODE.i_links_count <= 0)
    {
        rm(mip);
    }

    strcpy(parent, dirname(pathname));
    strcpy(child, basename(pathname));
    
    ino2 = getino(mip->dev, parent);
    
    if(ino2 <= 0) 
        return -1;

    mip2 = iget(mip->dev, ino2);
    
    rm_child(mip2, child);

    iput(mip->dev, mip);
    iput(mip->dev, mip2);
}

int symlink(char *pathname)
{
    char oldname[256], newname[256];
    int ino;
    MINODE *mip;

    if(split_paths(pathname, oldname, newname) <= 0) {return -1;}
    if(0 >= (ino = getino(dev, oldname)))
    {
        printf("Error: File does not exist\n");
        return -1;
    }
    
    creat_file(newname);
    if(0 >= (ino = getino(dev, newname)))
    {
        printf("Error: could not create file\n");
        return -1;
    }

    mip = iget(dev, ino);
    mip->INODE.i_mode &= ~0770000;
    mip->INODE.i_mode |= 0120000;
    mip->dirty = 1;

    strcpy((char*)mip->INODE.i_block, oldname);
    iput(mip->dev, mip); 
}

int my_chmod(char* pathname)
{
    char buf[1024];
    char nMode[256];
    char path[256];
    int ino, newMode, i;
    MINODE* mip;

    if(split_paths(pathname, nMode, path) <= 0) { return -1; }
     // converts nmode string to int
    newMode = strtoul(nMode, NULL, 8); 
    ino = getino(dev, path);
    
    if(ino <= 0)
    {
        printf("ERROR: INVALID PATHNAME\n");
        return -1;
    }
    
    mip = iget(dev, ino);
    i = ~0x1FF; // 0b1000000000
    mip->INODE.i_mode &= i; // clears out original mode of any non 1s
    mip->INODE.i_mode |= newMode; // combines old permissions with new permissions
    mip->dirty = 1;
    iput(dev, mip);
    
    return 1;
}
