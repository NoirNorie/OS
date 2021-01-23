/*
 * Common user mode functions
 * Copyright (c) 2001,2003,2004 David H. Hovemeyer <daveho@cs.umd.edu>
 * $Revision: 1.50 $
 * 
 * This is free software.  You are permitted to use,
 * redistribute, and modify it as specified in the file "COPYING".
 */

#include <geekos/errno.h>
#include <geekos/ktypes.h>
#include <geekos/kassert.h>
#include <geekos/int.h>
#include <geekos/mem.h>
#include <geekos/malloc.h>
#include <geekos/kthread.h>
#include <geekos/vfs.h>
#include <geekos/tss.h>
#include <geekos/user.h>

/*
 * This module contains common functions for implementation of user
 * mode processes.
 */

/*
 * Associate the given user context with a kernel thread.
 * This makes the thread a user process.
 */
void Attach_User_Context(struct Kernel_Thread* kthread, struct User_Context* context)
{
    KASSERT(context != 0);
    kthread->userContext = context;

    Disable_Interrupts();

    /*
     * We don't actually allow multiple threads
     * to share a user context (yet)
     */
    KASSERT(context->refCount == 0);

    ++context->refCount;
    Enable_Interrupts();
}

/*
 * If the given thread has a user context, detach it
 * and destroy it.  This is called when a thread is
 * being destroyed.
 */
void Detach_User_Context(struct Kernel_Thread* kthread)
{
    struct User_Context* old = kthread->userContext;

    kthread->userContext = 0;

    if (old != 0) {
	int refCount;

	Disable_Interrupts();
        --old->refCount;
	refCount = old->refCount;
	Enable_Interrupts();

//	Print("User context refcount == %d\n", refCount);
        if (refCount == 0)
            Destroy_User_Context(old);
    }
}

/*
 * Spawn a user process.
 * Params:
 *   program - the full path of the program executable file
 *   command - the command, including name of program and arguments
 *   pThread - reference to Kernel_Thread pointer where a pointer to
 *     the newly created user mode thread (process) should be
 *     stored
 * Returns:
 *   The process id (pid) of the new process, or an error code
 *   if the process couldn't be created.  Note that this function
 *   should return ENOTFOUND if the reason for failure is that
 *   the executable file doesn't exist.
 */
int Spawn(const char *program, const char *command, struct Kernel_Thread **pThread)
{
  int rc;          /* return code */
  void *buf;       /* buffer to store executable file (ELF format) */
  ulong_t fileLen; /* length of ELF file */
  
  /* load user program into a kernel buffer */
  rc = Read_Fully(program, &buf, &fileLen);  
  

  if(rc != 0) // File not found or no free memory
  {
    return rc;
  }


  /* create exeFormat data structure */
  struct Exe_Format *exeFormat;
  exeFormat = (struct Exe_Format *)Malloc(sizeof (struct Exe_Format));
  
  if (exeFormat == NULL) /* out of memory error */
    {
//      Free(buf);
      return ENOMEM;
    }
      
  memset(exeFormat, '\0', sizeof(struct Exe_Format)); /* clear memory by writing zeros */
  
  struct User_Context *pUserContext; /* declare userContext */
  
  /* fill exeFormat data structure */
  rc = Parse_ELF_Executable((char *)buf, fileLen, exeFormat);
      
  if(rc != 0)
  {
    Free (buf);
    Free (exeFormat); /* remove pUserContext data structure */
    return rc;
  }

  /* fill UserContext data structure */
  rc = Load_User_Program((char *)buf, fileLen, exeFormat,
			       command, &pUserContext);
  
  if (rc != 0)
    {
      Free (buf);
      Free (exeFormat);
      Free (pUserContext); /* remove pUserContext data structure */
      return rc;
    }
      
  bool detached = false;
  
  /* create the user thread */
  *pThread = Start_User_Thread(pUserContext, detached);
	  
  if (*pThread == NULL)
    {
      Free (buf);
      Free (pUserContext); /* remove pUserContext data structure */
      return EUNSPECIFIED;
    } 

	Free(exeFormat); /* remove exeFormat data structure */
	Free(buf);
      
  return (*pThread)->pid;
}

/*
 * If the given thread has a User_Context,
 * switch to its memory space.
 *
 * Params:
 *   kthread - the thread that is about to execute
 *   state - saved processor registers describing the state when
 *      the thread was interrupted
 */
void Switch_To_User_Context(struct Kernel_Thread* kthread, struct Interrupt_State* state)
{
  /* determine whether kthread has a userContext (a user thread) */
  if(kthread->userContext){        
    /* reset the stack pointer to the bottom of the stack */
    Set_Kernel_Stack_Pointer(((ulong_t)(kthread->stackPage)) + PAGE_SIZE);
    
    /* load the ldt of the current process into the cpu */
    Switch_To_Address_Space(kthread->userContext);
  }
}
