#include "functions.h"

int myrcp(char *f1, char *f2)
{
    struct stat *fileStats1, *fileStats2;
    char dest_change[PATH_MAX], src_holder[PATH_MAX], *baseHolder;
    int check1 = 0, check2 = 0;

    fileStats1 = (struct stat*) malloc(sizeof(struct stat));
    fileStats2 = (struct stat*) malloc(sizeof(struct stat));

    check1 = lstat(f1, fileStats1);
    check2 = lstat(f2, fileStats2);

    
    if(check1 == -1) //if f1 does not exist, reject
    {
        printf("Error: %s does not exist\n", f1);
        free(fileStats1);
        free(fileStats2);
        return -1; 
    }
    else if(!(S_ISREG(fileStats1->st_mode)) && !(S_ISLNK(fileStats1->st_mode)) && !(S_ISDIR(fileStats1->st_mode))) //if f1 is not a REG, LNK, or DIR file; reject
    {
        printf("Error: %s is not a LNK, REG, DIR file\n", f1);
        free(fileStats1);
        free(fileStats2);
        return -1; 
    }

    //Exits program if f2 exists and is a file other than LNK, REG, DIR. It is acceptable for f2 to not exist
    if(!(check2 == -1))
    {
        if(!(S_ISREG(fileStats2->st_mode)) && !(S_ISLNK(fileStats2->st_mode)) && !(S_ISDIR(fileStats2->st_mode))) 
        {
            printf("Error: %s is not a LNK, REG, DIR file\n", f2);
            free(fileStats1);
            free(fileStats2);
            return -1; 
        }
    }
    //Dependent on file type, different operations are performed
    if(S_ISREG(fileStats1->st_mode) || S_ISLNK(fileStats1->st_mode))
    {
        if(check2 == -1 || S_ISLNK(fileStats2->st_mode) || S_ISREG(fileStats2->st_mode))
        {
            return cpf2f(f1, f2);
        }
        else if(S_ISDIR(fileStats2->st_mode))
        {
            return cpf2d(f1, f2);
        }
    } 
    else if(S_ISDIR(fileStats1->st_mode)) //If f1 is a DIR file
    {
        if(!(check2 == -1) && !(S_ISDIR(fileStats2->st_mode))) //if f2 exists and is not a DIR
        {
            return -1;
        }
        else if(check2 == -1) // if f2 does not exist
        {
            mkdir(f2, 0777);
        }
        else
        {
            char dest_change[PATH_MAX + 1], src_holder[PATH_MAX + 1], *baseHolder;
            
            realpath(f1, src_holder);
            realpath(f2, dest_change);
            if(isFileSame(f1, f2)) // if files are the same
            {
                printf("Error: Files are the same\n");
                return -1;
            }
            else if(strncmp(src_holder, dest_change, strlen(src_holder)) == 0) // if the two files have the same paths for the length of f1
            {
                printf("Error: %s is a child of %s\n", f2, f1);
                return -1;
            }


            strcpy(src_holder, f1);
            baseHolder = basename(src_holder);
            strcpy(dest_change, f2);
            strcat(dest_change, "/");
            strcat(dest_change, baseHolder);
            mkdir(dest_change, 0777);
            return cpd2d(f1, dest_change);
        }

        return cpd2d(f1, f2);
    }
    
}

int cpf2f(char *f1, char *f2)
{
    int  check1, check2;
    struct stat *fileStats1, *fileStats2;
    
    fileStats1 = (struct stat*) malloc(sizeof(struct stat));
    fileStats2 = (struct stat*) malloc(sizeof(struct stat));

    check1 = lstat(f1, fileStats1);
    check2 = lstat(f2, fileStats2);

    if(isFileSame(f1,f2)) //if f1 and f2 have the same st_dev and st_ino, break
    {
        printf("Error: %s and %s are the same file\n", f1, f2);
        free(fileStats1);
        free(fileStats2);
        return -1;
    }
    else if(S_ISLNK(fileStats1->st_mode)) //if f1 is a LNK file
    {
        if(!(check2 == -1))
        {
            printf("Error: cannot copy LNK file to other file\n");
            free(fileStats1);
            free(fileStats2);
            return -1;
        }
        else
        {
            char pBuf[PATH_MAX];
            char* newPath;
            newPath = realpath(f1, pBuf);
            symlink(newPath, f2);
            printf("New LNK file created at %s\n", f2);
            return 1;
        }
    }
    else //copies file normally if no other conditons are met
    {
        int sourceFile = open(f1, O_RDONLY);
        int destinationFile = open(f2, O_WRONLY|O_CREAT|O_TRUNC, fileStats1->st_mode);
        int status, statusCopy;
        unsigned char buf[4096];

        while (1)
        {
            status = read(sourceFile, buf, 4096);
            if(status == -1)
            {
                printf("Error reading file %s\n", f1);
                exit(1);
        }
        statusCopy = status;

        if (statusCopy == 0)
        {
            break;
        }

        status = write(destinationFile, buf, statusCopy);
        if(status == -1)
        {
            printf("Error writing to file \n");
            exit(1);
            }
        }
        close(sourceFile);
        close(destinationFile);

        free(fileStats1);
        free(fileStats2);

        return 1;
    }
}

