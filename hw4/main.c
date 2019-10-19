#include "functions.h"

int main(int argc, char *argv[])
{
    if (argc < 3)
    {
        printf("Error: Not Enough Arguments\n");
        exit(0);
    }

    return myrcp(argv[1], argv[2]);
}