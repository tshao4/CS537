#include "testcount.h"
#include "counter.h"


// play around with these values
#define MAX_NUM_THREADS  100
#define MAX_INC_DEC_COUNTS 10000


counter_t c;

// ******************************** let's begin

int main() 
{
    inits();
    start_threads();
    return 0;
}

void inits()
{
    // init srand
    struct timeval tv;
    Counter_Init(&c, 0);
    gettimeofday(& tv, NULL);
    srand(tv.tv_usec);

}

void start_threads()
{

    int i, rv;
    int num_threads = rand() % MAX_NUM_THREADS + 1;
    pthread_t *workers = malloc(sizeof(pthread_t) * num_threads);
    struct counter_job_desc* jds = 
	malloc(num_threads* sizeof(struct counter_job_desc));
    int final_expected_value = 0;
    
    printf("\n  Creating %d num_threads ... \n\n", num_threads);


    // setup jobs
    for (i = 0; i < num_threads; i++) {
	jds[i].tid = i;
	jds[i].type = rand() % 2;
	jds[i].how_many_times = rand() % MAX_INC_DEC_COUNTS;
	
	printf("  Thread-%04d will ", jds[i].tid);
	if      (jds[i].type == INCREMENT) printf("++ ");
	else if (jds[i].type == DECREMENT) printf("-- ");
	printf("%6d times ...\n", jds[i].how_many_times);	

	
	if (jds[i].type == INCREMENT) 
	    final_expected_value += jds[i].how_many_times;
	else if (jds[i].type == DECREMENT) 
	    final_expected_value -= jds[i].how_many_times;
    }


    // run threads
    for (i = 0; i < num_threads; i++) {
	if ((rv = pthread_create(& workers[i], NULL, thread_code, & jds[i])))
	    myerror("cannot create worker thread");
    }
    
    for (i = 0; i < num_threads; i++)
        pthread_join(workers[i], NULL);
    printf("\n  REPORT: \n");
    printf("    Expected value : %10d \n", final_expected_value);
    printf("    Library  value : %10d \n", Counter_GetValue(&c));
    
    if (final_expected_value == Counter_GetValue(&c)) 
	printf("    RESULT: Same! Congrats! \n\n");
    else 
    	printf("    RESULT: Not same! Arghhh! \n\n");
    
}

void* thread_code(void *arg)
{
    struct counter_job_desc *jd = (struct counter_job_desc *) arg;
    int i = 0;
    for (i = 0; i < jd->how_many_times; i++) {
      //      printf(" thread %d is running \n", jd->tid);
	if (jd->type == DECREMENT) 
	    Counter_Decrement(&c);
	else if (jd->type == INCREMENT)
	    Counter_Increment(&c);
	else 
	    myerror(" type not inc or dec \n");
    }
    
    return NULL;
}


void myerror(char *arg)
{
    printf("  ERROR : %s \n", arg);
    exit(1);
}
