/*
 * conexiones.c
 *
 *  Created on: 19/5/2015
 *      Author: utnso
 */


#include "conexiones.h"

int escuchaNodos;
int escuchaMaRTA, MaRTA;
t_list* conexiones;
fd_set nodos;

int initConexiones();
void escucharNodos(int cantNodos);
void escucharMaRTA();
void cerrarConexiones();
void freeConexion(Conexion_t* conexion);


int initConexiones()
{
	conexiones = list_create();
	FD_ZERO(&nodos);
	escuchaNodos = socket(AF_INET, SOCK_STREAM, 0);
	escuchaMaRTA = socket(AF_INET, SOCK_STREAM, 0);
	if ((escuchaNodos == -1) && (escuchaMaRTA == -1)){
		log_error(log,"No se pudieron crear los socket para escuchar "
				"nuevas conexiones.");
	} else
	{
		log_info(log, "Los socket para escuchar nuevas conexiones se "
				"crearon correctamente.");
	}

	Sockaddr_in miDireccNodos;
	miDireccNodos.sin_family = AF_INET;
	miDireccNodos.sin_port = htons(PUERTO_LISTEN);
	inet_aton(IP_LISTEN, &(miDireccNodos.sin_addr));
	memset(&(miDireccNodos.sin_zero), '\0', 8);
	bind(escuchaNodos, (Sockaddr_in*) &miDireccNodos, sizeof(Sockaddr_in));

	Sockaddr_in miDireccMaRTA;
	miDireccMaRTA.sin_family = AF_INET;
	miDireccMaRTA.sin_port = htons(PUERTO_MARTA);
	inet_aton(IP_LISTEN, &(miDireccMaRTA.sin_addr));
	memset(&(miDireccMaRTA.sin_zero), '\0', 8);
	bind(escuchaMaRTA, (Sockaddr_in*) &miDireccMaRTA, sizeof(Sockaddr_in));


	if (listen(escuchaNodos, NODOS_MAX) == -1)
	{
		log_error(log,"No se pueden escuchar conexiones para Nodos."
				"El FileSystem se cerrara");
		return(-1);
	} else
	{
		log_info(log, "Escuchando conexiones.");
	}

	if (listen(escuchaMaRTA, 1) 		 == -1)
	{
		log_error(log,"No se pueden escuchar conexiones para MaRTA."
				"El FileSystem se cerrara");
		return(-1);
	} else
	{
		log_info(log, "Escuchando conexiones.");
	}
}


void escucharNodos(int cantNodos)
{
	int nuevoSocketfd;
	int sin_size = sizeof(Sockaddr_in);

	if (cantNodos == -1) cantNodos = 1;
	int i = 0;

	while ((i < cantNodos) || (cantNodos == -1))
	{
		Sockaddr_in their_addr;
		Conexion_t* conexionNueva = malloc(sizeof(Conexion_t));

		nuevoSocketfd = accept(escuchaNodos, &their_addr, &sin_size);

		conexionNueva->sockaddr_in = their_addr;
		conexionNueva->sockfd = nuevoSocketfd;
		list_add(conexiones, conexionNueva);
		FD_SET(nuevoSocketfd, &nodos);
		log_info(log, "Conectado con el nodo %s \n", inet_ntoa(their_addr.sin_addr));
		i++;
	}
	log_info(log, "Cantidad minima de nodos (%d) alcanzada.\n", LISTA_NODOS);
}

void escucharMaRTA()
{
	int sin_size = sizeof(Sockaddr_in);

	Sockaddr_in their_addr;
	Conexion_t* conexionNueva = malloc(sizeof(Conexion_t));

	MaRTA = accept(escuchaMaRTA, (Sockaddr_in*) &their_addr, &sin_size);

	conexionNueva->sockaddr_in = their_addr;
	conexionNueva->sockfd = MaRTA;
	log_info(log, "Conectado con MaRTA (%s) \n", inet_ntoa(their_addr.sin_addr));
}


void cerrarConexiones()
{
	list_iterate(conexiones, freeConexion);
}

void freeConexion(Conexion_t* conexion)
{
	close(conexion->sockfd);
	free(conexion);
}
