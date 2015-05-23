#include "ProcesoJob.h"
#include <pthread.h>
#include <commons/collections/list.h>
#include <commons/error.h>
#include <arpa/inet.h>
#include <commons/config.h>


int socketFd = -1;
t_list* listaHilos;
pthread_t threadMartaHandler;
t_config* configuracion;


void* martaHandler(void* arg){

	listaHilos = list_create();

	while(TRUE){
		//TODO - recv(PeticionMartaJob)
		//SOLO TEST

		HiloJob* hiloJob = malloc(sizeof(HiloJob));
		hiloJob->direccionNodo = NULL;
		hiloJob->estadoHilo = ESTADO_HILO_NUEVO;
		hiloJob->threadhilo = NULL;

		list_add(listaHilos,(void*)hiloJob);
		CrearHiloMapper(hiloJob);
		break;
		/////
	}
	list_destroy(listaHilos);
}

void IniciarConfiguracion(){

	configuracion = config_create("config.ini");
	if(!configuracion){
		error_show("Error leyendo archivo!\n");
		Terminar(EXIT_ERROR);
	}
}

void Terminar(int exitStatus){

	if(configuracion){
		config_destroy(configuracion);
	}
	if(socketFd != -1){
		close(socketFd);
	}

	printf("Finalizacion del ProcesoJob con exit_status=%d\n", exitStatus);
	exit(exitStatus);
}

void IniciarConexionMarta(){

	struct sockaddr_in their_addr;

	if ((socketFd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
		error_show("Error al crear socket para MARTA\n");
		Terminar(EXIT_ERROR);
	}

	char* ipMarta = config_get_string_value(configuracion,"IP_MARTA");
	char* puertoMarta = config_get_string_value(configuracion,"PUERTO_MARTA");

	their_addr.sin_family = AF_INET;
	their_addr.sin_port = htons(ipMarta);
	inet_aton(puertoMarta, &(their_addr.sin_addr));
	memset(&(their_addr.sin_zero), '\o', 8);
	//COMENTADO PARA TEST
/*
	if (connect(socketFd, (Sockaddr_in*) &their_addr, sizeof(Sockaddr_in))
			== -1) {
		error_show("Error al conectarse con MARTA\n");
		Terminar(EXIT_ERROR);
	}
*/
	/////
	printf("Conexion exitosa con Marta, ip : %s, puerto :%s\n",ipMarta,puertoMarta);
	pthread_create(&threadMartaHandler, NULL, martaHandler, NULL);
	pthread_join(threadMartaHandler,NULL);
}

int main(int argc,char* argv[] ){

	IniciarConfiguracion();
	IniciarConexionMarta();
	Terminar(EXIT_OK);
}
