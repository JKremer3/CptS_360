
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
    int (*fptrT1[ ])(char *)= {(int (*)()) ls, cd, pwd, quit, makedir, rmdir, creat_file, rm, link, unlink, symlink, my_chmod, touch};
    int (*fptrT2[ ])(int, char**) = {my_read, my_write, my_close, my_open, my_cat, my_cp};
    int index;
    char line[256];
    //int (*fptr[ ])(char *)= {(int (*)()) ls, cd, pwd, quit};

    
    
    init();
    // printf("Seg fault in Mount_root?\n");
    mount_root(argv[1]);
    //printf("Seg fault after Mount_root?\n");
    printf(" ******************************************\n ls, cd, pwd ,quit, mkdir, rmdir, touch,\n creat, rm, link, unlink, symlink , chmod\n touch, read, write, close, open, cat, cp\n******************************************\n");
    

    char comm[10], path[256], param[256];
    while(1)
    {
        memset(comm, 0, 10);
        memset(path, 0, 256);
        memset(param, 0, 256);
        printf("Enter command: cmd [pathname] \n");
        fgets(line, 128,stdin);
        line[strlen(line)-1] = 0;
        sscanf(line, "%s %s %s", comm, path, param);
        
        index = findCmd(comm);

        //printf("%s\n", param);
        if(index >= 0 && index <= 13)
        {
            if(param[0] != 0)
            {
                strcat(path, " ");
                strcat(path, param);
            }
            int r = fptrT1[index](path);
        }
        if (index > 13 && index <= 20)  
        {   

            char *argv[2] = {path, param};
            int r= fptrT2[index - 14](2, argv);
        }
        
        //printf("%s\n", path);

        if(index >= 0)
        {
           // printf("%s\n", path);
            int r = fptrT1[index](path);
        }

    }
}