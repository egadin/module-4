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
struct queue {
  thread_t *first;
  thread_t *last;
};

struct threadList {
  thread_t *running;
  struct queue ready;
  struct queue waiting;
  struct queue terminated;
};

struct threadList threadManager;
  int numThreads = 0;
int maxId=0;

/*******************************************************************************
                             Auxiliary functions

                      Add internal helper functions here.
********************************************************************************/
thread_t *getRunning() {
return threadManager.running;
}

void addToRunning(thread_t *newJob) {
  if (newJob!=NULL) {
  newJob->state=running;
  
  thread_t *temp=threadManager.running;
  threadManager.running=newJob;
  if(temp!=NULL){
    if (swapcontext(&temp->ctx, &newJob->ctx) < 0) {
      perror("swapcontext");
      exit(EXIT_FAILURE);
    }
  }
  else {
    setcontext(&newJob->ctx);
  }
}
}


void addToReady(thread_t *thread) {
  thread->state=ready;
  
   if (threadManager.ready.first==NULL) {
    threadManager.ready.first=thread;
    threadManager.ready.first->next=NULL;
  }
  else if(threadManager.ready.last==NULL) {
    threadManager.ready.first->next=thread;
    threadManager.ready.last=thread;
    threadManager.ready.last->next=NULL;
  }
  else {
    threadManager.ready.last->next=thread;
    threadManager.ready.last=thread;
    threadManager.ready.last->next=NULL;
}
}

thread_t *getFromReady() {
  if (threadManager.ready.first==NULL) {
    return NULL;
  }
  else {
    thread_t *toBeReturned=threadManager.ready.first;
    threadManager.ready.first=threadManager.ready.first->next;
    return toBeReturned;
  }
}

void addToWaiting(thread_t *thread) {
  thread->state=waiting;

  if (threadManager.waiting.first==NULL) {
    threadManager.waiting.first=thread;
    threadManager.waiting.first->next=NULL;
  }
  else if(threadManager.waiting.last==NULL) {
    threadManager.waiting.first->next=thread;
    threadManager.waiting.last=thread;
    threadManager.waiting.last->next=NULL;
  }
  else {
    threadManager.waiting.last->next=thread;
    threadManager.waiting.last=thread;
    threadManager.waiting.last->next=NULL;
  }

}

thread_t *getFromWaiting() {
  if (threadManager.waiting.first==NULL) {
    return NULL;
  }
  else {
    thread_t *toBeReturned=threadManager.waiting.first;
    threadManager.waiting.first=threadManager.waiting.first->next;
    return toBeReturned;
  }
}

void addToTerminated(thread_t *thread) {
  thread->state=terminated;
  numThreads--;
  
  if (threadManager.terminated.last==NULL) {
    threadManager.terminated.first=threadManager.terminated.last=thread;
  }
  else {
    threadManager.terminated.last->next=thread;
    threadManager.terminated.last=thread;
  }

}
/*
thread_t *getFromTerminated() {
  if (threadManager.terminated.first==NULL) {
    return NULL;
  }
  else {
    thread_t *toBeReturned=threadManager.terminated.first;
    threadManager.terminated.first=threadManager.terminated.first->next;
    return toBeReturned;
  }

}
*/
/*******************************************************************************
                    Implementation of the Simple Threads API
********************************************************************************/

int  init(){
 struct threadList tread = {NULL,{NULL,NULL},{NULL,NULL},{NULL,NULL}};
 threadManager=tread;
 return 1;
}

void start(){
 thread_t *startThread = getFromReady();
 addToRunning(startThread);
 printf("%lu : \n",(unsigned long int)&startThread->ctx);
 puts("i should not print");
}

tid_t spawn(void (*start)()){
  /* Start by creating a thread */
  thread_t *newThread=malloc(sizeof(thread_t));
  numThreads++;
  /* Initialize all the values of our new thread
   The new tid will be the largest one */
  maxId = maxId+1;
  newThread->tid = maxId;

  //  newThread.ctx = start;

 /* We make our new thread the bottom thread, it already
     points to our top thread */
  
  addToReady(newThread);
  
  /* Now we need to intialize contexts, we start by allocating
     memory to be used as the stack of the context. This following
     part is taken from "contexts.c" */
  void *stack = malloc(STACK_SIZE);
  
  if (stack == NULL) {
    perror("Allocating stack");
    exit(EXIT_FAILURE);
  }

  if (getcontext(&newThread->ctx) < 0) {
    perror("getcontext");
    exit(EXIT_FAILURE);
  }

  newThread->ctx.uc_link           = &newThread->next->ctx;
  newThread->ctx.uc_stack.ss_sp    = stack;
  newThread->ctx.uc_stack.ss_size  = STACK_SIZE;
  newThread->ctx.uc_stack.ss_flags = 0;
  
  makecontext(&newThread->ctx, start, 0);
   printf("%lu : \n",(unsigned long int)&newThread->ctx);
  //setcontext(&newThread.ctx);
  return -1;
}

void yield(){
  if(numThreads>1){
  if(threadManager.ready.first!=NULL) {
    thread_t *currentJob=getRunning();
    addToReady(currentJob);
    thread_t *newJob=getFromReady();
    addToRunning(newJob); //här ballade det ur
  }
  }
}

void  done(){

  thread_t*currentJob=getRunning();
    addToTerminated(currentJob);
    thread_t *jobFromWaiting=getFromWaiting();
    
    while (jobFromWaiting!=NULL) { //we never enter this loop
      addToReady(jobFromWaiting);
      jobFromWaiting->doneCaller=currentJob->tid;
      jobFromWaiting=getFromWaiting();;
    }
    free(currentJob);
    thread_t *nextJob=getFromReady();
    addToRunning(nextJob);

}

tid_t join() {
   thread_t *currentJob=getRunning();
   addToWaiting(currentJob);
   thread_t *newJob=getFromReady();
   addToRunning(newJob);
   return currentJob->doneCaller;
   
}

