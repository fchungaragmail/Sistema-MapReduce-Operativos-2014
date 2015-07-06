#include "HiloReducer.h"
#include "../Utils.h"
#include <commons/log.h>

extern char* scriptReduceStr;
extern t_log* logProcesoJob;

pthread_t* CrearHiloReducer(HiloJob* hiloJob) {

	pthread_t* hiloReducer;

	hiloJob->tipoHilo = TIPO_HILO_REDUCE;
	pthread_create(&hiloReducer, NULL, hiloReducerHandler, (void*) hiloJob);
	hiloJob->threadhilo = hiloReducer;

	return hiloReducer;
}

void* hiloReducerHandler(void* arg) {

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

	mensajeParaNodo = CreateMensaje("rd", NULL);

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
	 * ENVIO DE REDUCE Y RESPUESTA
	 */

	char* bufferComando = string_new();
	char* bufferData = string_new();
	string_append_with_format(&bufferComando, "reduceFileConCombiner %s %s",
			hiloJob->nombreArchivo, hiloJob->parametros);
	string_append(&bufferData, scriptReduceStr);

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
		mensajeDeNodo = CreateMensaje("reduceFileResponse 1", NULL);
		alternar = 0;
	} else {
		mensajeDeNodo = CreateMensaje("reduceFileResponse 0",
				"129.3.6.9 112.3.4.5");
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

	if (strncmp(comandoStr[MENSAJE_COMANDO], "reduceFileResponse", 18) == 0) {
		if (atoi(comandoStr[1]) == 1) {
			ReportarResultadoHilo(hiloJob, ESTADO_HILO_FINALIZO_OK);
		} else {
			hiloJob->parametrosError = strdup(mensajeDeNodo->data);
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
