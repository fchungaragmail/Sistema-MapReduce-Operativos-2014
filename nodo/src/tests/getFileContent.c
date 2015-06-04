/*
 * getFileContent.c
 *
 *  Created on: 3/6/2015
 *      Author: daniel
 */
//Mostrara el contenido de un archivo temporal
//Recibe 1:ruta del directorio temporal 2:nombre de un archivo temporal
#include <stdlib.h>
#include "commons/string.h"
#include <stdio.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
int main(int argc, char **argv) {
	//abrir el archivo
	char *rutaArchivo = malloc(256);
	sprintf(rutaArchivo, "%s%s", argv[1], argv[2]);//concateno ruta de directorio temporal y nombre de archivo
	int archivo = open(rutaArchivo, O_CREAT|O_RDONLY);

	//leer todo el archivo
	struct stat infoArchivo;
	stat(rutaArchivo, &infoArchivo);
	char *dataArchivo = malloc(infoArchivo.st_size);
	read(archivo, dataArchivo, infoArchivo.st_size);

	//muestro el contenido
	printf("%s\n", dataArchivo);
	return 0;
}


