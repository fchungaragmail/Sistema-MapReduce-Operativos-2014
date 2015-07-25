/*
 * aparearArchivos_new.c
 *
 *  Created on: 11/6/2015
 *      Author: utnso
 */

#include "aparearArchivos_new.h"
//#include "nodo_new.h"
#include <unistd.h>

#define DIR_TEMP "/tmp/"

char* aparearArchivos(char* listaArchivos) {

	int p[2];
	int apareoArchivo;
	char* buffer = malloc(sizeof(int));
	char* path = setName(DIRECTORY_PATH, ARCHIVO_NAME);
	char* apareoOrdenado = setName(DIRECTORY_PATH, "apareo_ordenado");


	apareoArchivo = creat(path, 0777); //si no existe lo crea (O_CREATE) y lo hace en
	//modo append (O_APPEND)
	char** archivo = string_split(listaArchivos, " ");

	int i = 0;
	while(archivo[i] != NULL)
	{
		char* tempFile = string_new();
		string_append_with_format(&tempFile, "/tmp%s", archivo[i]);
		int archivoFD = open(tempFile, O_RDONLY);
		struct stat infoArchivo;
		stat(tempFile, &infoArchivo);
		char *contenido = malloc(infoArchivo.st_size);

		read(archivoFD, contenido, infoArchivo.st_size);
		write(apareoArchivo, contenido, infoArchivo.st_size);
		free(contenido);
		free(tempFile);
		close(archivoFD);

		i++;
	}

	close(apareoArchivo);


	if(fork() == 0) {

	    close(0); //cierro entrada estandar, queda libre ese fd y el proximo fd se asociara a la entrada estandar
	    open(path, O_RDONLY); //proximo fd, se asocia a la entrada estandar

		close(1); //cierro salida estandar, el fd queda libre, proximo fd se asociara a stdout
		creat(apareoOrdenado, 0777); //fd que se asocia a salida standard

		system("sort");
		exit(EXIT_SUCCESS);//se aplica sort a lo que hay en stdin (fd del archivo) y sale por stdout (fd archivo ordenado)
	}

	wait(0);

	unlink(path);

	return apareoOrdenado;

}


char* setName(char* directory, char* name) {

	struct timeb time;
	char* finalName = string_new();
	char* miliseconds = string_new();

	ftime(&time);
	string_append_with_format(&miliseconds, "%hu", time.millitm);

	string_append_with_format(&finalName,"%s%s_%s" ,directory, name,miliseconds);

	return finalName;

}
