/*
 * nodo_job_functions_new.c
 *
 *  Created on: 11/6/2015
 *      Author: utnso
 */

#include "nodo_job_functions_new.h"


/*Ejecuta el script sobre el contenido del numeroBloque del espacioDatos, el resultado se
 * almacena en archivoTemporal1
 * Luego ejecuta sort sobre archivoTemporal1 y lo almacenta en archivoTemporal2
 * */
#define FALLO_MAPPING 0
#define OK_MAPPING 1
int mapping(char *script, int numeroBloque, char *archivoTemporal1,
		char* archivoTemporal2) {


	int p[2];
	if (pipe(p) < 0){
		log_info(log_nodo, "Fallo syscall PIPE() en mapping()");
		return FALLO_MAPPING;
	}

	//para escrbir el bloque en la tuberia

	int resultFork;
	if ((resultFork = fork()) < 0) {
		log_info(log_nodo, "Fallo syscall FORK() 1 en mapping()");
		return FALLO_MAPPING;

	}else if(resultFork == 0) {
		//Se ejecuta el hijo
		if(close(0) < 0){
			log_info(log_nodo, "Fallo syscall CLOSE() en mapping()");
			return FALLO_MAPPING;
		}

		if(dup(p[0]) < 0){
			log_info(log_nodo, "Fallo syscall DUP() en mapping()");
			return FALLO_MAPPING;
		}


		//cambio la salida standar
		if(close(1) < 0){
			log_info(log_nodo, "Fallo syscall CLOSE() en mapping()");
			return FALLO_MAPPING;
		}

		if(creat(archivoTemporal1, 0777) < 0){
			log_info(log_nodo, "Fallo syscall CREAT() en mapping()");
			return FALLO_MAPPING;
		}

		if(close(p[1]) < 0){
			log_info(log_nodo, "Fallo syscall CLOSE() en mapping()");
			return FALLO_MAPPING;
		}

		if(execv(script, NULL) < 0){
			log_info(log_nodo, "Fallo syscall EXEXV() en mapping()");
			return FALLO_MAPPING;
		}
		exit(EXIT_SUCCESS);

	}

	//continua el padre
	if(close(p[0]) < 0){
		log_info(log_nodo, "Fallo syscall CLOSE() en mapping()");
		return FALLO_MAPPING;
	}

	int32_t length;

	char *bloque = getBloque(numeroBloque, &length);
	if(bloque == NULL){
		log_info(log_nodo, "Fallo getBloque() en mapping()");
		return FALLO_MAPPING;
	}
	sleep(1);
	int retWrite = write(p[1], bloque, length);
	if( retWrite  < 0){
		perror("");
		log_info(log_nodo, "Fallo syscall WRITE() en mapping()");
		return FALLO_MAPPING;
	}

	log_info(log_nodo, "Write escribio %d", retWrite);


	if(close(p[1])< 0){
		perror("");
		log_info(log_nodo, "Fallo syscall CLOSE() en mapping()");
		return FALLO_MAPPING;
	} else{
		log_info(log_nodo, "Se realizo bien el syscall CLOSE() en mapping()");
	}



	if(waitpid(resultFork, NULL, 0) < 0){
		log_info(log_nodo, "Fallo syscall WAIT() en mapping()");
		return FALLO_MAPPING;
	}


	//para aplicar sort
	if ((resultFork = fork()) < 0) {
		log_info(log_nodo, "Fallo syscall FORK() 2  en mapping()");
		return FALLO_MAPPING;

	}else if(resultFork == 0) {
		if(close(0) < 0){
			log_info(log_nodo, "Fallo syscall CLOSE() en mapping()");
			return FALLO_MAPPING;
		}

		if(open(archivoTemporal1, O_RDONLY) < 0){
			log_info(log_nodo, "Fallo syscall OPEN() en mapping()");
			return FALLO_MAPPING;
		}

		if(close(1) < 0){
			log_info(log_nodo, "Fallo syscall CLOSE() en mapping()");
			return FALLO_MAPPING;
		}

		if(creat(archivoTemporal2, 0777) < 0){
			log_info(log_nodo, "Fallo syscall CREAT() en mapping()");
			return FALLO_MAPPING;
		}

		if(system("sort") < 0){
			log_info(log_nodo, "Fallo syscall EXEXLP() en mapping()");
			return FALLO_MAPPING;
		}

		exit(EXIT_SUCCESS);
	}

	free(bloque);

	if(waitpid(resultFork, NULL, 0) < 0){
		log_info(log_nodo, "Fallo syscall WAIT() en mapping()");
		return FALLO_MAPPING;
	}

	return OK_MAPPING;//MAPING OK

}

