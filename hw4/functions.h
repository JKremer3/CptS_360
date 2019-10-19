#include <stdio.h> // printf()
#include <stdlib.h> // exit()
#include <string.h> // string ops
#include <libgen.h> //basename(), dirname()
#include <fcntl.h> //open(), close(), read(), write ()

//stat syscalls
#include <sys/stat.h>
#include <unistd.h>

//opendir, readdir syscalls
#include <sys/types.h>
#include <dirent.h>
#include <linux/limits.h>

int myrcp(char *f1, char *f2);
int cpf2f(char *f1, char *f2);
int cpf2d(char *f1, char *f2);
int isFileSame(char *f1, char *f2);
int cpd2d(char *f1, char *f2);