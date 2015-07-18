/*
 * ConexionCenter.c
 *
 *  Created on: 22/5/2015
 *      Author: utnso
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <fcntl.h>
#include <semaphore.h>
#include <sys/ioctl.h>
#include <inttypes.h>

#include <commons/txt.h>
#include <commons/error.h>
#include <commons/collections/list.h>
#include "ConexionCenter.h"
#include "VariablesGlobales.h"
#include "FilesStatusCenter.h"
#include "protocolo.h"

// CONSTANTES

#define LOCALHOST "127.0.0.1"
#define K_PUERTO_LOCAL 9002
#define MAX_CONNECTIONS 100

#define K_FS_IP "192.168.1.8"
#define K_FS_PUERTO "3000"

// ESTRUCTURAS

struct _sockaddr_in {
	short int sin_family;  // familia de direcciones, AF_INET
	unsigned short int sin_port;    // Número de puerto
	struct in_addr sin_addr;    // Dirección de Internet
	unsigned char sin_zero[8]; // Relleno para preservar el tamaño original de struct sockaddr
};
typedef struct _sockaddr_in Sockaddr_in;

struct _conexion {
	int sockfd;
	pthread_t thread;
	Sockaddr_in sockaddr_in;
};
typedef struct _conexion Conexion;

// VARIABLES GLOBALES

int listener;// descriptor de socket a la escucha
fd_set master;   // conjunto maestro de descriptores de fichero
fd_set read_fds; // conjunto temporal de descriptores de fichero para select()
struct sockaddr_in remoteaddr; // dirección del cliente
int fdmax;        // número máximo de descriptores de fichero
int newfd;        // descriptor de socket de nueva conexión aceptada
int addrlen;
int conexionesProcesadas;

// FUNCIONES PUBLICAS
void initConexiones();
void closeServidores();
int connectToFS();
Message* listenConnections();

//FUNCIONES PRIVADAS
int createSocket(int puerto,int socketFd);
void createListener();
void prepareConnections();
void setnonblocking();
Message *crearMessageWithCommand(char *command,int socket);


void initConexiones()
{
	createListener();
	prepareConnections();
	conexionesProcesadas=-1;
	printf("Inicio del servidor MaRTA con exito\n\n");
}

void prepareConnections()
{
	FD_ZERO(&master);    // borra los conjuntos maestro y temporal
	FD_ZERO(&read_fds);

	// escuchar
	if (listen(listener, MAX_CONNECTIONS) == -1) {
		perror("listen");
		exit(1);
	}

	FD_SET(listener, &master);// añadir listener al conjunto maestro
	fdmax = listener; // seguir la pista del descriptor de fichero mayor// por ahora es éste
}

void createListener()
{
	int yes=1;// para setsockopt() SO_REUSEADDR, más abajo

	// obtener socket a la escucha
	if ((listener = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
		perror("socket");
		exit(1);
	}
	// obviar el mensaje "address already in use" (la dirección ya se está usando)
	if (setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, &yes,sizeof(int)) == -1) {
		perror("setsockopt");
		exit(1);
	}

	// enlazar
	Sockaddr_in myaddr;     // dirección del servidor
	myaddr.sin_family = AF_INET;
	myaddr.sin_addr.s_addr = INADDR_ANY;
	myaddr.sin_port = htons(K_PUERTO_LOCAL);
	memset(&(myaddr.sin_zero), '\0', 8);
	if (bind(listener, (struct sockaddr *)&myaddr, sizeof(myaddr)) == -1) {
		perror("bind");
		exit(1);
	}
}

Message* listenConnections()
{
	printf(" SELECT en espera !!!\n");
	printf("*******************\n");
	read_fds = master; // cópialo
	if (select(FD_SETSIZE, &read_fds, NULL, NULL, NULL) == -1) {
		perror("select");
		exit(1);
	}
	printf("SELECT recibio algo !!!\n");
	int i, j;
	// explorar conexiones existentes en busca de datos que leer
	for(i = 0; i <= fdmax; i++) {
		if (FD_ISSET(i, &read_fds)) { // ¡¡tenemos datos!!
			if (i == listener) {

				// gestionar nuevas conexiones
				addrlen = sizeof(remoteaddr);
				if ((newfd = accept(listener, (struct sockaddr *)&remoteaddr,&addrlen)) == -1) {
					perror("accept");
				} else {

					FD_SET(newfd, &master); // añadir al conjunto maestro
					if (newfd > fdmax) {fdmax = newfd;} // actualizar el máximo

					//setnonblocking();

					printf("selectserver: new connection from %s on "
						"socket %d\n", inet_ntoa(remoteaddr.sin_addr), newfd);
					return crearMessageWithCommand("newConnection",newfd);
				}
			} else {

				//*****************
				//Tenemos datos de algún cliente
				conexionesProcesadas++;
				printf("llegaron datos de un cliente - nroDeMsje %d\n",conexionesProcesadas);

				Message *_recvMesage = malloc(sizeof(Message));
				_recvMesage->mensaje = malloc(sizeof(mensaje_t));
				int estado = recibir(i,_recvMesage->mensaje);

				if(estado == DESCONECTADO){
					close(i); // bye!
					FD_CLR(i, &master); // eliminar del conjunto maestro
					char *log = string_from_format("fallo el recv del proceso con socket %d",i);
					log_trace(logFile,log); free(log);
					return crearMessageWithCommand("ProcesoCaido",i);
				}

				if(estado == CONECTADO){
					printf("se recibio dataSize %d \n",_recvMesage->mensaje->dataSize);
					printf("se recibio la data %s \n",_recvMesage->mensaje->data);
					printf("se recibio comandoSize %d \n",_recvMesage->mensaje->comandoSize);
					printf("se recibio el comando %s \n",_recvMesage->mensaje->comando);
					int _s = i;
					_recvMesage->sockfd = _s;
					return _recvMesage;

				}

				//*******************
				//VEO SI QUEDAN DATOS EN EL BUFFER
				/*int count;
				ioctl(socket, FIONREAD, &count);
				while(count>0){
					ioctl(socket, FIONREAD, &count);
					if(count >0){
						_recvMesage->mensaje = recibir(i);
						printf("todabia quedan msjes en el buffer y los mensajes son :\n");
					}
				}*/
			}
		}
	}
}

