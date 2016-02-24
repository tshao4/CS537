#include <stdio.h>
#include <sys/select.h>
#include "mfs.h"
#include "udp.h"

int port = -1;
int sd   = -1;
struct sockaddr_in addr1, addr2;

int MFS_Init_flag = 0;

int MFS_Init(char *hostname, int port) {

    if( MFS_Init_flag != 0 ) {
        fprintf(stderr, "MFS_Init can only be called once\n");
        exit(1);
    }
    MFS_Init_flag = 1;

    int rc = -1;

    sd = UDP_Open(port + 17);
    assert( sd >= 0 );

    rc = UDP_FillSockAddr(&addr1, hostname, port);
    if( rc != 0 ) {
        fprintf(stderr, "UDP_FillSockaddr() fails\n");
        exit(1);
    }
    
    return 0;
}

int MFS_Lookup(int pinum, char *name) {

    assert( MFS_Init_flag == 1 );

    int op_id = 1;

    char * buffer_write = malloc( UDP_BUFFER_SIZE ); 
    assert( buffer_write != NULL );
    char * cptr = buffer_write;
    int  * iptr  = (int*) buffer_write;
    *(iptr) = op_id;
    iptr++;

    *( iptr ) = pinum;
    iptr++;
    cptr = (char*)iptr;
    strncpy( cptr, name, BUFFER_SIZE );

    int rc = TIME_OUT;
    char * buffer_read = malloc( UDP_BUFFER_SIZE );
    assert( buffer_read != NULL );
    while( rc == TIME_OUT ) {
        rc = UDP_Write(sd, &addr1, buffer_write, UDP_BUFFER_SIZE);
        rc = Safe_UDP_Read(sd, &addr2, buffer_read, UDP_BUFFER_SIZE);
    }
    
    iptr = (int*) buffer_read;
    int inum = *iptr;

    free( buffer_read );
    free( buffer_write );
    
    if( inum < 0 ){
        return -1;
    }
    return inum;
}

int MFS_Stat(int inum, MFS_Stat_t *m) {

    assert( MFS_Init_flag == 1 );
    int op_id = 2;

    char * buffer_write = malloc( UDP_BUFFER_SIZE ); 
    assert( buffer_write != NULL );
    int  * iptr  = (int*) buffer_write;
    *(iptr) = op_id;
    iptr++;

    *( iptr ) = inum;
    iptr++;
    MFS_Stat_t * m_ptr = (MFS_Stat_t *) iptr;
    memcpy( m_ptr, m, sizeof(MFS_Stat_t) );

    int rc = TIME_OUT;
    char * buffer_read = malloc( UDP_BUFFER_SIZE );
    assert( buffer_read != NULL );
    while( rc == TIME_OUT ) {
        rc = UDP_Write(sd, &addr1, buffer_write, UDP_BUFFER_SIZE);
        rc = Safe_UDP_Read(sd, &addr2, buffer_read, UDP_BUFFER_SIZE);
    }

    iptr = (int*) buffer_read;
    int status = *iptr;
    iptr++;
    m_ptr = (MFS_Stat_t*) iptr;
    memcpy( m, m_ptr, sizeof(MFS_Stat_t) );

    free( buffer_read );
    free( buffer_write );

    if( status != 0 ) { return -1; }
    return 0;
}

int MFS_Write(int inum, char *buffer, int block) {

    assert( MFS_Init_flag == 1 );
    int op_id = 3;

    char * buffer_write = malloc( UDP_BUFFER_SIZE ); 
    assert( buffer_write != NULL );
    int  * iptr = (int*) buffer_write;
    char * cptr = buffer_write;
    *iptr = op_id;
    iptr++;

    *iptr = inum;
    iptr++;
    cptr = (char*)iptr;
    memcpy( cptr, buffer, BUFFER_SIZE );
    cptr += BUFFER_SIZE;
    iptr = (int*)cptr; 
    *iptr = block;

    int rc = TIME_OUT;
    char * buffer_read = malloc( UDP_BUFFER_SIZE );
    assert( buffer_read != NULL );
    while( rc == TIME_OUT ) {
        rc = UDP_Write(sd, &addr1, buffer_write, UDP_BUFFER_SIZE);
        rc = Safe_UDP_Read(sd, &addr2, buffer_read, UDP_BUFFER_SIZE);
    }

    iptr = (int*) buffer_read;
    int status = *iptr;

    free( buffer_read );
    free( buffer_write );

    if( status != 0 ) { return -1; }
    return 0;

}

