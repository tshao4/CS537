///////////////////////////////////////////////////////////////////////////////
//                   ALL STUDENTS COMPLETE THESE SECTIONS
// Title:            search
// Files:            search.c
// Semester:         CS537 Spring 2014
//
// Author:           Tianyu Shao
// Email:            tshao4@wisc.edu
// CS Login:         tshao
//
//////////////////////////// 80 columns wide //////////////////////////////////

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

/*	files structure which stores the file name
	and keyword occurrence counter
*/
struct files{
    char *fname;
    int count;
};

/*	bubble sort */
void sort(struct files toSort[] ,int length){
    
    int i, j;
    struct files tmp;
    
    for (i = 0; i < length; i++) {
        
        for (j = length - 1; j > i; j--) {
            
            if (toSort[j].count > toSort[j-1].count) {
                
                tmp = toSort[j-1];
                toSort[j-1] =  toSort[j];
                toSort[j] = tmp;
            }
        }
    }
}

int main(int argc, char *argv[]){
    
    /*  validate number of arguments */
    if(argc < 4){
        
        fprintf(stderr,
            "Usage: search <input-number> <key-word> <input-list> <output>\n");
        exit(1);
    }
    
	/*	number of input files */
    const int numIn = (int)strtol(argv[1], NULL, 10);
	/*	validate number of arguments based on number of input files */
    if(argc > numIn + 4 || argc < numIn + 3 || numIn == 0){
        
        fprintf(stderr,
            "Usage: search <input-number> <key-word> <input-list> <output>\n");
        exit(1);
    }
    
	
	struct files inFiles[numIn]; /* array of files */
    char *outFile = NULL; /* output file name */
	char *key = argv[2]; /* keyword */
	int i; /* for loop */
	
	FILE *fp; /* file pointer handling input & output */
	
    /*  store input file names & initialize counter,
		check if the output file name is provided,
        store output file name */
    if (argv[numIn + 3] != NULL){

        outFile = argv[argc - 1];
		fp = fopen(outFile, "w");
        if (fp == NULL){
            
            fprintf(stderr, "Error: Cannot open file '%s'\n", outFile);
            exit(1);
        }
		
        char inBuf[PATH_MAX + 1]; /* input file path */
        char outBuf[PATH_MAX + 1]; /* output file path */
        realpath(outFile, outBuf);
        
        for (i = 0; i < numIn; i++){
            
            inFiles[i].fname = argv[i+3];
            inFiles[i].count = 0;
            realpath(inFiles[i].fname, inBuf);
            
            if (!strcmp(inBuf, outBuf)){
                
                fprintf(stderr, "Input and output file must differ\n");
                exit(1);
            }
        }
        
        fclose(fp);
    }
    else{
        for (i = 0; i < numIn; i++){
            
            inFiles[i].fname = argv[i+3];
            inFiles[i].count = 0;
        }
    }


    for (i = 0; i < numIn; i++){
       
        fp = fopen(inFiles[i].fname, "r");
        if (fp == NULL){
            
            fprintf(stderr, "Error: Cannot open file '%s'\n", inFiles[i].fname);
            exit(1);
        }
        
        /*  determin the file length */
        unsigned long fLength;
        fseek(fp, 0, SEEK_END);
        fLength = ftell(fp);
        rewind(fp);
        
        char *buffer;
        buffer = malloc(sizeof(char) * (fLength + 1));
        char *cur = buffer;
        
        if (cur == NULL){
            fprintf(stderr, "Malloc failed\n");
            exit(1);
        }
        
        fread(cur, sizeof(char), fLength, fp);
        cur[fLength + 1] = '\0';
        
        /*  checking key word occurrence */
        unsigned long keyLength = strlen(key);
        
        while (*cur != '\0') {
            if (strncmp(cur++, key, keyLength)){
                continue;
            }
            cur += keyLength - 1;
            inFiles[i].count++;
        }
        
		free(buffer);
		fclose(fp);
    }
    
    /*  sort the list */
    sort(inFiles, numIn);
    
    /*  print the list */
    if (outFile){
        
        fp = fopen(outFile, "w");
        for (i = 0; i < numIn; i++){
            fprintf(fp, "%d %s\n", inFiles[i].count, inFiles[i].fname);
        }
        
		fclose(fp);
    }
    else{
        
        for (i = 0; i < numIn; i++){
            fprintf(stdout, "%d %s\n", inFiles[i].count, inFiles[i].fname);
        }
    }
    
	return 0;
}