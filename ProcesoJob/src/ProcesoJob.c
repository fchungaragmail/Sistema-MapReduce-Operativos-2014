#include "ProcesoJob.h"
#include <pthread.h>
#include <stdio.h>
#include <commons/collections/list.h>
#include <commons/error.h>
#include <arpa/inet.h>
#include <commons/config.h>
#include <commons/string.h>

int socketMartaFd = -1;
char* scriptMapperStr = NULL;
char* scriptReduceStr = NULL;
t_list* listaHilos;
pthread_t threadPedidosMartaHandler;
pthread_t threadProcesarHilos;
t_config* configuracion;
pthread_mutex_t mMarta;

void* pedidosMartaHandler(void* arg) {

	listaHilos = list_create();

	HacerPedidoMarta();

	while (TRUE) {
		mensaje_t* mensajeMarta;

		recibir(socketMartaFd, mensajeMarta);

		char** comandoStr = string_split(mensajeMarta->comando, " ");

		if (strncmp(comandoStr[0], "mapFile", 7) == 0) {
			PlanificarHilosMapper(mensajeMarta);
		} else if (strncmp(mensajeMarta->comando, "reduceFile", 10) == 0) {
			PlanificarHilosReduce(mensajeMarta);
		}

		FreeStringArray(&comandoStr);
		free(mensajeMarta);

	}
	list_destroy(listaHilos);
}

void IniciarConfiguracion() {

	configuracion = config_create("config.ini");
	if (!configuracion) {
		error_show("Error leyendo archivo configuracion!\n");
		Terminar(EXIT_ERROR);
	}

	scriptMapperStr = LeerArchivo(config_get_string_value(configuracion, "MAPPER"));
	scriptReduceStr = LeerArchivo(config_get_string_value(configuracion, "REDUCE"));

}

char* LeerArchivo(char* nombreArchivo){


	FILE* archivoMapper = fopen( nombreArchivo,"r");
	int sizeScript;
	if (!archivoMapper){
		error_show("Error leyendo archivo %s!\n", nombreArchivo);
		Terminar(EXIT_ERROR);
	}

	fseek(archivoMapper, 0, SEEK_END);
	sizeScript = ftell(archivoMapper);
	rewind(archivoMapper);

	char* archivoStr = malloc( 1 + sizeScript );
	fread(archivoStr,sizeScript,1,archivoMapper);

	fclose(archivoMapper);

	return archivoStr;
}

void Terminar(int exitStatus) {

	if (configuracion) {
		config_destroy(configuracion);
	}
	if (socketMartaFd != -1) {
		close(socketMartaFd);
	}

	printf("Finalizacion del ProcesoJob con exit_status=%d\n", exitStatus);
	exit(exitStatus);
}

