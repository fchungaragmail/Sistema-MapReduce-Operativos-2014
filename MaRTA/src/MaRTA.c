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

t_dictionary *filesToProcess;
t_dictionary *filesStates;
t_dictionary *nodosData;

sem_t semFilesToProcess;
sem_t semFilesToProcessPerJob;
sem_t semFilesStates;
sem_t semNodosData;
sem_t semNodoState;

sem_t leerConexiones;
sem_t semPrueba;

// MaRTA debe controlar el trafico en la red!!
int MaxPedidosEnRed = 5;
int pedidosEnRed;

Message *recvMessage;

void* listenConecctions(void* arg);

//JUAN - protocolo
void* probarConexiones(void* arg);
void enviar(int socket, mensaje_t* mensaje);
void destrabarSemPrueba();

//JUAN
struct _sockaddr_in {
	short int sin_family;  // familia de direcciones, AF_INET
	unsigned short int sin_port;    // Número de puerto
	struct in_addr sin_addr;    // Dirección de Internet
	unsigned char sin_zero[8]; // Relleno para preservar el tamaño original de struct sockaddr
};
typedef struct _sockaddr_in Sockaddr_in;

int main(void) {

	puts("MaRTA al ataque !!!");

	//init MaRTA
	filesToProcess = dictionary_create();
	filesStates = dictionary_create();
	nodosData = dictionary_create();
	sem_init(&semNodosData, 0, 1);
	sem_init(&semFilesStates, 0, 1);
	sem_init(&semFilesToProcess, 0, 1);
	sem_init(&semFilesToProcessPerJob, 0, 1);
	sem_init(&semNodoState, 0, 1);

	pthread_t connectionThread;
	//pthread_t connectToFSThread;

	sem_init(&leerConexiones, 0, 0);
	sem_init(&semPrueba, 0, 0);

	initFilesStatusCenter();
	initPlannerCenter();

	pthread_create(&connectionThread, NULL, listenConecctions,NULL);
	//pthread_create(&connectToFSThread, NULL,connectToFileSystem,NULL);


	sleep(2);
	printf("salio del sleep\n");
	//pthread_t hiloPrueba;
	//pthread_create(&hiloPrueba, NULL, probarConexiones,"hilo_1");
	probarConexiones("hilo_1");
	return EXIT_SUCCESS;
	while(1)
	{
		printf("MaRTA : esta esperando en el while principal\n");
		sem_wait(&leerConexiones);
		printf("MaRTA : esta en el while principal\n");
		printf("MaRTA : procesar mensaje\n");

		//MODELO PRODUCTOR CONSUMIDOR !!

		//si es un archivo nuevo, crear un nuevo hilo y un nuevo sem y pasar por param la direccion de memoria de este
		//"recvMessage" sera una var global a la cual tendran acceso los hilos

		//si corresponde a un hilo ya existente
		//MaRTA debe destrabar el semaforo correspondiente al hilo , para que este agarre la var global
	}
	closeServidores();

	return EXIT_SUCCESS;
}

void* listenConecctions(void *arg)
{
	initConexiones();
	printf("MaRTA-hilo : entro a listenConecctions\n");
	while(1)
		{
			recvMessage = listenConnections();
			//MODELO PRODUCTOR CONSUMIDOR !! --> ESCRIBIR EN UNA VAR GLOBAL Y DESTRABAR UN SEM
			printf("MaRTA-hilo : recibido msje en el while del hilo listenConecctions\n");
			printf("MaRTA-hilo : destrabo procesarMensage\n");
			sem_post(&leerConexiones);
		}

	pthread_exit(NULL);
}

void* connectToFileSystem(void *arg)
{
	int fileSystemSocket = connectToFS();
	addFSConnection(fileSystemSocket);
	pthread_exit(NULL);
}

// JUAN
#define IP_LISTEN "127.0.0.1"

void* probarConexiones(void* arg)
{
	char *str= (char*) arg;

	printf ("MaRTA-hilo : entro a PROBAR CONEXIONES !\n");
	int socketPrueba;
	struct sockaddr_in their_addr;

	socketPrueba = socket(AF_INET, SOCK_STREAM, 0);

	their_addr.sin_family = AF_INET;
	their_addr.sin_port = htons(K_PUERTO_LOCAL);
	inet_aton(IP_LISTEN, &(their_addr.sin_addr));
	memset(&(their_addr.sin_zero), '\o', 8);

	if(connect(socketPrueba, &their_addr, sizeof(Sockaddr_in))<0)
	{
		printf("MaRTA-hilo : CONECCTION FAILED\n");
		pthread_exit(NULL);
	}
	printf("MaRTA-hilo : CONNECTION SUCCESS \n");

	int f=0;
	while(f<20){

	sem_wait(&semPrueba);
	printf("MaRTA-hilo : destrabado enviar mensajes\n");

	mensaje_t* mensaje = malloc(sizeof(mensaje_t));
	char comando[] = "este es el comando que voy a mandar";
	mensaje->comando = malloc(strlen(comando)+1);
	strcpy(mensaje->comando,comando);
	mensaje->comandoSize = strlen(mensaje->comando);

	char data[] = "Aca irian los datos";
	mensaje->data = malloc(strlen(data)+1);
	strcpy(mensaje->data,data);
	mensaje->dataSize = strlen(mensaje->data);



	printf("probarConexiones - %s - va a mandar el msj nro %d \n",str,f);
	enviar(socketPrueba, mensaje);
	f++;
	free(mensaje);
	}

	pthread_exit(NULL);
}

void enviar(int socket, mensaje_t* mensaje)
{
	if(send(socket,&(mensaje->comandoSize), sizeof(int16_t), 0)<0){printf("ERROR EN EL SEND");};
	if(send(socket, mensaje->comando, mensaje->comandoSize, 0)<0){printf("ERROR EN EL SEND");};
	if(send(socket, &(mensaje->dataSize), sizeof(int32_t), 0)<0){printf("ERROR EN EL SEND");};
	if(send(socket, mensaje->data, mensaje->dataSize, 0)<0){printf("ERROR EN EL SEND");};
	printf("se envia mensaje \n");
}

