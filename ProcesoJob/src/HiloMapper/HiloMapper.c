#include "HiloMapper.h"

//TODO organizar mejor esto!!
extern socketMartaFd;

pthread_t* CrearHiloMapper(HiloJob* hiloJob){

	pthread_t* hiloMapper;

	pthread_create(&hiloMapper,NULL,hiloMapperHandler, (void*)hiloJob);
	hiloJob->threadhilo = hiloMapper;

	return hiloMapper;
}

void* hiloMapperHandler(void* arg){

	HiloJob* hiloJob = (HiloJob*) arg;


	if ((hiloJob->socketFd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
		reportarResultadoHilo(hiloJob,ESTADO_HILO_FINALIZO_CON_ERROR_DE_CONEXION);
		error_show("Error al crear socket para el nodo\n");
		return NULL;
	}


	if (connect(hiloJob->socketFd, (Sockaddr_in*) &hiloJob->direccionNodo, sizeof(Sockaddr_in))
			== -1) {
		reportarResultadoHilo(hiloJob,ESTADO_HILO_FINALIZO_CON_ERROR_DE_CONEXION);
		error_show("Error al conectarse con el nodo\n");
		return NULL;
	}

	printf("Conexion exitosa con un nodo");

	mensaje_t* mensajeNodo;
	recibir(hiloJob->socketFd,mensajeNodo);
	//TODO define

	reportarResultadoHilo(hiloJob,ESTADO_HILO_FINALIZO_OK);
}

void reportarResultadoHilo(HiloJob* hiloJob, EstadoHilo estado){

	mensaje_t* mensajeParaMarta = malloc(sizeof(mensaje_t));
	char* bufferMensaje;
	switch(estado){

	case ESTADO_HILO_FINALIZO_CON_ERROR_DE_CONEXION:
	case ESTADO_HILO_FINALIZO_CON_ERROR_EN_NODO:
		string_append_with_format(&bufferMensaje,"mapFileResponse %s 0",hiloJob->nombreArchivo);
		break;
	case ESTADO_HILO_FINALIZO_OK:
		string_append_with_format(&bufferMensaje,"mapFileResponse %s 1",hiloJob->nombreArchivo);
		break;

	}

	mensajeParaMarta->comandoSize = strlen(bufferMensaje);
	mensajeParaMarta->comando = string_duplicate(bufferMensaje);
	mensajeParaMarta->dataSize = 0;
	mensajeParaMarta->data = NULL;

	enviar(socketMartaFd, mensajeParaMarta);


	if(hiloJob->socketFd != -1){
		close(hiloJob->socketFd);
	}
	free(bufferMensaje);
	free(mensajeParaMarta);
	printf("Se termino el hilo con resultado: %d\n", estado);
}
