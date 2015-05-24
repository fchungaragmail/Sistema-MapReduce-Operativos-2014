/*
 * conexiones.c
 *
 *  Created on: 19/5/2015
 *      Author: utnso
 */


#include "conexiones.h"

int escuchaNodos;
int escuchaMaRTA, MaRTA;
int desbloquearSelect[2];
t_list* conexiones;
fd_set nodos;
pthread_mutex_t mNodos;
int nodosCount;

int initConexiones();
void escucharNodos(int nodosMax);
void escucharMaRTA();
void leerEntradas();
void cerrarConexiones();
void freeConexion(Conexion_t* conexion);

void probarConexiones();


int initConexiones()
{
	conexiones = list_create();
	FD_ZERO(&nodos);
	pthread_mutex_init(&mNodos, NULL);
	nodosCount = 0;
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

	pipe2(desbloquearSelect, O_NONBLOCK);
	FD_SET(desbloquearSelect[0], &nodos);
	nodosCount++;

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


void escucharNodos(int nodosMax)
{
	int nuevoSocketfd;
	int sin_size = sizeof(Sockaddr_in);
	int i = 0;

	while ((i < nodosMax) || (nodosMax == -1))
	{
		Sockaddr_in their_addr;
		Conexion_t* conexionNueva = malloc(sizeof(Conexion_t));

		if (nodosMax == 1) probarConexiones(); //PRUEBA

		nuevoSocketfd = accept(escuchaNodos, &their_addr, &sin_size);

		conexionNueva->sockaddr_in = their_addr;
		conexionNueva->sockfd = nuevoSocketfd;
		list_add(conexiones, conexionNueva);
		pthread_mutex_lock(&mNodos);
		FD_SET(nuevoSocketfd, &nodos);
		nodosCount++;
		pthread_mutex_unlock(&mNodos);
		write(desbloquearSelect[1], "", 1);


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
	list_destroy(conexiones);
	close(desbloquearSelect[0]);
	close(desbloquearSelect[1]);
}

void freeConexion(Conexion_t* conexion)
{
	close(conexion->sockfd);
	free(conexion);
}


void leerEntradas()
{
	while (1)
	{
		fd_set nodosSelect;
		int nodosCountSelect;
		pthread_mutex_lock(&mNodos);
		nodosSelect = nodos;
		nodosCountSelect = nodosCount;
		pthread_mutex_unlock(&mNodos);
		select(1024, &nodosSelect, NULL, NULL, NULL);

		if (FD_ISSET(desbloquearSelect[0], &nodosSelect))
		{
			char* dummy = malloc(1);
			read(desbloquearSelect[0], dummy, 1);
			free(dummy);
		}


		for (int i=0; i<conexiones->elements_count ;i++)
		{
			Conexion_t* conexion = list_get(conexiones,i);
			if (!FD_ISSET(conexion->sockfd,&nodosSelect)) continue;

			mensaje_t* mensajeRecibido;
			mensajeRecibido = recibirNodoFS(conexion->sockfd);
			log_info(log, "%s", mensajeRecibido->comando);
		}
	}
}


void probarConexiones()
{
	int socketPrueba;
	struct sockaddr_in their_addr;

	socketPrueba = socket(AF_INET, SOCK_STREAM, 0);

	their_addr.sin_family = AF_INET;
	their_addr.sin_port = htons(PUERTO_LISTEN);
	inet_aton(IP_LISTEN, &(their_addr.sin_addr));
	memset(&(their_addr.sin_zero), '\o', 8);

	connect(socketPrueba, &their_addr, sizeof(Sockaddr_in));

	mensaje_t* mensaje = malloc(sizeof(mensaje_t));
	char comando[] = "este es el comando que voy a mandar";
	mensaje->comando = malloc(strlen(comando));
	strcpy(mensaje->comando,comando);
	mensaje->comandoSize = strlen(mensaje->comando);

	char data[] = "Aca irian los datos";
	mensaje->data = malloc(strlen(data));
	strcpy(mensaje->data,data);
	mensaje->dataSize = strlen(mensaje->data);

	enviarNodoFS(socketPrueba, mensaje);
}

