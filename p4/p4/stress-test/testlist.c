#include "testlist.h"
#include "list.h"


// play around with these values

#define NUM_OPS_PER_THREAD 100
#define NUM_OF_THREADS 20
#define NUM_OF_LOOPS 100

int num_threads = 0;

// ******************************** let's begin

list_t l;

int main()
{
  printf("\n   Start testing, please wait, it might take a long time ...\n");
  int i;
  for (i = 0; i < NUM_OF_LOOPS; i++) {
    List_Init(&l);
    start_threads(i);
  }

  // should see this message if everything fine
  printf("\n    YOU are EXPERT! !\n");
  return 0;
}


void start_threads(int n)
{
  int i, rv;
  pthread_t *workers;
  int *mytid;
  struct job_desc* jds;

  int final_expected_count = 0;
  int final_expected_lc = 0;
  int final_expected_ic = 0;
  int final_expected_dc = 0;

  num_threads = NUM_OF_THREADS;
  workers = malloc(sizeof(pthread_t) * num_threads);
  jds = malloc(num_threads* sizeof(struct job_desc));

  //  printf("\n   Creating %d threads ... \n\n", num_threads);

  // run threads
  for (i = 0; i < num_threads; i++) {

    jds[i].tid = i;
    jds[i].count = 0; // will be updated by each thread
    jds[i].ic = 0;
    jds[i].dc = 0;
    jds[i].lc = 0;

    if ((rv = pthread_create(& workers[i], NULL, thread_code, & jds[i])))
      myerror("cannot create worker thread");
  }

  for (i = 0; i < num_threads; i++)
    pthread_join(workers[i], NULL);

  for (i = 0; i < num_threads; i++) {
    final_expected_ic += jds[i].ic;
    final_expected_dc += jds[i].dc;
    final_expected_lc += jds[i].lc;
  }

  printf("\n  REPORT for %d iteration: \n", n);
  printf("    Count(ic / dc / lc) : %8d / %8d / %8d\n",
	 final_expected_ic, final_expected_dc, final_expected_lc);

  free(workers);
  free(jds);
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
  struct job_desc *jd = (struct job_desc *) arg;
  int numOfOps = NUM_OPS_PER_THREAD;
  int i = 0;
  int *addlist = malloc(sizeof(int) * numOfOps);
  if (addlist == NULL) {
    printf(" cannot allocate more ..., reduce space \n");
    exit(-1);
  }

  //    printf("  START Thread-%04d will execute %6d ops \n", jd->tid,
  //       numOfOps);

  while (i < numOfOps) {
    addlist[i] = i + (jd->tid * NUM_OPS_PER_THREAD);
    List_Insert(&l, addlist + i, addlist[i]);
    (jd->ic)++;
    i++;
  }

  i = 0;
  while (i < numOfOps) {
    if (i % 3 == 0) {
      List_Delete(&l, addlist[i]);
      (jd->dc)++;
    }
    i++;
  }

  i = 0;
  while (i < numOfOps) {
    if (List_Lookup(&l, addlist[i]) != NULL)
      (jd->lc)++;
    i++;
  }

  //    printf("  END   Thread-%04d with %6d ic, %6d dc, %6d lc\n",
  //       jd->tid, jd->ic, jd->dc, jd->lc);
  free(addlist);
  return NULL;
}


void myerror(char *arg)
{
  printf("  ERROR : %s \n", arg);
  exit(-1);
}

