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

	printf("Conexion exitosa con nodo");


	mensaje_t* mensajeParaNodo = malloc(sizeof(mensaje_t));
	char* bufferComando = string_new();
	char* bufferData = string_new();
	string_append_with_format(&bufferComando, "aplicarMapper %i %s", hiloJob->nroBloque, hiloJob->nombreArchivo);
	string_append(&bufferData,scriptMapperStr);



	mensajeParaNodo->comandoSize = strlen(bufferComando);
	mensajeParaNodo->comando = bufferComando;
	mensajeParaNodo->dataSize = strlen(bufferData);
	mensajeParaNodo->data = bufferData;

	enviar(hiloJob->socketFd, mensajeParaNodo);

	printf("Enviado a MaRTA el comando: %s\n", mensajeParaNodo->comando);
	free(bufferComando);
	free(bufferData);
	free(mensajeParaNodo);


	mensaje_t* mensajeDeNodo;
	recibir(hiloJob->socketFd,mensajeDeNodo);
	//TODO define

	ReportarResultadoHilo(hiloJob,ESTADO_HILO_FINALIZO_OK);
}
