//
//  mysh.c
//  P2
//
//  Created by Terry Shao on 2/8/14.
//  Copyright (c)  2014 Terry Shao. All rights reserved.
//

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <limits.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>

#define SEQUENTIAL 1
#define PARALLEL 2
#define SINGLE 0

#define NORMAL 0
#define PIPE 1
#define REDIR 2

typedef struct job{
    
    char ** cmds[10];
    char on_duty;
    int nPipes;
    char *redir;
    
}Job;

void printErr(){
    
    char error_message[30] = "An error has occurred\n";
    write(STDERR_FILENO, error_message, strlen(error_message));
}
void parcmd (char * toPar, char * result[]) {
    
    int i = 0;
    int j;
    char *arg = strtok (toPar, " \t");
    
    while (arg) {
        
        result[i] = arg;
        i++;
        arg = strtok (NULL, " \t");
    }
    
    
    for (j = 0; j < i; j++) {
        
        result[j] = strtok (result[j], "\"");
        while (result[j] == NULL){
            
            result[j]++;
        }
    }
    
    for (j = i;j < 10; j++) {
        result[j] = 0;
    }
}

int parRePi (int num ,char * toPar, Job result[]) {
   
    char * j = 0;
    int i = 0;
    char * c[10];
    result[num].nPipes = 0;
    result[num].redir = 0;
    
    if ( (j = strchr (toPar, '>') )  && !strchr (toPar, '|') ) {
        
        if (strchr (j + 1, '>') ) {
            
            printErr();
            return -1;
        }
        
        char * arg = strtok (toPar, ">");
        char * red = strtok (NULL, ">");
        if ((result[num].redir = strtok (red, " ")) == NULL) {
            
            printErr();
            return -1;
        }

        if (strtok (NULL, " ") ) {
            
            printErr();
            return -1;
        }
        parcmd (arg, result[num].cmds[0]);
        i++;
    }
    
    j = 0;
    
    if ( (j = strchr (toPar, '|') ) ) {
        
        if ( (j = strchr(j + 1, '>') ) ) {
            
            if (strchr(j + 1, 'l') ) {
                
                printErr();
                return -1;
            }
        }
        
        c[result[num].nPipes] = strtok (toPar, "|");
        
        while (c[result[num].nPipes]) {
            
            result[num].nPipes++;
            c[result[num].nPipes] = strtok (NULL, "|");
        }
        
        result[num].nPipes--;

        for (i = 0; i < result[num].nPipes; i++) {
            
            parcmd (c[i], result[num].cmds[i]);
        }
        
        if (strchr (c[i], '>') ) {
            
            c[i] = strtok (c[i], ">");
            char * red = strtok (NULL, ">");
            if ((result[num].redir = strtok (red, " ")) == NULL) {
                
                printErr();
                return -1;
            }
            
            if (strtok (NULL, " ") ) {
                
                printErr();
                return -1;
            }
            
        }
        
        parcmd (c[i], result[num].cmds[i]);
        i++;
        
        for (;i < 10; i++) {
            result[num].cmds[i][0] = NULL;
        }
        
    }else{
        
        int k;
        if (result[num].redir) {
            k = 2;
        }else{
            k = 1;
        }
        for (; k < 10; k++) {
            
            result[num].cmds[k][0] = NULL;
        }

        if (!result[num].redir)
        parcmd (toPar, result[num].cmds[0]);
    }
    
    if (result[num].redir) {
        
        result[num].redir = strtok (result[num].redir, "\"");
        while (result[num].redir == NULL) {
            
            result[num].redir++;
        }
    }
    result[num].on_duty = 1;
    return NORMAL;
}

