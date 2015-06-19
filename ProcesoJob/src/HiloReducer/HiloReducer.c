#include "HiloReducer.h"

extern char* scriptReduceStr;

pthread_t* CrearHiloReducer(HiloJob* hiloJob){

	pthread_t* hiloReducer;

	pthread_create(&hiloReducer,NULL,hiloReducerHandler, (void*)hiloJob);
	hiloJob->threadhilo = hiloReducer;

	return hiloReducer;
}

void* hiloReducerHandler(void* arg){

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
	mensajeDeNodo->data = NULL;
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
	 * ENVIO DE REDUCE Y RESPUESTA
	 */


	char* bufferComando = string_new();
	char* bufferData = string_new();
	string_append_with_format(&bufferComando, "reduceFileConCombiner %s %s", hiloJob->nombreArchivo, hiloJob->parametros);
	string_append(&bufferData,scriptReduceStr);

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
		mensajeDeNodo->comando = strdup("reduceFileResponse 1");
		mensajeDeNodo->data = NULL;
		alternar = 0;
	} else{
		mensajeDeNodo->comando = strdup("reduceFileResponse 0");
		mensajeDeNodo->data = strdup("129.3.6.9 112.3.4.5");
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

	if(strncmp(comandoStr[MENSAJE_COMANDO], "reduceFileResponse", 18 ) == 0){
		if(atoi(comandoStr[1]) == 1){
			ReportarResultadoHilo(hiloJob,ESTADO_HILO_FINALIZO_OK);
		} else{
			hiloJob->parametrosError = strdup(mensajeDeNodo->data);
			ReportarResultadoHilo(hiloJob,ESTADO_HILO_FINALIZO_CON_ERROR_EN_NODO);
		}
	} else {
		printf("COMANDO %s DESCONOCIDO!\n", mensajeDeNodo->comando );
		ReportarResultadoHilo(hiloJob,ESTADO_HILO_FINALIZO_CON_ERROR_EN_NODO);
	}

	FreeStringArray(&comandoStr);
}

