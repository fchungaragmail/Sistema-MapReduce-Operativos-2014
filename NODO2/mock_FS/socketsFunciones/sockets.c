/*
 * sockets.c
 *
 *  Created on: 27/05/2014
 *      Author: utnso
 */
#include "sockets.h"
#include <stdint.h>

/* Funciones Servidor */
/*
* Se le pasa un socket de servidor y acepta en el una conexion de cliente.
* devuelve el descriptor del socket del cliente o -1 si hay problemas.
*/
int32_t aceptar_cliente (int descriptor){
	socklen_t longitud_cliente;
	struct sockaddr cliente;
	int32_t hijo;

		/*
		* La llamada a la funcion accept requiere que el parametro
		* longitud_cliente contenga inicialmente el tamano de la
		* estructura Cliente que se le pase. A la vuelta de la
		* funcion, esta variable contiene la longitud de la informacion
		* util devuelta en Cliente
		*/
	longitud_cliente = sizeof(cliente);
	hijo = accept (descriptor, &cliente, &longitud_cliente);


	if (hijo == SOCKET_ERROR){
		perror("");
		return SOCKET_ERROR;
	}


		/*
		* Se devuelve el descriptor en el que esta "enchufado" el cliente.
		*/
	return hijo;
}


/*
* Abre un socket servidor de tipo AF_INET. Devuelve el descriptor
* del socket o -1 si hay probleamas
*/
int32_t abrir_servidor (int puerto){
	struct sockaddr_in direccion;
	int descriptor;

	/*
	* se abre el socket
	*/
	descriptor = socket (AF_INET, SOCK_STREAM, 0);
	if (descriptor == SOCKET_ERROR){
		perror("");
		return SOCKET_ERROR;
	}
	//--Liberar puerto despues de cerrarlo
		int optval = 1;
			if (setsockopt(descriptor, SOL_SOCKET, SO_REUSEADDR, &optval,sizeof optval) == -1){
				return SOCKET_ERROR;
			}
	/*
	* Se rellenan los campos de la estructura Direccion, necesaria
	* para la llamada a la funcion bind()
	*/
	direccion.sin_family = AF_INET;
	direccion.sin_port = htons(puerto);
	direccion.sin_addr.s_addr =INADDR_ANY;
	if (bind (descriptor, (struct sockaddr *)&direccion, sizeof(direccion)) == SOCKET_ERROR){
		perror("");
		close (descriptor);
		return SOCKET_ERROR;
	}

	/*
	* Se avisa al sistema que comience a atender llamadas de clientes
	*/
	if (listen (descriptor, CONECCIONES_PENDIENTES) == SOCKET_ERROR){
		perror("");
		close (descriptor);
		return SOCKET_ERROR;
	}

	/*
	* Se devuelve el descriptor del socket servidor
	*/
	return descriptor;
}

/* Funciones Clientes */
int32_t new_connection(struct t_conection* conexion){

	struct sockaddr_in direccion;
	struct hostent *host;
	int32_t new_connect_descr;

	host = gethostbyname(conexion->ip);

	if (host == NULL){
		perror("");
		return -1;
	}

	direccion.sin_family = AF_INET;
	direccion.sin_addr.s_addr = ((struct in_addr *)(host->h_addr))->s_addr;
	direccion.sin_port = htons(conexion->puerto);

	new_connect_descr = socket (AF_INET, SOCK_STREAM, 0);

	if (new_connect_descr == -1){
		perror("");
		return -1;
	}

	if (connect (new_connect_descr, (struct sockaddr*)&direccion, sizeof(direccion)) == -1){
		perror("");
		return -1;
	}

	return new_connect_descr;
}
