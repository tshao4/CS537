#ifndef __testhash_h__
#define __testhash_h__

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
void  simple_test();
void  fi(int x);
void  fr(int x);

void inc_expected_count();
void dec_expected_count();
void inc_expected_sum(int x);
void dec_expected_sum(int x);


// Describe what a thread should do
// For example, if the 0th thread should

struct hash_job_desc
{
    int tid;   // tid who will do this job
    int count; // successful count per thread
  int ic; // #insert operation
  int dc; // #delete operation
  int lc; // #successful lookup operation
};





#endif



