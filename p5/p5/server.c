#include "cs537.h"
#include "request.h"
#include <assert.h>

// 
// server.c: A very, very simple web server
//
// To run:
//  server <portnum (above 2000)> <num_threads> <buffer_size> <sch_p> <N>
//
// Repeatedly handles HTTP requests sent to this port number.
// Most of the work is done within routines written in request.c
//

int num_requests = 0;
int done_requests = 0;
int epoch = 0;
int N;
request_t * buffer;
int * length;
thread_t * threads;
int time_offset;

int sch_p = -1;
int num_threads;
int buffer_size;

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t empty = PTHREAD_COND_INITIALIZER;
pthread_cond_t full = PTHREAD_COND_INITIALIZER;

// CS537: TODO make it parse the new arguments too
void getargs(int *port, int argc, char *argv[])
{
    if (argc != 5 && argc != 6) {
        fprintf(stderr, "Usage: %s <port> <num_threads> <buffer_size> <sch_p> <N>\n",
                argv[0]);
        exit(1);
    }
    
    *port = atoi(argv[1]);
    num_threads = atoi(argv[2]);
    buffer_size = atoi(argv[3]);
    
    if (num_threads <= 0 || buffer_size <= 0) {
        fprintf(stderr, "Usage: %s <port> <num_threads> <buffer_size> <sch_p> <N>\n",
                argv[0]);
        fprintf(stderr, "Non-positive num_threads or buffer_size\n");
        exit(1);
    }
    
    if (strcasecmp(argv[4], "FIFO") == 0) {
        sch_p = FIFO;
    }else if (strcasecmp(argv[4], "SFF") == 0){
        sch_p = SFF;
    }else if (strcasecmp(argv[4], "SFF-BS") == 0){
		if (argc != 6){
            fprintf(stderr, "Usage: %s <port> <num_threads> <buffer_size> <sch_p> <N>\n",
                    argv[0]);
			exit(1);
		}
        sch_p = SFF_BS;
        N = atoi(argv[5]);
        if (N <= 0) {
            fprintf(stderr, "Usage: %s <port> <num_threads> <buffer_size> <sch_p> <N>\n",
                    argv[0]);
            fprintf(stderr, "Non-positive N\n");
			exit(1);
        }
    }else{
        printf("Bad schedule policy\n");
        exit(1);
    }
}

int index_get(int sch){
    
    static int use = 0;
    int tmp, i, u, size_c;
    
    switch (sch) {
        case FIFO:
            
            tmp = use;
            use = (use+1) % buffer_size;
            return tmp;
            
            break;
        case SFF:
            
            i = 0;
            while( !length[i] ){
                i++;
            }
            
            u = i;
            size_c = length[i];
            
            for( i = i + 1; i < buffer_size; i++ ) {
                if( length[i] > 0 && length[i] < size_c ) {
                    u = i;
                    size_c = length[i];
                }
            }
            return u;
            
            break;
        case SFF_BS:
            
            i = 0;
            while( !length[i] || buffer[i].request_id / N > epoch ){
                i++;
            }
            
            u = i;
            size_c = length[i];
            
            for( i = i + 1; i < buffer_size; i++ ) {
                if( length[i] > 0 && length[i] < size_c && buffer[i].request_id / N <= epoch ) {
                    u = i;
                    size_c = length[i];
                }
            }
            return u;
            break;
        default:
            unix_error("this scheduling policy is not implemented\n");
            exit(1);
            break;
    }
}

int index_fill(int sch){
    
    static int fill = 0;
    int tmp, i;
    
    switch (sch) {
        case FIFO:
            
            tmp = fill;
            fill = (fill + 1) % buffer_size;
            return tmp;
            
            break;
        case SFF:
        case SFF_BS:
            
            i = 0;
            while(length[i] ){
                i++;
            }
            return i;
            
            break;
        default:
            unix_error("this scheduling policy is not implemented\n");
            exit(1);
            break;
    }
}

