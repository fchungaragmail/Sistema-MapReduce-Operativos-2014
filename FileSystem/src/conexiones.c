/*
 * conexiones.c
 *
 *  Created on: 19/5/2015
 *      Author: utnso
 */


#include "conexiones.h"

int escuchaConexiones;
int MaRTA;
int desbloquearSelect[2];
t_list* conexiones;
pthread_mutex_t mConexiones;
fd_set nodos;
pthread_mutex_t mNodos;
FILE* logFile;

int initConexiones();
void escucharConexiones();
void leerEntradas();
void cerrarConexiones();
void cerrarConexion(Conexion_t* conexion);
void freeConexion(Conexion_t* conexion);

void probarConexiones();


int initConexiones()
{
	conexiones = list_create();
	pthread_mutex_init(&mConexiones, NULL);
	FD_ZERO(&nodos);
	pthread_mutex_init(&mNodos, NULL);
	escuchaConexiones = socket(AF_INET, SOCK_STREAM, 0);
	if (escuchaConexiones == -1){
		log_error(logFile,"No se pudo crear el socket para escuchar "
				"nuevas conexiones.");
	} else
	{
		log_info(logFile, "El socket para escuchar nuevas conexiones se "
				"creo correctamente.");
	}

	pipe2(desbloquearSelect, O_NONBLOCK);
	FD_SET(desbloquearSelect[0], &nodos);


	int yes=1;
	if (setsockopt(escuchaConexiones,SOL_SOCKET,SO_REUSEADDR,&yes,sizeof(int)) == -1) {
	    perror("setsockopt");
	    exit(1);
	}


	Sockaddr_in miDirecc;
	miDirecc.sin_family = AF_INET;
	miDirecc.sin_port = htons(PUERTO_LISTEN);
	inet_aton(IP_LISTEN, &(miDirecc.sin_addr));
	memset(&(miDirecc.sin_zero), '\0', 8);
	if (0 > bind(escuchaConexiones, (Sockaddr_in*) &miDirecc, sizeof(Sockaddr_in)))
	{
		log_error(logFile, "No se pudo bindear el socket para escuchar conexiones.");
	}


	if (listen(escuchaConexiones, NODOS_MAX+1) == -1)
	{
		log_error(logFile,"No se pueden escuchar conexiones."
				"El FileSystem se cerrara");
		return(-1);
	} else
	{
		log_info(logFile, "Escuchando conexiones.");
	}
}


void escucharConexiones()
{
	int nuevoSocketfd;
	int sin_size = sizeof(Sockaddr_in);

	while (true)
	{
		Sockaddr_in their_addr;
		Conexion_t* conexionNueva = malloc(sizeof(Conexion_t));

		nuevoSocketfd = accept(escuchaConexiones, &their_addr, &sin_size);

		sem_init(&(conexionNueva->respuestasR),0,1);
		sem_init(&(conexionNueva->respuestasP),0,0);
		strcpy(conexionNueva->nombre, "NombreGenerico");
		conexionNueva->sockfd = nuevoSocketfd;
		conexionNueva->estado = CONECTADO;
		pthread_mutex_init(&(conexionNueva->mSocket), NULL);
		pthread_mutex_init(&(conexionNueva->mEstadoBloques), NULL);
		conexionNueva->totalBloques = 0;
		conexionNueva->estadoBloques = malloc(1);

		pthread_mutex_lock(&mConexiones);
		list_add(conexiones, conexionNueva);
		pthread_mutex_unlock(&mConexiones);

		pthread_mutex_lock(&mNodos);
		FD_SET(nuevoSocketfd, &nodos);
		write(desbloquearSelect[1], "", 1);
		pthread_mutex_unlock(&mNodos);
		sem_post(&sPersistencia);

		log_info(logFile, "Nueva conexion con %s. \n"
				"Esperando identificacion.", inet_ntoa(their_addr.sin_addr));
	}
}


void cerrarConexiones()
{
	close(escuchaConexiones);
	//list_iterate(conexiones, freeConexion);
	//list_destroy(conexiones);
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
		pthread_mutex_lock(&mNodos);
		nodosSelect = nodos;
		pthread_mutex_unlock(&mNodos);
		select(FD_SETSIZE, &nodosSelect, NULL, NULL, NULL);

		//Chequeo si salio del select por una nueva conexion
		if (FD_ISSET(desbloquearSelect[0], &nodosSelect))
		{
			char* dummy = malloc(1);
			int count = 1;
			while (count != 0)
			{
				read(desbloquearSelect[0], dummy, 1);
				ioctl(desbloquearSelect[0], FIONREAD, &count);
			}
			free(dummy);
		}

		//Chequeo los fd de las conexiones
		for (int i=0; i<conexiones->elements_count ;i++)
		{
			pthread_mutex_lock(&mConexiones);
			Conexion_t* conexion = list_get(conexiones,i);
			pthread_mutex_unlock(&mConexiones);
			if (conexion->sockfd < 0) continue;
			if (true != FD_ISSET(conexion->sockfd,&nodosSelect))
			{
				continue;
			}

			int estado;
			mensaje_t* mensaje = malloc(sizeof(mensaje_t));
			estado = recibir(conexion->sockfd, mensaje);
			if (estado == CONECTADO)
			{
				pthread_t tProcesar;
				argumentos_t* args = malloc(sizeof(argumentos_t));
				args->conexion = conexion;
				args->mensaje = mensaje;

				pthread_create(&tProcesar, NULL, procesarComandoRemoto, args);
				//ioctl(conexion->sockfd, FIONREAD, &count);
			} else
			{
				cerrarConexion(conexion);
				continue;
			}
		}
	}
}

void cerrarConexion(Conexion_t* conexion)
{
	pthread_mutex_lock(&mNodos);
	FD_CLR(conexion->sockfd, &nodos);
	pthread_mutex_unlock(&mNodos);
	close(conexion->sockfd);
	conexion->estado = DESCONECTADO;

	if (strcmp(conexion->nombre,MARTA) != 0)
	{
		actualizarEstadoArchivos();
	}

	log_info(logFile, "Desconectado de %s", conexion->nombre);
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
	char comando[] = "nombre Nodo1 2147483648";
	mensaje->comando = malloc(strlen(comando)+1);
	strcpy(mensaje->comando,comando);
	mensaje->comandoSize = (strlen(mensaje->comando) + 1);

	memcpy(mensaje->data, NULL, 0);
	mensaje->dataSize = 0;

	enviar(socketPrueba, mensaje);

	while(1)
	{
		sleep(5);
	//	persistirEstructuras();
	}
}

