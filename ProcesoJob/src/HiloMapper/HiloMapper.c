#include "HiloMapper.h"


extern char* scriptMapperStr;

pthread_t* CrearHiloMapper(HiloJob* hiloJob){

	pthread_t* hiloMapper;

	pthread_create(&hiloMapper,NULL,hiloMapperHandler, (void*)hiloJob);
	hiloJob->threadhilo = hiloMapper;

	return hiloMapper;
}

void* hiloMapperHandler(void* arg){

	HiloJob* hiloJob = (HiloJob*) arg;
	int estadoConexion = CONECTADO;

#ifndef BUILD_PARA_TEST
	if ((hiloJob->socketFd = socket(AF_INET, SOCK_STREAM, 0)) == -1) { //<--- CREO QUE SI OCURRE ESTO, FALLO JOB Y DEBE ABORTAR
		ReportarResultadoHilo(hiloJob,ESTADO_HILO_FINALIZO_CON_ERROR_DE_CONEXION);
		error_show("Error al crear socket para el nodo\n");
		return NULL;
	}


	if (connect(hiloJob->socketFd, (Sockaddr_in*) &hiloJob->direccionNodo, sizeof(Sockaddr_in))
			== -1) {
		ReportarResultadoHilo(hiloJob,ESTADO_HILO_FINALIZO_CON_ERROR_DE_CONEXION);
		error_show("Error al conectarse con el nodo\n");
		return NULL;
	}
#else
	hiloJob->socketFd = 99;
#endif
	printf("-----Conexion exitosa con nodo ip:%s puerto:%d-----\n", inet_ntoa(hiloJob->direccionNodo.sin_addr), ntohs(hiloJob->direccionNodo.sin_port));


	/*
	 *  HANDSHAKE
	 */

	//TODO mejorar la creacion de mensaje_t
	mensaje_t* mensajeDeNodo;
	mensaje_t* mensajeParaNodo = malloc(sizeof(mensaje_t));
	mensajeParaNodo->comando= strdup("handshake");
	mensajeParaNodo->comandoSize = strlen("handshake");
	mensajeParaNodo->data = NULL;
	mensajeParaNodo->dataSize = NULL;

#ifndef BUILD_PARA_TEST
	enviar(hiloJob->socketFd, mensajeParaNodo);
#endif
	printf("-----Enviado al nodo %s-----\nComando: %s\nData: %s\n----------\n", inet_ntoa(hiloJob->direccionNodo.sin_addr), mensajeParaNodo->comando, mensajeParaNodo->data);
	free(mensajeParaNodo);
	mensajeParaNodo = NULL;

#ifndef BUILD_PARA_TEST
	estadoConexion = recibir(hiloJob->socketFd,mensajeDeNodo);
#else
	estadoConexion = CONECTADO;
	mensajeDeNodo = malloc(sizeof(mensaje_t));
	mensajeDeNodo->comando = strdup("bienvenido");
	mensajeDeNodo->data = string_new();
#endif

	if(estadoConexion == DESCONECTADO){
		printf("Nodo %s desconectado!\n", inet_ntoa(hiloJob->direccionNodo.sin_addr));
		ReportarResultadoHilo(hiloJob,ESTADO_HILO_FINALIZO_CON_ERROR_DE_CONEXION);
		return NULL;
	}

	printf("-----Recibido del nodo %s-----\nComando: %s\nData: %s\n----------\n", inet_ntoa(hiloJob->direccionNodo.sin_addr), mensajeDeNodo->comando, mensajeDeNodo->data);
	free(mensajeDeNodo);
	mensajeDeNodo = NULL;


	/*
	 * ENVIO DE MAP Y RESPUESTA
	 */


	char* bufferComando = string_new();
	char* bufferData = string_new();
	string_append_with_format(&bufferComando, "mapFile %s %i", hiloJob->nombreArchivo, hiloJob->nroBloque);
	string_append(&bufferData,scriptMapperStr);

	mensajeParaNodo = malloc(sizeof(mensaje_t));
	mensajeParaNodo->comandoSize = strlen(bufferComando);
	mensajeParaNodo->comando = bufferComando;
	mensajeParaNodo->dataSize = strlen(bufferData);
	mensajeParaNodo->data = bufferData;

#ifndef BUILD_PARA_TEST
	enviar(hiloJob->socketFd, mensajeParaNodo);
#endif
	printf("-----Enviado al nodo %s-----\nComando: %s\nData: %s\n----------\n", inet_ntoa(hiloJob->direccionNodo.sin_addr), mensajeParaNodo->comando, mensajeParaNodo->data);
	free(bufferComando);
	free(bufferData);
	free(mensajeParaNodo);

#ifdef BUILD_PARA_TEST
	int tiempoParaDormir = rand() % 5;
	sleep(tiempoParaDormir);
#endif


#ifndef BUILD_PARA_TEST
	estadoConexion = recibir(hiloJob->socketFd,mensajeDeNodo);
#else
	estadoConexion = CONECTADO;
	mensajeDeNodo = malloc(sizeof(mensaje_t));
	static int alternar = 1;
	if(alternar == 1 ){
		mensajeDeNodo->comando = strdup("mapFileResponse 1");
		mensajeDeNodo->data = NULL;
		alternar = 0;
	} else{
		mensajeDeNodo->comando = strdup("mapFileResponse 0");
		mensajeDeNodo->data = NULL;
		alternar = 1;
	}
#endif

	if(estadoConexion == DESCONECTADO){
		printf("Nodo %s desconectado!\n", inet_ntoa(hiloJob->direccionNodo.sin_addr));
		ReportarResultadoHilo(hiloJob,ESTADO_HILO_FINALIZO_CON_ERROR_DE_CONEXION);
		return NULL;
	}

	printf("-----Recibido del nodo %s-----\nComando: %s\nData: %s\n----------\n", inet_ntoa(hiloJob->direccionNodo.sin_addr), mensajeDeNodo->comando, mensajeDeNodo->data);

	char** comandoStr = string_split(mensajeDeNodo->comando, " ");

	if(strncmp(comandoStr[MENSAJE_COMANDO], "mapFileResponse", 15 ) == 0){
		if(atoi(comandoStr[1]) == 1){
			ReportarResultadoHilo(hiloJob,ESTADO_HILO_FINALIZO_OK);
		} else{
			ReportarResultadoHilo(hiloJob,ESTADO_HILO_FINALIZO_CON_ERROR_EN_NODO);
		}
	} else {
		printf("COMANDO %s DESCONOCIDO!\n", mensajeDeNodo->comando );
		ReportarResultadoHilo(hiloJob,ESTADO_HILO_FINALIZO_CON_ERROR_EN_NODO);
	}

	FreeStringArray(&comandoStr);
}
