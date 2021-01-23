/*
 * GeekOS C code entry point
 * Copyright (c) 2001,2003,2004 David H. Hovemeyer <daveho@cs.umd.edu>
 * Copyright (c) 2003, Jeffrey K. Hollingsworth <hollings@cs.umd.edu>
 * Copyright (c) 2004, Iulian Neamtiu <neamtiu@cs.umd.edu>
 * $Revision: 1.51 $
 * 
 * This is free software.  You are permitted to use,
 * redistribute, and modify it as specified in the file "COPYING".
 */

#include <geekos/bootinfo.h>
#include <geekos/string.h>
#include <geekos/screen.h>
#include <geekos/mem.h>
#include <geekos/crc32.h>
#include <geekos/tss.h>
#include <geekos/int.h>
#include <geekos/kthread.h>
#include <geekos/trap.h>
#include <geekos/timer.h>
#include <geekos/keyboard.h>




/*
 * Kernel C code entry point.
 * Initializes kernel subsystems, mounts filesystems,
 * and spawns init process.
 */

void OS13_Kernel_Thread();

void Main(struct Boot_Info* bootInfo)
{
    Init_BSS();
    Init_Screen();
    Init_Mem(bootInfo);
    Init_CRC32();
    Init_TSS();
    Init_Interrupts();
    Init_Scheduler();
    Init_Traps();
    Init_Timer();
    Init_Keyboard();


    Set_Current_Attr(ATTRIB(BLACK, GREEN|BRIGHT));
    Print("Welcome to GeekOS!\n");
    Set_Current_Attr(ATTRIB(BLACK, GRAY));

    Start_Kernel_Thread(OS13_Kernel_Thread, 0, PRIORITY_NORMAL, false);

    /* Now this thread is done. */
    Exit(0);
}

