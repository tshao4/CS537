#ifndef __tester_h__
#define __tester_h__

#include <pthread.h>  
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>

#include <unistd.h>


// function declarations
void  inits();
void  start_threads();
void* thread_code(void *arg);
void  myerror(char *arg);


// Describe what a thread should do
// For example, if the 0th thread should
// increment 200 times, the cjd will look like:
// cjd { tid = 0, type = 1, how_many_times = 200 }
// with this example, we know the final expected value
#define DECREMENT 0
#define INCREMENT 1
struct counter_job_desc
{
    int tid;  // tid who will do this job
    int type; // increment/decrement?
    int how_many_times; // how many times?
};

#endif