int cpf2d(char *f1, char *f2)
{
    DIR *dest_dir = opendir(f2);
    struct dirent *dp;
    char f1Holder[PATH_MAX], *newFile, f2Holder[PATH_MAX];
    int fileExists = 0, dirFileCheck = 0;
    struct stat *dirFileStat = (struct stat*) malloc(sizeof(struct stat));

    strcpy(f1Holder, f1);
    newFile = basename(f1Holder);
    dp = readdir(dest_dir);

    while(dp)
    {
        if(strcmp(dp->d_name, newFile) == 0)
        {
            strcpy(f1Holder, f1);
            strcat(f1Holder, "/");
            strcat(f1Holder, dp->d_name);

            dirFileCheck = lstat(f1Holder, dirFileStat);

            strcpy(f2Holder, f2);
            strcat(f2Holder, "/");
            strcat(f2Holder, dp->d_name);

            if(S_ISDIR(dirFileStat->st_mode))
            {
                free(dirFileStat);
                return cpf2d(f1, f2Holder);
            }
            else
            {
                free(dirFileStat);
                return cpf2f(f1, f2Holder);
            }
        }
        
        dp = readdir(dest_dir);
    }

    strcpy(f2Holder, f2);
    strcat(f2Holder, "/");
    strcat(f2Holder, newFile);

    return cpf2f(f1,f2Holder);
}

int isFileSame(char *f1, char *f2)
{
    struct stat * fileStat1 = (struct stat*) malloc(sizeof(struct stat)), *fileStat2 = (struct stat*) malloc(sizeof(struct stat));
    lstat(f1, fileStat1);
    lstat(f2, fileStat2);

    if( (fileStat1->st_dev == fileStat2->st_dev) && (fileStat1->st_ino == fileStat2->st_ino))
    {
        free(fileStat1);
        free(fileStat2);
        return 1;
    }
    else
        return 0; 

}

int cpd2d(char *f1, char *f2)
{
    DIR * src_dir = opendir(f1);
    char src_string[PATH_MAX], dest_string[PATH_MAX], *baseHolder;
    struct dirent *src_file;
    struct stat *srcFileStat = (struct stat*) malloc(sizeof(struct stat));
    int didCopySucceed = 0;

    strcpy(src_string, f1);
    baseHolder = basename(src_string);
    strcpy(dest_string, f2);

    /*if(strstr(dest_string,baseHolder) )
    {
        printf("ERROR: %s is a child directory of %s\n", f2, f1);
        return -1;
    }*/

    src_file = readdir(src_dir);
    
    while(src_file)
    {
       // printf("Entered loop\n");
        if((strcmp(src_file->d_name, ".") == 0)|| (strcmp(src_file->d_name, "..") == 0))
        {
                //printf("Entered ./.. catch\n");
        }
        else
        {
            strcpy(src_string, f1);
            strcat(src_string, "/");
            strcat(src_string, src_file->d_name);
            lstat(src_string, srcFileStat);

            //printf("Entered Else in loop\n");

            if(S_ISDIR(srcFileStat->st_mode)) 
            {   
                //printf("Entered is_dir\n");
                strcpy(dest_string, f2);
                strcat(dest_string, "/");
                strcat(dest_string, src_file->d_name);

                mkdir(dest_string, 0777);
                //printf("Entering recursive cpd2d\n");
                didCopySucceed =cpd2d(src_string, dest_string);
              //  printf("completed recursion\n");
            }   
            else
            {   
         //       printf("Entered copyF2d in cpD2D\n");
                didCopySucceed = cpf2d(src_string, f2);
           //     printf("completed cpf2d\n");
            }
        
            if(didCopySucceed == -1)
            {
                printf("Error: Copying from %s to %s failed\n",f1,f2);
                free (srcFileStat);
                return -1;
            }

            
        }

        src_file = readdir(src_dir);
    }

    free(srcFileStat);
    return didCopySucceed;
}