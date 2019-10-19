#include "function.h"

char* getInput()
{
    char buffer[1024], *temp = NULL;
    int length = -1;

    printf("USER@mysh: ");
    fgets(buffer, 1024, stdin); 
    length = strlen(buffer);
    temp = (char*) malloc(sizeof(char) * length);
    strcpy(temp, buffer);

    //Chops off return character
    if(temp[length-1] == '\n')
    {
        temp[length-1] = 0;
    }
    
    return temp;
}

char** parseInput(char* input)
{
    int count = 0;
    char *string = NULL;
    char *tempString = NULL;
    // Allocate char* inputArr[20]
    char** inputArray = (char**)malloc(sizeof(char*)*20);
    string = strtok(input, " ");

    //iterates through string, tokenizing based on spaces for each command
    while(string)
    {
        tempString = (char *)malloc(sizeof(char)*strlen(string));
        strcpy(tempString, string);
        inputArray[count] = tempString;
        count++;
        string = strtok(NULL, " ");
    }

    inputArray[count] = NULL;

    return inputArray;
}

void handleCommand(char** input, char* const envp[])
{
    int i = 0;

    //if command is cd or exit, run those commands, otherwise fork child
    if(strcmp(input[0], "cd") == 0)
    {
        cd(input[1]);
    }
    else if(strcmp(input[0], "exit") == 0)
    {
        exitProg();
    }
    else
    {
        forkChild(input, envp);
    }

    //frees the content within input array once it has been worked on, then frees the input array.
    while(input[i])
    {
        free(input[i++]);
    }

    free(input);
}

//changes the directory
void cd(char* pathname)
{
    if(pathname)
    {
        chdir(pathname);
    }
    else
    {
        chdir(homePath);
    }
}

void exitProg()
{
    exit(1);
}

void otherCommand(char** input, char* const envp[])
{
    char original[128];
    char new[1024];
    int i = 0;
    strcpy(original, input[0]);

   //Check each bin folder for command by concatenating pathname with input[0]
    while(commandPath[i++]) 
    {
        strcpy(new, commandPath[i]);
        strcat(new, "/");
        strcat(new, original);
        execve(new, input, envp);
    }
}

void findPath()
{
    char *tok;
    int i = 0;
    char *name = "PATH";
    char *temp = getenv(name);
    
    tok = strtok(temp, ":");
    
    //fills the commandPath array with tokens from PATH
    while(tok != NULL)
    {
        commandPath[i++] = tok;
        tok = strtok(NULL, ":");
    }

    commandPath[i] = NULL;
    i = 0;
}

void findHome()
{
    int i = 0, j = 0;
    const char * name = "HOME";
    char *temp = getenv(name);

    homePath = (char*) malloc (sizeof(char) * strlen(temp)); //Allocates memory for home path
    //sets each character of temp into homePath. Does not need to rely on pointer to temp
    while(temp[i])
    {
        homePath[j++] = temp[i++];
    }
    homePath[j] = 0;
}

int forkChild(char** input, char* const envp[])
{
    int pid;
    int status = 0;
    int isPipe = 0;
    int i = 0;
    char** head = NULL;
    char** tail = NULL;

    head = (char**)malloc(sizeof(char*)*20);
    tail = (char**)malloc(sizeof(char*)*20);
    pid = fork();

    if(pid < 0)
    {
        printf ("Could not fork child\n");
        exit(1);
    }

    if(pid)
    {   
        pid = wait(&status);
    }
    else
    {    
        // Check for pipe
        isPipe = hasPipe(input, head, tail);
        if(isPipe)
        {
            doPipe(head, tail, envp);
        }
        else
        {
            runProg(input, envp);
        }
    }

    free(head);
    free(tail);

    return status;
}

char* isRedirect(char** input, int* red)
{
    int i = 1;
    char* temp = NULL;
    char* temp2 = NULL;

    while(!(*red) && input[i])
    {
        if(!(strcmp(input[i], "<"))) // Redirect input
        {
            *red = 1;
        }
        else if(!(strcmp(input[i], ">"))) //Redirect output
        {
            *red = 2;
        }
        else if(!(strcmp(input[i], ">>"))) //Redirect output & append
        {
            *red = 3;
        }

        if(*red)
        {
            // set redirect indicator in input array to NULL
            temp = input[i+1];          
            temp2 = input[i];
            input[i] = NULL;
            input[i+1] = NULL;
            free(temp2);
        }

        i++;
    }

    return temp;
}

void redirect(char* file, int red)
{
    switch(red)
    {
        case 1: close(0);
                open(file, O_RDONLY);
                break;
        case 2: close(1);
                open(file, O_WRONLY|O_CREAT, 0644);
                break;
        case 3: close(1);
                open(file, O_APPEND|O_WRONLY|O_CREAT, 0644);
                break;
        default: printf("Error, bad redirect\n");
                break;
    }
}

int hasPipe(char** input, char** head, char** tail)
{
    int i = 0, j = 0, found = 0;
    char *temp;

    while(!found && input[i])
    {
        if(!strcmp(input[i], "|")) // Pipe found
        {
            found = 1;
            temp = input[i];
            input[i++] = NULL;
            free(temp);
            
            while(input[i]) // Set tail
            {
                tail[j++] = input[i++];
            }
            
            tail[j] = NULL;
            input[i] = NULL;
            j = 0;
            i = 0;
            
            while(input[i]) // Set head
            {
                head[j++] = input[i++];
            }
            
            head[j] = NULL;
            free(input);
        }
     
        i++;
    }
    
    return found;
}

void doPipe(char** head, char**tail, char* const envp[])
{
       int pd[2];
   int pid;
   
   pipe(pd); // Create pipe

    // Create new child to share pipe

   if(fork() == 0) //Child process 
   {
        close(pd[0]);
        dup2(pd[1], 1);
        close(pd[1]);
        runProg(head, envp);
    }
    else // Parent process
    {
        close(pd[1]);
        dup2(pd[0], 0);
        close(pd[0]);
        runProg(tail, envp);
    }
}

void runProg(char** input, char* const envp[])
{
       int red = 0;
    char* file = NULL;

    // Check for file redirection
    
    file = isRedirect(input, &red);
    if(red)
    {
        redirect(file, red);
    }
    
    otherCommand(input, envp);
}
