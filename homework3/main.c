#include "function.h"

int main (int argc, char** argv, char* const envp[])
{
    char* inputString;
    char** inputArray;
  
    findPath();
    findHome();

    while(1)
    {
        inputString = getInput();
        inputArray = parseInput(inputString);
        handleCommand(inputArray, envp);
    }
	return 0;
}