#include "ProcesoJob.h"
#include <pthread.h>
#include <stdio.h>
#include <commons/collections/list.h>
#include <commons/error.h>
#include <arpa/inet.h>
#include <commons/config.h>
#include <commons/string.h>
#include <commons/log.h>

int socketMartaFd = -1;
char* scriptMapperStr = NULL;
char* scriptReduceStr = NULL;
pthread_t threadPedidosMartaHandler;
pthread_t threadProcesarHilos;
t_config* configuracion;
pthread_mutex_t mMarta;
t_log* logProcesoJob;

void* pedidosMartaHandler(void* arg) {

	HacerPedidoMarta();

	while (TRUE) {
		mensaje_t* mensajeMarta;

#ifndef BUILD_PARA_TEST
		recibir(socketMartaFd, mensajeMarta);
#else
		static int alternar = 0;
		if (alternar == 0) {
			mensajeMarta = malloc(sizeof(mensaje_t));
			mensajeMarta->comando = strdup("mapFile todo1.txt");
			mensajeMarta->data =
					strdup(
							"255.0.0.1 250 11 /ruta_temp1 244.0.1.7 250 12 /ruta_temp2 128.3.1.3 250 999 ruta_temp3");
			alternar = 1;
		} else if (alternar == 1) {
			mensajeMarta = malloc(sizeof(mensaje_t));
			mensajeMarta->comando = strdup("reduceFileSinCombiner todoSC1.txt");
			mensajeMarta->data =
					strdup(
							"227.4.6.1 999 2 /ruta_temp1 /ruta_temp2 117.4.5.1 259 2 /ruta_temp3 /ruta_temp4 167.5.4.1 250 1 ruta_temp5");
			alternar = 2;
		} else if (alternar == 2) {
			mensajeMarta = malloc(sizeof(mensaje_t));
			mensajeMarta->comando = strdup(
					"reduceFileConCombiner-Pedido1 todoCC1.txt");
			mensajeMarta->data =
					strdup(
							"227.4.6.1 999 archTempCC1 2 /ruta_temp1 /ruta_temp2 117.4.5.1 259 archTempCC2 2 /ruta_temp3 /ruta_temp4 167.5.4.1 250 archTempCC3 1 ruta_temp5");
			alternar = 3;
		} else {
			mensajeMarta = malloc(sizeof(mensaje_t));
			mensajeMarta->comando = strdup(
					"reduceFileConCombiner-Pedido2 todoCC2.txt");
			mensajeMarta->data =
					strdup(
							"227.4.6.1 999 1 /ruta_temp1 117.4.5.1 259 1 /ruta_temp3 167.5.4.1 250 1 ruta_temp5");
			alternar = 0;
		}
#endif
		printf(
				"-----Recibido mensaje de Marta-----\nComando: %s \nData: %s \n------------------\n",
				mensajeMarta->comando, mensajeMarta->data);
		char** comandoStr = string_split(mensajeMarta->comando, " ");

		if (strcmp(comandoStr[MENSAJE_COMANDO], "mapFile") == 0) {
			PlanificarHilosMapper(mensajeMarta);
		} else if (strcmp(comandoStr[MENSAJE_COMANDO], "reduceFileSinCombiner")
				== 0
				|| strcmp(comandoStr[MENSAJE_COMANDO],
						"reduceFileConCombiner-Pedido2") == 0) {
			PlanificarHilosReduce(mensajeMarta, FALSE);
		} else if (strcmp(comandoStr[MENSAJE_COMANDO],
				"reduceFileConCombiner-Pedido1") == 0) {
			PlanificarHilosReduce(mensajeMarta, TRUE);
		}

		FreeStringArray(&comandoStr);
		free(mensajeMarta);

#ifdef BUILD_PARA_TEST
		sleep(15);
#endif

	}
}

void IniciarConfiguracion() {

	configuracion = config_create("config.ini");
	if (!configuracion) {
		error_show("Error leyendo archivo configuracion!\n");
		Terminar(EXIT_ERROR);
	}

	scriptMapperStr = LeerArchivo(
			config_get_string_value(configuracion, "MAPPER"));
	scriptReduceStr = LeerArchivo(
			config_get_string_value(configuracion, "REDUCE"));

	logProcesoJob = log_create("./ProcesoJob.log", "ProcesoJob", TRUE,
			LOG_LEVEL_TRACE);

}

char* LeerArchivo(char* nombreArchivo) {

	FILE* archivoScript = fopen(nombreArchivo, "rb");
	int sizeScript;
	if (!archivoScript) {
		log_error(logProcesoJob, "Error leyendo archivo %s!\n", nombreArchivo);
		Terminar(EXIT_ERROR);
	}

	fseek(archivoScript, 0, SEEK_END);
	sizeScript = ftell(archivoScript);
	rewind(archivoScript);

	char* archivoStr = malloc(sizeScript);
	fread(archivoStr, sizeof(char) , sizeScript , archivoScript);

	fclose(archivoScript);

	return archivoStr;
}

