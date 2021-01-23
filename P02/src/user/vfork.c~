#include <conio.h>
#include <geekos/syscall.h>

int externVal = 0;

int main(int argc, char **argv)
{
	int pid;
	int localVal = 0;
	
	Print ( " Vfork Exam Program ... \n" );

	pid=VFORK();

	externVal++;
	localVal++;

	if ( pid == 0 ) // Child process 
	{
		Print ( " 	< Child Process PID : %x > \n", pid);
		Print ( " Extern Value [in Data Segment] = %d ", externVal );
		Print ( " Local  Value [in Stack Segment] = %d ", ++localVal );
		_Exit();
	} 
	else 		// Parent process
	{
		Print ( " 	< Parent Process PID : %x > \n", pid );	
		Print ( " Extern Value [in Data Segment] = %d ", externVal );
		Print ( " Local  Value [in Stack Segment] = %d ", ++localVal );
	}

	return 0;
}
