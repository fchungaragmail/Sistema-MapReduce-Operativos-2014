/*
 * ScriptFunctions.c
 *
 *  Created on: 20/5/2015
 *      Author: utnso
 */
#include <stdio.h>
#include <fcntl.h>
#include <ctype.h>

#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>

#define NUM_PIPES     2

#define PARENT_WRITE_PIPE  0
#define PARENT_READ_PIPE   1

int pipes[NUM_PIPES][2];

/* always in a pipe[], pipe[0] is for read and
   pipe[1] is for write */
#define READ_FD  0
#define WRITE_FD 1

#define PARENT_READ_FD  ( pipes[PARENT_READ_PIPE][READ_FD]   )
#define PARENT_WRITE_FD ( pipes[PARENT_WRITE_PIPE][WRITE_FD] )

#define CHILD_READ_FD   ( pipes[PARENT_WRITE_PIPE][READ_FD]  )
#define CHILD_WRITE_FD  ( pipes[PARENT_READ_PIPE][WRITE_FD]  )



FILE* procesarScript(char* direccionScript, char *bloqueAProcesar, FILE *archivoTemporal)
{

    int outfd[2];
    int infd[2];

    // pipes for parent to write and read
    pipe(pipes[PARENT_READ_PIPE]);
    pipe(pipes[PARENT_WRITE_PIPE]);

    if(!fork()) {
        char *argv[]={ direccionScript };

        dup2(CHILD_READ_FD, STDIN_FILENO);
        dup2(CHILD_WRITE_FD, STDOUT_FILENO);

        /* Close fds not required by child. Also, we don't
           want the exec'ed program to know these existed */
        close(CHILD_READ_FD);
        close(CHILD_WRITE_FD);
        close(PARENT_READ_FD);
        close(PARENT_WRITE_FD);

         execv(argv[0], argv);
        //luego el script como ya esta redireccionado le devolveria al padre el resultado
    } else {

    	int count;

        /* close fds not required by parent */
        close(CHILD_READ_FD);
        close(CHILD_WRITE_FD);

        // Write to child’s stdin
        size_t len = strlen(bloqueAProcesar);
        write(PARENT_WRITE_FD, bloqueAProcesar, len);

        // Read from child’s stdout
        char *bloqueProcesado;
        FILE* fp = fdopen(PARENT_READ_FD, "r");//PARENT_READ_FD es un fd
        bloqueProcesado = (char *)inputString(fp,1);//1 valor por default

        //size_t lenBlckProcesado = strlen(bloqueProcesado);
        //fwrite(bloqueProcesado,sizeof(char),lenBlckProcesado,archivoTemporal);
        return archivoTemporal;
        //hay que hacer free de _bloqueProcesado cuando se deje de usar !!
    }

    return -1;
}


void *inputString(FILE* fp, size_t size){
//The size is extended by the input with the value of the provisional
    char *str;
    int ch;
    size_t len = 0;
    str = realloc(NULL, sizeof(char)*size);//size is start size
    if(!str)return str;
    while(EOF!=(ch=fgetc(fp)) && ch != '\n'){
        str[len++]=ch;
        if(len==size){
            str = realloc(str, sizeof(char)*(size+=16));
            if(!str)return str;
        }
    }
    str[len++]='\0';

    return realloc(str, sizeof(char)*len);
}

