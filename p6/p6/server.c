#include <stdio.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include "udp.h"
#include "mfs.h"
#include "server.h"

int image_fd;
Bitmap_t Inode_BitMap;
Bitmap_t Data_BitMap;
Inode_t *inode_table;
Block_t *data_region;

int main(int argc, char *argv[])
{

	if(argc != 3){
		fprintf(stderr,"usage: server [portnum] [file-system-image]\n");
		fprintf(stderr,"you supplied %d args\n", argc);
		exit(1);
	}
	int portnum = atoi( argv[1] );
	if( ! (portnum > 0 ) ) {
		fprintf(stderr, "  portnum = %d;  this should be a pos number",portnum);
	}

	const int sd = UDP_Open(portnum);
	assert(sd > -1);

	inode_table = (Inode_t *)malloc(MFS_BLOCK_NUMS * sizeof(Inode_t));
	data_region = (Block_t *)malloc(MFS_BLOCK_NUMS * sizeof(Block_t));
	if(inode_table == NULL || data_region == NULL){
		fprintf(stderr, "malloc error\n");
		exit(-1);
	}

	if( (image_fd = open(argv[2],O_RDWR)) == -1){
		image_fd = Image_Init(argv[2]);
	}

	int rc = -1;
	struct sockaddr_in s;
	char buffer_read[ UDP_BUFFER_SIZE];
	char buffer_reply[UDP_BUFFER_SIZE];
	char buffer[BUFFER_SIZE];
	char * ptr = buffer_read;

    int block  = -1;
	int blocks = -1;
	int pinum  = -1;
	int inum   = -1;
	int size   = -1;
	int status = -1;
	int type   = -1;
	char * name   = NULL;

	int * iptr   = NULL;
	char * cptr  = NULL;
	MFS_Stat_t * mptr  = NULL;

	while (1) {

		block  = -1;
		blocks = -1;
		pinum  = -1;
		inum   = -1;
		size   = -1;
		status = -1;
		type   = -1;
		name   = NULL;
		iptr  = NULL;
		cptr  = NULL;

		rc = UDP_Read(sd, &s, buffer_read, UDP_BUFFER_SIZE);
		ptr = buffer_read;
		if (rc < 1) {
			rc = -1;
			continue;
		}

		iptr = (int*) ptr;
		int * op_id = iptr;
		iptr++;
		switch(*op_id)
		{
			case 0:
				printf("op_id == 0 \n");
				break;

			case 1:
				pinum = *( iptr );
				iptr++;
				cptr = (char*)iptr;
				name = cptr;

				inum = Server_LookUp(pinum, name );

				iptr = (int*) buffer_reply;
				*iptr = inum;

				break;


			case 2:


				inum = *( iptr );
				iptr++;
				mptr = (MFS_Stat_t*)iptr;

				status = Server_Stat(inum, mptr );

				iptr = (int*) buffer_reply;
				*iptr = status;

				iptr++;
				memcpy((void*) iptr, (void*)mptr, sizeof( MFS_Stat_t ) );

				break;

			case 3:

				inum = *( iptr );
				iptr++;
				cptr = (char*)iptr;
				memcpy( buffer, cptr, BUFFER_SIZE );
				cptr += BUFFER_SIZE;
				iptr = (int*) cptr;
				block = *iptr;

				status = Server_Write( inum, buffer, block );

				iptr = (int *) buffer_reply;
				*iptr = status;

				break;

			case 4:

				inum = *( iptr );
				iptr++;
				cptr = (char*)iptr;
				memcpy( buffer, cptr, BUFFER_SIZE );
				cptr += BUFFER_SIZE;
				iptr = (int*) cptr;
				block = *iptr;

				status = Server_Read( inum, buffer, block );

				iptr = (int *) buffer_reply;
				*iptr = status;
                iptr++;
                cptr = (char*) iptr;
                memcpy(cptr, buffer, BUFFER_SIZE);

				break;

			case 5:

				pinum = *( iptr );
				iptr++;
				type = *iptr;
				iptr++;
				name = (char*) iptr;

				status = Server_Creat( pinum, type, name );

				iptr = (int *) buffer_reply;
				*iptr = status;

				break;

			case 6:

				pinum  = *(iptr++);
				name   = (char*)(iptr);

				status = Server_Unlink( pinum, name );
				iptr  = (int*) buffer_reply;
				*iptr = status;

				break;

			default:
				fprintf(stderr, "bad function number %d\n", *op_id );
				exit(1);
		}
        
		rc = UDP_Write(sd, &s, buffer_reply, UDP_BUFFER_SIZE);

	}

	return 0;
}

void Set_Bit(Bitmap_t *map, int index){
	if(index < 0 || index >= MFS_BLOCK_NUMS){
		fprintf(stderr, "index error\n");
		exit(-1);
	}
	map->bits[index] = true;
}

