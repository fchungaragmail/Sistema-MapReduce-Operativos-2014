#include "ProcesoJob.h"
#include "Utils.h"
#include <pthread.h>
#include <stdio.h>
#include <commons/collections/list.h>
#include <commons/collections/dictionary.h>
#include <commons/error.h>
#include <arpa/inet.h>
#include <commons/config.h>
#include <commons/string.h>
#include <commons/log.h>
#include <fcntl.h>

int socketMartaFd = -1;
char* scriptMapperStr = NULL;
char* scriptReduceStr = NULL;
pthread_t threadPedidosMartaHandler;
pthread_t threadProcesarHilos;
t_config* configuracion;
pthread_mutex_t mMarta;
t_log* logProcesoJob;
t_dictionary* diccArchivos = NULL;

void* pedidosMartaHandler(void* arg) {

	HacerPedidoMarta();

	while (TRUE) {
		mensaje_t* mensajeMarta;

#ifndef BUILD_CON_MOCK_MARTA
		mensajeMarta = malloc(sizeof(mensaje_t));
		int resultado = recibir(socketMartaFd, mensajeMarta);
#else
		static int alternar = 1;
		if (alternar == 0) {
			mensajeMarta =
					CreateMensaje("mapFile todo1.txt",
							"192.168.0.103 6000 0 prueba.txt");
			alternar = 1;
		} else if (alternar == 1) {
			mensajeMarta =
					CreateMensaje("reduceFileSinCombiner prueba-final.txt",
							"192.168.0.103 6000 1 prueba.txt");
			alternar = 2;
		} else if (alternar == 2) {
			mensajeMarta =
					CreateMensaje("reduceFileConCombiner-Pedido1 todoCC1.txt",
							"227.4.6.1 999 archTempCC1 2 /ruta_temp1 /ruta_temp2 117.4.5.1 259 archTempCC2 2 /ruta_temp3 /ruta_temp4 167.5.4.1 250 archTempCC3 1 ruta_temp5");
			alternar = 3;
		} else if (alternar == 3) {
			mensajeMarta =
					CreateMensaje("reduceFileConCombiner-Pedido2 todoCC2.txt",
							"227.4.6.1 999 1 /ruta_temp1 117.4.5.1 259 1 /ruta_temp3 167.5.4.1 250 1 ruta_temp5");
			alternar = 4;
		} else {
			mensajeMarta = CreateMensaje("FileSuccess todo1.txt", NULL);
			alternar = 0;
		}
		int resultado = CONECTADO;
#endif
		if(resultado == DESCONECTADO){
			log_error(logProcesoJob,"Se perdió la conexion con MaRTA");
			Terminar(EXIT_ERROR);
			break;
		}

		log_info(logProcesoJob, "Recibido del MaRTA:\nComando: %s\nData: %s\n",
				mensajeMarta->comando, mensajeMarta->data);

		char** comandoStr = string_split(mensajeMarta->comando, " ");

		if (strcmp(comandoStr[MENSAJE_COMANDO], "mapFile") == 0) {
			PlanificarHilosMapper(mensajeMarta);
		} else if (strcmp(comandoStr[MENSAJE_COMANDO], "reduceFileSinCombiner")
				== 0
				|| strcmp(comandoStr[MENSAJE_COMANDO],
						"reduceFileConCombiner-Pedido2") == 0) {
			PlanificarHilosReduce(mensajeMarta, FALSE, NULL);
		} else if (strcmp(comandoStr[MENSAJE_COMANDO],
				"reduceFileConCombiner-Pedido1") == 0) {
			PlanificarHilosReduce(mensajeMarta, TRUE, NULL);
		} else if (strcmp(comandoStr[MENSAJE_COMANDO],
				"reduceFinal") == 0) {
			PlanificarHilosReduce(mensajeMarta, TRUE, config_get_string_value(configuracion, "RESULTADO"));
		} else if (strcmp(comandoStr[MENSAJE_COMANDO], "FileSuccess") == 0) {
			char* dataDiccionario = dictionary_remove(diccArchivos,
					comandoStr[MENSAJE_COMANDO_NOMBREARCHIVO]);
			if (dataDiccionario) {
				free(dataDiccionario);
				log_debug(logProcesoJob,
						"Se completo %s, quedan %d archivos.\n",
						comandoStr[MENSAJE_COMANDO_NOMBREARCHIVO],
						dictionary_size(diccArchivos));
				if (dictionary_is_empty(diccArchivos)) {
					break;
				}
			} else {
				log_warning(logProcesoJob,
						"Nombre de archivo: %s incorrecto o ya fue completado con anterioridad!\n",
						comandoStr[MENSAJE_COMANDO_NOMBREARCHIVO]);
			}

		}

		FreeStringArray(&comandoStr);
		FreeMensaje(mensajeMarta);

#ifdef BUILD_CON_MOCK_MARTA
		sleep(500);
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

	diccArchivos = dictionary_create();

}

char* LeerArchivo(char* nombreArchivo) {

	int archivo = open(nombreArchivo,O_RDONLY);
	if (archivo == -1 ) {
		log_error(logProcesoJob, "Error leyendo archivo %s!\n", nombreArchivo);
		Terminar(EXIT_ERROR);
	}

	//leer todo el archivo
	struct stat infoArchivo;
	stat(nombreArchivo, &infoArchivo);
	char *contenido = malloc(infoArchivo.st_size);
	read(archivo, contenido, infoArchivo.st_size);

	return contenido;
}

void Terminar(int exitStatus) {

	if (configuracion) {
		config_destroy(configuracion);
	}
	if (socketMartaFd != -1) {
		close(socketMartaFd);
	}
	log_info(logProcesoJob,
			"Finalizacion del ProcesoJob con exit_status = %d\n", exitStatus);
	exit(exitStatus);
}

void IniciarConexionMarta() {

	char* ipMarta = config_get_string_value(configuracion, "IP_MARTA");
	char* puertoMarta = config_get_string_value(configuracion, "PUERTO_MARTA");

#ifndef BUILD_CON_MOCK_MARTA
	struct sockaddr_in their_addr;

	if ((socketMartaFd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
		log_error(logProcesoJob,"Error al crear socket para MARTA\n");
		Terminar(EXIT_ERROR);
	}

	their_addr.sin_family = AF_INET;
	their_addr.sin_port = htons(atoi(puertoMarta));
	inet_aton(ipMarta, &(their_addr.sin_addr));
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

		mensaje_t* mensaje = CreateMensaje(buffer, NULL);

#ifndef BUILD_CON_MOCK_MARTA
		enviar(socketMartaFd, mensaje);
#endif
		log_info(logProcesoJob,
				"Se envió a MaRTA el siguiente mensaje\nComando: %s\nData: %s\n",
				mensaje->comando, mensaje->data);

		dictionary_put(diccArchivos, archivos[i], string_duplicate("")); //Por ahora no hay info inportante por cada archivo

		free(buffer);
		FreeMensaje(mensaje);

	}

}

void ReportarResultadoHilo(HiloJobInfo* hiloJobInfo, EstadoHilo estado) {

	mensaje_t* mensajeParaMarta = NULL;
	char* bufferComandoStr = string_new();
	char* bufferDataStr = string_new();

	string_append_with_format(&bufferComandoStr,
			hiloJobInfo->tipoHilo == TIPO_HILO_MAPPER ?
					"mapFileResponse %s" : "reduceFileResponse %s",
			hiloJobInfo->nombreArchivo);

	switch (estado) {

	case ESTADO_HILO_FINALIZO_CON_ERROR_DE_CONEXION:
	case ESTADO_HILO_FINALIZO_CON_ERROR_EN_NODO:
		string_append(&bufferComandoStr, " 0");
		string_append_with_format(&bufferDataStr, "%s",
				hiloJobInfo->parametrosError != NULL ?
						hiloJobInfo->parametrosError :
						inet_ntoa(hiloJobInfo->direccionNodo.sin_addr));
		break;
	case ESTADO_HILO_FINALIZO_OK:
		string_append(&bufferComandoStr, " 1");
		break;

	}

	mensajeParaMarta = CreateMensaje(bufferComandoStr, bufferDataStr);

	/*
	 * varios hilos pueden estar reportando a marta
	 */
#ifndef BUILD_CON_MOCK_MARTA
	pthread_mutex_lock(&mMarta);
	enviar(socketMartaFd, mensajeParaMarta);
	pthread_mutex_unlock(&mMarta);

	if(hiloJobInfo->socketFd != -1) {
		close(hiloJobInfo->socketFd);
	}
#endif

	log_info(logProcesoJob,
			"Se envió a MaRTA el siguiente mensaje\nComando: %s\nData: %s\n",
			mensajeParaMarta->comando, mensajeParaMarta->data);
	free(bufferComandoStr);
	FreeMensaje(mensajeParaMarta);
	FreeHiloJobInfo(hiloJobInfo);

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

		HiloJobInfo* hiloJobInfo = malloc(sizeof(HiloJobInfo));
		hiloJobInfo->direccionNodo = their_addr;
		hiloJobInfo->threadhilo = NULL;
		hiloJobInfo->socketFd = -1;
		hiloJobInfo->parametros = NULL;
		hiloJobInfo->parametrosError = NULL;
		hiloJobInfo->nroBloque = atoi(dataStr[i++]);
		hiloJobInfo->nombreArchivo = string_duplicate(dataStr[i++]);

		char* parametrosBuffer = string_new();
		string_append_with_format(&parametrosBuffer, "%s", config_get_string_value(configuracion, "MAPPER"));

		hiloJobInfo->parametros = strdup(parametrosBuffer);

		free(parametrosBuffer);
		CrearHiloJob(hiloJobInfo, TIPO_HILO_MAPPER);
		log_info(logProcesoJob,
				"Se creó un Hilo Mapper con los siguientes parametros\nNombre del Archivo temporal: %s\nNumero de bloque :%d\n",
				hiloJobInfo->nombreArchivo, hiloJobInfo->nroBloque);

	}

	FreeStringArray(&dataStr);
}

