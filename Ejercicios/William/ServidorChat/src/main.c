#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <commons/txt.h>
#include <commons/error.h>
#include <commons/collections/list.h>

#define LOCALHOST "127.0.0.1"
#define PUERTO 9999
#define BACKLOG 10
#define BUFFER_SIZE 100

/*
 *
 * DECLARACION DE FUNCIONES
 *
 *
 */

void* escucharConexiones(void* ptr);
void* consolaChat(void* ptr);
void* clienteHandler(void* ptr);
void IniciarServidor();
void CerrarServidor();

/*
 *
 * ESTRUCTURAS
 *
 */

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

/*
 *
 * VARIABLES GLOBALES
 *
 */

int socketFd;
t_list* listaConexiones;
pthread_t threadEscucharConexiones;
pthread_t threadConsolaChat;

int main() {

	IniciarServidor();
	pthread_create(&threadEscucharConexiones, NULL, escucharConexiones, NULL);
	pthread_create(&threadConsolaChat, NULL, consolaChat, NULL);

	pthread_join(threadConsolaChat, NULL);
	CerrarServidor();
	printf("FINALIZACION CON EXITO.\n");
	return 0;

}

void* escucharConexiones(void* ptr) {

	int nuevoSocketfd;
	int sin_size = sizeof(Sockaddr_in);

	while (1) {

		Sockaddr_in their_addr;
		Conexion* conexionNueva = malloc(sizeof(Conexion));
		pthread_t hiloCliente;

		nuevoSocketfd = accept(socketFd, (Sockaddr_in*) &their_addr, &sin_size);

		conexionNueva->sockaddr_in = their_addr;
		conexionNueva->sockfd = nuevoSocketfd;
		conexionNueva->thread = hiloCliente;
		pthread_create(&hiloCliente, NULL, clienteHandler,
				(void*) conexionNueva);
		list_add(listaConexiones, (void*) conexionNueva);

	}

}

void* consolaChat(void* ptr) {

	char buffer[BUFFER_SIZE];
	while (1) {
		scanf("%s", buffer);

		if (strncmp("EXIT", buffer, 5) == 0) {
			break;
		}

		BroadCast(buffer);
	}
}

void IniciarServidor() {

	listaConexiones = list_create();

	socketFd = socket(AF_INET, SOCK_STREAM, 0);
	if (socketFd == -1) {
		error_show("ERROR INICIANDO EL SOCKET!!\n");
		exit(-1);
	}

	Sockaddr_in my_addr;
	my_addr.sin_family = AF_INET;
	my_addr.sin_port = htons(PUERTO);
	inet_aton(LOCALHOST, &(my_addr.sin_addr));
	memset(&(my_addr.sin_zero), '\0', 8);

	if (bind(socketFd, (Sockaddr_in*) &my_addr, sizeof(Sockaddr_in)) == -1) {
		error_show("ERROR EN EL BINDEO!!\n");
		CerrarServidor();
		exit(-1);
	}

	listen(socketFd, BACKLOG);
	printf("INICIO DEL SERVIDOR CON EXITO\n");

}

void CerrarServidor() {

	list_clean(listaConexiones);
	if (close(socketFd) == -1) {
		error_show("ERROR CERRANDO EL SOCKET!\n");
	}

}

void* clienteHandler(void* conexionCliente) {
	Conexion conexion = *((Conexion*) conexionCliente);
	char buffer[BUFFER_SIZE];

	while (1) {
		recv(conexion.sockfd, buffer, BUFFER_SIZE, 0);
		BroadCast(buffer);

	}

}

void BroadCast(char* mensaje) {

	int n;
	char buffer[BUFFER_SIZE];
	strcpy(buffer, mensaje);
	int count = list_size(listaConexiones);

	for (n = 0; n < count; ++n) {

		Conexion* conexion = (Conexion*) list_get(listaConexiones, n);
		send(conexion->sockfd, buffer, BUFFER_SIZE, 0);
		printf(buffer);
		printf("\n");
	}

}
