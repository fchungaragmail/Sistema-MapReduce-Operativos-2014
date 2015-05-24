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

#include "FilesStatusCenter.h"
#include "ConexionCenter.h"
#include "PlannerCenter.h"
#include "Utilities.h"

pthread_mutex_t mutex;
pthread_mutex_t mutex2;
sem_t escucharConexiones;
sem_t leerConexiones;

struct Message *recvMessage;

void* listenConecctions(void* arg);

int main(void) {

	puts("MaRTA al ataque !!!");

	pthread_t connectionThread;
	recvMessage=malloc(sizeof(Message));

	sem_init(&escucharConexiones, 0, 1);
	sem_init(&leerConexiones, 0, 0);

	pthread_create(&connectionThread, NULL, listenConecctions,NULL);
	initFilesStatusCenter();

	while(1)
	{
		sem_wait(&leerConexiones);

		printf("esta en el while principal\n");
		//el planif lo devuelve y MaRTA lo envia
		//MODELO PRODUCTOR CONSUMIDOR !!
		processMessage(recvMessage);

		sem_post(&escucharConexiones);
	}
	closeServidores();

	//destroy mutexs
	pthread_mutex_unlock(&mutex);
	pthread_mutex_unlock(&mutex2);
	pthread_mutex_destroy(&mutex);
	pthread_mutex_destroy(&mutex2);

	return EXIT_SUCCESS;
}

void* listenConecctions(void *arg)
{
	initConexiones();

	while(1)
		{
			printf("esta en el while del hilo listenConecctions\n");
			sem_wait(&escucharConexiones);

			recvMessage = listenConnections();
			//MODELO PRODUCTOR CONSUMIDOR !! --> ESCRIBIR EN UNA VAR GLOBAL Y DESTRABAR UN SEM

			sem_post(&leerConexiones);
		}

	pthread_exit(NULL);
}