void IniciarConexionMarta() {

	struct sockaddr_in their_addr;

	if ((socketMartaFd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
		error_show("Error al crear socket para MARTA\n");
		Terminar(EXIT_ERROR);
	}

	char* ipMarta = config_get_string_value(configuracion, "IP_MARTA");
	char* puertoMarta = config_get_string_value(configuracion, "PUERTO_MARTA");

	their_addr.sin_family = AF_INET;
	their_addr.sin_port = htons(ipMarta);
	inet_aton(puertoMarta, &(their_addr.sin_addr));
	memset(&(their_addr.sin_zero), '\o', 8);

	if (connect(socketMartaFd, (Sockaddr_in*) &their_addr, sizeof(Sockaddr_in))
			== -1) {
		error_show("Error al conectarse con MARTA\n");
		Terminar(EXIT_ERROR);
	}

	printf("Conexion exitosa con Marta, ip : %s, puerto :%s\n", ipMarta,
			puertoMarta);

	pthread_create(&threadPedidosMartaHandler, NULL, pedidosMartaHandler, NULL);
	pthread_mutex_init(&mMarta,NULL);

}

void HacerPedidoMarta() {

	int soportaCombiner = config_get_int_value(configuracion, "COMBINER");

	char** archivos = config_get_array_value(configuracion, "ARCHIVOS");

	int i;
	for (i = 0; archivos[i] != NULL; ++i) {

		char* buffer = string_new();
		string_append_with_format(&buffer, "archivoAProcesar %s %i",
				archivos[i], soportaCombiner);

		mensaje_t* mensaje = malloc(sizeof(mensaje_t));

		mensaje->comandoSize = strlen(buffer);
		mensaje->comando = buffer;
		mensaje->dataSize = 0;
		mensaje->data = NULL;


		enviar(socketMartaFd, mensaje);


		printf("Enviado a MaRTA el comando: %s\n", mensaje->comando);
		free(buffer);
		free(mensaje);

	}

}

void ReportarResultadoHilo(HiloJob* hiloJob, EstadoHilo estado){

	mensaje_t* mensajeParaMarta = malloc(sizeof(mensaje_t));
	char* bufferComandoStr = string_new();
	char* bufferDataStr = string_new();

	switch(estado){

	case ESTADO_HILO_FINALIZO_CON_ERROR_DE_CONEXION:
		string_append_with_format(&bufferComandoStr,"mapFileResponse %s 0",hiloJob->nombreArchivo);
		string_append_with_format(&bufferDataStr,"%s", inet_ntoa(hiloJob->direccionNodo.sin_addr));
		break;
	case ESTADO_HILO_FINALIZO_OK:
		string_append_with_format(&bufferComandoStr,"mapFileResponse %s 1",hiloJob->nombreArchivo);
		break;

	}

	mensajeParaMarta->comandoSize = strlen(bufferComandoStr);
	mensajeParaMarta->comando = bufferComandoStr;
	mensajeParaMarta->dataSize = strlen(bufferDataStr);
	mensajeParaMarta->data = bufferDataStr;

	/*
	 * varios hilos pueden estar reportando a marta
	 */
	pthread_mutex_lock(&mMarta);
	enviar(socketMartaFd, mensajeParaMarta);
	pthread_mutex_unlock(&mMarta);

	if(hiloJob->socketFd != -1){
		close(hiloJob->socketFd);
	}
	free(bufferComandoStr);
	free(mensajeParaMarta);
	printf("Se termino el hilo con resultado: %d\n", estado);
}


void PlanificarHilosMapper(mensaje_t* mensaje){

	char** dataStr = string_split(mensaje->data," ");


	//direccionNodo puertoEscucha nroDeBloque rutaArchivoTemporal

	int i = 0;
	while(dataStr[i] != NULL){

		Sockaddr_in their_addr;

		their_addr.sin_family = AF_INET;
		their_addr.sin_port = htons(dataStr[i++]);
		inet_aton(dataStr[i++], &(their_addr.sin_addr));
		memset(&(their_addr.sin_zero), '\o', 8);

		HiloJob* hiloJob = malloc(sizeof(HiloJob));
		hiloJob->direccionNodo = their_addr;
		hiloJob->threadhilo = NULL;
		hiloJob->socketFd = -1;
		hiloJob->nroBloque = atoi(dataStr[i++]);
		hiloJob->nombreArchivo = string_duplicate(dataStr[i++]);

		list_add(listaHilos, (void*) hiloJob);
		CrearHiloMapper(hiloJob);

	}

	FreeStringArray(&dataStr);
}


void PlanificarHilosReduce(mensaje_t* mensaje){

	//TODO
}

void FreeStringArray(char*** stringArray){
	char** array = *stringArray;

	int i;
	for(i = 0; array[i] != NULL; ++i){
		free(array[i]);
	}

	free(array);
}


int main(int argc, char* argv[]) {

	IniciarConfiguracion();
	IniciarConexionMarta();

	pthread_join(threadPedidosMartaHandler,NULL);
	Terminar(EXIT_OK);
}
