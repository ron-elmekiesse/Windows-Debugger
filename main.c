/*
	Ron Elmekiesse 19.2.2021
	Hey guys, this is my 64 bit debugger.
	have fun using it, for new people - enter 'help' in console to get familiar with the commands (:
*/

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include "debugger_loop.h"

void welcome();

int main()
{
	welcome();
	start_debugger();
	printf("Thanks for running my Windows Debugger!\n");
	
    return 0;
}

void welcome()
{
	printf("O O O        O O O O O         O        O    O O O O O O      /       O O O O O  \n");
	printf("O    O     O           O       O O      O    O               /      O            \n");
	printf("O    O    O             O      O  O     O    O              /     O              \n");
	printf("O   O    O               O     O   O    O    O O O O O O           O             \n");
	printf("O O      O               O     O    O   O    O                       O O O O O   \n");
	printf("O  O      O             O      O     O  O    O                               O   \n");
	printf("O   O      O           O       O      O O    O                              O    \n");
	printf("O    O       O O O O O         O        O    O                     O O O O O     \n\n");

	printf("O O O O         O O O           O O O O O O O                                    \n");
	printf("O       O       O     O        O                                                 \n");
	printf("O        O      O    O        O                                                  \n");
	printf("O        O      O  O         O                                                   \n");
	printf("O        O      O    O        O    O O O O O                                     \n");
	printf("O       O       O     O        O           O                                     \n");
	printf("O      O        O      O        O          O                                     \n");
	printf("O O O O         O O O O          O O O O O O                                     \n\n");
}