void Terminar(int exitStatus) {

	if (configuracion) {
		config_destroy(configuracion);
	}
	if (socketMartaFd != -1) {
		close(socketMartaFd);
	}
	log_info(logProcesoJob, "Finalizacion del ProcesoJob con exit_status=%d\n",
			exitStatus);
	exit(exitStatus);
}

void IniciarConexionMarta() {

	char* ipMarta = config_get_string_value(configuracion, "IP_MARTA");
	char* puertoMarta = config_get_string_value(configuracion, "PUERTO_MARTA");

#ifndef BUILD_PARA_TEST
	struct sockaddr_in their_addr;

	if ((socketMartaFd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
		log_error(logProcesoJob,"Error al crear socket para MARTA\n");
		Terminar(EXIT_ERROR);
	}

	their_addr.sin_family = AF_INET;
	their_addr.sin_port = htons(atoi(ipMarta));
	inet_aton(puertoMarta, &(their_addr.sin_addr));
	memset(&(their_addr.sin_zero), '\o', 8);

	if (connect(socketMartaFd, (Sockaddr_in*) &their_addr, sizeof(Sockaddr_in))
			== -1) {
		log_error(logProcesoJob,"Error al conectarse con MARTA, con IP: %s PUERTO: %s\n", ipMarta, puertoMarta);
		Terminar(EXIT_ERROR);
	}
#else
	socketMartaFd = 999;
#endif
	log_info(logProcesoJob, "Conexion exitosa con Marta, IP: %s, PUERTO: %s\n",
			ipMarta, puertoMarta);

	pthread_create(&threadPedidosMartaHandler, NULL, pedidosMartaHandler, NULL);
	pthread_mutex_init(&mMarta, NULL);

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

#ifndef BUILD_PARA_TEST
		enviar(socketMartaFd, mensaje);
#endif
		log_info(logProcesoJob,
				"Se envió a MaRTA el siguiente mensaje\nComando: %s\nData: %s\n",
				mensaje->comando, mensaje->data);
		free(buffer);
		free(mensaje);

	}

}

void ReportarResultadoHilo(HiloJob* hiloJob, EstadoHilo estado) {

	mensaje_t* mensajeParaMarta = malloc(sizeof(mensaje_t));
	char* bufferComandoStr = string_new();
	char* bufferDataStr = string_new();

	string_append_with_format(&bufferComandoStr,
			hiloJob->tipoHilo == TIPO_HILO_MAPPER ?
					"mapFileResponse %s" : "reduceFileResponse %s",
			hiloJob->nombreArchivo);

	switch (estado) {

	case ESTADO_HILO_FINALIZO_CON_ERROR_DE_CONEXION:
	case ESTADO_HILO_FINALIZO_CON_ERROR_EN_NODO:
		string_append(&bufferComandoStr, " 0");
		string_append_with_format(&bufferDataStr, "%s",
				hiloJob->parametrosError != NULL ?
						hiloJob->parametrosError :
						inet_ntoa(hiloJob->direccionNodo.sin_addr));
		break;
	case ESTADO_HILO_FINALIZO_OK:
		string_append(&bufferComandoStr, " 1");
		break;

	}

	mensajeParaMarta->comandoSize = strlen(bufferComandoStr);
	mensajeParaMarta->comando = bufferComandoStr;
	mensajeParaMarta->dataSize = strlen(bufferDataStr);
	mensajeParaMarta->data = bufferDataStr;

	/*
	 * varios hilos pueden estar reportando a marta
	 */
#ifndef BUILD_PARA_TEST
	pthread_mutex_lock(&mMarta);
	enviar(socketMartaFd, mensajeParaMarta);
	pthread_mutex_unlock(&mMarta);

	if(hiloJob->socketFd != -1) {
		close(hiloJob->socketFd);
	}
#endif

	log_info(logProcesoJob,
			"Se envió a MaRTA el siguiente mensaje\nComando: %s\nData: %s\n",
			mensajeParaMarta->comando, mensajeParaMarta->data);
	free(bufferComandoStr);
	free(mensajeParaMarta);

}

void PlanificarHilosMapper(mensaje_t* mensaje) {

	char** dataStr = string_split(mensaje->data, " ");

	//direccionNodo puertoEscucha nroDeBloque rutaArchivoTemporal

	int i = 0;
	while (dataStr[i] != NULL) {

		Sockaddr_in their_addr;

		their_addr.sin_family = AF_INET;
		inet_aton(dataStr[i++], &(their_addr.sin_addr));
		their_addr.sin_port = htons(atoi(dataStr[i++]));
		memset(&(their_addr.sin_zero), '\o', 8);

		HiloJob* hiloJob = malloc(sizeof(HiloJob));
		hiloJob->direccionNodo = their_addr;
		hiloJob->threadhilo = NULL;
		hiloJob->socketFd = -1;
		hiloJob->parametros = NULL;
		hiloJob->parametrosError = NULL;
		hiloJob->nroBloque = atoi(dataStr[i++]);
		hiloJob->nombreArchivo = string_duplicate(dataStr[i++]);

		CrearHiloMapper(hiloJob);
		log_info(logProcesoJob,
				"Se creó un Hilo Mapper con los siguientes parametros\nNombre del Archivo temporal: %s\nNumero de bloque :%d\n",
				hiloJob->nombreArchivo, hiloJob->nroBloque);

	}

	FreeStringArray(&dataStr);
}

