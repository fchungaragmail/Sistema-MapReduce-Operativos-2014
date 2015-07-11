/*
 * nodo_fs_functions_new.c
 *
 *  Created on: 11/6/2015
 *      Author: utnso
 */


#include "nodo_fs_functions_new.h"
#include "nodo_new.h"



char* getBloque(int numeroBloque) {

	//abro el espacio de datos para lectura
	int archivo = open(ARCHIVO_BIN,O_RDONLY);

	//calculo offset
	long int offset = numeroBloque * TAMANIO_BLOQUE;

	//leo el contenido del bloque
	lseek(archivo, offset, SEEK_SET);
	char *contenido = malloc(TAMANIO_BLOQUE);
	read(archivo, contenido, TAMANIO_BLOQUE);

	return contenido;
}



t_fileContent *getFileContent(char *archivoTemporal) {

	/*obtengo la ruta del archivo
	char *rutaArchivo = malloc(256);
	sprintf(rutaArchivo, "%s%s", directorioTemporal, archivoTemporal);
	*/

	//abrir el archivo
	int archivo = open(archivoTemporal,O_RDONLY);
	perror("");

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



int setBloque(int numeroBloque, char* datos, int32_t tamaño) {

	//MAPEO CON ESCRITURA DE ARCHIVO
	int archivo = open(ARCHIVO_BIN,O_RDWR);
	long int offset = numeroBloque * TAMANIO_BLOQUE;

	//parametros mmap(): 1=null, 2=tamaño a mapear, 3=operaciones permitidas sobre el mapeo, 4=si el mapeo es privado o compartido, 4=archivo, 5=desplazamiento desde el inicio del archivo
	void *bloque= mmap(NULL,  tamaño,  PROT_WRITE, MAP_SHARED,  archivo,  offset);
	close(archivo);//mmap ya tiene una copia del fd

	//escribir sobre el mapeo el mensaje
	memcpy(bloque, datos, tamaño);

	//se va actualizar el archivo
	munmap(bloque, tamaño);


	//muestro la parte del bloque que se escribio
	/*
	archivo = open(ARCHIVO_BIN, O_RDWR);
	char *msj = malloc(100);
	lseek(archivo, offset, SEEK_SET);
	read(archivo, msj, 100);
	printf("%s\n", msj);
	close(archivo);
	*/
	return 0;
}

