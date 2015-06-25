/*
 * nodo_new.h
 *
 *  Created on: 11/6/2015
 *      Author: utnso
 */

#ifndef NODO_NEW_H_
#define NODO_NEW_H_


#define MESSAGE_LENGTH 256
#define SHAKEHAND_MESSAGE_LENGTH 50
#define COMANDO_LENGTH 50

#include <stdio.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include "commons/string.h"
#include <sys/stat.h>
#include "nodo_fs_functions_new.h"
#include "nodo_job_functions_new.h"
#include "sockets_struct_new.h"

extern char* ARCHIVO_BIN;


void getConfig();
int initServer(int*);
void connectToFileSistem(int*);
void setValuesToSockaddr(Sockaddr_in*, int, char*);
void *fs_nodo_conection_handler(void*);
void *map_conection_handler(void*);
void *reduce_conection_handler(void*);


#endif /* NODO_NEW_H_ */
