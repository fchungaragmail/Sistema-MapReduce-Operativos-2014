#include "HiloMapper.h"

pthread_t* CrearHiloMapper(HiloJob* hiloJob){

	pthread_t* hiloMapper;

	pthread_create(&hiloMapper,NULL,hiloMapperHandler, (void*)hiloJob);
	hiloJob->threadhilo = hiloMapper;

	return hiloMapper;
}

void* hiloMapperHandler(void* arg){

	HiloJob* hiloJob = (HiloJob*) arg;

	int terminoProceso = FALSE;
	int hiloNodoSocketFd = -1;


	if ((hiloNodoSocketFd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
		reportarResultadoHilo(hiloJob,ESTADO_HILO_FINALIZO_CON_ERROR_DE_CONEXION);
		error_show("Error al crear socket para el nodo\n");
		return NULL;
	}


	if (connect(hiloNodoSocketFd, hiloJob->direccionNodo, sizeof(Sockaddr_in))
			== -1) {
		reportarResultadoHilo(hiloJob,ESTADO_HILO_FINALIZO_CON_ERROR_DE_CONEXION);
		error_show("Error al conectarse con el nodo\n");
		return NULL;
	}

	printf("Conexion exitosa con un nodo");

	mensaje_t* mensajeNodo;
	recibir(hiloNodoSocketFd,mensajeNodo);
	//TODO define

	reportarResultadoHilo(hiloJob,ESTADO_HILO_FINALIZO_OK);
}

void reportarResultadoHilo(HiloJob* hiloJob, EstadoHilo estado){

	switch(estado){

	case ESTADO_HILO_FINALIZO_CON_ERROR_DE_CONEXION:
		break;
	case ESTADO_HILO_FINALIZO_CON_ERROR_EN_NODO:
		break;
	case ESTADO_HILO_FINALIZO_OK:
		break;

	}

	close(hiloJob->socketFd);
	printf("Se termino el hilo con resultado: %d\n", estado);
}
