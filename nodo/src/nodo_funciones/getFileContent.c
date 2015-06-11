/*
 * getFileContent.c
 *
 *  Created on: 3/6/2015
 *      Author: daniel
 */
#include <stdlib.h>
#include "commons/string.h"
#include <stdio.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
//Retorna el fileContent del archivoTemporal
typedef struct{
	char* contenido;
	long int size;
}t_fileContent;

t_fileContent *getFileContent(char *archivoTemporal) {

	/*obtengo la ruta del archivo
	char *rutaArchivo = malloc(256);
	sprintf(rutaArchivo, "%s%s", directorioTemporal, archivoTemporal);
	*/

	//abrir el archivo
	int archivo = open(archivoTemporal,O_RDONLY);

	//leer todo el archivo
	struct stat infoArchivo;
	stat(archivoTemporal, &infoArchivo);
	char *contenido = malloc(infoArchivo.st_size);
	read(archivo, contenido, infoArchivo.st_size);

	t_fileContent *fileContent = malloc(sizeof(t_fileContent));
	fileContent->contenido = contenido;
	fileContent->size = infoArchivo.st_size;
	return fileContent;
}


