/*
 * mock_FS.c
 *
 *  Created on: 19/6/2015
 *      Author: daniel
 */

/*
 * getFileContent.c
 *
 *  Created on: 3/6/2015
 *      Author: daniel
 */
//Retorna el fileContent del archivoTemporal

#include <stdio.h>
#include "socketsFunciones/sockets.h"
#include "protocolo_new.h"
#include <stdlib.h>
#include <stdlib.h>
#include "commons/string.h"
#include <stdio.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include "socketsFunciones/LecturaYEscritura.h"
#include "getBloque.h"

int main(int argc, char **argv) {
	int fs_server = abrir_servidor(8000);
	int un_cliente;

	mensaje_t *msj = malloc(sizeof(mensaje_t));

	while(1){
		 un_cliente = aceptar_cliente(fs_server);
		 printf("nuevo cliente conectado al FS %d\n", un_cliente);

		 /*
		 //SETBLOQUE
		 puts("enviando comando setbloque");
		 msj->comandoSize = strlen("setBloque 1");

		 msj->comando = "setBloque 1";
		 msj->dataSize = TAMANIO_BLOQUE;
		 msj->data = getBloque("/home/daniel/workspaceC/mock_FS/201301hourly.txt", 0);
		 enviar(un_cliente, msj);
		 free(msj->data);
		 //free(msj->comando);

		 //GET BLOQUE
		 puts("enviando comando getbloque");
		 msj->comandoSize = strlen("getBloque 0");

		 msj->comando = "getBloque 0";
		 msj->dataSize = 0;
		 msj->data = NULL;
		 enviar(un_cliente, msj);

		 puts("recibiendo bloque");
		 recibir(un_cliente, msj);
		 printf("tamaño %d\n", msj->dataSize);
		 //printf("contenido %s\n", msj->data);
		 free(msj->data);

		 //GET FILECONTENT
		 puts("enviando comando getFileContent");
		 msj->comandoSize = strlen("getFileContent temporal.txt");
		 msj->comando = "getFileContent temporal.txt";
		 msj->dataSize = 0;
		 msj->data = NULL;
		 enviar(un_cliente, msj);

		 puts("recibiendo archivo temporal");
		 recibir(un_cliente, msj);
		 printf("tamaño %d\n", msj->dataSize);
		 //msj->data[5] = '\0';
		 //printf("contenido %s\n", msj->data);
		 free(msj->data);
	*/

		 recibir(un_cliente, msj);
		 msj->comandoSize = strlen("fileContent");
		 msj->comando = "fileContent";
		 msj->dataSize = strlen("1\n2\n");
		 msj->data = "1\n2\n";
		 enviar(un_cliente, msj);

	}
}
