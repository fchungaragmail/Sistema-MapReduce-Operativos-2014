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

sem_t semPrueba;

int MaxPedidosEnRed = 5; // MaRTA debe controlar el trafico en la red !!
int pedidosEnRed;
Message *recvMessage;

void initMaRTA();
void* connectToFileSystem(void *arg);

//*******************************************
//PRUEBAS de conexiones CLI-SERV
void* probarConexiones(void* arg);
void enviar(int socket, mensaje_t* mensaje);

struct _sockaddr_in {
	short int sin_family;  // familia de direcciones, AF_INET
	unsigned short int sin_port;    // Número de puerto
	struct in_addr sin_addr;    // Dirección de Internet
	unsigned char sin_zero[8]; // Relleno para preservar el tamaño original de struct sockaddr
};
typedef struct _sockaddr_in Sockaddr_in;
//*******************************************

int main(void) {

	puts("MaRTA al ataque !!!");

	initMaRTA();

	//PRUEBAS de conexiones CLI-SERV
	//sem_init(&semPrueba, 0, 0);
	//pthread_t hiloCliente;
	//pthread_create(&hiloCliente, NULL, probarConexiones,"hiloCliente_1");

	int i=0;
	while(i<6)
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
//**************************************************
// PRUEBAS de conexiones CLI-SERV

#define IP_LISTEN "127.0.0.1"

void* probarConexiones(void* arg)
{
	char *str= (char*) arg;

	sleep(2);

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

	printf("MaRTA-hilo : esperando en sem - nroDeVez %d\n",f);
	sem_wait(&semPrueba);
	printf("MaRTA-hilo : destrabado sem - nroDeVez %d\n",f);

	printf("MaRTA-hilo : esperando en sleep - nroDeVez %d\n",f);
	sleep(3);
	printf("MaRTA-hilo : destrabado sleep - nroDeVez %d\n",f);

	mensaje_t* mensaje = malloc(sizeof(mensaje_t));
	char comando[] = "este es el comando que voy a mandar";
	mensaje->comando = malloc(strlen(comando)+1);
	strcpy(mensaje->comando,comando);
	mensaje->comandoSize = (strlen(mensaje->comando)+1);

	//char data[] = "Aca irian los datos";
	//mensaje->data = malloc(strlen(data)+1);
	//strcpy(mensaje->data,data);
	mensaje->dataSize = f;


	printf("probarConexiones - %s - va a mandar el msj nro %d \n",str,f);
	enviar(socketPrueba, mensaje);
	f++;
	free(mensaje);
	}

	pthread_exit(NULL);
}

void enviar(int socket, mensaje_t* mensaje)
{
	if(send(socket,&(mensaje->comandoSize), sizeof(int16_t), 0)<0){printf("ERROR EN EL SEND\n");};
	if(send(socket, mensaje->comando, mensaje->comandoSize, 0)<0){printf("ERROR EN EL SEND\n");};
	if(send(socket, &(mensaje->dataSize), sizeof(int32_t), 0)<0){printf("ERROR EN EL SEND\n");};
	//if(send(socket, mensaje->data, mensaje->dataSize, 0)<0){printf("ERROR EN EL SEND");};

}