int sepMultCmd (char * toSep, Job result[]) {
    
    if (strchr (toSep, ';')  && strchr (toSep, '+') ) 
        write (STDERR_FILENO, ";, +\n", 4);
    
    if (strchr (toSep, ';') ) {
        
        int i = 0;
        int j;
        char *argt[10];
        argt[0] = strtok (toSep, ";");
        
        while (argt[i]) {
            i++;
            argt[i] = strtok (NULL, ";");
        }

        for (j = 0; j < i; j++) {
            
            if (parRePi (j, argt[j], result)  == -1) {
                return -1;
            }
        }
        
        return SEQUENTIAL;
        
    }else if (strchr (toSep, '+') ) {
        
        int i = 0;
        int j;
        char *argt[10];
        argt[0] = strtok (toSep, "+");
        
        while (argt[i]) {
            i++;
            argt[i] = strtok (NULL, "+");
        }
        
        for (j = 0; j < i; j++) {
            
            if (parRePi (j, argt[j], result)  == -1) {
                return -1;
            }
        }
        
        return PARALLEL;
        
    }else{
        
        
        if (parRePi (0, toSep, result)  == -1) {
            return -1;
        }
        return SINGLE;
    }
             
    return 0;
}

void exeByMode (Job toExe[], int mode, pid_t id) {
    
    int i = 0;
    int j = 0;
    
    pid_t pid;
    
    switch (mode) {
        
        case SEQUENTIAL:{
            
            while (toExe[j].on_duty) {
                
                if (strcmp(*toExe[j].cmds[0], "quit") == 0) {
                    
                    exit (0);
                }else
                
                if (strcmp (*toExe[j].cmds[0] , "pwd")  == 0) {
                    
                    if (toExe[j].cmds[0][1]) {
                        
                        printErr();
                        j++;
                        continue;
                    }
                    
                    char tmp[PATH_MAX + 1];
                    getcwd (tmp, PATH_MAX + 1);
                    write (STDOUT_FILENO, tmp, sizeof (char)  * strlen (tmp) );
                    write (STDOUT_FILENO, "\n", sizeof (char) );
                    j++;
                    continue;
                    
                }else
                    
                if (strcmp (*toExe[j].cmds[0], "cd")  == 0) {
                    
                    if (toExe[j].cmds[0][1] == NULL) {
                        
                        chdir (getenv ("HOME") );
                    }else
                    if (chdir (toExe[j].cmds[0][1])  != 0) {
                        printErr();
                        j++;
                        continue;
                    }
                    j++;
                    continue;
                }
                
                pid = fork ();
                
                if (pid < 0) {
                    
                    printErr();
                    exit (1);
                    
                }else if (pid == 0) {
                    
                    pid_t pids;
                    int fd[2];
                    int in = STDIN_FILENO;
                    
                    for (i = 0; i < toExe[j].nPipes; i++) {
                        
                        if (pipe (fd) < 0) {
                            
                            printErr();
                            exit (1);
                        }
                        
                        pids = fork();
                        
                        if (pids < 0) {
                            
                            printErr();
                            
                        }else if (pids == 0) {
                            
                            dup2 (in, STDIN_FILENO);
                            dup2 (fd[1], STDOUT_FILENO);
                            
                            close (fd[0]);
                            
                            execvp (*toExe[j].cmds[i], toExe[j].cmds[i]);
                            printErr();
                            exit (1);
                            
                        }else{
                            
                            while (wait (NULL)  != -1 || errno != ECHILD) {}
                            close (fd[1]);
                            in = fd[0];
                        }
                    }

                    if (in != STDIN_FILENO) {
                        
                        dup2 (fd[0], STDIN_FILENO);
                        close (fd[0]);
                    }
                    
                    if (toExe[j].redir){
                        
                        int fdo = open(toExe[j].redir, O_WRONLY | O_CREAT | O_TRUNC, S_IRWXU);
                        if (fdo < 0) {
                            
                            printErr();
                            exit (1);
                        }
                        dup2 (fdo, STDOUT_FILENO);
                        close (fdo);
                    }
                    
                    execvp (*toExe[j].cmds[i], toExe[j].cmds[i]);
                    printErr();
                    exit (0);
                    
                }else{
                    
                    while (wait (NULL)  != -1 || errno != ECHILD) {}
                }
                
                j++;
            }
            break;
        }
        case PARALLEL:{
            
            while (toExe[j].on_duty) {
                
                pid = fork ();
                
                if (pid < 0) {
                    
                    printErr();
                    exit (1);
                    break;
                }else if (pid == 0)  break;
                
                j++;
            }

            if (getpid ()  != id) {

                pid_t pids;
                int fd[2];
                int in = STDIN_FILENO;
                
                for (i = 0; i < toExe[j].nPipes; i++) {
                    
                    if (pipe (fd) < 0) {
                        
                        printErr();
                        exit (1);
                    }
                    
                    pids = fork();
                    
                    if (pids < 0) {
                        
                        printErr();
                        
                    }else if (pids == 0) {
                        
                        dup2 (in, STDIN_FILENO);
                        dup2 (fd[1], STDOUT_FILENO);
                        
                        close (fd[0]);
                        
                        execvp (*toExe[j].cmds[i], toExe[j].cmds[i]);
                        printErr();
                        exit (1);
                        
                    }else{
                        
                        while (wait (NULL)  != -1 || errno != ECHILD) {}
                        close (fd[1]);
                        in = fd[0];
                    }
                }
                
                if (in != STDIN_FILENO) {
                    
                    dup2 (fd[0], STDIN_FILENO);
                    close (fd[0]);
                }
                
                if (toExe[j].redir){
                    
                    int fdo = open(toExe[j].redir, O_WRONLY | O_CREAT | O_TRUNC, S_IRWXU);
                    if (fdo < 0) {
                        
                        printErr();
                        exit (1);
                    }
                    dup2 (fdo, STDOUT_FILENO);
                    close (fdo);
                }

                execvp (*toExe[j].cmds[i], toExe[j].cmds[i]);
                printErr();
                exit (0);
            }else{
                
                while (wait (NULL)  != -1 || errno != ECHILD) {}

            }
            
            break;
        }
        case SINGLE:{
            
            if (strcmp(*toExe[0].cmds[0], "quit") == 0) {
                
                exit (0);
            }else
                
            if (strcmp (*toExe[0].cmds[0] , "pwd")  == 0) {
                
                if (toExe[0].cmds[0][1]) {
                    
                    printErr();
                }
                
                char tmp[PATH_MAX + 1];
                getcwd (tmp, PATH_MAX + 1);
                write (STDOUT_FILENO, tmp, sizeof (char)  * strlen (tmp) );
                write (STDOUT_FILENO, "\n", sizeof (char) );
                
            }else
                
            if (strcmp (*toExe[0].cmds[0], "cd")  == 0) {
                    
                if (toExe[0].cmds[0][1] == NULL) {
                    
                    if (chdir (getenv ("HOME") )  != 0) {
                        
                        printErr();
                    }

                }else
                if (chdir (toExe[0].cmds[0][1])  != 0) {
                            
                    printErr();
                }
                i++;
            }

            
            pid = fork ();
            
            if (pid < 0) {
                
                printErr();
                exit (1);
                
            }else if (pid == 0) {

                pid_t pids;
                int fd[2];
                int in = STDIN_FILENO;
                
                for (; i < toExe[0].nPipes; i++) {
                    
                    if (pipe (fd) < 0) {
                        
                        printErr();
                        exit (1);
                    }
                    
                    pids = fork();
                    
                    if (pids < 0) {
                        
                        printErr();
                    
                    }else if (pids == 0) {
                    
                        dup2 (in, STDIN_FILENO);
                        dup2 (fd[1], STDOUT_FILENO);
                        
                        close (fd[0]);
                        
                        
                        execvp (*toExe[0].cmds[i], toExe[0].cmds[i]);
                        printErr();
                        exit (1);
                        
                    }else{
                        
                        while (wait (NULL)  != -1 || errno != ECHILD) {}
                        close (fd[1]);
                        in = fd[0];
                    }
                }

                if (in != STDIN_FILENO) {
                    
                    dup2 (fd[0], STDIN_FILENO);
                    close (fd[0]);
                }
                
                if (toExe[j].redir){
                    
                    int fdo = open(toExe[j].redir, O_WRONLY | O_CREAT | O_TRUNC, S_IRWXU);
                    if (fdo < 0) {
                        
                        printErr();
                        exit (1);
                    }
                    dup2 (fdo, STDOUT_FILENO);
                    close (fdo);
                }
                
                execvp (*toExe[0].cmds[i], toExe[0].cmds[i]);
                printErr();
                exit (1);
                
            }else{
                
                while (wait (NULL)  != -1 || errno != ECHILD) {}
            }

            break;
        }
    }
}

