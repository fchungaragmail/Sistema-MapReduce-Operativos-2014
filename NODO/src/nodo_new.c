/*
 * nodo_new.c
 *
 *  Created on: 11/6/2015
 *      Author: utnso
 */

#include "sockets_struct_new.h"
#include "nodo_new.h"


//Global Variables
int PUERTO_FS;
char* IP_FS;
char* ARCHIVO_BIN;
char* DIR_TEMP;
char* NODO_NUEVO;
char* IP_NODO;
int PUERTO_NODO;


//Headers
void getConfig();
int initServer();
void *fs_conection_handler(void*);
//void *map_conection_handler(void*);
//void *reduce_conection_handler(void*);
//void *nodo_conection_handler(void*);


//Main Programm
int main() {

	int sockFD;
	int sockAccept;
	socklen_t size;
	Sockaddr_in client_sock;
//	pthread_t nodo_handler;
	pthread_t fs_handler;
//	pthread_t map_handler;
//	pthread_t reduce_handler;
	mensaje_t* buffer_send;
	mensaje_t* buffer_recv;

	printf("0\n");

//	memset(buffer_recv, 0, MESSAGE_LENGTH);

	printf("1\n");

	getConfig();

	sockFD = initServer();

	size = sizeof(struct sockaddr_in);

	while(1) {
	sockAccept = accept(sockFD, (struct sockaddr*) &client_sock, &size);
	if (sockAccept != 1) {
		printf("No se pudo aceptar conexión\n");
		close(sockFD);
		close(sockAccept);
		return -1;
	}

	printf("Se obtuvo una conexión desde: %s\n", inet_ntoa(client_sock.sin_addr));
	strcpy(buffer_send, "Bienvenido al servidor que sirve\n");
	send(sockAccept, &buffer_send, MESSAGE_LENGTH, 0);

	recibir(sockAccept, buffer_recv);

	if(strcmp(buffer_recv->id, "fs") == 0) {
		pthread_create(&fs_handler, NULL, fs_conection_handler, buffer_recv->comando);
	}
//
//	if(strcmp(buffer_recv, "nd") == 0) {
//		pthread_create(&nodo_handler, NULL, nodo_conection_handler, NULL);
//	}
//
//	if(strcmp(buffer_recv, "mp") == 0){
//		pthread_create(&map_handler, NULL, map_conection_handler, NULL);
//	}
//
//	if(strcmp(buffer_recv, "rd") == 0) {
//		pthread_create(&reduce_handler, NULL, reduce_conection_handler, NULL);
//	}


	} //while


	return 0;
}


void getConfig() {

	t_config* archivoConfig = config_create("/home/utnso/Escritorio/Ejercicios/NODO/NODO.config");

	PUERTO_FS = config_get_int_value(archivoConfig, "PUERTO_FS");
	IP_FS = config_get_string_value(archivoConfig, "IP_FS");
	ARCHIVO_BIN = config_get_string_value(archivoConfig, "ARCHIVO_BIN");
	DIR_TEMP = config_get_string_value(archivoConfig, "DIR_TEMP");
	NODO_NUEVO = config_get_string_value(archivoConfig, "NODO_NUEVO");
	IP_NODO = config_get_string_value(archivoConfig, "IP_NODO");
	PUERTO_NODO = config_get_int_value(archivoConfig, "PUERTO_NODO");

}


int initServer() {

	int sockFD;
	Sockaddr_in my_sock;

	my_sock.sin_family = AF_INET;
	my_sock.sin_port = htons(PUERTO_NODO);
	memset(&my_sock.sin_zero, 0, 8);
	inet_aton(IP_NODO, &my_sock.sin_addr);

	sockFD = socket(AF_INET, SOCK_STREAM, 0);
	if(sockFD == -1) {
		printf("Error al crear socket\n");
		return -1;
	} else {
		printf("Socket creado correctamente...\n");
	}

	if(bind(sockFD, (struct sockaddr*) &my_sock, sizeof(my_sock)) == -1) {
		printf("Fallo al hacer el bind...\n");
		return -1;
	} else {
		printf("Bind realizado con exito...\n");
	}

	if(listen(sockFD, 5) == -1) {
		printf("Fallo en listen\n");
	} else {
		printf("Socket escuchando conexiones...\n");
	}

	return sockFD;
}


void *fs_conection_handler(void* ptr){

	char* comando = ptr;
	char** result[4];
	result[4] = string_split(comando, " ");
	if (strcmp(*result[0], "getBloque") == 0) {
		getBloque(atoi(*result[1]));  //sacar el primer parametro
	}
	if (strcmp(*result[0], "setBloque") == 0) {   //setBloque numero [datos]
		setBloque(**result[1], *result[2]);
	}
	if (strcmp(*result[0], "getFileContent") == 0) { //getFileContent nombre
		getFileContent(*result[1]);
	}

}
//void *map_conection_handler(void* ptr){}
//void *reduce_conection_handler(void* ptr){}
//void *nodo_conection_handler(void* ptr){}

