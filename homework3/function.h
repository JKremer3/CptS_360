 /* Jeff Kremer, CptS 360 */
 
 #include <stdio.h>
 #include <stdlib.h>
 #include <string.h>
 #include <fcntl.h>
 #include <unistd.h>

 char *commandPath[15];
 char *homePath;

 char* getInput();
 char** parseInput(char* input);
 void handleCommand(char** input, char* const envp[]);
 void cd(char* pathname);
 void exitProg();
 void otherCommand(char** input, char* const envp[]);
 void findPath();
 void findHome();
 int forkChild(char** input, char* const envp[]);
 char* isRedirect(char** input, int* red);
 void redirect(char* file, int red);
 int hasPipe(char** input, char** head, char** tail);
 void doPipe(char** head, char**tail, char* const envp[]);
 void runProg(char** input, char* const envp[]);