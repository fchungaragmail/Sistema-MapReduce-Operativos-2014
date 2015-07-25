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
	int i = 0;
	char* buffer = malloc(sizeof(int));
	char* path = setName(DIRECTORY_PATH, ARCHIVO_NAME);
	char* apareoOrdenado = setName(DIRECTORY_PATH, "apareo_ordenado");

	pipe(p);

	if(fork() == 0) {

		close(p[0]);

		apareoArchivo = open(path, O_APPEND | O_CREAT | O_RDWR); //si no existe lo crea (O_CREATE) y lo hace en
															    //modo append (O_APPEND)
		char** archivo = string_split(listaArchivos, " ");

		for (; i < sizeof(archivo); i++) {
			int archivoFD = open(archivo[i], O_RDONLY);
			struct stat infoArchivo;
			stat(archivo[i], &infoArchivo);
			char *contenido = malloc(infoArchivo.st_size);

			while(read(archivoFD, contenido, sizeof(infoArchivo.st_size)) != 0) { //leer size del archivo de una usando stat
				write(apareoArchivo, contenido, sizeof(infoArchivo.st_size));
			}
			close(archivoFD);
			free(contenido);
		}

		close(apareoArchivo);
	}

	wait(0); //esperar a que termine el proceso hijo que crea el archivo

	if(fork() == 0) {

	    close(0); //cierro entrada estandar, queda libre ese fd y el proximo fd se asociara a la entrada estandar
	    open(path, O_RDONLY); //proximo fd, se asocia a la entrada estandar

		close(1); //cierro salida estandar, el fd queda libre, proximo fd se asociara a stdout
		creat(apareoOrdenado, 0777); //fd que se asocia a salida standard

		system("sort"); //se aplica sort a lo que hay en stdin (fd del archivo) y sale por stdout (fd archivo ordenado)
	}

	wait(0);

	return apareoOrdenado;

}


char* setName(char* directory, char* name) {

	struct timeb time;
	char* finalName = malloc(140);
	char* miliseconds = malloc(sizeof(int));

	ftime(&time);

	sprintf(miliseconds, "%d", time.millitm);

	finalName = strcat(finalName, directory);
	finalName = strcat(finalName, name);
	finalName = strcat(finalName, "_");
	finalName = strcat(finalName, miliseconds);


	return finalName;

}
