#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/time.h>
#include "testlist.h"
#include "pthread.h"
#include "spin.h"
#include "counter.h"
#include "list.h"
#include "hash.h"

counter_t ccc;
list_t lt;
hash_t hs;

void* inc(){
    
    int i;
    
    for (i = 0; i < 1000000; i++) {
        Counter_Increment(&ccc);
    }
    return NULL;
}

void* l_insert(){
    
    int i;
    
    for (i = 0; i < 1000000; i++) {
        List_Insert(&lt, &i, i);
    }
    return NULL;
}

void* h_insert(){
    
    int i;
    
    for (i = 0; i < 1000000; i++) {
        Hash_Insert(&hs, &i, i);
    }
    return NULL;
}

/*
int main(){
    
    pthread_t t[20];
    int i, j, k, l, m;
    struct  timeval start;
    struct  timeval end;
    
    double sum = 0;
    
    //////////////////////////////////////////////////////////////////////
    //                           Counter Test                           //
    //////////////////////////////////////////////////////////////////////
     
    for (k = 1; k < 21; k++) {
        for (j = 0; j < 10; j++) {
            
            Counter_Init(&ccc, 0);
            gettimeofday(&start, NULL);
            
            for (i = 0; i < k; i++) {
                pthread_create(&t[i], NULL, inc, NULL);
            }
            
            for (i = 0; i < k; i++) {
                pthread_join(t[i], NULL);
            }
            
            gettimeofday(&end, NULL);
            sum += (end.tv_sec-start.tv_sec) + (end.tv_usec - start.tv_usec)/1000000.0;
        }
        
        printf("#Threads:%d\tTime:%lf\tSum:%d\n", k, sum/10, ccc.value);
        sum = 0;
    }

    
    //////////////////////////////////////////////////////////////////////
    //                       Linked List Test                           //
    //////////////////////////////////////////////////////////////////////
    
    /*
     for (k = 1; k < 21; k++) {
     for (j = 0; j < 10; j++) {
     
     List_Init(&lt);
     gettimeofday(&start,NULL);
     
     for (i = 0; i < k; i++) {
     pthread_create(&t[i], NULL, l_insert, NULL);
     }
     
     for (i = 0; i < k; i++) {
     pthread_join(t[i], NULL);
     }
     
     gettimeofday(&end,NULL);
     sum += (end.tv_sec-start.tv_sec) + (end.tv_usec - start.tv_usec)/1000000.0;
     
     node_t* tmp = lt.head;
     for (l = 0; l < k * 1000000; l++) {
     node_t* next = tmp->next;
     free(tmp);
     tmp = next;
     }
     
     }
     
     printf("#Threads:%d\t\tTime:%lf\n", k, sum/10);
     sum = 0;
     }
     */
    
    //////////////////////////////////////////////////////////////////////
    //                         Hash Table Test                          //
    //////////////////////////////////////////////////////////////////////
    
    /*
     for (k = 1; k < 21; k++) {
     for (j = 0; j < 10; j++) {
     
     Hash_Init(&hs, 100);
     gettimeofday(&start,NULL);
     
     for (i = 0; i < k; i++) {
     pthread_create(&t[i], NULL, h_insert, NULL);
     }
     
     for (i = 0; i < k; i++) {
     pthread_join(t[i], NULL);
     }
     
     gettimeofday(&end,NULL);
     sum += (end.tv_sec-start.tv_sec) + (end.tv_usec - start.tv_usec)/1000000.0;
     
     for (m = 0; m < 100; m++) {
     
     node_t* tmp = hs.lists[m].head;
     for (l = 0; l < k * 10000; l++) {
     node_t* next = tmp->next;
     free(tmp);
     tmp = next;
     }
     }
     }
     
     printf("#Threads:%d\t\tTime:%lf\n", k, sum/10);
     sum = 0;
     }
    */
    
    //////////////////////////////////////////////////////////////////////
    //                     Hash Table Scaling Test                      //
    //////////////////////////////////////////////////////////////////////
    
    /*
    for (k = 1; k < 21; k++) {
        for (j = 0; j < 10; j++) {
            
            Hash_Init(&hs, k);
            gettimeofday(&start,NULL);
            
            for (i = 0; i < 20; i++) {
                pthread_create(&t[i], NULL, h_insert, NULL);
            }
            
            for (i = 0; i < 20; i++) {
                pthread_join(t[i], NULL);
            }
            
            gettimeofday(&end,NULL);
            sum += (end.tv_sec-start.tv_sec) + (end.tv_usec - start.tv_usec)/1000000.0;
            
            for (m = 0; m < k; m++) {
                
                node_t* tmp = hs.lists[m].head;
                while (tmp != NULL){
                    node_t* next = tmp->next;
                    free(tmp);
                    tmp = next;
                }
            }
        }
        
        printf("#Buckets:%d\t\tTime:%lf\n", k, sum/10);
        sum = 0;
    }
    *//*
    
    return 0;
}*/


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
