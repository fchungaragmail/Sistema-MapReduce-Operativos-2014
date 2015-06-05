void *getBloque(int numero);
void setBloque(int numero, void* datos);
void *getFileContent(char *nombreFile);

#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#define ESPACIO_DATOS "/home/daniel/Escritorio/espacioTemporal/datos.bin"
#define TAMANIO_BLOQUE 20971520 // 20mb
/*
 * Retorna el contenido de un bloque del espacio temporal
 * Se mapea el bloque en memoria
 * truncate -s 1G datos.bin PARA CREAR ARCHIVO DE DATOS DESDE LA TERMINAL
 */
void *getBloque(int numeroDeBloque){
}
