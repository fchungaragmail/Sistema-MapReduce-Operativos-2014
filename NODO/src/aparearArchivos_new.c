/*
 * aparearArchivos_new.c
 *
 *  Created on: 11/6/2015
 *      Author: utnso
 */


#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <string.h>
#include <stdlib.h>
#include <commons/string.h>

#define TAMANIO_BLOQUE 20971520


char* aparearArchivos(char* listaArchivos) {

	int apareoArchivo;
	int i = 0;
	char* buffer = malloc(sizeof(int));
	char* path = "/home/utnso/Escritorio/apareo/apareo.txt";

	apareoArchivo = open(path, O_APPEND | O_CREAT | O_RDWR); //si no existe lo crea (O_CREATE) y lo hace en
																                                   //modo append (O_APPEND)
	char** archivo = string_split(listaArchivos, " ");

	for (; i < sizeof(archivo); i++) {
		int archivoFD = open(archivo[i], O_RDONLY);
		while(read(archivoFD, buffer, sizeof(buffer)) != 0) {
			write(apareoArchivo, buffer, sizeof(buffer));
		}
		close(archivoFD);
	}



	close(apareoArchivo);

	return path;

}
