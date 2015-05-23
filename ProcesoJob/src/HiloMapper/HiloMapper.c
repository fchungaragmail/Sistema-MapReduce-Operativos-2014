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

	//SOLO PARA TEST
	hiloJob->estadoHilo = ESTADO_HILO_FINALIZO_CON_ERROR_DE_CONEXION;
	reportarResultadoHilo(hiloJob);
	return NULL;
	////
	if ((hiloNodoSocketFd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
		error_show("Error al crear socket para el nodo\n");
	}


	if (connect(hiloNodoSocketFd, hiloJob->direccionNodo, sizeof(Sockaddr_in))
			== -1) {
		error_show("Error al conectarse con el nodo\n");
		close(hiloNodoSocketFd);
	}

	printf("Conexion exitosa con un nodo");


	while(!terminoProceso){
		//Procesar ?
	}
	close(hiloNodoSocketFd);
}

void reportarResultadoHilo(HiloJob* hiloJob){


	printf("Se termino el hilo con resultado: %d\n", hiloJob->estadoHilo);
}
