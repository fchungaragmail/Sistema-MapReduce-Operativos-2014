/*
 ============================================================================
 Name        : MaRTA.c
 Author      : Nicolas Buzzano
 Version     :
 Copyright   : Your copyright notice
 Description : Hello World in C, Ansi-style
 ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <commons/collections/dictionary.h>
#include "FilesStatusCenter.h"
#include "ConexionCenter.h"
#include "PlannerCenter.h"
#include "Utilities.h"
#include "VariablesGlobales.h"
#include <commons/log.h>
#include <commons/string.h>

#include "Simulador.h"

//Variables GLOBALES

t_dictionary *filesToProcess;
t_dictionary *filesStates;
t_dictionary *nodosData;
t_dictionary *fullDataTables;

sem_t semFullDataTables;
sem_t semFilesToProcess;
sem_t semFilesToProcessPerJob;
sem_t semNodosData;
sem_t semNodoState;
sem_t semEnviar;
sem_t semProdCons;
t_list *hilosData;
Message *recvMessage;
bool fileSystemDisponible;
FILE *logFile;

void initMaRTA();
void administrarHilos();
void connectToFileSystem();

int main(void) {

	puts("MaRTA al ataque !!!");
	initMaRTA();

#ifdef K_SIMULACION

	int i=0;
	while(i<31)
	{
		recvMessage = simular();
		sem_wait(&semProdCons);
		administrarHilos();

		if(i== 29){
			sleep(10000000);
		}
		i++;
	}

	closeServidores();
	return EXIT_SUCCESS;

#else

		int i=0;
		while(1)//fileSystemDisponible)
		{
			recvMessage = listenConnections();
			//administrarHilos();
		}

		closeServidores();
		return EXIT_SUCCESS;

#endif
}

void planificarHilo(void* args){

	t_dictionary *hiloDic = (t_dictionary*)args;
	sem_t *semHilo = dictionary_get(hiloDic,K_HiloDic_Sem);
	t_list *colaDePedidos = dictionary_get(hiloDic,K_HiloDic_PedidosQueue);
	initPlannerCenter();
	while(1){

		sem_wait(semHilo);
		Message *recvMessage = list_get(colaDePedidos,0);
		bool finalizarHilo = processMessage(recvMessage);
		list_remove(colaDePedidos,0);
		sem_post(&semProdCons);
		if(finalizarHilo){
			break;
		}
		if(!finalizarHilo){
			sem_post(&semProdCons);
		}
	}
	char *path = dictionary_get(hiloDic,K_HiloDic_Path);
	int jobSocket = dictionary_get(hiloDic,K_HiloDic_JobSocket);
	char *log = string_from_format("finalizo el hilo con path %s y numeroDeSocket %d",path,jobSocket);
	log_debug(logFile,log);

	free(log);
	//free(path); --> "administrarHilos" mira los paths
	sem_destroy(semHilo);
	list_destroy(colaDePedidos);
	//dictionary_destroy(hiloDic); --> "administrarHilos" mira el path q esta en eeste dic

	sem_post(&semProdCons);

	pthread_exit(0);
}

void initMaRTA(){

	//init MaRTA
	logFile = log_create("./MaRTA.log","MaRTA", true, LOG_LEVEL_DEBUG);

	filesToProcess = dictionary_create();
	nodosData = dictionary_create();
	fullDataTables = dictionary_create();
	hilosData = list_create();

	sem_init(&semNodosData, 0, 1);
	sem_init(&semFilesToProcess, 0, 1);
	sem_init(&semFilesToProcessPerJob, 0, 1);
	sem_init(&semNodoState, 0, 1);
	sem_init(&semFullDataTables, 0, 1);
	sem_init(&semEnviar, 0, 1);
	sem_init(&semProdCons, 0, 1);

#ifdef K_SIMULACION

	initFilesStatusCenter();

#else

	initFilesStatusCenter();
	initConexiones();

	//CONEXION A FILE SYSTEM !!!
	//connectToFileSystem();
	//fileSystemDisponible = true;


#endif

}

void administrarHilos(){

	int command = obtenerIdParaComando(recvMessage);
	if(command == K_NewConnection){
		sem_post(&semProdCons);
		return;
	}
	char *path = deserializeFilePath(recvMessage,command);
	int size = list_size(hilosData);
	int i;
	bool hiloYaExiste = false;

	if(command == K_ProcesoCaido){

		if(recvMessage->sockfd == getFSSocket()){

			fileSystemDisponible = false;
			log_trace(logFile,"El FileSystem cayo, MaRTA no puede seguir operando.");
			//TODO liberar todas las estructuras.
			return;
		}

		for(i=0;i<size;i++){

			t_dictionary *hiloDic = list_get(hilosData,i);
			int jobSocket = dictionary_get(hiloDic,K_HiloDic_JobSocket);

			if(jobSocket == recvMessage->sockfd){
				sem_t *semHilo = dictionary_get(hiloDic,K_HiloDic_Sem);
				sem_t _semHilo = *semHilo;
				t_list *colaDePedidos = dictionary_get(hiloDic,K_HiloDic_PedidosQueue);
				//TODO liberar elementos de colaDePedidos
				list_clean(colaDePedidos);
				list_add(colaDePedidos,recvMessage);
				sem_post(&_semHilo);
			}
		}
	}

	//TODO FS - Implementar bien esto, para muchos archivos falla
	int fsSocket = getFSSocket();
	if(fsSocket == recvMessage->sockfd){

		for(i=0;i<size;i++){
			t_dictionary *hiloDic = list_get(hilosData,i);
			int hiloJobSocket = dictionary_get(hiloDic,K_HiloDic_JobSocket);
			char *hiloPath = dictionary_get(hiloDic,K_HiloDic_Path);

			if(strcmp(hiloPath,path)==0){
				hiloYaExiste = true;
				t_list *colaDePedidos = dictionary_get(hiloDic,K_HiloDic_PedidosQueue);
				sem_t *semHilo = dictionary_get(hiloDic,K_HiloDic_Sem);
				list_add(colaDePedidos,recvMessage);
				sem_post(semHilo);
				break;
			}
		}
	}

	//JOB
	for(i=0;i<size;i++){

		t_dictionary *hiloDic = list_get(hilosData,i);
		int hiloJobSocket = dictionary_get(hiloDic,K_HiloDic_JobSocket);
		char *hiloPath = dictionary_get(hiloDic,K_HiloDic_Path);

		if((strcmp(hiloPath,path)==0)&&(hiloJobSocket==recvMessage->sockfd)){

			//EL HILO YA EXISTE
			hiloYaExiste = true;
			t_list *colaDePedidos = dictionary_get(hiloDic,K_HiloDic_PedidosQueue);
			sem_t *semHilo = dictionary_get(hiloDic,K_HiloDic_Sem);
			list_add(colaDePedidos,recvMessage);
			sem_post(semHilo);
			break;
		}
	}

	if(hiloYaExiste == false){

		//LANZAR HILO NUEVO
		sem_t *semHilo = malloc(sizeof(sem_t));
		sem_init(semHilo, 0, 1);
		char *_path = deserializeFilePath(recvMessage,command);
		t_list *colaDePedidos = list_create();
		list_add(colaDePedidos,recvMessage);
		int hiloSocket = recvMessage->sockfd;

		t_dictionary *hiloDic = dictionary_create();
		dictionary_put(hiloDic,K_HiloDic_Sem,semHilo);
		dictionary_put(hiloDic,K_HiloDic_Path,_path);
		dictionary_put(hiloDic,K_HiloDic_JobSocket,hiloSocket);
		dictionary_put(hiloDic,K_HiloDic_PedidosQueue,colaDePedidos);
		list_add(hilosData,hiloDic);

		pthread_t tPlanificador;
		pthread_create(&tPlanificador, NULL, (void*)planificarHilo, hiloDic);
	}
	free(path);
}

void connectToFileSystem()
{
	int fileSystemSocket = connectToFS();
	addFSConnection(fileSystemSocket);
}
