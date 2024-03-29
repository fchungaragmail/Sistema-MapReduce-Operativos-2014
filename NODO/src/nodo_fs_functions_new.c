/*
 * nodo_fs_functions_new.c
 *
 *  Created on: 11/6/2015
 *      Author: utnso
 */
#define _FILE_OFFSET_BITS 64

#include "nodo_fs_functions_new.h"
#include "nodo_new.h"
//#include "commons/string.h"


char* getBloque(int numeroBloque, int32_t* length) {

	pthread_mutex_lock(&getBloqueMutex);

	//abro el espacio de datos para lectura
	int archivo = open(ARCHIVO_BIN,O_RDONLY);
	if(archivo < 0){
		log_info(log_nodo, "Fallo syscall OPEN() en getBloque()");
		return NULL;
	}

	//calculo offset
	long int offset = numeroBloque * TAMANIO_BLOQUE;

	if(	lseek(archivo, offset, SEEK_SET) < 0){
		log_info(log_nodo, "Fallo syscall OPEN() en getBloque()");
		return NULL;
	}

	char *contenido = malloc(TAMANIO_BLOQUE);

	if(	contenido == NULL){
		log_info(log_nodo, "Fallo syscall OPEN() en getBloque()");
		return NULL;
	}

	if(	read(archivo, contenido, TAMANIO_BLOQUE) < 0){
		log_info(log_nodo, "Fallo syscall READ() en getBloque()");
		return NULL;
	}

	if(close(archivo) < 0){
		log_info(log_nodo, "Fallo syscall CLOSE() en getBloque()");
		return NULL;
	}


	*length = strnlen(contenido, TAMANIO_BLOQUE);

	pthread_mutex_unlock(&getBloqueMutex);

	return contenido;
}



t_fileContent *getFileContent(char *archivoTemporal) {

	/*obtengo la ruta del archivo
	char *rutaArchivo = malloc(256);
	sprintf(rutaArchivo, "%s%s", directorioTemporal, archivoTemporal);
	*/

	char *rutaTemporal = string_duplicate("/tmp");
	string_append(&rutaTemporal,archivoTemporal);
	//abrir el archivo
	int archivo = open(rutaTemporal,O_RDONLY);
	if (archivo == -1) {
		log_info(log_nodo, "FALLO AL ABRIR ARCHIVO EN GETFILECONTENT");
	}
	//perror("");

	//leer todo el archivo
	struct stat infoArchivo;
	stat(rutaTemporal, &infoArchivo);
	char *contenido = malloc(infoArchivo.st_size);
	read(archivo, contenido, infoArchivo.st_size);
	close(archivo);

	t_fileContent *fileContent = malloc(sizeof(t_fileContent));
	fileContent->contenido = contenido;
	fileContent->size = infoArchivo.st_size;

	return fileContent;
}



int setBloque(int numeroBloque, char* datos, int32_t tamanio) {

	//MAPEO CON ESCRITURA DE ARCHIVO
	int archivo = open(ARCHIVO_BIN,O_RDWR);
	long int offset = numeroBloque * TAMANIO_BLOQUE;

	//parametros mmap(): 1=null, 2=tamaño a mapear, 3=operaciones permitidas sobre el mapeo, 4=si el mapeo es privado o compartido, 4=archivo, 5=desplazamiento desde el inicio del archivo
	void *bloque= mmap(NULL,  tamanio,  PROT_WRITE, MAP_SHARED,  archivo,  offset);
	close(archivo);//mmap ya tiene una copia del fd

	//escribir sobre el mapeo el mensaje
	memcpy(bloque, datos, tamanio);

	//se va actualizar el archivo
	munmap(bloque, tamanio);

	free(datos);


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

void borrarBloque(int numeroBloque)
{
	if (numeroBloque >= 0)
	{
		int archivo = open(ARCHIVO_BIN,O_RDWR);
		long int offset = numeroBloque * TAMANIO_BLOQUE;
		void *bloque= mmap(NULL,  TAMANIO_BLOQUE,  PROT_WRITE, MAP_SHARED,  archivo,  offset);
		close(archivo);

		memset(bloque, 0, TAMANIO_BLOQUE);
		munmap(bloque, TAMANIO_BLOQUE);
	} else
	{
	    struct stat sb;
	    stat (ARCHIVO_BIN, & sb);
	    uint32_t tamanio = sb.st_size;

		int archivo = open(ARCHIVO_BIN,O_RDWR);
		void *bloque= mmap(NULL,  tamanio,  PROT_WRITE, MAP_SHARED,  archivo,  0);
		close(archivo);
		memset(bloque, 0, tamanio);
		munmap(bloque, tamanio);
	}
}

