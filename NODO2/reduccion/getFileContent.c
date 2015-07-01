/*
 * getFileContent.c
 *
 *  Created on: 3/6/2015
 *      Author: daniel
 */
#include "getFileContent.h"
//Retorna el fileContent del archivoTemporal
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


