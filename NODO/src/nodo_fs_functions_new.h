/*
 * nodo_fs_functions_new.h
 *
 *  Created on: 11/6/2015
 *      Author: utnso
 */

#ifndef NODO_FS_FUNCTIONS_NEW_H_
#define NODO_FS_FUNCTIONS_NEW_H_


#include <stdio.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include "commons/string.h"
#include <sys/stat.h>


#define TAMANIO_BLOQUE 20971520 // 20mb


typedef struct{
	char* contenido;
	int32_t size;
}t_fileContent;



//Retorna el contenido(20Mb) del numeroBloque del espacioDatos
char* getBloque(int numeroBloque, int32_t* length);


//Retorna el fileContent del archivoTemporal
t_fileContent *getFileContent(char *archivoTemporal);


//Escribe en el numeroBloque del espacioDatos los datos enviados
int setBloque(int numeroBloque, char* datos, int32_t tamanio);

//Borra un bloque, o todo el espacio de datos si se envia -1;
void borrarBloque(int numeroBloque);


#endif /* NODO_FS_FUNCTIONS_NEW_H_ */
