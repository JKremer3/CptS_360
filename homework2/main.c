#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libgen.h>

typedef struct node{
    char name[64];
    char fileType;
    struct node *childPtr, *sibPtr, *parPtr;
}Node;

Node *root, *cwd, *start;
char line[128];
char command[16], pathNameLine[64];
char dname[64], bname[64];
char *cmd[13] = {"mkdir", "rmdir","ls", "cd", "pwd", "creat", "rm", "reload",
            "save", "menu", "quit", NULL};
char *name[100][20];
int n;
FILE *fp;

Node* initialize(Node* tmp, char filetype, Node* parNode, char* nodeName)
{
    //printf("Entered init\n");
    tmp = (Node *) malloc(sizeof(Node));
   // printf("Memory Allocated");
    tmp ->fileType = filetype;
    strcpy(tmp->name, nodeName);
    //printf("Name and File type set\n");
    tmp ->childPtr = NULL;
    tmp ->parPtr = parNode;
    tmp ->sibPtr = NULL;
    //printf("Pointers set\n");
    if(tmp = NULL)
    {
        printf("alloc failed\n");
    }
    //printf("init complete\n");
}

int dbname(char *pathName)
{
    char temp[128];
    strcpy(temp, pathName);
    strcpy(dname, dirname(temp));
    strcpy(temp, pathName);
    strcpy(bname, basename(temp));
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

Node * search_child(Node* child, char *name)
{
    /*Node *currNode, *nextNode;

    if(parent->childPtr == NULL)
    {
        printf("Child not found\n");
        return 0;
    }
    else
    {
        //printf("child nodes found\n");
        if(strcmp(parent->childPtr->name, name)== 0)
        {
            return parent->childPtr;
        }
        else
        {
            currNode = parent->childPtr;

      //      printf("iterating through children\n");
            while(currNode->sibPtr != NULL)
            {
                nextNode = currNode->sibPtr;
                
                if(strcmp(nextNode->name,name)==0)
                    return nextNode;
                
                currNode = nextNode;
            }

            return 0;
        }
    }*/
    while (child != NULL && strcmp(child->name, name))
        child = child->sibPtr;
    return child;
    
}

int tokenize (char *path)
{
    char *s = NULL;
    int i = 0;
    s = strtok(path, "/");
    //printf("entering while loop of tokenize\n");
    while (s)
    {
        strcpy(name[i], s);
        //printf("name successfully copied\n");
        s = strtok(0, "/");
        i++;
    }

    return i;
}

Node *path2node(char *pathname)
{
  // return the node pointer of a pathname, or 0 if the node does not exist.  


   if (pathname[0] = '/') 
        start = root;
   else                 
        start = cwd;
   // printf("start set\n");
    n = tokenize(pathname); // NOTE: pass a copy of pathname
    //printf("n = %d\n", n);

   Node *p = start;
    //printf("Node P set\n");
   
   for (int i=0; i < n; i++){
       p = search_child(p, name[i]);
      // printf("p: name = %s\n", p->name);
       if (p==0) return 0;            // if name[i] does not exist
   }
   return p;
    /*Node* currNode = cwd;
   char* token = strtok(pathname, "/");

   if(!strcmp(pathname, "."))
        return currNode;
    
    while (token != NULL && currNode != NULL)
    {
        if(currNode->fileType == 'D')
        {
            currNode = search_child(currNode, token);
            token = strtok(NULL, "/");
        }else{
            currNode = NULL;
            break;
        }
    }
    return currNode;*/
}

//returns the node with name specified in path, relative to starting node
/*Node* locationHelper(Node* startNode, char *nodeName)
{
    Node* tmp = startNode->childPtr;

    if(tmp == NULL)
    {
        return 0;
    }
    else if (tmp->name == nodeName)
    {
        return tmp;
    }
    else
    {
        while(tmp != NULL)
        {
            if(tmp->name == nodeName)
            {
                return tmp;
            }

            tmp = tmp->sibPtr;
        }
    }

    return 0;
}*/

//Find the "older" sibling of a given node
Node* getOlderSibling(Node* nodeToCheck)
{
    Node *nodeParent = nodeToCheck->parPtr, *olderSib = nodeParent->childPtr;

    while(olderSib->sibPtr != nodeToCheck);
        olderSib = olderSib -> sibPtr;

    return olderSib;
}

int mkdir(char *pathName)
{
    Node *nodeHolder, *nodeHolder2;
    char* pathHolder, *dnameHolder;
    int i = 0;

    strcpy(pathHolder,pathName); // copy path to avoid destruction
    dbname(pathHolder); //find directory string and path name

    nodeHolder = path2node(pathName);

    //If the directory location does not exist, or the final location is a file
    // return an error
    if (nodeHolder == NULL || nodeHolder->fileType == 'F')
    {
        printf("Error: path does not exist \n");
        return 0;
    }

    //If the childPtr of that location is NUll, immediately set childPtr to the new node
    //Otherwise traverse the childPtr's sibling tree and 
    if(nodeHolder->childPtr == NULL)
    {
        nodeHolder->childPtr = initialize(nodeHolder->childPtr, 'D', nodeHolder, bname);
        if(root->childPtr == NULL)
        {
            printf("Child ptr not set\n");
        }
        printf("Directory %s succesfully created\n",nodeHolder->childPtr->name);
        return 1;
    }
    else
    {
        nodeHolder2 = nodeHolder->childPtr; //Enter the child tree of the location node
        while (nodeHolder2->sibPtr != NULL )  //iterate through sibling list until reaching the end
        {
            //printf("checking other directories for same name and type");
            if(nodeHolder2->name == bname && nodeHolder2->fileType == 'D')
            {   
                printf("error directory already exists\n");
                return 0;
            }

            nodeHolder2 = nodeHolder2->sibPtr;
        }
        
        //Creates new node at end of sibling list
        nodeHolder2->sibPtr = initialize(nodeHolder2->sibPtr, 'D', nodeHolder, bname);
        printf("Directory %s succesfully created\n",nodeHolder2->sibPtr->name);
        return 1;
    }
}

int rmdir(char *pathName)
{
    Node *nodeHolder, *sibPointer;
    char pathHolder[128], *dnameHolder;
    int i = 0;

    strcpy(pathHolder,pathName); // copy path to avoid destruction
    dbname(pathHolder); //find directory string and path name
    
    nodeHolder = path2node(pathName);
    
    //If the directory location does not exist, or the final location is a file
    // return an error
    if (nodeHolder == NULL || nodeHolder->fileType == 'F')
    {
        printf("Error: path does not exist \n");
        return 0;
    }
    else if(nodeHolder != root)
    {
        printf("Error: cannot delete root");
    }
    else if(nodeHolder->childPtr == NULL && nodeHolder->fileType == 'D')
    {
        if(nodeHolder->sibPtr == NULL)
        {
            free(nodeHolder);
        }
        else
        {
             if(nodeHolder != nodeHolder->parPtr->childPtr)
            {
                printf("entered older sibling checking\n");
                sibPointer = getOlderSibling(nodeHolder);
                sibPointer->sibPtr = nodeHolder->sibPtr;
                free(nodeHolder);
            }
            else
            {
                nodeHolder->parPtr->childPtr = nodeHolder->sibPtr;
                free(nodeHolder);
            }   
        }
    }
    else
    {
        printf("Error: Directory is not empty. Cannot be deleted \n");
    }
}

int ls(char *pathName)
{
    Node *lsNode, *migratingNode;

    lsNode = path2node(pathName);
    migratingNode = lsNode->childPtr;

    if(migratingNode != NULL)
    {

        while(migratingNode != NULL)
        {
            printf("%s %c\n",migratingNode->name, migratingNode->fileType);
            migratingNode = migratingNode->sibPtr;
        }
        return 1;
    }
    else
    {
        printf("\n");
    }

    return 1;
}

int cd(char *pathName)
{
    Node* currNode;

    currNode = path2node(pathName);
    if(currNode != NULL)
    {
        cwd = currNode;
        printf("cwd changed to %s\n", cwd->name);
    }
    else
        return 0;
    
    return 1;
}

int pwd(char *pathName)
{
    int stepCount = 0;
    char* workingDirectoryNames[100], dirString[128];
    Node* currNode = cwd;

     //printf("vars intialized\n");
    
    while(currNode != NULL)
    {
        strcpy(workingDirectoryNames[stepCount], currNode->name);
        //printf("Name added to workingDirectory");
        stepCount++;
       // printf("currnode ->parPtr-> name = %s\n",currNode->parPtr->name);
        currNode = currNode->parPtr;
        //printf("currnode set to parent\n");
    }
    // printf("directorynames counted\n");

    stepCount--;
    strcat(dirString, workingDirectoryNames[stepCount]);
    stepCount--;

    //printf("processing names\n");
    for(stepCount; stepCount > 0; stepCount--)
    {   
        strcat(dirString, workingDirectoryNames[stepCount]);
        // printf("Name set\n");
        strcat(dirString, "/");
    }
    //strcat(dirString, cwd->name);
   // printf("CONCATENTATION COMPLETE\n");

    printf("Current Directory: %s\n", dirString);
}

int filePWD(Node* saveNode)
{
    int stepCount = 0;
    char* workingDirectoryNames[100], *dirString;
    Node* saveNodeHolder = saveNode;

    dirString = (char*) malloc(128 * sizeof(char));

    while(saveNodeHolder != NULL)
    {
        strcpy(workingDirectoryNames[stepCount], saveNodeHolder ->name);
        stepCount++;
    }

    strcat(dirString, workingDirectoryNames[stepCount]);
    stepCount--;

    for(stepCount; stepCount >= 0; stepCount--)
    {
        strcat(dirString, workingDirectoryNames[stepCount]);
        strcat(dirString, "/");
    }

    fprintf(fp,"%c %s\n",cwd->fileType ,dirString);
}

int creat(char *pathName)
{
    Node *nodeHolder, *nodeHolder2;
    char* pathHolder, *dnameHolder;
    int i = 0;

    strcpy(pathHolder,pathName); // copy path to avoid destruction
    dbname(pathHolder); //find directory string and path name
    
    nodeHolder = path2node(pathName);

    //If the directory location does not exist, or the final location is a file
    // return an error
    if (nodeHolder == NULL || nodeHolder->fileType == 'F')
    {
        printf("Error: path does not exist \n");
        return 0;
    }

    //If the childPtr of that location is NUll, immediately set childPtr to the new node
    //Otherwise traverse the childPtr's sibling tree and 
    if(nodeHolder->childPtr == NULL)
    {
        nodeHolder->childPtr = initialize(nodeHolder->childPtr, 'F', nodeHolder, bname);
        printf("Directory %s succesfully created\n",nodeHolder->sibPtr->name);
        return 1;
    }
    else
    {
        nodeHolder2 = nodeHolder->childPtr; //Enter the child tree of the location node
        while (nodeHolder2->sibPtr != NULL )  //iterate through sibling list until reaching the end
        {
            if(nodeHolder2->name == bname && nodeHolder2->fileType == 'F')
            {   
                printf("error File already exists\n");
                return 0;
            }

            nodeHolder2 = nodeHolder2->sibPtr;
        }
        
        //creates new node at end of sibling list
        nodeHolder2->sibPtr = initialize(nodeHolder2->sibPtr, 'F', nodeHolder, bname);
        printf("File %s succesfully created\n",nodeHolder2->sibPtr->name);
        return 1;
    }
}

int rm(char *pathName)
{
    Node *nodeHolder, *sibPointer;
    char* pathHolder, *dnameHolder;
    int i = 0;

    strcpy(pathHolder,pathName); // copy path to avoid destruction
    dbname(pathHolder); //find directory string and path name
    
    nodeHolder = path2node(pathName);
    
    //If the directory location does not exist, or the final location is a file
    // return an error
    if (nodeHolder == NULL || nodeHolder->fileType == 'D')
    {
        printf("Error: File does not exist \n");
        return 0;
    }
    else if(nodeHolder->sibPtr == NULL)
    {
        free(nodeHolder);
        printf("File succesfully deleted\n");
    }
    else
    {
        if(nodeHolder != nodeHolder->parPtr->childPtr)
        {
            sibPointer = getOlderSibling(nodeHolder);
            sibPointer->sibPtr = nodeHolder->sibPtr;
            free(nodeHolder);
            printf("File succesfully deleted\n");
        }
        else
        {
            nodeHolder->parPtr->childPtr = nodeHolder->sibPtr;
            free(nodeHolder);
            printf("File succesfully deleted\n");
        }
    }
    
}

int reload(char *pathName)
{
    char pathHolder[100] = {'/0'}, fileType = '/0';

    fp =fopen(pathName, "r+");
    
    if(fp = NULL)
    {
        printf("Error: File does not exist\n");
        return 0;
    }
    else
    {   
        fullDelete(root);
        fscanf(fp, "%c %s", fileType, pathHolder);
        root = initialize(root, 'D', NULL, "/");
        cwd = root;
        while(!feof)
        {
            fscanf(fp, "%c %s", fileType, pathHolder);
            if(fileType == 'D')
            {
                mkdir(pathHolder);
            }
            else
            {
                creat(pathHolder);
            }
        }
    }
    printf("Reload completed\n");
    fclose(fp);

}

int save(char *pathName)
{
    Node* saveNode = root;
    fp = fopen("myfile", "w+");
    saveHelp(saveNode);
    printf("Directory tree saved as 'myfile'\n");
    
    fclose(fp);
}

void saveHelp(Node *currNode)
{
    if(currNode != NULL)
    {
        filePWD(currNode);
        saveHelp(currNode->childPtr);
        saveHelp(currNode->sibPtr);
    }
}

int menu(char *pathName)
{
    printf("=========Menu=========\n mkdir: create a new directory\n creat: create a new file \n rmdir: delete a directory (directory must be empty)\n rm: deletes file\n cd: change current working directory\n ls: list child items in directory\n pwd: print working directory\n reload: load saved file tree\n save: save current file tree\n quit: exit program and save tree\n ===========================\n");
}

int quit(char *pathName)
{
    save("");
    fullDelete(root);
    exit(0);
}

void fullDelete(Node* currNode)
{
    if(currNode != NULL)
    {
        fullDelete(currNode->childPtr);
        fullDelete(currNode->sibPtr);
        free(currNode);
    }
}

int main(){
    int (*fptr[ ])(char *)= {(int (*)())mkdir, rmdir, ls, cd, pwd, creat, rm, reload, save,menu, quit};
    int index;

    root = NULL;
    start = NULL;
    cwd = NULL;
    //printf("Entered main\n");
    root = initialize(root, 'D', NULL, "/");
    cwd = root;
    while(1){
        printf("Input a command line: ");
        fgets(line, 128,stdin);
        line[strlen(line)-1] = 0;
        sscanf(line, "%s %s", command, pathNameLine);
         //printf("sscanf complete\n");
        index = findCmd(command);
         //printf("index set\n");
        if(index >= 0)
        { 
            int r = fptr[index](pathNameLine);
        }
    }
}
