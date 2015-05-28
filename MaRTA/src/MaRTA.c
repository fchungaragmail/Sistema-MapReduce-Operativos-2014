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

sem_t escucharConexiones;
sem_t leerConexiones;
sem_t semPrueba;

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
	pthread_t connectionThread;
	//pthread_t connectToFSThread;
	recvMessage=malloc(sizeof(Message));
	recvMessage->mensaje=malloc(sizeof(mensaje_t));

	sem_init(&escucharConexiones, 0, 1);
	sem_init(&leerConexiones, 0, 0);
	sem_init(&semPrueba, 0, 1);

	initFilesStatusCenter();
	initPlannerCenter();

	pthread_create(&connectionThread, NULL, listenConecctions,NULL);
	//pthread_create(&connectToFSThread, NULL,connectToFileSystem,NULL);


	sleep(2);
	printf("salio del sleep\n");
	pthread_t hiloPrueba;
	pthread_create(&hiloPrueba, NULL, probarConexiones,"hilo_1");
	/*
	int t=0;
	while (t<4){
		sleep(0.5);
		if(t==1){pthread_create(&hiloPrueba, NULL, probarConexiones,"hilo_1");};
		if(t==2){pthread_create(&hiloPrueba, NULL, probarConexiones,"hilo_2");};
		if(t==3){pthread_create(&hiloPrueba, NULL, probarConexiones,"hilo_3");};
		t++;
	}
*/
	while(1)
	{
		printf("MaRTA : esta esperando en el while principal\n");
		sem_wait(&leerConexiones);
		printf("MaRTA : esta en el while principal\n");

		//el planif lo devuelve y MaRTA lo envia
		//MODELO PRODUCTOR CONSUMIDOR !!
		processMessage(recvMessage);
		sem_post(&semPrueba);
		sem_post(&escucharConexiones);
	}
	closeServidores();

	//destroy semaphores
	sem_destroy(&leerConexiones);
	sem_destroy(&escucharConexiones);

	return EXIT_SUCCESS;
}

void* listenConecctions(void *arg)
{
	initConexiones();

	while(1)
		{
			printf("MaRTA-hilo : esta esperando en el while del hilo listenConecctions\n");
			sem_wait(&escucharConexiones);
			printf("MaRTA-hilo : entro en el while del hilo listenConecctions\n");

			recvMessage = listenConnections();

			//MODELO PRODUCTOR CONSUMIDOR !! --> ESCRIBIR EN UNA VAR GLOBAL Y DESTRABAR UN SEM
			printf("MaRTA-hilo : recibido msje en el while del hilo listenConecctions\n");
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

	mensaje_t* mensaje = malloc(sizeof(mensaje_t));
	char comando[] = "este es el comando que voy a mandar";
	mensaje->comando = malloc(strlen(comando));
	strcpy(mensaje->comando,comando);
	mensaje->comandoSize = strlen(mensaje->comando);

	char data[] = "Aca irian los datos";
	mensaje->data = malloc(strlen(data));
	strcpy(mensaje->data,data);
	mensaje->dataSize = strlen(mensaje->data);

	int f=0;
	while(f<20){
		printf("probarConexiones - %s - va a mandar el msj \n",str);
		sem_wait(&semPrueba);
		enviar(socketPrueba, mensaje);
		f++;
	}

	pthread_exit(NULL);
}
void destrabarSemPrueba()
{
	sem_post(&semPrueba);
}
void enviar(int socket, mensaje_t* mensaje)
{
	//int i,j;
	//memcpy(&i,&(mensaje->comandoSize),sizeof(mensaje->comandoSize));
	//memcpy(&j,&(mensaje->dataSize),sizeof(mensaje->comandoSize));

	if(send(socket,&(mensaje->comandoSize), sizeof(int), 0)<0){printf("ERROR EN EL SEND");};
	if(send(socket, mensaje->comando, mensaje->comandoSize, 0)<0){printf("ERROR EN EL SEND");};
	if(send(socket, &(mensaje->dataSize), sizeof(long), 0)<0){printf("ERROR EN EL SEND");};
	if(send(socket, mensaje->data, mensaje->dataSize, 0)<0){printf("ERROR EN EL SEND");};
}

