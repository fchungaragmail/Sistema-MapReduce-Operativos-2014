/*
 * nodo_new.h
 *
 *  Created on: 11/6/2015
 *      Author: utnso
 */

#ifndef NODO_NEW_H_
#define NODO_NEW_H_


#define MESSAGE_LENGTH 256
#define MESSAGE_PROTOCOL_LENGTH 2

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

extern char* ARCHIVO_BIN;


#endif /* NODO_NEW_H_ */
