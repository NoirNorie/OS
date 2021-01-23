/*
 * Copyright (c) 2001,2003,2004 David H. Hovemeyer <daveho@cs.umd.edu>
 * Copyright (c) 2003,2013,2014 Jeffrey K. Hollingsworth <hollings@cs.umd.edu>
 *
 * All rights reserved.
 *
 * This code may not be resdistributed without the permission of the copyright holders.
 * Any student solutions using any of this code base constitute derviced work and may
 * not be redistributed in any form.  This includes (but is not limited to) posting on
 * public forums or web sites, providing copies to (past, present, or future) students
 * enrolled in similar operating systems courses the University of Maryland's CMSC412 course.
 */

#include <conio.h>
#include <process.h>


#define MAXTABLESIZE 50

int main(int argc __attribute__ ((unused)), char **argv
         __attribute__ ((unused))) {

struct Process_Info ptable[MAXTABLESIZE];

PS(ptable,MAXTABLESIZE);


Print("PID PPID PRIO STAT AFF TIME COMMAND\n");

char stat=0;
char current_cpu =0;

for(int i=0; i < MAXTABLESIZE;i++){

if(ptable[i].pid == 0) return 0;

if(ptable[i].status==STATUS_RUNNABLE){
 stat='R';
 current_cpu=ptable[i].currCore;
}
else if(ptable[i].status==STATUS_BLOCKED) stat='B';
else if(ptable[i].status==STATUS_ZOMBIE) stat='Z';
else stat='E'; // erorr


Print("%3d %4d %4d %2c%2c %3c %4d %s\n",ptable[i].pid,ptable[i].parent_pid,ptable[i].priority,(stat=='R')?current_cpu+'0':' ', stat,ptable[i].affinity==-1?'A':ptable[i].affinity+'0',ptable[i].totalTime,ptable[i].name);
}



// format string for one process line should be "%3d %4d %4d %2c%2c %3c %4d %s\n"
    return 1;
}
