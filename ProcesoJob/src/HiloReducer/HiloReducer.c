#include "HiloReducer.h"


pthread_t* CrearHiloReducer(HiloJob* hiloJob){

	pthread_t* hiloMapper;

	pthread_create(&hiloMapper,NULL,hiloReducerHandler, (void*)hiloJob);
	hiloJob->threadhilo = hiloMapper;

	return hiloMapper;
}

void* hiloReducerHandler(void* arg){

	HiloJob* hiloJob = (HiloJob*) arg;


	if ((hiloJob->socketFd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
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

	printf("Conexion exitosa con un nodo");

	//TODO

	ReportarResultadoHilo(hiloJob,ESTADO_HILO_FINALIZO_OK);
}

