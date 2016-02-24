//
//  main.c
//  537P1
//
//  Created by Jason on 1/27/14.
//  Copyright (c) 2014 Jason. All rights reserved.
//

#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <string.h>

struct count {
    char *fileName;
    int occurrence;
};

int main(int argc, char * argv[])
{
    const int numFile = (int) strtol(argv[1], NULL, 10);
    if (argc < numFile + 3 || argc > numFile + 4) {
        fprintf(stderr, "Usage: search <input-number> <key-word> <input-list> <output>\n");
        exit(1);
    }
    
    int i;
    char *outFilePath = NULL;
    FILE *outFile = NULL;
    char outFileBuf[PATH_MAX +1];

    struct count result[numFile];
    if (argc >= numFile + 4) {
        outFilePath = realpath(argv[argc - 1], outFileBuf);

    }
    
    FILE *File;
    for (i = 0; i < numFile; i++) {
        char buf[PATH_MAX + 1];
        char *inFilePath = realpath(argv[i+3], buf);
        printf("%s\n", inFilePath);
        if (inFilePath == NULL) {
            fprintf(stderr, "Error: Cannot open file '%s'\n", argv[i+3]);
            exit(1);
        }

        if ((outFileBuf != NULL) && (strcmp(inFilePath, outFilePath) == 0)) {
            fprintf(stderr, "Input and output file must differ\n");
            fclose(outFile);
            exit(1);
        }
        File = fopen(inFilePath, "r");
        if (!File) {
            fprintf(stderr, "Error: Cannot open file '%s'\n", argv[i+3]);
            exit(1);
        }
        
        fseek(File, 0, SEEK_END);
        int size = (int) ftell(File);
        fseek(File, 0, SEEK_SET);
        
        char *input = (char *)malloc(size+1);
        char *curPos = input;
        int numOccur = 0;
        if (!input) {
            fprintf(stderr, "Malloc failed\n");
            exit(1);
        }
        fread(input, size, 1, File);
        while ((curPos = strstr(curPos, argv[2]))){
            curPos = curPos + strlen(argv[2]);
            numOccur++;
        }
        result[i].fileName = argv[i+3];
        result[i].occurrence = numOccur;
        free(input);
        fclose(File);
    }
    
    int c, d, position;
    struct count swap;
    for ( c = 0 ; c < (numFile - 1) ; c++ )
    {
        position = c;
        for ( d = c + 1 ; d < numFile ; d++ )
        {
            if ( result[position].occurrence < result[d].occurrence )
                position = d;
        }
        if ( position != c )
        {
            swap = result[c];
            result[c] = result[position];
            result[position] = swap;
        }
    }
    
    int j;
    if (argc < numFile + 4) {
        for (j = 0; j < numFile; j++) {
            printf("%d %s\n", result[j].occurrence, result[j].fileName);
        }
    }else{
        if (!outFilePath) {
            outFile = fopen(argv[argc - 1], "w");
        }else{
            outFile = fopen(outFilePath, "w");
        }
        if (!outFile) {
            fprintf(stderr, "Error: Cannot open file '%s'\n", argv[argc - 1]);
            exit(1);
        }
        for (j = 0; j < numFile; j++) {
            fprintf(outFile,"%d %s\n", result[j].occurrence, result[j].fileName);
        }
        
        fclose(outFile);
    }

    
    exit(0);
}

