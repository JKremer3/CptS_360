#include "Lab6.h"

//////////// gd.c ////////////
int get_gd(int dev, char *buf)
{
    int GDBLOCK = 2;
    get_block(dev, GDBLOCK, buf);
}

int get_block(int dev, int blk, char *buf)
{
    if (-1 == lseek(dev, (long)(blk*BLKSIZE), 0))
    {
        printf("%s\n", strerror(errno));
        assert(0);
    }
    read(dev, buf, BLKSIZE);
}

///////////end gd.c//////////


////////// bmap.c ///////////
int bmap()
{
  char buf[BLKSIZE];
  int  imap, ninodes;
  int  i;

  // read gd block
  get_block(fd, 2, buf);
  sp = (SUPER *)buf;

  ninodes = sp->s_inodes_count;
  printf("ninodes = %d\n", ninodes);

  // read bmap black
  get_block(fd, 3, buf);
  gp = (GD *)buf;

  imap = gp->bg_inode_bitmap;
  printf("bmap = %d\n", imap);

  // read inode_bitmap block
  get_block(fd, imap, buf);

  for (i=0; i < ninodes; i++){
    (tst_bit(buf, i)) ?	putchar('1') : putchar('0');
    if (i && (i % 8)==0)
       printf(" ");
  }
  printf("\n");
}

///////// end bmap.c/////////

///////// dir.c /////////////


///////// end dir.c//////////

///////// balloc.c///////////
int balloc(int dev)
{

}

///////// end balloc.c///////