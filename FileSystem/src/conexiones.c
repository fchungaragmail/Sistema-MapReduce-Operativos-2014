/*
 * conexiones.c
 *
 *  Created on: 19/5/2015
 *      Author: utnso
 */


#include "conexiones.h"

int nuevasConexiones;

int initConexiones();
void escucharConexiones();


int initConexiones()
{
	nuevasConexiones = socket(AF_INET, SOCK_STREAM, 0);
	if (nuevasConexiones == -1) {
		log_error(log,"No se pudo crear el socket para escuchar "
				"nuevas conexiones.");
	} else
	{
		log_info(log, "El socket para escuchar nuevas conexiones se "
				"creo correctamente");
	}

	Sockaddr_in my_addr;
	my_addr.sin_family = AF_INET;
	my_addr.sin_port = htons(PUERTO_LISTEN);
	inet_aton("127.0.0.1", &(my_addr.sin_addr));
	memset(&(my_addr.sin_zero), '\0', 8);

	bind(nuevasConexiones, (Sockaddr_in*) &my_addr, sizeof(Sockaddr_in));


	if (listen(nuevasConexiones, NODOS_MAX) == -1)
	{
		log_error(log,"No se pueden escuchar conexiones."
				"El FileSystem se cerrara");
		return(-1);
	} else
	{
		log_info(log, "Escuchando conexiones.");
	}
}


void escucharConexiones()
{
	int nuevoSocketfd;
	int sin_size = sizeof(Sockaddr_in);

	while (1)
	{

		Sockaddr_in their_addr;
		Conexion* conexionNueva = malloc(sizeof(Conexion));

		nuevoSocketfd = accept(nuevasConexiones, (Sockaddr_in*) &their_addr, &sin_size);

		conexionNueva->sockaddr_in = their_addr;
		conexionNueva->sockfd = nuevoSocketfd;
		log_info(log, "Conectado con el nodo %s", their_addr.sin_addr);
	}
}