void Unset_Bit(Bitmap_t *map, int index){
	if(index < 0 || index >= MFS_BLOCK_NUMS){
		fprintf(stderr, "index error\n");
		exit(-1);
	}
	map->bits[index] = false;
}


int First_Empty(Bitmap_t *map){
	int index;
	for(index = 0; index < MFS_BLOCK_NUMS; index++){
		if(map->bits[index] == false){
			break;
		}
	}

	if(index == MFS_BLOCK_NUMS){
		index = -1;
	}

	return index;
}

void BitMap_Init(Bitmap_t *map){
	int idx;
	for (idx = 0; idx < MFS_BLOCK_NUMS; idx ++){
		map->bits[idx] = false;
	}
}

void Inode_Init(Inode_t *inode){
	inode->type = MFS_REGULAR_FILE;
	inode->size = 0;
	inode->blocks = 0;
	int i;
	for(i = 0; i < MFS_PTR_NUMS; i++){
		inode->ptr[i] = -1;
	}
}

int Image_Init(const char * filename){
	int fd, i;
	if((fd = open(filename,O_RDWR|O_CREAT|O_TRUNC, S_IRUSR|S_IWUSR)) == -1){
		exit(-1);
	}

	BitMap_Init(&Inode_BitMap);
	BitMap_Init(&Data_BitMap);

	for(i = 0; i < MFS_BLOCK_NUMS; i++){
		Inode_Init(&inode_table[i]);
	}
    
	inode_table[0].type = MFS_DIRECTORY;
	inode_table[0].size = 2 * sizeof(MFS_DirEnt_t);
	inode_table[0].blocks = 1;
	inode_table[0].ptr[0] = 0;

	MFS_DirEnt_t *root_dot = (MFS_DirEnt_t *)data_region[0].data;
	MFS_DirEnt_t *root_dotdot = root_dot + 1;

	root_dot -> inum = 0;
	strcpy(root_dot -> name,".");

	root_dotdot -> inum = 0;
	strcpy(root_dotdot -> name, "..");

	int j;
	for(j = 2; j < MFS_BLOCK_SIZE / sizeof(MFS_DirEnt_t); j ++){
		(root_dot + j) -> inum = -1;
	}

	Set_Bit(&Inode_BitMap, 0);
	Set_Bit(&Data_BitMap,0);


	if(write(fd, &Inode_BitMap, sizeof(Bitmap_t)) != sizeof(Bitmap_t)){
		fprintf(stderr, "write error\n");
		exit(-1);
	}

	if(write(fd,&Data_BitMap, sizeof(Bitmap_t)) != sizeof(Bitmap_t)){
		fprintf(stderr,"write error\n");
		exit(-1);
	}

	if(write(fd,inode_table, MFS_BLOCK_NUMS*sizeof(Inode_t)) != MFS_BLOCK_NUMS*sizeof(Inode_t)){
		fprintf(stderr,"write error\n");
		exit(-1);
	}


	if(write(fd,data_region, MFS_BLOCK_NUMS*sizeof(Block_t)) != MFS_BLOCK_NUMS*sizeof(Block_t)){
		fprintf(stderr,"write error\n");
		exit(-1);
	}

	fsync(fd);

	return fd;

}

void Data_Init(){

	if(lseek(image_fd,0,SEEK_SET) != 0){
		fprintf(stderr,"lseek error\n");
		exit(-1);
	}

	if(read(image_fd, &Inode_BitMap, sizeof(Bitmap_t)) != sizeof(Bitmap_t)){
		fprintf(stderr,"read error\n");
		exit(-1);
	}

	if(read(image_fd, &Data_BitMap,sizeof(Bitmap_t)) != sizeof(Bitmap_t)){
		fprintf(stderr,"read error\n");
		exit(-1);
	}

	if(read(image_fd,inode_table, MFS_BLOCK_NUMS*sizeof(Inode_t)) != MFS_BLOCK_NUMS*sizeof(Inode_t)){
		fprintf(stderr,"read error\n");
		exit(-1);
	}

	if(read(image_fd,data_region, MFS_BLOCK_NUMS*sizeof(Block_t)) != MFS_BLOCK_NUMS*sizeof(Block_t)){
		fprintf(stderr,"read error\n");
		exit(-1);
	}

}