//reducing(buffer_recv->data, archivosParaReduce ,result[1]);
/*
 * cantidad de archivos nodo local
 * 		RAT1, RAT2, ...
 *
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
#include "aparearArchivos_new.h"
void aplicarScriptReduce(char *script, char *archivoTemporal1,
		char* archivoTemporal2);
int procesarArchivoRemoto(int conexionNodoRemoto, char* nombreArchivoRemoto);
int32_t conectarseANodoRemoto(char *ip, int puerto);

int reduce(char *script, char *archivosParaReduce, char *archivoTemporalFinal,
		char* ipNodoFallido) {

	char *archivosParaApareo = string_new();
	char **pedidoReduce = string_split(archivosParaReduce, " ");

	//nodo local
	int i;
	for (i = 1; i <= atoi(pedidoReduce[0]); ++i) {
		if (i != 1) {
			string_append(&archivosParaApareo, " ");
		}
		string_append(&archivosParaApareo,
				pedidoReduce[i]);
	}

	//int cantidadNodosRemotos = atoi(pedidoReduce[i++]);
	//i++;
	//nodos remotos
	while (pedidoReduce[i] != NULL) {

		//establezco conexion con un nodo remoto
		char *ip = pedidoReduce[i++];
		int puerto = atoi(pedidoReduce[i++]);

		int conexionNodo = conectarseANodoRemoto(ip, puerto);

		if (conexionNodo == -1) {
			string_append(&ipNodoFallido, ip);
			return -1;
		}
		//solicito archivos remotos y los proceso
		int cantidadArchivosRemotos = atoi(pedidoReduce[i++]);
		int k;
		int resultadoProcesamiento;
		for (k = 1; k <= cantidadArchivosRemotos; ++k) {
			resultadoProcesamiento = procesarArchivoRemoto(conexionNodo, pedidoReduce[i]);

			if( resultadoProcesamiento == -1){
				string_append(&ipNodoFallido, ip);
			}
			string_append(&archivosParaApareo, " ");
			string_append(&archivosParaApareo,
					pedidoReduce[i++]);
		}
	}

	//apareo
	char *archivoApareado = aparearArchivos(archivosParaApareo);
	//char *archivoApareado = "/home/daniel/workspaceC/reduccion/tmp/temporalApareado.txt";
	aplicarScriptReduce(script, archivoApareado, archivoTemporalFinal);

	free(archivosParaApareo);
	return 0;
}

#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <sys/wait.h>
#include "nodo_fs_functions_new.h"
//aplica el script sobre archivoTemporal1 y lo guarda en archivoTemporal2
void aplicarScriptReduce(char *script, char *archivoTemporal1,
		char* archivoTemporal2) {

	int p[2];
	pipe(p);
	pid_t childPid;

	//para escrbir el bloque en la tuberia
	if (fork() == 0) {
		childPid = getpid();
		close(p[0]);
		t_fileContent *archivoTemporal = getFileContent(archivoTemporal1);
		write(p[1], archivoTemporal->contenido, archivoTemporal->size);
		exit(EXIT_SUCCESS);
	}

	waitpid(childPid, 0, 0);
	//para aplicar el script
	if (fork() == 0) {
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
int32_t conectarseANodoRemoto(char *ip, int puerto) {
	return new_connection(ip, puerto);
}

#include "protocolo_new.h"
#define DIR_TEMP "/tmp/"
/*
 * Se obtiene el archivo remoto y se guarda en el espacio temporal
 * */
int procesarArchivoRemoto(int conexionNodoRemoto, char* nombreArchivoRemoto) {
	//GET FILECONTENT
	mensaje_t *msj = malloc(sizeof(mensaje_t));
	puts("enviando comando getFileContent");
	char *comando = malloc(50);
	sprintf(comando, "%s %s", "getFileContent", nombreArchivoRemoto);
	msj->comandoSize = strlen(comando);
	msj->comando = comando;
	msj->dataSize = 0;
	msj->data = NULL;
	enviar(conexionNodoRemoto, msj);

	puts("recibiendo archivo temporal");
	int resultado = recibir(conexionNodoRemoto, msj);
	printf("tamaño %d\n", msj->dataSize);

	if( resultado == DESCONECTADO){
		return -1;
	}

	//SE GUARDA EL ARCHIVO EN EL ESPACIO TEMPORAL
	char *pathArchivoTemporal = malloc(50);
	sprintf(pathArchivoTemporal, "%s%s", DIR_TEMP, nombreArchivoRemoto);
	int archivoTemporal = creat(pathArchivoTemporal, 0777);
	write(archivoTemporal, msj->data, msj->dataSize);
	free(msj->data);
}
