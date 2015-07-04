/*
 * aparearArchivos.h
 *
 *  Created on: 24/6/2015
 *      Author: utnso
 */

#ifndef APAREARARCHIVOS_NEW_H_
#define APAREARARCHIVOS_NEW_H_

#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <string.h>
#include <stdlib.h>
#include <commons/string.h>
#include <sys/timeb.h>

#define TAMANIO_BLOQUE 20971520
#define DIRECTORY_PATH "/tmp"
#define ARCHIVO_NAME "/apareo"


char* setName(char*, char*);
char* aparearArchivos(char* listaArchivos);


#endif /* APAREARARCHIVOS_NEW_H_ */
