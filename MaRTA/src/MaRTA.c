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

#include "Simulador.h"

//Variables GLOBALES

t_dictionary *filesToProcess;
t_dictionary *filesStates;
t_dictionary *nodosData;
t_dictionary *fullDataTables;

sem_t semFullDataTables;
sem_t semFilesToProcess;
sem_t semFilesToProcessPerJob;
sem_t semFilesStates;
sem_t semNodosData;
sem_t semNodoState;

t_list *hilosData;
Message *recvMessage;

void initMaRTA();
void administrarHilos();
void* connectToFileSystem(void *arg);

int main(void) {

	puts("MaRTA al ataque !!!");

	initMaRTA();

	int i=0;
	while(i<30)
	{
		//***********
		//recvMessage = listenConnections();
		//administrarHilos();
		//***********
		recvMessage = simular();
		processMessage(recvMessage);
		i++;
		//***********
	}

	printf("MaRTA FINALIZO !!!");
	//closeServidores();
	return EXIT_SUCCESS;
}

void planificarHilo(void* args){

	t_dictionary *hiloDic = (t_dictionary*)args;
	initPlannerCenter();
	sem_t *semHilo = dictionary_get(hiloDic,K_HiloDic_Sem);
	t_list *colaDePedidos = dictionary_get(hiloDic,K_HiloDic_PedidosQueue);

	while(1){

		sem_wait(semHilo);
		Message *recvMessage = list_get(colaDePedidos,0);
		bool finalizarHilo = processMessage(recvMessage);
		list_remove(colaDePedidos,0);

		if(finalizarHilo){ break; }
	}

	 pthread_exit(0);
}

void initMaRTA(){

	//init MaRTA
	filesToProcess = dictionary_create();
	filesStates = dictionary_create();
	nodosData = dictionary_create();
	fullDataTables = dictionary_create();
	hilosData = list_create();

	sem_init(&semNodosData, 0, 1);
	sem_init(&semFilesStates, 0, 1);
	sem_init(&semFilesToProcess, 0, 1);
	sem_init(&semFilesToProcessPerJob, 0, 1);
	sem_init(&semNodoState, 0, 1);
	sem_init(&semFullDataTables, 0, 1);

	//CONEXION A FILE SYSTEM !!!
	//connectToFileSystem();

	initFilesStatusCenter();
	initPlannerCenter();
	//initConexiones();
}

void administrarHilos(){

	int command  = deserializeComando(recvMessage);
	char *path = deserializeFilePath(recvMessage,command);
	int size = list_size(hilosData);
	int i;
	bool hiloYaExiste = false;

	if(command = K_Job_JobCaido){
		for(i=0;i<size;i++){

			t_dictionary *hiloDic = list_get(hilosData,i);
			int jobSocket = dictionary_get(hiloDic,K_HiloDic_JobSocket);
			if(jobSocket == recvMessage->sockfd){

				sem_t *semHilo = dictionary_get(hiloDic,K_HiloDic_Sem);
				t_list *colaDePedidos = dictionary_get(hiloDic,K_HiloDic_PedidosQueue);
				list_clean(colaDePedidos);
				list_add(colaDePedidos,recvMessage);
				sem_post(semHilo);
			}
		}
		return;
	}

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
		sem_t *semHilo;
		sem_init(semHilo, 0, 1);
		char *_path = deserializeFilePath(recvMessage,command);
		t_list *colaDePedidos = list_create();
		list_add(colaDePedidos,recvMessage);

		t_dictionary *hiloDic = dictionary_create();
		dictionary_put(hiloDic,K_HiloDic_Sem,semHilo);
		dictionary_put(hiloDic,K_HiloDic_Path,_path);
		dictionary_put(hiloDic,K_HiloDic_JobSocket,recvMessage->sockfd);
		dictionary_put(hiloDic,K_HiloDic_PedidosQueue,colaDePedidos);

		list_add(hilosData,hiloDic);

		pthread_t tPlanificador;
		pthread_create(&tPlanificador, NULL, (void*)planificarHilo, hiloDic);

	}
	free(path);
}

void* connectToFileSystem(void *arg)
{
	int fileSystemSocket = connectToFS();
	addFSConnection(fileSystemSocket);
	pthread_exit(NULL);
}
