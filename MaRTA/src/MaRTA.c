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

sem_t semFilesToProcess;
sem_t semFilesToProcessPerJob;
sem_t semFilesStates;
sem_t semNodosData;
sem_t semNodoState;

int pedidosEnRed;
Message *recvMessage;

void initMaRTA();
void* connectToFileSystem(void *arg);

int main(void) {

	puts("MaRTA al ataque !!!");

	initMaRTA();

	int i=0;
	while(i<12)
	{
		//recvMessage = listenConnections(); //--> SERVIDOR con el select();

		recvMessage = simular();
		processMessage(recvMessage);
		i++;
		//***********************************
		//si es un archivo nuevo, crear un nuevo hilo y un nuevo sem y pasar por param la direccion de memoria de este
		//"recvMessage" sera una var global a la cual tendran acceso los hilos

		//si corresponde a un hilo ya existente
		//MaRTA debe destrabar el semaforo correspondiente al hilo , para que este agarre la var global
		//***********************************
	}
	printf("MaRTA FINALIZO !!!");
	//closeServidores();
	return EXIT_SUCCESS;
}

void* connectToFileSystem(void *arg)
{
	int fileSystemSocket = connectToFS();
	addFSConnection(fileSystemSocket);
	pthread_exit(NULL);
}

void initMaRTA(){

	//init MaRTA
	filesToProcess = dictionary_create();
	filesStates = dictionary_create();
	nodosData = dictionary_create();
	sem_init(&semNodosData, 0, 1);
	sem_init(&semFilesStates, 0, 1);
	sem_init(&semFilesToProcess, 0, 1);
	sem_init(&semFilesToProcessPerJob, 0, 1);
	sem_init(&semNodoState, 0, 1);

	initFilesStatusCenter();
	initPlannerCenter();
	//initConexiones();

	//CONEXION A FILE SYSTEM !!!
	//pthread_t connectToFSThread;
	//pthread_create(&connectToFSThread, NULL,connectToFileSystem,NULL);
}
