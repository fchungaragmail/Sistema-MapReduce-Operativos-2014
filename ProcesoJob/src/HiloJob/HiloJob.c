#include "HiloJob.h"
#include "../Utils.h"
#include <commons/log.h>

extern char* scriptMapperStr;
extern int scriptMapperSize;
extern char* scriptReduceStr;
extern int scriptReduceSize;
extern t_log* logProcesoJob;

extern int cantidadDeHilosActivos;
extern pthread_mutex_t mHilos;
pthread_t* CrearHiloJob(HiloJobInfo* hiloJobInfo, TipoHilo tipoHilo) {

	pthread_t* hiloJob;

	hiloJobInfo->tipoHilo = tipoHilo;
	pthread_create(&hiloJob, NULL, hiloJobHandler, (void*) hiloJobInfo);
	hiloJobInfo->threadhilo = hiloJob;
	pthread_mutex_lock(&mHilos);
	cantidadDeHilosActivos++;
	pthread_mutex_unlock(&mHilos);
	return hiloJob;
}

void* hiloJobHandler(void* arg) {

	HiloJobInfo* hiloJobInfo = (HiloJobInfo*) arg;
	int estadoConexion = CONECTADO;

#ifndef BUILD_CON_MOCK_NODO
	if ((hiloJobInfo->socketFd = socket(AF_INET, SOCK_STREAM, 0)) == -1) { //<--- CREO QUE SI OCURRE ESTO, FALLO JOB Y DEBE ABORTAR
		log_error(logProcesoJob, "Error al crear socket para nodo %s\n",
				inet_ntoa(hiloJobInfo->direccionNodo.sin_addr));
		ReportarResultadoHilo(hiloJobInfo,
				ESTADO_HILO_FINALIZO_CON_ERROR_DE_CONEXION);
		return NULL;
	}

	if (connect(hiloJobInfo->socketFd,
			(Sockaddr_in*) &hiloJobInfo->direccionNodo, sizeof(Sockaddr_in))
			== -1) {
		log_error(logProcesoJob,
				"Intento de conexion fallida con nodo IP:%s PUERTO:%d\n",
				inet_ntoa(hiloJobInfo->direccionNodo.sin_addr),
				ntohs(hiloJobInfo->direccionNodo.sin_port));
		ReportarResultadoHilo(hiloJobInfo,
				ESTADO_HILO_FINALIZO_CON_ERROR_DE_CONEXION);
		return NULL;
	}
#else
	hiloJobInfo->socketFd = 99;
#endif
	log_info(logProcesoJob, "Conexion exitosa con nodo IP:%s PUERTO:%d\n",
			inet_ntoa(hiloJobInfo->direccionNodo.sin_addr),
			ntohs(hiloJobInfo->direccionNodo.sin_port));

	/*
	 *  HANDSHAKE
	 */

	mensaje_t* mensajeDeNodo = NULL;
	mensaje_t* mensajeParaNodo = NULL;

	if (hiloJobInfo->tipoHilo == TIPO_HILO_MAPPER) {
		mensajeParaNodo = CreateMensaje("mp", NULL);
	} else { //TIPO_HILO_REDUCE
		if (hiloJobInfo->subTipoHilo == SUBTIPO_REDUCE_CON_COMBINER_FINAL) {
			mensajeParaNodo = CreateMensaje("rdf", NULL);
		} else {
			mensajeParaNodo = CreateMensaje("rd", NULL);
		}
	}

#ifndef BUILD_CON_MOCK_NODO
	enviar(hiloJobInfo->socketFd, mensajeParaNodo);
#endif
	log_info(logProcesoJob,
			"Enviado al nodo IP:%s PUERTO:%d\nComando: %s\nData: %s\n",
			inet_ntoa(hiloJobInfo->direccionNodo.sin_addr),
			ntohs(hiloJobInfo->direccionNodo.sin_port),
			mensajeParaNodo->comando, mensajeParaNodo->data);
	FreeMensaje(mensajeParaNodo);

#ifndef BUILD_CON_MOCK_NODO
	mensajeDeNodo = malloc(sizeof(mensaje_t));
	estadoConexion = recibir(hiloJobInfo->socketFd, mensajeDeNodo);
#else
	estadoConexion = CONECTADO;
	mensajeDeNodo = CreateMensaje("bienvenido", NULL);
#endif

	if (estadoConexion == DESCONECTADO) {
		log_error(logProcesoJob,
				"Se perdió la conexion con nodo IP:%s PUERTO:%d\n",
				inet_ntoa(hiloJobInfo->direccionNodo.sin_addr),
				ntohs(hiloJobInfo->direccionNodo.sin_port));
		ReportarResultadoHilo(hiloJobInfo,
				ESTADO_HILO_FINALIZO_CON_ERROR_DE_CONEXION);
		return NULL;
	}

	log_info(logProcesoJob,
			"Recibido del nodo IP:%s PUERTO:%d\nComando: %s\nData: %s\n",
			inet_ntoa(hiloJobInfo->direccionNodo.sin_addr),
			ntohs(hiloJobInfo->direccionNodo.sin_port), mensajeDeNodo->comando,
			mensajeDeNodo->data);

	FreeMensaje(mensajeDeNodo);

	/*
	 * ENVIO DE MAP/REDUCE Y RECEPCION DE RESPUESTA
	 */

	char* bufferComando = string_new();

	if (hiloJobInfo->tipoHilo == TIPO_HILO_MAPPER) {
		string_append_with_format(&bufferComando, "mapFile %s %s %i",
				hiloJobInfo->nombreArchivo, hiloJobInfo->parametros,
				hiloJobInfo->nroBloque);
	} else { //TIPO_HILO_REDUCE
		string_append_with_format(&bufferComando, "reduceFile %s %s",
				hiloJobInfo->nombreArchivo, hiloJobInfo->parametros);
	}

	mensajeParaNodo = CreateMensajeParaHilo(bufferComando,
			hiloJobInfo->tipoHilo == TIPO_HILO_MAPPER ?
					scriptMapperStr : scriptReduceStr,
			hiloJobInfo->tipoHilo == TIPO_HILO_MAPPER ?
					scriptMapperSize : scriptReduceSize);

#ifndef BUILD_CON_MOCK_NODO
	enviar(hiloJobInfo->socketFd, mensajeParaNodo);
#endif

	log_info(logProcesoJob, "Enviado al nodo IP:%s PUERTO:%d\nComando: %s\n",
			inet_ntoa(hiloJobInfo->direccionNodo.sin_addr),
			ntohs(hiloJobInfo->direccionNodo.sin_port),
			mensajeParaNodo->comando);
	free(bufferComando);
	FreeMensaje(mensajeParaNodo);

#ifdef BUILD_CON_MOCK_NODO
	int tiempoParaDormir = rand() % 5;
	sleep(tiempoParaDormir);
#endif

#ifndef BUILD_CON_MOCK_NODO
	mensajeDeNodo = malloc(sizeof(mensaje_t));
	estadoConexion = recibir(hiloJobInfo->socketFd, mensajeDeNodo);
#else
	estadoConexion = CONECTADO;
	mensajeDeNodo = NULL;

	if (hiloJobInfo->tipoHilo == TIPO_HILO_MAPPER) {
		static int alternar = 1;
		if (alternar == 1) {
			mensajeDeNodo = CreateMensaje("mapFileResponse 1", NULL);
			alternar = 0;
		} else {
			mensajeDeNodo = CreateMensaje("mapFileResponse 0", NULL);
			alternar = 1;
		}
	} else { //TIPO_HILO_REDUCE
		static int alternar = 1;
		if (alternar == 1) {
			mensajeDeNodo = CreateMensaje("reduceFileResponse 1", NULL);
			alternar = 0;
		} else {
			mensajeDeNodo = CreateMensaje("reduceFileResponse 0",
					"129.3.6.9 112.3.4.5");
			alternar = 1;
		}
	}

#endif

	if (estadoConexion == DESCONECTADO) {
		log_error(logProcesoJob,
				"Se perdió la conexion con nodo IP:%s PUERTO:%d\n",
				inet_ntoa(hiloJobInfo->direccionNodo.sin_addr),
				ntohs(hiloJobInfo->direccionNodo.sin_port));
		ReportarResultadoHilo(hiloJobInfo,
				ESTADO_HILO_FINALIZO_CON_ERROR_DE_CONEXION);
		return NULL;
	}

	log_info(logProcesoJob,
			"Recibido del nodo IP:%s PUERTO:%d\nComando: %s\nData: %s\n",
			inet_ntoa(hiloJobInfo->direccionNodo.sin_addr),
			ntohs(hiloJobInfo->direccionNodo.sin_port), mensajeDeNodo->comando,
			mensajeDeNodo->data);

	char** comandoStr = string_split(mensajeDeNodo->comando, " ");

	if (strcmp(comandoStr[MENSAJE_COMANDO], "mapFileResponse") == 0
			|| strcmp(comandoStr[MENSAJE_COMANDO], "reduceFileResponse") == 0) {
		if (atoi(comandoStr[1]) == 1) {
			ReportarResultadoHilo(hiloJobInfo, ESTADO_HILO_FINALIZO_OK);
		} else {
			if (strlen(mensajeDeNodo->data) > 0) {
				hiloJobInfo->parametrosError = strdup(mensajeDeNodo->data);
			}
			ReportarResultadoHilo(hiloJobInfo,
					ESTADO_HILO_FINALIZO_CON_ERROR_EN_NODO);
		}
	} else {
		log_error(logProcesoJob,
				"Comando %s desconocido!! Por parte de nodo IP:%s PUERTO:%d\n",
				comandoStr[MENSAJE_COMANDO],
				inet_ntoa(hiloJobInfo->direccionNodo.sin_addr),
				ntohs(hiloJobInfo->direccionNodo.sin_port));
		ReportarResultadoHilo(hiloJobInfo,
				ESTADO_HILO_FINALIZO_CON_ERROR_EN_NODO);
	}

	FreeStringArray(&comandoStr);
}