int main (int argc, const char * argv[]) 
{

    if (argc > 2) {
        
        printErr();
        exit (1);
    }
    FILE * fp = NULL;
    char bmode = 0;
    
    if (argc == 2) {
        
        fp = fopen (argv[1], "r");
        if (fp == NULL) {
            
            printErr();
            exit (1);
        }
        bmode = 1;
    }
    
    Job  jobs[20];
    int i, j, k;
    
    for (i = 0; i < 20; i++) {
        for (j = 0; j < 10; j++) {
            
            jobs[i].cmds[j] = malloc (50);
        }
    }
        
    pid_t id = getpid ();
    char mode;
    
    while (1) {
        
        char cmd[1000] = {0};
        
        for (i = 0; i < 20; i++) {
            
            if (jobs[i].on_duty == 1) {
                
                for (j = 0; j < 10; j++) {
                    
                    for (k = 0; k < 50; k++)
                        jobs[i].cmds[j][k] = NULL;
                }
            }
            jobs[i].on_duty = 0;
            jobs[i].nPipes = 0;
            jobs[i].redir = 0;
        }
        
        if (bmode) {
            
            if (fgets (cmd, 1000, fp) == NULL){
                
                exit (0);
            }
            
            write (STDOUT_FILENO, cmd, strlen(cmd));
            
            if (cmd[512] != 0 && cmd[512] != '\n') {
                
                cmd[512] = 0;
                printErr();
                continue;
            }
            
        }else{
            
            write (STDOUT_FILENO, "537sh> ", sizeof (char)  * 7);
            
            fgets (cmd, 1000, stdin);
            
            if (cmd[512] != 0 && cmd[512] != '\n') {
                
                cmd[512] = 0;
                printErr();
                continue;
            }

        }

        if (*cmd == '\n')  continue;
        
        if (cmd[strlen (cmd)  - 1] == '\n') {
            
            cmd[strlen (cmd)  - 1] = 0;
        }
        
        if ( (mode = sepMultCmd (cmd, jobs) )  == -1)  continue;
       
        if (mode == PARALLEL) {
            
            int i = 0;
            char err = 0;
            while (*jobs[i].cmds[0]) {
                
                int j;
                for (j = 0; j < 10; j++){
                    if (*jobs[i].cmds[j]) {

                        if (strcmp (*jobs[i].cmds[j], "quit") == 0
                            || strcmp (*jobs[i].cmds[j], "pwd") == 0
                            || strcmp (*jobs[i].cmds[j], "cd") == 0) {
                            
                                err = 1;
                        }
                    }
                }
                i++;
            }
            
            if (err == 1) {
                
                printErr();
                continue;
            }
        }
        
        if (*jobs->cmds[1] == NULL && jobs[0].redir == 0 && *jobs[1].cmds[0] == NULL) {

            if (*jobs->cmds[0] == NULL)  continue;
            
            if (strcmp (*jobs->cmds[0], "quit")  == 0) {
                
                exit (0);
            }else
            
            if (strcmp (*jobs->cmds[0] , "cd")  == 0) {
                
                if (jobs->cmds[0][1] == NULL) {
                    
                    chdir (getenv ("HOME") );
                }else
                if (chdir (jobs->cmds[0][1])  != 0) {
                    printErr();
                    continue;
                }
                continue;
            }else
            
            if (strcmp (*jobs->cmds[0] , "pwd")  == 0) {
                
                if (jobs->cmds[0][1]) {
                    
                    printErr();
                    continue;
                }
                
                char tmp[PATH_MAX + 1];
                getcwd (tmp, PATH_MAX + 1);
                write (STDOUT_FILENO, tmp, sizeof (char)  * strlen (tmp) );
                write (STDOUT_FILENO, "\n", sizeof (char) );
                continue;
            }
        }
            exeByMode (jobs, mode, id);
    }
    
    for (i = 0; i < 50; i++) {
        for (j = 0; j < 10; j++) {
            
            free (jobs[i].cmds[j]);
        }
    }
    return 0;
}