int connectToFS(){

	struct sockaddr_in their_addr;
	int socketFd=-1;

	if ((socketFd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
		error_show("Error al crear socket para MARTA\n");
		exit(-1);
	}
	their_addr.sin_family = AF_INET;
	their_addr.sin_port = htons(atoi(K_FS_PUERTO));
	inet_aton(K_FS_IP, &(their_addr.sin_addr));
	memset(&(their_addr.sin_zero), '\o', 8);

	if (connect(socketFd, (Sockaddr_in*) &their_addr, sizeof(Sockaddr_in))
			== -1) {
		error_show("Error al conectarse con FS\n");
		return (EXIT_FAILURE);
	}

	printf("Conexion exitosa con FS, ip : %s, puerto :%s\n",K_FS_IP,K_FS_PUERTO);

	FD_SET(socketFd, &master); // añadir al conjunto maestro
	if (socketFd > fdmax) {fdmax = socketFd;} // actualizar el máximo

	int _socket = socketFd;
	addFSConnection(_socket);

	//HANDSHAKE A FS
	Message *msj = crearMessageWithCommand("nombre MaRTA",_socket);
	enviar(msj->sockfd,msj->mensaje);
	return socketFd;

}
void setnonblocking()
{
	int opts;

	opts = fcntl(newfd,F_GETFL);
	if (opts < 0) {
		perror("fcntl(F_GETFL)");
		exit(EXIT_FAILURE);
	}
	opts = (opts | O_NONBLOCK);
	if (fcntl(newfd,F_SETFL,opts) < 0) {
		perror("fcntl(F_SETFL)");
		exit(EXIT_FAILURE);
	}
	printf("NON-BLOCKING REALIZADO\n");
}

void closeServidores()
{
	int j;
	for(j = 0; j <= fdmax; j++) {
	// cerrar todos los sockets
		if (FD_ISSET(j, &master)) {
			// excepto al listener
			if (j != listener) {
				close(j);
			}
		}
	}
}

int createSocket(int puerto,int socketFd)
{
	socketFd = socket(AF_INET, SOCK_STREAM, 0);
			if (socketFd == -1) {
				error_show("ERROR INICIANDO EL SOCKET!!\n");
				exit(-1);
			}

			Sockaddr_in my_addr;
			my_addr.sin_family = AF_INET;
			my_addr.sin_port = htons(puerto);
			inet_aton(LOCALHOST, &(my_addr.sin_addr));
			memset(&(my_addr.sin_zero), '\0', 8);

			if (bind(socketFd, (Sockaddr_in*) &my_addr, sizeof(Sockaddr_in)) == -1) {
				error_show("ERROR EN EL BINDEO!!\n");
				closeServidores();
				exit(-1);
			}

	printf("createSocket realizado con exito\n");
	return socketFd;
}

Message *crearMessageWithCommand(char *command,int socket)
{
	Message *newConnection;
	newConnection=malloc(sizeof(Message));
	newConnection->mensaje=malloc(sizeof(mensaje_t));

	newConnection->mensaje->comandoSize=(strlen(command)+1);
	newConnection->mensaje->comando=malloc(strlen(command)+1);
	strcpy(newConnection->mensaje->comando,command);
	int _s = socket;
	newConnection->sockfd= _s;

	return newConnection;
}
