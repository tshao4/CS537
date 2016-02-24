//
//  test.c
//  p3
//
//  Created by Terry Shao on 3/7/14.
//  Copyright (c) 2014 Terry Shao. All rights reserved.
//

#include <stdio.h>
#include <stdlib.h>
#include "mem.h"

int main(int argc, const char * argv[]) {
    
    if (Mem_Init(3, 1) == -1) {
        perror("Mem_Init error");
        exit(1);
    }
    
    void * pp = Mem_Alloc(1000);
    
    void * pq = Mem_Alloc(2000);
    
    void * qq = Mem_Alloc(548);
    
    Mem_Free(pp);
    
    Mem_Free(pq);
    
    Mem_Free(qq);
    
    Mem_Dump();
    
    return 0;
}
