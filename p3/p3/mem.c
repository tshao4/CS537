//
//  mem.c
//  p3
//
//  Created by Terry Shao on 2/26/14.
//  Copyright (c) 2014 Terry Shao. All rights reserved.
//

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include "mem.h"

#define FREE_PATTERN 0xDEADBEEF
#define PAD_PATTERN 0xABCDDCBA
#define PAD_SZ 64
#define ROUND_BYTES 4

#pragma pack(push, 1)
typedef struct node{
    
    int size;
    struct node * next;
}Node;
#pragma pack(pop)

// Global varibles
int m_error;
void * ptr = NULL;
Node * head;
int SZ_NODE = sizeof(Node);
int memSZ;
int debugMode = 0;

void writeFreePattern(Node * toWrite) {
    
    int i;
    for (i = 0; i < -toWrite->size + PAD_SZ * 2; i += sizeof(FREE_PATTERN)) {
        *(unsigned int *)((char *)toWrite + SZ_NODE + i) = FREE_PATTERN;
    }
}

void writePadPattern(Node * toWrite) {
    
    int i;
    for (i = 0; i < PAD_SZ; i += sizeof(PAD_PATTERN)) {
        *(unsigned int *)((char *)toWrite + SZ_NODE + i) = PAD_PATTERN;
        *(unsigned int *)((char *)toWrite + SZ_NODE + i+ PAD_SZ + toWrite->size) = PAD_PATTERN;

    }
}

int checkFreePattern(Node * toCheck) {
    
    int i;
    for (i = 0; i < -toCheck->size + PAD_SZ * 2; i += sizeof(FREE_PATTERN)) {
        if (*(unsigned int *)((char *)toCheck + SZ_NODE + i) != FREE_PATTERN) {
            return -1;
        }
    }
    return 0;
}

int checkPadPattern(Node * toCheck) {
    
    int i;
    for (i = 0; i < PAD_SZ; i += sizeof(PAD_PATTERN)) {
        if (*(unsigned int *)((char *)toCheck + SZ_NODE + i) != PAD_PATTERN) {
            return -1;
        }
        if (*(unsigned int *)((char *)toCheck + SZ_NODE + i + PAD_SZ + toCheck->size) != PAD_PATTERN) {
            return -1;
        }
    }
    return 0;
}

/**
 *	Initialize a chunk of memory for later allocation.
 *
 *	@param sizeOfRegion the size of memory to be initialized
 *	@param debug specifies debug mode
 */
int Mem_Init(int sizeOfRegion, int debug) {
    
    if (debug < 0 || debug > 1) {
        m_error = E_BAD_ARGS;
        return -1;
    }
    
    debugMode = debug;
    
    // Check if Mem_Init is called more than once then failure
    if (ptr) {
        
        m_error = E_BAD_ARGS;
        return -1;
    }
    
    // Check if sizeOfRegion is valid ( > 0? or failure )
    if (sizeOfRegion <= 0) {
        
        m_error = E_BAD_ARGS;
        return -1;
    }
    
    // Round sizeofRegion using getpagesize()
    memSZ = sizeOfRegion;
    size_t pageSZ = getpagesize();
    if (sizeOfRegion%pageSZ != 0) {
        memSZ += pageSZ - (memSZ % pageSZ);
    }
    
    // Round Node size
    if (sizeof(Node)%ROUND_BYTES != 0) {
        SZ_NODE += ROUND_BYTES - sizeof(Node) % ROUND_BYTES;
    }
    
    // Request memory using mmap()
    int fd = open("/dev/zero", O_RDWR);
    if (!fd) {
        
        // perror("fd open\n");
        return -1;
    }
    ptr = mmap(NULL, memSZ, PROT_READ | PROT_WRITE, MAP_PRIVATE, fd, 0);
    if (ptr == MAP_FAILED || ptr == NULL) {
        // perror("mmap\n");
        m_error = E_BAD_ARGS;
        return -1;
    }
    
    close(fd);
    
    head = (Node *)ptr;
    head->next = NULL;
    
    if (debugMode) {
        
        head->size = -(memSZ - SZ_NODE - PAD_SZ * 2);
        writeFreePattern(head);
    }else{
        
        head->size = -(memSZ - SZ_NODE);
    }
    
    // printf("Aquired Memory: %d\n", memSZ);
    
    // On success
    return 0;
}

void *Mem_Alloc(int size) {
    
    if (size <= 0) {
        return NULL;
    }
    int actSZ = size;
    if (actSZ % ROUND_BYTES != 0) {
        actSZ += ROUND_BYTES - (actSZ % ROUND_BYTES);
    }
    
    Node * p = head;
    
    if (debugMode) {
        
        Node * q = head;
        int set = 0;
        
        while (q != NULL) {
            if (set == 0 && q->size <= -actSZ) {
                p = q;
                set = 1;
            }
            
            // Check pattern
            if (q->size < 0) {
                if (checkFreePattern(q) == -1) {
                    m_error = E_CORRUPT_FREESPACE;
                    return NULL;
                }
            }else{
                if (checkPadPattern(q) == -1) {
                    m_error = E_CORRUPT_FREESPACE;
                    return NULL;
                }
            }
            
            q = q->next;
        }
        if (!set) {
            m_error = E_NO_SPACE;
            return NULL;
        }
        
        Node * tmp;
        
        if (p->size + actSZ + SZ_NODE + PAD_SZ * 2 < 0) {
            tmp = (Node *)((void *)p + SZ_NODE + actSZ + PAD_SZ * 2);
            tmp->size = p->size + actSZ + SZ_NODE + PAD_SZ * 2;
            tmp->next = p->next;
            p->next = tmp;
            p->size = actSZ;
            writeFreePattern(tmp);
        }else{
            p->size = -p->size;
        }
        writePadPattern(p);
        // printf("Mem_Alloc returns %p\n", (void *)p + SZ_NODE + PAD_SZ);
        return (void *)p + SZ_NODE + PAD_SZ;
    }else{
        
        while (p != NULL) {
            if (p->size <= -actSZ) {
                break;
            }
            p = p->next;
        }
        if (p == NULL) {
            m_error = E_NO_SPACE;
            return NULL;
        }
        
        Node * tmp;
        
        if (p->size + actSZ + SZ_NODE < 0) {
            tmp = (Node *)((void *)p + SZ_NODE + actSZ);
            tmp->size = p->size + actSZ + SZ_NODE;
            tmp->next = p->next;
            p->next = tmp;
            p->size = actSZ;
        }else{
            p->size = -p->size;
        }
        // printf("Mem_Alloc returns %p\n", (void *)p + SZ_NODE);
        return (void *)p + SZ_NODE;
    }
}

