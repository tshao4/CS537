//
//  main.c
//  test_queue
//
//  Created by Terry Shao on 4/10/14.
//  Copyright (c) 2014 Terry Shao. All rights reserved.
//

#include <stdio.h>
#include <stdlib.h>
#include "queue.h"

int main(int argc, const char * argv[])
{

    queue_t q;
    Queue_Init(&q);
    
    for (int i = 0; i < 10; i++) {
        int tmp = rand();
        Queue_Push(&q, tmp);
    }

    for (int i = 10; i > 0; i--) {
        printf("%d\n", Queue_Pop(&q));
    }
    
    return 0;
}

