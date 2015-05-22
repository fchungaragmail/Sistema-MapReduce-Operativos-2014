/*
 * ConexionCenter.c
 *
 *  Created on: 22/5/2015
 *      Author: utnso
 */

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <commons/txt.h>
#include <commons/error.h>
#include <commons/collections/list.h>

// CONSTANTES

#define LOCALHOST "127.0.0.1"
#define PUERTO_FS 9999
#define PUERTO_JOB 8888
#define MAX_JOBS 20
#define BUFFER_SIZE_HEAD 10

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

int socketFd_Job;
int socketFd_FS;

pthread_t threadEscucharConexiones;

void initConexiones();
void listenFS();
void listenJobs();
void closeServidores();
int createSocket(int puerto,int socketFd);

void initConexiones()
{
	listaConexiones = list_create();

	socketFd_Job = createSocket(PUERTO_JOB,socketFd_Job);
	socketFd_FS =  createSocket(PUERTO_FS,socketFd_FS);

	printf("INICIO DEL SERVIDOR CON EXITO\n");
}

void listenFS()//ya esta en un hilo de MaRTA
{
	listen(socketFd_FS,1);//solo hay una instancia de FS

	int nuevoSocketfd;
	int sin_size = sizeof(Sockaddr_in);

	Sockaddr_in their_addr;
	Conexion* conexionNueva = malloc(sizeof(Conexion));
	pthread_t hiloCliente;

	nuevoSocketfd = accept(socketFd_FS, (Sockaddr_in*) &their_addr, &sin_size);

	conexionNueva->sockaddr_in = their_addr;
	conexionNueva->sockfd = nuevoSocketfd;
	conexionNueva->thread = hiloCliente;
	list_add(listaConexiones, (void*) conexionNueva);

	char buffer_head[BUFFER_SIZE_HEAD];

	while (1) {
			recv(conexionNueva.sockfd, buffer_head, BUFFER_SIZE_HEAD, 0);
			procesarMensaje(buffer);
	}
}

void procesarMensaje(char* mensaje) {

	int n;
	char buffer[BUFFER_SIZE_HEAD];
	strcpy(buffer, mensaje);
	int count = list_size(listaConexiones);

	for (n = 0; n < count; ++n) {


		Conexion* conexion = (Conexion*) list_get(listaConexiones, n);
		send(conexion->sockfd, buffer, BUFFER_SIZE, 0);
		printf(buffer);
		printf("\n");
	}

}

void listenJobs() //ya esta en un hilo de MaRTA
{
	listen(socketFd_Job, MAX_JOBS);

	int nuevoSocketfd;
	int sin_size = sizeof(Sockaddr_in);

	while (1) {

		Sockaddr_in their_addr;
		Conexion* conexionNueva = malloc(sizeof(Conexion));
		pthread_t hiloCliente;

		nuevoSocketfd = accept(socketFd_Job, (Sockaddr_in*) &their_addr, &sin_size);

		conexionNueva->sockaddr_in = their_addr;
		conexionNueva->sockfd = nuevoSocketfd;
		conexionNueva->thread = hiloCliente;
		pthread_create(&hiloCliente, NULL, clienteHandler,
				(void*) conexionNueva); // lo 1ero q me pasa es la direccion de su arch. a procesar
		list_add(listaConexiones, (void*) conexionNueva);

	}
}

void closeServidores()
{
	list_clean(listaConexiones);
	if (close(socketFd_Job) == -1) {
		error_show("ERROR CERRANDO EL SOCKET DEL JOB!\n");
	}
	if (close(socketFd_FS) == -1) {
		error_show("ERROR CERRANDO EL SOCKET DEL FS!\n");
	}
}

static int createSocket(int puerto,int socketFd)
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

	return socketFd;

}