int Mem_Free(void *ptr) {
    
    if (ptr == NULL) {
        return 0;
    }
    
    char isHead = 0;
    
    Node * p = head;
    
    if (debugMode) {
        
        if ((void *)p + SZ_NODE + PAD_SZ != ptr) {
        
            while (p != NULL) {
                if ((void *)p->next + SZ_NODE + PAD_SZ == ptr) {
                    break;
                }
                p = p->next;
            }
        }
        else isHead = 1;
        
        if (p == NULL) {
            m_error = E_BAD_POINTER;
            return -1;
        }else if (isHead){
            if(checkPadPattern(p) == -1){
                m_error = E_PADDING_OVERWRITTEN;
                return -1;
            }
        }else{
            if(checkPadPattern(p->next) == -1){
                m_error = E_PADDING_OVERWRITTEN;
                return -1;
            }
        }
        
        if (isHead) {
            
            p->size = -p->size;
            
            if (p->next != NULL) {
                if (p->next->size < 0){
                    p->size += p->next->size - SZ_NODE - PAD_SZ * 2;
                    p->next = p->next->next;
                }
            }
           
            writeFreePattern(p);
            
        }else{
            p->next->size = -p->next->size;
            
            if (p->next->next != NULL) {
                if (p->next->next->size < 0) {
                    p->next->size += p->next->next->size - SZ_NODE - PAD_SZ * 2;
                    p->next->next = p->next->next->next;
                }
            }
            
            if (p->size < 0) {
                p->size += p->next->size - SZ_NODE - PAD_SZ * 2;
                p->next = p->next->next;
                writeFreePattern(p);
            }else{
                writeFreePattern(p->next);
            }
            
        }
        
    }else{
        
        if ((void *)p + SZ_NODE != ptr) {
            
            while (p != NULL) {
                if ((void *)p->next + SZ_NODE == ptr) {
                    break;
                }
                p = p->next;
            }
        }else isHead = 1;
        
        if (isHead) {
            
            p->size = -p->size;
            
            if (p->next != NULL) {
                if (p->next->size < 0){
                    p->size += p->next->size - SZ_NODE;
                    p->next = p->next->next;
                }
            }
        }else{
            p->next->size = -p->next->size;
            
            if (p->next->next != NULL) {
                if (p->next->next->size < 0) {
                    p->next->size += p->next->next->size - SZ_NODE;
                    p->next->next = p->next->next->next;
                }
            }
            
            if (p->size < 0) {
                p->size += p->next->size - SZ_NODE;
                p->next = p->next->next;
            }
        }
    }

    return 0;
}

void Mem_Dump() {
    
    int i;

    for (i = 0; i < memSZ; i += sizeof(FREE_PATTERN)) {
        printf("%08x", *(unsigned int*)((char*)ptr + i));
    }
    
    printf("\n");
    
    Node * tmp = head;
    int block = 0;
    
    if(debugMode){
        
        while (tmp != NULL) {
            
            if (tmp->size < 0) {
                printf("Block %d:\n\tHeader:\t%p\n\tBase:\t%p\n\tSize:\t%d\n\tIsFree: YES\n",
                       block, tmp, (void*)tmp + SZ_NODE + PAD_SZ, -tmp->size);
            }else{
                printf("Block %d:\n\tHeader:\t%p\n\tBase:\t%p\n\tSize:\t%d\n\tIsFree: NO\n",
                       block, tmp, (void*)tmp + SZ_NODE + PAD_SZ, tmp->size);
            }
            block++;
            
            tmp = tmp->next;
        }
    }else{
        
        while (tmp != NULL) {
            if (tmp->size < 0) {
                printf("Block %d:\n\tHeader:\t%p\n\tBase:\t%p\n\tSize:\t%d\n\tIsFree: YES\n",
                       block, tmp, (void*)tmp + SZ_NODE, -tmp->size);
            }else{
                printf("Block %d:\n\tHeader:\t%p\n\tBase:\t%p\n\tSize:\t%d\n\tIsFree: NO\n",
                       block, tmp, (void*)tmp + SZ_NODE, tmp->size);
            }
            block++;
            
            tmp = tmp->next;
        }

    }
}

int main()
{
    void * a, * b, * c, * d;
    Mem_Init(4096, 1);
    a = Mem_Alloc(123);
    b = Mem_Alloc(234);
    c = Mem_Alloc(345);
    d = Mem_Alloc(456);
    Mem_Dump();
    
    return 0;
}