void add(request_t request){
    
    int fill = index_fill(sch_p);
    
    request = requestParse(request);
    
    if( request.is_static == -1 )
    {
        return;
    }
    
    buffer[fill] = request;
    length[fill] = request.file_size;
    num_requests++;
}

request_t get(){
    
    int use = index_get(sch_p);
    
    request_t tmp = buffer[use];
    length[use] = 0;
    num_requests--;
    return tmp;
}

int get_time(){
    struct timeval t;
    int rc = gettimeofday(&t,NULL);
    assert(rc==0);
    return 1e3*(t.tv_sec - time_offset)+ (int) (t.tv_usec / 1e3);
}

int get_time_offset(){
	struct timeval t;
    int rc = gettimeofday(&t, NULL);
	assert(rc==0);
	return t.tv_sec;
}

void * consumer(){
    
    int stat_id = 0;
    
    while (1) {
        if (pthread_equal(pthread_self(), threads[stat_id].id) != 0) {
            break;
        }
        stat_id = (stat_id+1) % num_threads;
    }
    threads[stat_id].Stat_thread_id = stat_id;
    
    while (1) {
        pthread_mutex_lock(&mutex);
        while (num_requests == 0) {
            pthread_cond_wait(&full, &mutex);
        }
        
        request_t cur_request = get();
        done_requests++;
        
        if (sch_p == SFF_BS && done_requests % N == 0) {
            epoch++;
        }
        
        if (cur_request.is_static) {
            int i;
            for (i = 0; i < buffer_size; i++){
                if (length[i] > 0) {
                    buffer[i].Stat_req_age++;
                }
            }
        }
        
        pthread_cond_signal(&empty);
        pthread_mutex_unlock(&mutex);
        
        int connfd = cur_request.connfd;
        double time_start = get_time();
        cur_request.Stat_req_dispatch = time_start - cur_request.Stat_req_arrival;
        requestHandle(cur_request, &threads[stat_id]);
        Close(connfd);
    }
}


int main(int argc, char *argv[]) {
    
    int port, listenfd, connfd, clientlen, i;
    int cur_req_num = 0;
    struct sockaddr_in clientaddr;
    time_offset = get_time_offset();

    getargs(&port, argc, argv);
    
    buffer = (request_t *)malloc(sizeof(request_t) * buffer_size);
    if (buffer == NULL) { 
		unix_error("malloc error\n"); 
	}
    
    length = (int *)malloc(sizeof(int) * buffer_size);
    if (length == NULL) { 
		unix_error("malloc error\n"); 
	}
    for (i = 0; i <= buffer_size; i++ ){
        length[i] = 0;
    }
    
    threads = (thread_t *) malloc(sizeof(thread_t) * num_threads);
    if (threads == NULL) { 
		unix_error("malloc error\n"); 
	}
    
    pthread_cond_init( &empty, NULL);
    pthread_cond_init( &full,  NULL);
    pthread_mutex_init(&mutex, NULL);
    for( i = 0; i < num_threads; i++){
        threads[i].Stat_thread_count   = 0;
        threads[i].Stat_thread_static  = 0;
        threads[i].Stat_thread_dynamic = 0;
        
        pthread_create(&(threads[i].id), NULL, consumer, NULL);
    }
    listenfd = Open_listenfd(port);
    
    while (1) {
        
        clientlen = sizeof(clientaddr);
        connfd = Accept(listenfd, (SA *)&clientaddr, (socklen_t *) &clientlen);
        
        request_t cur_request;
        cur_request.connfd = connfd;
        cur_request.Stat_req_arrival = get_time();
        cur_request.Stat_req_age = 0;
        cur_request.request_id = cur_req_num;
        cur_req_num++;
        
        pthread_mutex_lock(&mutex);
        while(num_requests == buffer_size ){
            pthread_cond_wait(&empty, &mutex);
        }
        add(cur_request);
        
        pthread_cond_signal(&full);
        pthread_mutex_unlock(&mutex);
    }
}