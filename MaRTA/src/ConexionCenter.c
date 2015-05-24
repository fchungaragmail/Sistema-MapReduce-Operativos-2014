/*
 * ConexionCenter.c
 *
 *  Created on: 22/5/2015
 *      Author: utnso
 */
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>

#include <commons/txt.h>
#include <commons/error.h>
#include <commons/collections/list.h>

#include "ConexionCenter.h"

// CONSTANTES

#define LOCALHOST "127.0.0.1"
#define PUERTO 6704
#define MAX_CONNECTIONS 20
#define BUFFER_SIZE 10

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
struct sockaddr_in myaddr;     // dirección del servidor
struct sockaddr_in remoteaddr; // dirección del cliente
int fdmax;        // número máximo de descriptores de fichero
int newfd;        // descriptor de socket de nueva conexión aceptada
char buf[256];    // buffer para datos del cliente
int nbytes;
int yes=1;        // para setsockopt() SO_REUSEADDR, más abajo
int addrlen;

void initConexiones();
void closeServidores();
int createSocket(int puerto,int socketFd);
void createListener();
void prepareConnections();
Message* listenConnections();
Message* createErrorMessage();

void initConexiones()
{
	createListener();
	prepareConnections();

	printf("INICIO DEL SERVIDOR-MaRTA CON EXITO\n");
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
	// añadir listener al conjunto maestro
	FD_SET(listener, &master);
	// seguir la pista del descriptor de fichero mayor
	fdmax = listener; // por ahora es éste
	// bucle principal

}

Message* listenConnections()
{
	printf(" SELECT  !!!\n");
	read_fds = master; // cópialo
	if (select(fdmax+1, &read_fds, NULL, NULL, NULL) == -1) {
		perror("select");
		exit(1);
	}
	printf("SI PASA POR ACA EL SELECT SIGUIO !!!\n");
	int i, j;
	// explorar conexiones existentes en busca de datos que leer
	for(i = 0; i <= fdmax; i++) {
		if (FD_ISSET(i, &read_fds)) { // ¡¡tenemos datos!!
			if (i == listener) {
				// gestionar nuevas conexiones
				addrlen = sizeof(remoteaddr);
				if ((newfd = accept(listener, (struct sockaddr *)&remoteaddr,
														 &addrlen)) == -1) {
					perror("accept");
				} else {
					FD_SET(newfd, &master); // añadir al conjunto maestro
					if (newfd > fdmax) {    // actualizar el máximo
						fdmax = newfd;
					}
					printf("selectserver: new connection from %s on "
						"socket %d\n", inet_ntoa(remoteaddr.sin_addr), newfd);

					Message *newConection;
					newConection=malloc(sizeof(Message));
					newConection->head=K_NewConnection;
					return newConection;
				}
			} else {
				// gestionar datos de un cliente
				if ((nbytes = recv(i, buf, sizeof(buf), 0)) <= 0) {
					// error o conexión cerrada por el cliente
					if (nbytes == 0) {
						// conexión cerrada
						printf("selectserver: socket %d hung up\n", i);
					} else {
						perror("recv");
					}
					close(i); // bye!
					FD_CLR(i, &master); // eliminar del conjunto maestro
				} else {
					// tenemos datos de algún cliente
					printf("llegaron datos de algun cliente");
					mensaje_t *msj;
					msj = malloc(sizeof(mensaje_t));
					msj = recibir(i);

					printf("se recibio comandoSize %d \n",msj->comandoSize);
					printf("se recibio comandoData %s \n",msj->comando);

					Message* message;
					message=malloc(sizeof(Message));
					message->sockfd=K_FSMessage;
					//message->messageData = CUERPO;
					//message->head = HEAD;

					return message;

					/*for(j = 0; j <= fdmax; j++) {
						// ¡enviar a todo el mundo!
						if (FD_ISSET(j, &master)) {
							// excepto al listener y a nosotros mismos
							if (j != listener && j != i) {
								if (send(j, buf, nbytes, 0) == -1) {
									perror("send");
								}
							}
						}
					}*/
				}
			} // Esto es ¡TAN FEO!
		}
	}

	printf("ConexionCenter: fallo listenConnections");	//nunca deberia llegar aca
	return createErrorMessage();						//xq el select() es bloqueante
}

Message* createErrorMessage()
{
	Message *errorMsg;
	errorMsg=malloc(sizeof(Message));
	errorMsg->head=K_Error;
	return errorMsg;
}

void closeServidores()
{
/*	list_clean(listaConexiones);
	if (close(listener) == -1) {
		error_show("ERROR CERRANDO EL SOCKET DEL JOB!\n");
	}*/
	/*
	list_clean(listaConexiones);
	if (close(socketFd_Job) == -1) {
		error_show("ERROR CERRANDO EL SOCKET DEL JOB!\n");
	}
	if (close(socketFd_FS) == -1) {
		error_show("ERROR CERRANDO EL SOCKET DEL FS!\n");
	}*/
}

void createListener()
{
	// obtener socket a la escucha
	if ((listener = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
		perror("socket");
		exit(1);
	}
	// obviar el mensaje "address already in use" (la dirección ya se está usando)
	if (setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, &yes,
														sizeof(int)) == -1) {
		perror("setsockopt");
		exit(1);
	}
	// enlazar
	myaddr.sin_family = AF_INET;
	myaddr.sin_addr.s_addr = INADDR_ANY;
	myaddr.sin_port = htons(PUERTO);
	memset(&(myaddr.sin_zero), '\0', 8);
	if (bind(listener, (struct sockaddr *)&myaddr, sizeof(myaddr)) == -1) {
		perror("bind");
		exit(1);
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
