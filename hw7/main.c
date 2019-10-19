
#include "func.h"

//char *cmd[5] = {"ls", "cd", "pwd","quit", NULL};

int main(int argc, char *argv[])
{
    if (argc > 2)
    {
        printf("Error: Use is 'level1Sys [diskName]'\n");
        exit(0);
    }
    //printf("%s %s %s\n", argv[0], argv[1], argv[2]);
    int (*fptr[ ])(char *)= {(int (*)()) ls, cd, pwd, quit, makedir, rmdir, creat_file, rm, link, unlink, symlink, my_chmod};
    int index;
    char line[256];
    //int (*fptr[ ])(char *)= {(int (*)()) ls, cd, pwd, quit};

    
    
    init();
    // printf("Seg fault in Mount_root?\n");
    mount_root(argv[1]);
    //printf("Seg fault after Mount_root?\n");

    printf("***************************\n ls, cd, pwd ,quit, mkdir, rmdir,\n creat, rm, link, unlink, symlink , chmod\n ************************\n");
    char comm[10], path[256];
    while(1)
    {
        memset(comm, 0, 10);
        memset(path, 0, 256);
        printf("Enter command: cmd [pathname] \n");
        fgets(line, 128,stdin);
        line[strlen(line)-1] = 0;
        sscanf(line, "%s %s", comm, path);
        index = findCmd(comm);

        if(index >= 0)
        {
            int r = fptr[index](path);
        }

    }
}