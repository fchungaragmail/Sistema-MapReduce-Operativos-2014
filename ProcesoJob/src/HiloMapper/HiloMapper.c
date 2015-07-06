#include "HiloMapper.h"
#include "../Utils.h"
#include <commons/log.h>

extern char* scriptMapperStr;
extern t_log* logProcesoJob;

pthread_t* CrearHiloMapper(HiloJob* hiloJob) {

	pthread_t* hiloMapper;

	hiloJob->tipoHilo = TIPO_HILO_MAPPER;
	pthread_create(&hiloMapper, NULL, hiloMapperHandler, (void*) hiloJob);
	hiloJob->threadhilo = hiloMapper;

	return hiloMapper;
}

void* hiloMapperHandler(void* arg) {

	HiloJob* hiloJob = (HiloJob*) arg;
	int estadoConexion = CONECTADO;

#ifndef BUILD_PARA_TEST
	if ((hiloJob->socketFd = socket(AF_INET, SOCK_STREAM, 0)) == -1) { //<--- CREO QUE SI OCURRE ESTO, FALLO JOB Y DEBE ABORTAR
		log_error(logProcesoJob,"Error al crear socket para nodo %s\n",inet_ntoa(hiloJob->direccionNodo.sin_addr));
		ReportarResultadoHilo(hiloJob,ESTADO_HILO_FINALIZO_CON_ERROR_DE_CONEXION);
		return NULL;
	}

	if (connect(hiloJob->socketFd, (Sockaddr_in*) &hiloJob->direccionNodo, sizeof(Sockaddr_in))
			== -1) {
		log_error(logProcesoJob,"Intento de conexion fallida con nodo IP:%s PUERTO:%d\n", inet_ntoa(hiloJob->direccionNodo.sin_addr), ntohs(hiloJob->direccionNodo.sin_port));
		ReportarResultadoHilo(hiloJob,ESTADO_HILO_FINALIZO_CON_ERROR_DE_CONEXION);
		return NULL;
	}
#else
	hiloJob->socketFd = 99;
#endif
	log_info(logProcesoJob, "Conexion exitosa con nodo IP:%s PUERTO:%d\n",
			inet_ntoa(hiloJob->direccionNodo.sin_addr),
			ntohs(hiloJob->direccionNodo.sin_port));

	/*
	 *  HANDSHAKE
	 */

	mensaje_t* mensajeDeNodo;
	mensaje_t* mensajeParaNodo;

	mensajeParaNodo = CreateMensaje("mp", NULL);

#ifndef BUILD_PARA_TEST
	enviar(hiloJob->socketFd, mensajeParaNodo);
#endif
	log_info(logProcesoJob,
			"Enviado al nodo IP:%s PUERTO:%d\nComando: %s\nData: %s\n",
			inet_ntoa(hiloJob->direccionNodo.sin_addr),
			ntohs(hiloJob->direccionNodo.sin_port), mensajeParaNodo->comando,
			mensajeParaNodo->data);
	FreeMensaje(mensajeParaNodo);

#ifndef BUILD_PARA_TEST
	estadoConexion = recibir(hiloJob->socketFd,mensajeDeNodo);
#else
	estadoConexion = CONECTADO;
	mensajeDeNodo = CreateMensaje("bienvenido", NULL);
#endif

	if (estadoConexion == DESCONECTADO) {
		log_error(logProcesoJob,
				"Se perdió la conexion con nodo IP:%s PUERTO:%d\n",
				inet_ntoa(hiloJob->direccionNodo.sin_addr),
				ntohs(hiloJob->direccionNodo.sin_port));
		ReportarResultadoHilo(hiloJob,
				ESTADO_HILO_FINALIZO_CON_ERROR_DE_CONEXION);
		return NULL;
	}

	log_info(logProcesoJob,
			"Recibido del nodo IP:%s PUERTO:%d\nComando: %s\nData: %s\n",
			inet_ntoa(hiloJob->direccionNodo.sin_addr),
			ntohs(hiloJob->direccionNodo.sin_port), mensajeDeNodo->comando,
			mensajeDeNodo->data);
	FreeMensaje(mensajeDeNodo);

	/*
	 * ENVIO DE MAP Y RESPUESTA
	 */

	char* bufferComando = string_new();
	char* bufferData = string_new();
	string_append_with_format(&bufferComando, "mapFile %s %i",
			hiloJob->nombreArchivo, hiloJob->nroBloque);
	string_append(&bufferData, scriptMapperStr);

	mensajeParaNodo = CreateMensaje(bufferComando, bufferData);

#ifndef BUILD_PARA_TEST
	enviar(hiloJob->socketFd, mensajeParaNodo);
#endif
	log_info(logProcesoJob,
			"Enviado al nodo IP:%s PUERTO:%d\nComando: %s\nData: %s\n",
			inet_ntoa(hiloJob->direccionNodo.sin_addr),
			ntohs(hiloJob->direccionNodo.sin_port), mensajeParaNodo->comando,
			mensajeParaNodo->data);
	free(bufferComando);
	free(bufferData);
	FreeMensaje(mensajeParaNodo);

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
	if (alternar == 1) {
		mensajeDeNodo = CreateMensaje("mapFileResponse 1", NULL);
		alternar = 0;
	} else {
		mensajeDeNodo = CreateMensaje("mapFileResponse 0", NULL);
		alternar = 1;
	}
#endif

	if (estadoConexion == DESCONECTADO) {
		log_error(logProcesoJob,
				"Se perdió la conexion con nodo IP:%s PUERTO:%d\n",
				inet_ntoa(hiloJob->direccionNodo.sin_addr),
				ntohs(hiloJob->direccionNodo.sin_port));
		ReportarResultadoHilo(hiloJob,
				ESTADO_HILO_FINALIZO_CON_ERROR_DE_CONEXION);
		return NULL;
	}

	log_info(logProcesoJob,
			"Recibido del nodo IP:%s PUERTO:%d\nComando: %s\nData: %s\n",
			inet_ntoa(hiloJob->direccionNodo.sin_addr),
			ntohs(hiloJob->direccionNodo.sin_port), mensajeDeNodo->comando,
			mensajeDeNodo->data);

	char** comandoStr = string_split(mensajeDeNodo->comando, " ");

	if (strncmp(comandoStr[MENSAJE_COMANDO], "mapFileResponse", 15) == 0) {
		if (atoi(comandoStr[1]) == 1) {
			ReportarResultadoHilo(hiloJob, ESTADO_HILO_FINALIZO_OK);
		} else {
			ReportarResultadoHilo(hiloJob,
					ESTADO_HILO_FINALIZO_CON_ERROR_EN_NODO);
		}
	} else {
		log_error(logProcesoJob,
				"Comando %s desconocido!! Por parte de nodo IP:%s PUERTO:%d\n",
				comandoStr[MENSAJE_COMANDO],
				inet_ntoa(hiloJob->direccionNodo.sin_addr),
				ntohs(hiloJob->direccionNodo.sin_port));
		ReportarResultadoHilo(hiloJob, ESTADO_HILO_FINALIZO_CON_ERROR_EN_NODO);
	}

	FreeStringArray(&comandoStr);
}
