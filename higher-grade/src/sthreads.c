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
thread_t* top=NULL;
thread_t* bottom=NULL;


//ska göra tre separata listor en för ready en för waiting och en för terminated.



/*******************************************************************************
                             Auxiliary functions

                      Add internal helper functions here.
********************************************************************************/
thread_t *getRunning(thread_t*current) {
  if (current->state == running)
    return current;
  else
    getRunning(current->next);
}



/*******************************************************************************
                    Implementation of the Simple Threads API
********************************************************************************/


int  init(){
  return 1;
}


tid_t spawn(void (*start)()){
  thread_t* newThread = (thread_t*) malloc(sizeof(thread_t));
  newThread->state=ready;
  makecontext(&newThread->ctx, start, 0);
  if (top==NULL){
    newThread->next=NULL;
    top=newThread;
    bottom=newThread;
    newThread->next=NULL;
  }
  else {
    newThread->next=top;
    bottom->next=newThread;
    bottom=newThread;
  }
  
  return -1;
}

void yield(){
  thread_t *current=getRunning(top);
  current->state=ready;
  current->next
  top
}

void  done(){
}

tid_t join() {
  return -1;

}
