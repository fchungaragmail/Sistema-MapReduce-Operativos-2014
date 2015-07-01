/*
 * reduce.c
 *
 *  Created on: 27/6/2015
 *      Author: daniel
 */


//reducing(buffer_recv->data, archivosParaReduce ,result[1]);
/*
 * cantidad de archivos nodo local
 * 		RAT1, RAT2, ...
 * cantidad de nodos remotos
 * 		ip puerto nodo remoto1
 * 			cantidad de archivos nodo remoto1
 * 				RAT1, RAT2, ...
 * 		ip puerto nodo remoto2
 * 			cantidad de archivos nodo remoto2
 * 				RAT1, RAT2, ...
 *
 * Algoritmo:
 *
 * declarar archivosParaApareo
 *
 * Para el nodo local
 * 		Para cada nombre de archivo temporal
 * 			concatenar el nombre de archivo actual con los archivosParaApereo
 * 		fin
 * fin
 *
 * Para los nodos remotos
 * 		Para cada nodo remoto
 * 			conectarse con nodo remoto
 * 			Para cada nombre de archivo remoto
 * 				solicitar archivo temporal
 * 				guardar archivo temporal en el espacio temporal
 * 				concatenar el nombre de archivo actual con los archivosParaApereo
 * 			fin
 * 		fin
 * fin
 *
 * aparearArchivos
 *
 * aplicar el script
 *
 * fin
*/
#include <stdlib.h>
#include "commons/string.h"
#include <string.h>

void aplicarScriptReduce(char *script, char *archivoTemporal1, char* archivoTemporal2);
void procesarArchivoRemoto(int conexionNodoRemoto, char* nombreArchivoRemoto);
int conectarseANodoRemoto(char *ip, int puerto);
void reducing(char *script, char *archivosParaReduce, char *archivoTemporalFinal){

	char *archivosParaApareo = malloc(150);
	char **pedidoReduce = string_split(archivosParaReduce, " ");

	//nodo local
	int i;
	for (i = 1; i <= atoi(pedidoReduce[0]); ++i) {
		if (i != 1) {
			archivosParaApareo = strcat(archivosParaApareo, " ");
		}
		archivosParaApareo = strcat(archivosParaApareo, pedidoReduce[i]);
	}

	int cantidadNodosRemotos = atoi(pedidoReduce[i++]);
	int j = 0;

	//nodos remotos
	while(j != cantidadNodosRemotos){

		//establezco conexion con un nodo remoto
		char *ip = pedidoReduce[i++];
		int puerto = atoi(pedidoReduce[i++]);

		int conexionNodo = conectarseANodoRemoto(ip, puerto);

		//solicito archivos remotos y los proceso
		int cantidadArchivosRemotos = atoi(pedidoReduce[i++]);
		int k;
		for (k = 1; k <= cantidadArchivosRemotos; ++k) {
			procesarArchivoRemoto(conexionNodo, pedidoReduce[i]);
			archivosParaApareo = strcat(archivosParaApareo, " ");
			archivosParaApareo = strcat(archivosParaApareo, pedidoReduce[i++]);
		}
		j++;
	}

	//apareo
	//archivoApareado = aparearArchivos(archivosParaApareo);
	char *archivoApareado = "/home/daniel/workspaceC/reduccion/tmp/temporalApareado.txt";
	aplicarScriptReduce(script, archivoApareado, archivoTemporalFinal);

}

#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <sys/wait.h>
#include "getFileContent.h"
//aplica el script sobre archivoTemporal1 y lo guarda en archivoTemporal2
void aplicarScriptReduce(char *script, char *archivoTemporal1, char* archivoTemporal2) {

		int p[2];
		pipe(p);

		//para escrbir el bloque en la tuberia
		if(fork()==0)
		{
			close(p[0]);
			t_fileContent  *archivoTemporal = getFileContent(archivoTemporal1);
			write(p[1], archivoTemporal->contenido, archivoTemporal->size);
			exit (EXIT_SUCCESS);
		}

		wait(0);
		//para aplicar el script
		if(fork()==0)
		{
			close(p[1]);

			//cambio la entrada standar por la tuberia
			close(0);
			dup(p[0]);

			//cambio la salida standar
			close(1);
			creat(archivoTemporal2, 0777);

			system(script);

		}
}


#include "socketsFunciones/sockets.h"
int conectarseANodoRemoto(char *ip, int puerto){
	return new_connection(ip, puerto);
}

#include "protocolo_new.h"
#define DIR_TEMP "/home/daniel/workspaceC/reduccion/tmp/"
/*
 * Se obtiene el archivo remoto y se guarda en el espacio temporal
 * */
void procesarArchivoRemoto(int conexionNodoRemoto, char* nombreArchivoRemoto){
	 //GET FILECONTENT
	 mensaje_t *msj = malloc(sizeof(mensaje_t));
	 puts("enviando comando getFileContent");
	 char *comando = malloc(50);
	 sprintf(comando,"%s %s","getFileContent", nombreArchivoRemoto);
	 msj->comandoSize = strlen(comando);
	 msj->comando = comando;
	 msj->dataSize = 0;
	 msj->data = NULL;
	 enviar(conexionNodoRemoto, msj);

	 puts("recibiendo archivo temporal");
	 recibir(conexionNodoRemoto, msj);
	 printf("tamaño %d\n", msj->dataSize);

	 //SE GUARDA EL ARCHIVO EN EL ESPACIO TEMPORAL
	 char *pathArchivoTemporal = malloc(50);
	 sprintf(pathArchivoTemporal,"%s%s", DIR_TEMP, nombreArchivoRemoto);
	 int archivoTemporal = creat(pathArchivoTemporal, 0777);
	 write(archivoTemporal, msj->data, msj->dataSize);
	 free(msj->data);
}

int main(int argc, char **argv) {
	reducing("sort", "1 tmp1.txt 1 127.0.0.1 8000 1 temporal.txt", "/home/daniel/workspaceC/reduccion/tmp/temporalfinal.txt");
	return 0;
}