void PlanificarHilosReduce(mensaje_t* mensaje, int conCombiner, char* nombreArchivo) {

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

			HiloJobInfo* hiloJobInfo = malloc(sizeof(HiloJobInfo));
			hiloJobInfo->direccionNodo = their_addr;
			hiloJobInfo->threadhilo = NULL;
			hiloJobInfo->socketFd = -1;
			hiloJobInfo->parametros = NULL;
			hiloJobInfo->parametrosError = NULL;

			if(nombreArchivo){
				hiloJobInfo->nombreArchivo = string_duplicate(nombreArchivo);
				i++;
			} else{
				hiloJobInfo->nombreArchivo = string_duplicate(dataStr[i++]);
			}

			int archivosTotalesAProcesar = atoi(dataStr[i++]);
			int leerHasta = i + archivosTotalesAProcesar;

			char* parametrosBuffer = string_new();

			string_append_with_format(&parametrosBuffer, "%s %d %s", config_get_string_value(configuracion, "REDUCE"),
					archivosTotalesAProcesar, dataStr[i++]);

			while (i < leerHasta) {
				string_append_with_format(&parametrosBuffer, " %s",
						dataStr[i++]);
			}
			hiloJobInfo->parametros = strdup(parametrosBuffer);

			CrearHiloJob(hiloJobInfo, TIPO_HILO_REDUCE);

			free(parametrosBuffer);
			log_info(logProcesoJob,
					"Se creó un Hilo Reduce desde un pedido de Marta con combiner con los siguientes parametros\nNombre del Archivo temporal: %s\nParametros: %s\n",
					hiloJobInfo->nombreArchivo, hiloJobInfo->parametros);

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

		HiloJobInfo* hiloJobInfo = malloc(sizeof(HiloJobInfo));
		hiloJobInfo->direccionNodo = their_addr;
		hiloJobInfo->threadhilo = NULL;
		hiloJobInfo->socketFd = -1;
		hiloJobInfo->parametros = NULL;
		hiloJobInfo->parametrosError = NULL;
		hiloJobInfo->nombreArchivo = string_duplicate(
				comandoStr[MENSAJE_COMANDO_NOMBREARCHIVO]);

		if(nombreArchivo){
			hiloJobInfo->nombreArchivo = string_duplicate(nombreArchivo);
		} else{
			hiloJobInfo->nombreArchivo = string_duplicate(comandoStr[MENSAJE_COMANDO_NOMBREARCHIVO]);
		}

		char* parametrosBuffer = string_new();

		string_append_with_format(&parametrosBuffer, "%s %s",config_get_string_value(configuracion, "REDUCE"), dataStr[i++]);
		while (dataStr[i] != NULL) {
			string_append_with_format(&parametrosBuffer, " %s", dataStr[i++]);
		}

		hiloJobInfo->parametros = strdup(parametrosBuffer);
		CrearHiloJob(hiloJobInfo, TIPO_HILO_REDUCE);

		free(parametrosBuffer);
		log_info(logProcesoJob,
				"Se creó un Hilo Reduce sin combiner con los siguientes parametros\nNombre del Archivo temporal: %s\nParametros: %s\n",
				hiloJobInfo->nombreArchivo, hiloJobInfo->parametros);

	}

	FreeStringArray(&dataStr);
	FreeStringArray(&comandoStr);

}

int main(int argc, char* argv[]) {

#ifdef BUILD_CON_MOCK_NODO
	srand(time(NULL));
#endif
	IniciarConfiguracion();
	IniciarConexionMarta();

	pthread_join(threadPedidosMartaHandler, NULL);
	Terminar(EXIT_OK);
}