void PlanificarHilosReduce(mensaje_t* mensaje, int conCombiner) {

	char** comandoStr = string_split(mensaje->comando, " ");
	char** dataStr = string_split(mensaje->data, " ");

	if (conCombiner) {

		//*comando: "reduceFileConCombiner-Pedido1 pathArchivo"
		//*data:      Nodo1 nombreArchTemp1 CantDeArchEnNodoAProcesar RAT1 RAT2 -etc...-
		//                ...Nodo2 nombreArchTemp2 CantDeArchEnNodoAProcesar RTA1 RAT2 -etc...
		//		...Nodo3 ....

		int i = 0;
		while (dataStr[i] != NULL) {

			Sockaddr_in their_addr;

			their_addr.sin_family = AF_INET;
			inet_aton(dataStr[i++], &(their_addr.sin_addr));
			their_addr.sin_port = htons(atoi(dataStr[i++]));
			memset(&(their_addr.sin_zero), '\o', 8);

			HiloJob* hiloJob = malloc(sizeof(HiloJob));
			hiloJob->direccionNodo = their_addr;
			hiloJob->threadhilo = NULL;
			hiloJob->socketFd = -1;
			hiloJob->parametros = NULL;
			hiloJob->parametrosError = NULL;
			hiloJob->nombreArchivo = string_duplicate(dataStr[i++]);

			int archivosTotalesAProcesar = atoi(dataStr[i++]);
			int leerHasta = i + archivosTotalesAProcesar;

			char* parametrosBuffer = string_new();
			string_append_with_format(&parametrosBuffer, "%d %s",
					archivosTotalesAProcesar, dataStr[i++]);

			while (i < leerHasta) {
				string_append_with_format(&parametrosBuffer, " %s",
						dataStr[i++]);
			}
			hiloJob->parametros = parametrosBuffer;

			CrearHiloReducer(hiloJob);
			log_info(logProcesoJob,
					"Se creó un Hilo Reduce desde un pedido de Marta con combiner con los siguientes parametros\nNombre del Archivo temporal: %s\nParametros: %s\n",
					hiloJob->nombreArchivo, hiloJob->parametros);

		}
	} else {

		//*comando: "reduceFileSinCombiner NombreArchTempFinal "
		//*data:	ipLoc puertoLoc  CantDeArchEnNodoLocalAProcesar RAT1 RAT2-...etc...-
		//          ipRem puertoRem CantDeArchEnNodoRemotoAProcesar RTA1 RAT2 RAT3 -etc...
		//			...ipRem2 puertoRem2...."
		int i = 0;

		Sockaddr_in their_addr;

		their_addr.sin_family = AF_INET;
		inet_aton(dataStr[i++], &(their_addr.sin_addr));
		their_addr.sin_port = htons(atoi(dataStr[i++]));
		memset(&(their_addr.sin_zero), '\o', 8);

		HiloJob* hiloJob = malloc(sizeof(HiloJob));
		hiloJob->direccionNodo = their_addr;
		hiloJob->threadhilo = NULL;
		hiloJob->socketFd = -1;
		hiloJob->parametros = NULL;
		hiloJob->parametrosError = NULL;
		hiloJob->nombreArchivo = string_duplicate(
				comandoStr[MENSAJE_COMANDO_NOMBREARCHIVO]);

		char* parametrosBuffer = string_new();

		string_append_with_format(&parametrosBuffer, "%s", dataStr[i++]);
		while (dataStr[i] != NULL) {
			string_append_with_format(&parametrosBuffer, " %s", dataStr[i++]);
		}

		hiloJob->parametros = parametrosBuffer;
		CrearHiloReducer(hiloJob);
		log_info(logProcesoJob,
				"Se creó un Hilo Reduce sin combiner con los siguientes parametros\nNombre del Archivo temporal: %s\nParametros: %s\n",
				hiloJob->nombreArchivo, hiloJob->parametros);

	}

	FreeStringArray(&dataStr);
	FreeStringArray(&comandoStr);

}

void FreeStringArray(char*** stringArray) {
	char** array = *stringArray;

	int i;
	for (i = 0; array[i] != NULL; ++i) {
		free(array[i]);
	}

	free(array);
}

int main(int argc, char* argv[]) {

#ifdef BUILD_PARA_TEST
	srand(time(NULL));
#endif
	IniciarConfiguracion();
	IniciarConexionMarta();

	pthread_join(threadPedidosMartaHandler, NULL);
	Terminar(EXIT_OK);
}
