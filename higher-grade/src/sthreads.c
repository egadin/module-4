/* On Mac OS (aka OS X) the ucontext.h functions are deprecated and requires the
   following define.
*/
#define _XOPEN_SOURCE 700

/* On Mac OS when compiling with gcc (clang) the -Wno-deprecated-declarations
   flag must also be used to suppress compiler warnings.
*/

#include <signal.h>   /* SIGSTKSZ (default stack size), MINDIGSTKSZ (minimal
                         stack size) */
#include <stdio.h>    /* puts(), printf(), fprintf(), perror(), setvbuf(), _IOLBF,
                         stdout, stderr */
#include <stdlib.h>   /* exit(), EXIT_SUCCESS, EXIT_FAILURE, malloc(), free() */
#include <ucontext.h> /* ucontext_t, getcontext(), makecontext(),
                         setcontext(), swapcontext() */
#include <stdbool.h>  /* true, false */

#include "sthreads.h"

/* Stack size for each context. */
#define STACK_SIZE SIGSTKSZ*100

/*******************************************************************************
                             Global data structures

                Add data structures to manage the threads here.
********************************************************************************/
thread_t top, bottom;
int maxId;


/*******************************************************************************
                             Auxiliary functions

                      Add internal helper functions here.
********************************************************************************/




/*******************************************************************************
                    Implementation of the Simple Threads API
********************************************************************************/


int  init(){
  return 1;
}


tid_t spawn(void (*start)()){
  /* Start by creating a thread */
  thread_t newThread;
  
  /* Initialize all the values of our new thread
   The new tid will be the largest one */
  maxId = maxId+1;
  newThread.tid = maxId;
  newThread.state = ready;
  //  newThread.ctx = start;
  newThread.next = &top;

  /* We make our new thread the bottom thread, it already
     points to our top thread */
  bottom.next = &newThread;
  bottom = newThread;

  /* Now we need to intialize contexts, we start by allocating
     memory to be used as the stack of the context. This following
     part is taken from "contexts.c" */
  void *stack = malloc(STACK_SIZE);
  
  if (stack == NULL) {
    perror("Allocating stack");
    exit(EXIT_FAILURE);
  }

  if (getcontext(&newThread.ctx) < 0) {
    perror("getcontext");
    exit(EXIT_FAILURE);
  }

  newThread.ctx.uc_link           = &newThread.next->ctx;
  newThread.ctx.uc_stack.ss_sp    = stack;
  newThread.ctx.uc_stack.ss_size  = STACK_SIZE;
  newThread.ctx.uc_stack.ss_flags = 0;

  makecontext(&newThread.ctx, start, 0);
  
  return -1;
}

void yield(){
  
}

void  done(){
}

tid_t join() {
  return -1;
}
