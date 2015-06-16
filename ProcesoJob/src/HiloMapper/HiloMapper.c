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


	mensaje_t* mensajeParaNodo = malloc(sizeof(mensaje_t));
	char* bufferComando = string_new();
	char* bufferData = string_new();
	string_append_with_format(&bufferComando, "aplicarMapper %i %s", hiloJob->nroBloque, hiloJob->nombreArchivo);
	string_append(&bufferData,scriptMapperStr);



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
	//mensaje_t* mensajeDeNodo;
	//recibir(hiloJob->socketFd,mensajeDeNodo);
	//TODO define

	ReportarResultadoHilo(hiloJob,ESTADO_HILO_FINALIZO_OK);
}