int Server_LookUp(int pinum, char *name){

	if(pinum < 0){
		return -1;
	}

	int idx;


	Data_Init();

	//return -2 if invalid pinum
	if(Inode_BitMap.bits[pinum] == false){
		return -1;
	}else if(inode_table[pinum].type != MFS_DIRECTORY){
		return -1;
	}

	int curr_blk_num;
	for(idx = 0; idx < MFS_PTR_NUMS; idx++){
		if((curr_blk_num = inode_table[pinum].ptr[idx]) == -1){
			continue;
		}

		int entries_in_curr_blk = MFS_BLOCK_SIZE / sizeof(MFS_DirEnt_t);

		MFS_DirEnt_t *entries = (MFS_DirEnt_t *)data_region[curr_blk_num].data;
		int j; 
		for (j = 0; j < entries_in_curr_blk; j++){
			if(entries[j].inum == -1){
				continue;
			}
			if(strcmp(entries[j].name, name) == 0){
				return entries[j].inum;
			}
		}

	}

	//return -3 if name doesn't exist in pinum
	return -1;
}

int Server_Stat(int inum, MFS_Stat_t *m){

	Data_Init();

	if(Inode_BitMap.bits[inum] == false){
		return -1;
	}

	m -> type = inode_table[inum].type;
	m -> size = inode_table[inum].size;
	m -> blocks = inode_table[inum].blocks;

	return 0;
}

int  Server_Write(int inum, char * buffer, int block){

	int i;
	Data_Init();

	if(Inode_BitMap.bits[inum] == false){
		return -1;
	}

	if(inode_table[inum].type != MFS_REGULAR_FILE){
		return -1;
	}

	if(block < 0 || block > 9){
		return -1;
	}


	int to_write_block;
	if(inode_table[inum].ptr[block] == -1){
		to_write_block = First_Empty(&Data_BitMap);
		Set_Bit(&Data_BitMap,to_write_block);
		for(i = 0; i <= block; i++){
			if(inode_table[inum].ptr[i] == -1){
			inode_table[inum].size += MFS_BLOCK_SIZE;
			inode_table[inum].blocks ++;
			}
		}
		inode_table[inum].ptr[block] = to_write_block;
	}else{
		to_write_block = inode_table[inum].ptr[block];
	}


	for(i = 0; i < MFS_BLOCK_SIZE; i++){
		data_region[to_write_block].data[i] = buffer[i];
	}

	Data_Write();

	return 0;
}

void Data_Write(){

	if(lseek(image_fd,0,SEEK_SET) != 0){
		fprintf(stderr,"lseek error\n");
		exit(-1);
	}

	if(ftruncate(image_fd,0) != 0){
		fprintf(stderr,"truncate error\n");
		exit(-1);
	}

	if(write(image_fd, &Inode_BitMap, sizeof(Bitmap_t)) != sizeof(Bitmap_t)){
		fprintf(stderr, "write error\n");
		exit(-1);
	}

	if(write(image_fd,&Data_BitMap, sizeof(Bitmap_t)) != sizeof(Bitmap_t)){
		fprintf(stderr,"write error\n");
		exit(-1);
	}

	if(write(image_fd,inode_table, MFS_BLOCK_NUMS*sizeof(Inode_t)) != MFS_BLOCK_NUMS*sizeof(Inode_t)){
		fprintf(stderr,"write error\n");
		exit(-1);
	}


	if(write(image_fd,data_region, MFS_BLOCK_NUMS*sizeof(Block_t)) != MFS_BLOCK_NUMS*sizeof(Block_t)){
		fprintf(stderr,"write error\n");
		exit(-1);
	}

	fsync(image_fd);

}


int Server_Read(int inum, char *buffer, int block){

	Data_Init();

	if(Inode_BitMap.bits[inum] == false){
		return -1;
	}

	if(block < 0 || block > 9){
		return -1;
	}else if(inode_table[inum].ptr[block] == -1){
		return -1;
	}


	int to_read_block = inode_table[inum].ptr[block];

	int j;
	for(j = 0; j < MFS_BLOCK_SIZE; j++){
		buffer[j] = data_region[to_read_block].data[j];
	}

	return 0;

}

int Server_Creat(int pinum, int type, char *name){

    Data_Init();

	if(Inode_BitMap.bits[pinum] == false || inode_table[pinum].type != MFS_DIRECTORY){
		return -1;
	}

	if(Server_LookUp(pinum,name) >= 0){
		return 0;
	}

	int inum = First_Empty(&Inode_BitMap);
	Set_Bit(&Inode_BitMap, inum);

	if(Add_Entry(pinum, inum, name, inode_table, data_region) != 0){
		fprintf(stderr,"add entry error\n");
	}

	if(type == MFS_REGULAR_FILE){
		inode_table[inum].type = type;
		inode_table[inum].size = 0;
		inode_table[inum].blocks = 0;
		int i;
		for(i = 0; i < MFS_PTR_NUMS; i++){
			inode_table[inum].ptr[i] = -1;
		}
	}

	if(type == MFS_DIRECTORY){
		int to_write_block = First_Empty(&Data_BitMap);
		Set_Bit(&Data_BitMap,to_write_block);
		inode_table[inum].type = type;
		inode_table[inum].size = 2 * sizeof(MFS_DirEnt_t);
		inode_table[inum].blocks = 1;
		inode_table[inum].ptr[0] = to_write_block;
		int i;
		for(i = 1; i < MFS_PTR_NUMS; i++){
			inode_table[inum].ptr[i] = -1;
		}

		MFS_DirEnt_t * entries = (MFS_DirEnt_t *) data_region[to_write_block].data;

		entries[0].inum = inum;
		strcpy(entries[0].name, ".");
		entries[1].inum = pinum;
		strcpy(entries[1].name, "..");

		for(i = 2; i < MFS_BLOCK_SIZE / sizeof(MFS_DirEnt_t); i++){
			entries[i].inum = -1;
		}
	}

	Data_Write();

	return 0;
}


