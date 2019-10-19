#include <stdio.h>
#include <stdlib.h>

typedef unsigned int u32;

char *ctable = "0123456789ABCDEF";
int  BASE = 10; 
int BASE16 = 16;
int BASE8 = 8;

int *FP;

int main(int argc, char *argv[ ], char *env[ ])
{
  int a,b,c;
  printf("enter main\n");
  
  printf("&argc = %x, argv = %x, env = %x\n", &argc, argv, env);
  printf("&a = %8x, &b = %8x, &c = %8x\n", &a, &b, &c);

//(1). Write C code to print values of argc and argv[] entries
  myprintf("argc = %d\n", argc);
  for(int i = 0; i < argc; i++)
  {
    myprintf("argv[%d] = %s\n", i, argv[i]);
  }
  
  myprintf("cha = %c \t string = %s \n dec = %d , hex = %x, oct = %o neg = %d\n", 'A', "this is a test", 100, 100, 100, -100);

  a=1; b=2; c=3;
  A(a,b);
  printf("exit main\n");
}

int A(int x, int y)
{
  int d,e,f;
  printf("enter A\n");
  // PRINT ADDRESS OF d, e, f
  printf("&d = %8x, &e = %8x, &f = %8x\n", &d, &e, &f);

  d=4; e=5; f=6;
  B(d,e);
  printf("exit A\n");
}

int B(int x, int y)
{
  int g,h,i;
  printf("enter B\n");
  // PRINT ADDRESS OF g,h,i
  printf("&g = %8x, &h = %8x, &i = %8x\n", &g, &h, &i);

  g=7; h=8; i=9;
  C(g,h);
  printf("exit B\n");
}

int C(int x, int y)
{
  int u, v, w, i, *p;

  printf("enter C\n");
  // PRINT ADDRESS OF u,v,w,i,p;
  printf("&u = %8x, &v = %8x, &w = %8x, &i = %8x, &p = %8x\n", &u, &v, &w, &i, &p);
  u=10; v=11; w=12; i=13;

  FP = (int *)getebp();

//(2). Write C code to print the stack frame link list.
    while (FP != NULL)
        {
            printf("%8x -> ", FP);
            FP = (int*)*FP;
        }
    printf("\n");

 p = (int *)&p;
 p = p - 3;
/*(3). Print the stack contents from p to the frame of main()
     YOU MAY JUST PRINT 128 entries of the stack contents.*/
  for(int i = 0; i < 100; i++)
  {
    printf("%x\t%x\n", p, *p);
    p++;
  }

/*(4). On a hard copy of the print out, identify the stack contents
     as LOCAL VARIABLES, PARAMETERS, stack frame pointer of each function.*/
}

///////////////////////////////////////////////Begin Part 2////////////////////////////////////

int rpu(u32 x)
{  
    char c;
    if (x){
       c = ctable[x % BASE];
       rpu(x / BASE);
       putchar(c);
    }
}

int rpu8(u32 x)
{
    char c;
    if (x){
       c = ctable[x % BASE8];
       rpu8(x / BASE8);
       putchar(c);
    }
}

int rpu16(u32 x)
{
    char c;
    if (x){
       c = ctable[x % BASE16];
       rpu16(x / BASE16);
       putchar(c);
    }
}

int printu(u32 x)
{
   (x==0)? putchar('0') : rpu(x);
   putchar(' ');
}

int printd(int x)
{
    if (x < 0)
    {
        x *= -1;
        putchar('-');
        printu(x);
    }
    else
    {
        printu(x);
    }
}

int printo(u32 x)
{
    putchar('0');
    (x==0) ? putchar('0') :rpu8(x);
    putchar(' ');
}

int printx (u32 x)
{
    putchar('0');
    putchar('x');
    
    (x==0) ? putchar('0') :rpu16(x);
    putchar(' ');
}

int prints(char *string)
{
    while(*string)
    {
        putchar(*string);
        string++;
    }
}

void myprintf(char* fmt, ...)
{
    char *cp = fmt;
    int *ip = (int*) &fmt + 1;

    while(*cp)
    {
        if(*cp != '%')
        {
            putchar(*cp);
        }else{
            switch(*(cp+1))
            {
                case 'c':
                    putchar((char)*ip++);
                    break;
                case 's':
                    prints((char *)*ip++);
                    break;
                case 'd':
                    printd((int) *ip++);
                    break;
                case 'u':
                    printu((u32) *ip++);
                    break;
                case 'o':
                    printo((u32) *ip++);
                    break;
                case 'x':
                    printx((u32) *ip++);
                    break;
                default:
                    putchar(*fmt);
                    break;
            }
            cp++;
        }
        cp++;
    }
}