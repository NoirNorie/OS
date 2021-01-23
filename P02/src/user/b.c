/*
 * A test program for GeekOS user mode
 */

#include <conio.h>
#include "process.h"

int externVal = 0;

int main(int argc, char** argv)
{
	int pid;

    int i;
    Print_String("I am the b program\n");
    for (i = 0; i < argc; ++i) {
	Print("Arg %d is %s\n", i,argv[i]);
    }
	
	Print ( " Vfork Exam Program ... \n" );
int local = 0;
	pid=vFork();

	if ( pid == 0 ) // Child process 
	{
		externVal = 10;
local = 10;
Print("%x", &local);
		Print ( " 	< Child Process > \n" );
		Print ( " Extern Value [in Data Segment] = %d \n", externVal );
		Print ( " Extern Value [in Data Segment] = %d \n", local);
		vExit();
	} 
	else 		// Parent process
	{
int aaaa = 100;
local = 100;
Print("%x", &local);
		Print ( " 	< Parent Process > \n" );	
		Print ( " Extern Value [in Data Segment] = %d \n", externVal );
		Print ( " Extern Value [in Data Segment] = %d \n", local);
Print("%d", aaaa);
		Exit(0);
	}
   return 0;
}