void OS13_Kernel_Thread {
	Keycode key;
	int row, col, irow, icol, cnt=0;
	int line[24], i=0, j=0;
	char buffer[79][25];	
	char copy[79][25];

	int flag = 0x0000, flag1 = 0x00000;
	uchar_t attr; 

	bool shift_state = false;

	for(i=0; i<25; i++) {
		for(j=0; j<79; j++) {
			buffer[i][j] = NULL;
			copy[i][j] = NULL;
		}
	}

	while(1) {
		key = Wait_For_Key();
		if( !(key & KEY_RELEASE_FLAG) ) {
			Get_Cursor(&row, &col);
			switch( key ) {
				case 0x0d :					// Enter key
					line[row] = col;
					Print("\n");
					if(row==24) {
						Clear_Screen();
						Put_Cursor(0,0);
					}	
					break;
				case 0x08 :  					// backspace							
					if( !(row==0 && col==0) ) {
						if( col!= 0 ) {
							Put_Cursor(row, col-1);
							Print(" ");	
							Put_Cursor(row, col-1);
						} else {
							if( line[row-1] != 79) {
								Put_Cursor(row-1,line[row-1]);
							} else {
								Put_Cursor(row-1, 79);
								Print(" ");
								Put_Cursor(row-1, 79);
							}
						}
					}
					break;
				case KEY_KPUP : Put_Cursor(row-1,col); flag1 = 0x10001; break;		// 0x0381
				case KEY_KPLEFT : Put_Cursor(row,col-1); flag1 = 0x10010; break;		// 0x0384
				case KEY_KPRIGHT : Put_Cursor(row,col+1); flag1 = 0x10100; break;		// 0x0386
				case KEY_KPDOWN : Put_Cursor(row+1,col); flag1 = 0x11000; break;	// 0x0389
				case KEY_KPHOME : Put_Cursor(row, 0);  break;				// 0x0380
				case KEY_KPEND : 
					for(i=79; i>=0; i--) {
						if( buffer[row][i] != NULL ) {
							col = i;							
							break;
						}
					}
					Put_Cursor(row, col+1);
					break;				// KEYPAD END
				case 0x4064 :						// CTRL+D
				case 68 :							// D
						Print("\nBYE\n");
						Exit(0); 
/******************************************************************/
/*				case 0x1381 : break;			// SHIFT + KEYPAD UP
				case 0x1384 : 					// SHIFT + KEYPAD LEFT
					flag |= 0x0001;
					if( flag == 0x0011 ) { 
						Set_Current_Attr(ATTRIB(BLACK, GRAY));							
						Put_Cursor(row, col-1);
						Print("%c", buffer[row][col-1]);
						Put_Cursor(row, col-1);					
						flag = 0x
					}
					else if( flag == 0x0001 ) {
						Set_Current_Attr( ATTRIB(BLACK, RED | BRIGHT) );					
						Put_Cursor(row, col-1);
						Print("%c", buffer[row][col-1]);
						Put_Cursor(row, col-1);
					}
					break;
				case 0x1386 : 					// SHIFT + KEYPAD RIGHT
					flag |= 0x0010;
					if( flag == 0x0011 ) {
						Set_Current_Attr(ATTRIB(BLACK, GRAY));
						Print("%c", buffer[row][col]);
					} else if ( flag = 0x0010 ) {
						Set_Current_Attr( ATTRIB(BLACK, RED | BRIGHT) );
						Print("%c", buffer[row][col]);
					}
					break;
				case 0x1389 :  break;			// SHIFT + KEYPAD DOWN
*/
				case 0x1380 : 					// KEYPAD HOME
					flag |= 0x0001;
					if( flag == 0x0001 ) {					
						Put_Cursor(row, 0);
						Set_Current_Attr( ATTRIB(BLACK, RED | BRIGHT) );
						for(i=0; i<col; i++) {
							Print("%c", buffer[row][i]);
						}	
						Put_Cursor(row, 0);
					} else if ( flag == 0x0011 ) {
						Put_Cursor(row,0);
						for( i=0; i<=col; i++ ) {
							Set_Current_Attr(ATTRIB(BLACK, GRAY));						
							Print("%c", buffer[row][i]);
						}	
						Put_Cursor(row, 0);
						flag = 0x0000;
					}	
			 		break;				
				case 0x1388 : 					// KEYPAD END
					flag |= 0x0010;
					if( flag == 0x0010 ) {
						Set_Current_Attr( ATTRIB(BLACK, RED | BRIGHT) );
						for( i=79; i>=0; i-- ) {
							if( buffer[row][i] != NULL ) 
								break;
						}
						Put_Cursor(row, col);
						for( ; col<=i; col++ ) {
							Print("%c", buffer[row][col]);
						}
					} else if( flag == 0x0011 ) {
						for( i=79; i>=0; i-- ) {
							if( buffer[row][i] != NULL ) 
								break;
						}
						for( ; col<=i; col++ ) {
							Set_Current_Attr(ATTRIB(BLACK, GRAY));						
							Print("%c", buffer[row][col]);
						}
						flag = 0x0000;
					}				
					break; 
/******************************************************************/
				case 0x4063 : 								// CTRL + C
					if( flag == 0x0001 || flag == 0x0010 ) {
						for(i=0; i<=25; i++) {
							for(j=0; j<=79; j++) {
								copy[i][j] = NULL;
							}
						}
						for(i=0; i<=25; i++) {
							for(j=0; j<=79; j++) {	
								attr = Get_Current_Attr();
								if( attr == 12 ) {
									Get_Cursor(&irow, &icol);
									copy[i][j] = buffer[i][j];
									cnt++;
								}
							}
						}					
						Put_Cursor(irow, icol);
					}	
					break;
				case 0x4076 : 									// CTRL + V
					for(i=col; i<=cnt; i++) {
						if( copy[row][i] != NULL ) {
							Set_Current_Attr(ATTRIB(BLACK, GRAY));								
							Print("%c", copy[row][i]); 
						}
					}
				break;
				default :
					line[row] = col;
					Print("%c", key);
					buffer[row][col] = key;
					flag = 0;
			} // switch End
			Get_Cursor(&row, &col);
			if( flag1 != 0 ) {
				Put_Cursor(row, 0);
				for( i=0; i<79; i++) {
					Set_Current_Attr(ATTRIB(BLACK, GRAY));						
					Print("%c", buffer[row][i]);
				}			
				flag1 = 0x00000;
				Put_Cursor(row,col);
			}
		} // if End
	} // while End
}

