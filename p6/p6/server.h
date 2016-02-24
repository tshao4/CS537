#ifndef __SERVER_H__
#define __SERVER_H__

#include "mfs.h"
#include <stdbool.h>

#define MFS_BLOCK_NUMS (4096)
#define MFS_PTR_NUMS 10

typedef struct Inode{
	int type;
	int size;
	int blocks;
	int ptr[MFS_PTR_NUMS];
} Inode_t;


typedef struct BitMap{
	bool bits[MFS_BLOCK_NUMS];
} Bitmap_t;

typedef struct Block{
	char data[MFS_BLOCK_SIZE];
} Block_t;

int Image_Init(const char * filename);
void Data_Init();
void Data_Write();
void add_block(int inum);

int Server_LookUp(int pinum, char *name);
int Server_Stat(int inum, MFS_Stat_t *m);
int Server_Write(int inum, char *buffer, int block);
int Server_Read(int inum, char *buffer, int block);
int Server_Creat(int pinum, int type, char *name);
int Server_Unlink(int pinum, char *name);

int Add_Entry(int pinum, int inum, char *name, Inode_t *inode_table, Block_t *data_region);
int Remove_Entry(int pinum, int inum, char *name, Inode_t *inode_table, Block_t *data_region);
#endif 
