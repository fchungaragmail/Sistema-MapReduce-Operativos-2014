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
void* consolaChat(void* ptr);
void* recibirMensajes(void* ptr);
void IniciarCliente();
void CerrarCliente();

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

/*
 *
 * VARIABLES GLOBALES
 *
 */

int socketFd;
t_list* listaConexiones;
pthread_t threadRecibirMensajes;
pthread_t threadConsolaChat;

int main() {

	IniciarCliente();
	pthread_create(&threadConsolaChat, NULL, consolaChat, NULL);
	pthread_create(&threadRecibirMensajes, NULL, recibirMensajes, NULL);

	pthread_join(threadConsolaChat, NULL);
	CerrarCliente();
	printf("FINALIZACION CON EXITO.\n");
	return 0;

}

void* recibirMensajes(void* ptr) {

	char buffer[BUFFER_SIZE];
	while (1) {

		recv(socketFd, buffer, BUFFER_SIZE, 0);
		printf(buffer);
		printf("\n");

	}

}

void* consolaChat(void* ptr) {

	char buffer[BUFFER_SIZE];
	while (1) {
		scanf("%s", buffer);
		send(socketFd, buffer, BUFFER_SIZE, 0);

		if (strncmp("EXIT", buffer, 5) == 0) {
			break;
		}
	}
}

void IniciarCliente() {

	struct sockaddr_in their_addr; // información de la dirección de destino

	if ((socketFd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
		perror("socket");
		exit(1);
	}

	their_addr.sin_family = AF_INET;    // Ordenación de bytes de la máquina
	their_addr.sin_port = htons(PUERTO); // short, Ordenación de bytes de la red
	inet_aton(LOCALHOST, &(their_addr.sin_addr));
	memset(&(their_addr.sin_zero), '\o', 8); // poner a cero el resto de la estructura

	if (connect(socketFd, (Sockaddr_in*) &their_addr, sizeof(struct sockaddr))
			== -1) {
		error_show("ERROR AL CONECTARSE");
		CerrarCliente();
		exit(1);
	}
	printf("CONECTADO, BIENVENIDO AL CHAT!\n");

}

void CerrarCliente() {

	if (close(socketFd) == -1) {
		error_show("ERROR CERRANDO EL SOCKET!\n");
	}

}

