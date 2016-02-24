/******************************** /
\ testhash3.c                     \
/                                 /
\ Multi-threaded test with 20-40  \
/ hash buckets, 2000-4000 ops per /
\ thread, repeated 100 times.     \
/*********************************/

#include "testhash.h"
#include "hash.h"

#define NUM_OF_THREADS 10
#define MIN_NUM_OF_BUCKETS 20
#define ADDITIONAL_NUM_OF_BUCKETS 20
#define NUM_OPS_PER_THREAD 2000

#define NUM_OF_LOOPS 10
hash_t h;

int num_threads = 0;

int main()
{
  printf("\n   Start testing, please wait...\n");
  int i;
  for(i = 0; i < NUM_OF_LOOPS; i++) {
    // Use a constant seed so that the test uses the same number of threads
    // and operations for everybody.
    srand(i);
    Hash_Init(&h, rand() % ADDITIONAL_NUM_OF_BUCKETS + MIN_NUM_OF_BUCKETS);
    start_threads(i);
  }
  // If the program got here without failing and quitting, 
  // then the test passed.
  printf("\n  Test succeeded \n");
  printf("    Congrats! you are an expert! ... \n");
  return 0;
}

void start_threads(int n)
{
  int i, rv;
  pthread_t *workers;
  int *mytid;
  struct hash_job_desc* jds;

  int final_expected_count = 0;
  int final_expected_ic = 0;
  int final_expected_lc = 0;
  int final_expected_dc = 0;

  //  num_threads = rand() % ADDITIONAL_NUM_OF_THREADS + MIN_NUM_OF_THREADS;
  num_threads = NUM_OF_THREADS;
  workers = malloc(sizeof(pthread_t) * num_threads);
  jds = malloc(num_threads* sizeof(struct hash_job_desc));

  // run threads
  for (i = 0; i < num_threads; i++) {

    jds[i].tid = i;
    jds[i].ic = 0;
    jds[i].lc = 0;
    jds[i].dc = 0;

    if ((rv = pthread_create(& workers[i], NULL, thread_code, & jds[i])))
      myerror("cannot create worker thread");
  }

  for (i = 0; i < num_threads; i++) {
    pthread_join(workers[i], NULL);
  }

  for (i = 0; i < num_threads; i++) {
    final_expected_ic += jds[i].ic;
    final_expected_dc += jds[i].dc;
    final_expected_lc += jds[i].lc;
  }

  printf("\n  REPORT for %d iteration: \n", n);
  printf("    Count (ic / dc / lc) : %8d / %8d / %8d\n",
           final_expected_ic, final_expected_dc, final_expected_lc);

  free(jds);
  free(workers);
  if (final_expected_ic == final_expected_dc + final_expected_lc) {
    //    printf("    Congrats! you are an expert! ... \n");
    printf("    Pass iteration %d. \n", n);
    if (n < NUM_OF_LOOPS -1) 
      printf("    Don't be too happy. More to come :D\n");
  }
  else {
    printf("    ##############################################\n");
    printf("    #Ooopss! cannot have a party tonight! ... \n");
    printf("    #Test fail iteration %d\n", n);
    printf("    #Expected: ic = dc + lc\n");
    printf("    ##############################################\n");
    exit(-1);

  }
}

void* thread_code(void *arg)
{
  struct hash_job_desc *jd = (struct hash_job_desc *) arg;
  int numOfOps = NUM_OPS_PER_THREAD;
  int i = 0;
  int *addlist = malloc(sizeof(int) * numOfOps);
  if (addlist == NULL) {
    printf(" cannot allocate more ..., reduce space \n");
    exit(-1);
  }

  while (i < numOfOps) {
    //    addlist[i] = rand() % (numOfOps * num_threads);
    // since key is unique...
    addlist[i] = i + (numOfOps * jd->tid);
    Hash_Insert(&h, addlist + i, addlist[i]);
    (jd->ic)++;
    i++;
  }

  i = 0;
  while (i < numOfOps) {
    if (i % 3 == 0) {
      Hash_Delete(&h, addlist[i]);
      (jd->dc)++;
    }
    i++;
  }

  i = 0;
  while (i < numOfOps) {
    if (Hash_Lookup(&h, addlist[i]) != NULL)
      (jd->lc)++;
    i++;
  }


  //  printf("  END   Thread-%04d with %6d ic, %6d dc, %6d lc\n",
  //         jd->tid, jd->ic, jd->dc, jd->lc);

  free(addlist);

  return NULL;
}


void myerror(char *arg)
{
  printf("Error: %s\n", arg);
  printf("Test failed.\n");
  exit(-1);
}