int Add_Entry(int pinum, int inum, char *name, Inode_t *inode_table, Block_t *data_region){
	int idx1, idx2;
	int curr_blk_num;
	bool insert = false;
retry_add_entry:
	for(idx1 = 0; idx1 < MFS_PTR_NUMS; idx1 ++){
		if(inode_table[pinum].ptr[idx1] == -1){
			continue;
		}
		curr_blk_num = inode_table[pinum].ptr[idx1];

		MFS_DirEnt_t *entries = (MFS_DirEnt_t *)data_region[curr_blk_num].data;
		for(idx2 = 0; idx2 < MFS_BLOCK_SIZE / sizeof(MFS_DirEnt_t); idx2 ++){
			if(entries[idx2].inum == -1){
				entries[idx2].inum = inum;
				strcpy(entries[idx2].name, name);
				insert = true;
				break;
			}
		}

		if(insert == true){
			break;
		}
	}

	if(insert == false){
		add_block(pinum);
		goto retry_add_entry;
	}

	inode_table[pinum].size += sizeof(MFS_DirEnt_t);

	return 0;
}

void add_block(int inum){
	if(Inode_BitMap.bits[inum] == false){
		fprintf(stderr,"error\n");
		exit(-1);
	}


	if(inode_table[inum].type != MFS_DIRECTORY ){
		fprintf(stderr, "error\n");
		exit(-1);
	}

	int i;
	for(i = 0; i < MFS_PTR_NUMS; i ++){
		if(inode_table[inum].ptr[i] == -1){
			break;
		}
	}

	int curr_block = First_Empty(&Data_BitMap);
	inode_table[inum].ptr[i] = curr_block;
	Set_Bit(&Data_BitMap, curr_block);

	inode_table[inum].blocks++;

		MFS_DirEnt_t *entries = (MFS_DirEnt_t *)data_region[curr_block].data;
		for(i = 0; i < MFS_BLOCK_SIZE / sizeof(MFS_DirEnt_t); i++){
			entries[i].inum = -1;
		}

}



int Server_Unlink(int pinum, char *name){

	Data_Init();

	if(Inode_BitMap.bits[pinum] == false){
		return -1;
	}


	if(inode_table[pinum].type != MFS_DIRECTORY){
		return -1;
	}

	int inum = Server_LookUp(pinum, name);
	if(inum < 0){
		return 0;
	}

	if(inode_table[inum].type == MFS_DIRECTORY){
		if(inode_table[inum].size > 2 * sizeof(MFS_DirEnt_t)){
			return -1;
		}
	}

	int i;
	for(i = 0; i < MFS_PTR_NUMS; i++){
		if(inode_table[inum].ptr[i] != -1){
			Unset_Bit(&Data_BitMap, inode_table[inum].ptr[i]);
		}
	}
	Unset_Bit(&Inode_BitMap, inum);

	Remove_Entry(pinum, inum, name, inode_table, data_region);

	Data_Write();

	return 0;
}



int Remove_Entry(int pinum, int inum, char *name, Inode_t *inode_table, Block_t *data_region){

	int idx1, idx2;
	int curr_blk_num;
	bool found = false;
	for(idx1 = 0; idx1 < MFS_PTR_NUMS; idx1 ++){
		if(inode_table[pinum].ptr[idx1] == -1){
			continue;
		}
		curr_blk_num = inode_table[pinum].ptr[idx1];

		MFS_DirEnt_t *entries = (MFS_DirEnt_t *)data_region[curr_blk_num].data;
		for(idx2 = 0; idx2 < MFS_BLOCK_SIZE / sizeof(MFS_DirEnt_t); idx2 ++){
			if(entries[idx2].inum == inum && strcmp(entries[idx2].name, name) == 0){
				entries[idx2].inum = -1;
				found = true;
				break;
			}
		}

		if(found == true){
			break;
		}
	}

	inode_table[pinum].size -= sizeof(MFS_DirEnt_t);

	return 0;
}