int MFS_Read(int inum, char *buffer, int block) {

    assert( MFS_Init_flag == 1 );
    int op_id = 4;

    char * buffer_write = malloc( UDP_BUFFER_SIZE ); 
    assert( buffer_write != NULL );
    int  * iptr = (int*) buffer_write;
    char * cptr = buffer_write;
    *iptr = op_id;
    iptr++;

    *iptr = inum;
    iptr++;
    cptr = (char*) iptr;
    memcpy(cptr, buffer, BUFFER_SIZE);
    cptr += BUFFER_SIZE;
    iptr = (int *) cptr;
    *iptr = block;

    int rc = TIME_OUT;
    char * buffer_read = malloc( UDP_BUFFER_SIZE );
    assert( buffer_read != NULL );
    while( rc == TIME_OUT ) {
        rc = UDP_Write(sd, &addr1, buffer_write, UDP_BUFFER_SIZE);
        rc = Safe_UDP_Read(sd, &addr2, buffer_read, UDP_BUFFER_SIZE);
    }

    iptr = (int*) buffer_read;
    int status = *iptr;
    iptr++;
    cptr = (char*) iptr;
    memcpy(buffer, cptr, BUFFER_SIZE);

    free( buffer_read );
    free( buffer_write );

    if( status != 0 ) { return -1; }
    return 0;

}

int MFS_Creat(int pinum, int type, char *name) {

    assert( MFS_Init_flag == 1 );
    int op_id = 5;

    char * buffer_write = malloc( UDP_BUFFER_SIZE ); 
    assert( buffer_write != NULL );
    int  * iptr = (int*) buffer_write;
    char * cptr = buffer_write;
    *iptr = op_id;
    iptr++;
    
    *iptr = pinum;
    iptr++;
    *iptr = type;
    iptr++;
    cptr = (char*) iptr;
    strncpy( cptr, name, BUFFER_SIZE );

    int rc = TIME_OUT;
    char * buffer_read = malloc( UDP_BUFFER_SIZE );
    assert( buffer_read != NULL );
    while( rc == TIME_OUT ) {
        rc = UDP_Write(sd, &addr1, buffer_write, UDP_BUFFER_SIZE);
        rc = Safe_UDP_Read(sd, &addr2, buffer_read, UDP_BUFFER_SIZE);
    }

    iptr = (int*) buffer_read;
    int status = *iptr;

    free( buffer_read );
    free( buffer_write );

    if( status != 0 ) { return -1; }
    return 0;

}

int MFS_Unlink(int pinum, char *name) {

    assert( MFS_Init_flag == 1 );
    int op_id = 6;

    char * buffer_write = malloc( UDP_BUFFER_SIZE ); 
    assert( buffer_write != NULL );
    int  * iptr = (int*) buffer_write;
    char * cptr = buffer_write;
    *iptr = op_id;
    iptr++;

    *iptr = pinum;
    iptr++;
    cptr = (char*) iptr;
    strncpy( cptr, name, BUFFER_SIZE );

    int rc = TIME_OUT;
    char * buffer_read = malloc( UDP_BUFFER_SIZE );
    assert( buffer_read != NULL );
    while( rc == TIME_OUT ) {
        rc = UDP_Write(sd, &addr1, buffer_write, UDP_BUFFER_SIZE);
        rc = Safe_UDP_Read(sd, &addr2, buffer_read, UDP_BUFFER_SIZE);
    }

    iptr = (int*) buffer_read;
    int status = *iptr;

    free( buffer_read );
    free( buffer_write );

    if( status != 0 ) { return -1; }
    return 0;

}