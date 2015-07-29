/*
 * nodo_new.c
 *
 *  Created on: 11/6/2015
 *      Author: utnso
 */

#include "nodo_new.h"

pthread_mutex_t getBloqueMutex;

//Main Programm
int main() {

	int sockFD; //socket nodo servidor
	int sockAccept; //socket nodo servidor acepta conexiones
	int* sockFS = malloc(sizeof(int)); //socket nodo cliente para conectarse con FS
	int* sockForThread = malloc(sizeof(int)); //sockFS que paso por parametro al thread que queda hablando con FS
	bool isFinalReduce;
	socklen_t size;
	Sockaddr_in client_sock; //sockaddr que se usa en accept de nodo servidor
	pthread_t nodo_handler;
	pthread_t fs_handler;
	pthread_t map_handler;
	pthread_t reduce_handler;
	mensaje_t* buffer_shakehand = malloc(sizeof(mensaje_t));
	mensaje_t* buffer_send = malloc(sizeof(mensaje_t));
	log_nodo = log_create("./log_nodo", "NODO", true, LOG_LEVEL_TRACE);
	getConfig();
	sem_init(&sMaps,0,5);
	pthread_mutex_init(&getBloqueMutex, NULL);

	connectToFileSistem(sockFS);
	pthread_create(&fs_handler, NULL, fs_nodo_conection_handler, sockFS);
	//pthread_join(fs_handler, NULL);

	int value = initServer(&sockFD);
	if (value == -1) {
		printf("No se pudo crear socket que atiende conexiones");
		printf("***********************\n");
		return -1;
	}

	size = sizeof(struct sockaddr_in);

	//comunicacion jobs
	while (1) {
		sockAccept = accept(sockFD, (struct sockaddr*) &client_sock, &size);
		if (sockAccept < 0) {
			printf("No se pudo aceptar conexión\n");
			printf("***********************\n");
			close(sockFD);
			close(sockAccept);
			return -1;
		}


		recibir(sockAccept, buffer_shakehand); //recibo mensaje shakehand

		buffer_send->comandoSize = SHAKEHAND_MESSAGE_LENGTH;
		buffer_send->comando = "Bienvenido al Nodo";
		buffer_send->dataSize = 1; //es 1 pq no envio nada y hace un malloc de 1 asi no ocupa memoria
		buffer_send->data = "\0"; //aca no le envio nada
		enviar(sockAccept, buffer_send);

		if (strcmp(buffer_shakehand->comando, "nd") == 0) {
			int* sockAux = malloc(sizeof(int));
			//sockAux = &sockAccept;
			*sockAux = sockAccept;
			printf("Se obtuvo una conexión desde NODO: %s\n",
					inet_ntoa(client_sock.sin_addr));
			pthread_create(&nodo_handler, NULL, fs_nodo_conection_handler,
					sockAux);
		}

		if (strcmp(buffer_shakehand->comando, "mp") == 0) {
			int* sockAux = malloc(sizeof(int));
			//sockAux = &sockAccept;
			*sockAux = sockAccept;
			printf("Se obtuvo una conexión desde JOB: %s\n",
					inet_ntoa(client_sock.sin_addr));
			pthread_create(&map_handler, NULL, map_conection_handler, sockAux);
		}

		if (strcmp(buffer_shakehand->comando, "rd") == 0) {
			int* sockAux = malloc(sizeof(int));
			//sockAux = &sockAccept;
			*sockAux = sockAccept;
			reduceArguments_t* reduceArguments = malloc(sizeof(reduceArguments_t));
			reduceArguments->isFinalReduce = false;
			reduceArguments->sockFS = sockFS;
			reduceArguments->sockJob = sockAux;
			pthread_create(&reduce_handler, NULL, reduce_conection_handler,
					reduceArguments);
		}

		if(strcmp(buffer_shakehand->comando, "rdf") == 0) {
			int* sockAux = malloc(sizeof(int));
			*sockAux = sockAccept;
			reduceArguments_t* reduceArguments = malloc(sizeof(reduceArguments_t));
			reduceArguments->isFinalReduce = true;
			reduceArguments->sockFS = sockFS;
			reduceArguments->sockJob = sockAux;
			pthread_create(&reduce_handler, NULL, reduce_conection_handler, reduceArguments);
		}



		/*
		int* sockAux = malloc(sizeof(int));
		//sockAux = &sockAccept;
		*sockAux = sockAccept;
		printf("Se obtuvo una conexión desde JOB: %s\n",
				inet_ntoa(client_sock.sin_addr));
		pthread_create(&map_handler, NULL, map_conection_handler, sockAux);
		*/




	} //while

	return 0;
}

void getConfig() {

	t_config* archivoConfig = config_create(
			"./NODO.config");

	PUERTO_FS = config_get_int_value(archivoConfig, "PUERTO_FS");
	IP_FS = config_get_string_value(archivoConfig, "IP_FS");
	ARCHIVO_BIN = config_get_string_value(archivoConfig, "ARCHIVO_BIN");
	DIR_TEMP = config_get_string_value(archivoConfig, "DIR_TEMP");
	NODO_NUEVO = config_get_string_value(archivoConfig, "NODO_NUEVO");
	IP_NODO = config_get_string_value(archivoConfig, "IP_NODO");
	PUERTO_NODO = config_get_int_value(archivoConfig, "PUERTO_NODO");

}

int initServer(int* sockFD) {

	Sockaddr_in my_sock;

	setValuesToSockaddr(&my_sock, PUERTO_NODO, IP_NODO);

	*sockFD = socket(AF_INET, SOCK_STREAM, 0);
	if (*sockFD == -1) {
		printf("Error al crear socket\n");
		printf("***********************\n");
		return -1;
	} else {
		printf("Socket creado correctamente...\n");
		printf("***********************\n");
	}
	//--Liberar puerto despues de cerrarlo
			int optval = 1;
				if (setsockopt(*sockFD, SOL_SOCKET, SO_REUSEADDR, &optval,sizeof optval) == -1){
					return -1;
				}
	if (bind(*sockFD, (struct sockaddr*) &my_sock, sizeof(my_sock)) == -1) {
		printf("Fallo al hacer el bind\n");
		//perror("");
		printf("***********************\n");
		return -1;
	} else {
		printf("Bind realizado con exito\n");
		printf("***********************\n");
	}

	if (listen(*sockFD, 100) == -1) {
		printf("Fallo en listen\n");
		printf("***********************\n");
		return -1;
	} else {
		printf("Socket escuchando conexiones\n");
		printf("***********************\n");
	}

	return 0;

}

void connectToFileSistem(int* sock) {

	Sockaddr_in server_addr;
	char* ipPuertoStr = string_new();
	mensaje_t* message = malloc(sizeof(mensaje_t));

	setValuesToSockaddr(&server_addr, PUERTO_FS, IP_FS);

	//server_addr.sin_family = AF_INET;
	//server_addr.sin_port = htons(PUERTO_FS);
	//inet_aton(IP_FS, &(server_addr.sin_addr)); //la IP hay que pasarla tomandola del .config pero tiraba error
	//bzero(&(server_addr.sin_zero), 8);

	*sock = socket(AF_INET, SOCK_STREAM, 0);
	if (*sock == -1) {
		printf("Fallo en creacion Socket para conectarse a FS\n");
		printf("***********************\n");
	} else {
		printf("Socket para conexion con FS creado correctamente\n");
		printf("***********************\n");
	}

	if (connect(*sock, (struct sockaddr*) &server_addr, sizeof(server_addr))
			== -1) {
		printf("Fallo en establecer conexión\n");
		printf("***********************\n");

	} else {
		printf("Conexión establecida con FileSystem por puerto %d\n",
				PUERTO_FS);
		printf("***********************\n");
	}

	//leer todo el archivo
	struct stat infoArchivo;
	stat(ARCHIVO_BIN, &infoArchivo);

	string_append_with_format(&ipPuertoStr, "nombre %s:%d %lu", IP_NODO, PUERTO_NODO, infoArchivo.st_size);//tamaño espacio datos leer tamaño del datos.bin con stat
	message->comando = ipPuertoStr;
	message->comandoSize = strlen(ipPuertoStr) + 1;
	message->data = NULL;
	message->dataSize = 0;

	enviar(*sock, message);

	free(ipPuertoStr);
	free(message);
	//send(*sock, message, sizeof(message), 0);

}

void setValuesToSockaddr(Sockaddr_in* addr, int port, char* ip) {

	addr->sin_family = AF_INET;
	addr->sin_port = htons(port);
	inet_aton(ip, &(addr->sin_addr));
	bzero(&(addr->sin_zero), 8);

}

void *fs_nodo_conection_handler(void* ptr) {

	int sockFD = *((int*) ptr);
	char** result;

	while (1) {

		mensaje_t* buffer_recv = malloc(sizeof(mensaje_t));
		mensaje_t* buffer_send = malloc(sizeof(mensaje_t));
		recibir(sockFD, buffer_recv); //Aca hay que hacer algo si devuelve 0
		result = string_split(buffer_recv->comando, " ");

		if (strcmp(result[0], "getBloque") == 0) {
			log_info(log_nodo,"SOLICITUD GET BLOQUE");
			char* bloque = malloc(TAMANIO_BLOQUE);
			int32_t length;
			bloque = getBloque(atoi(result[1]), &length);  //getBloque bloque
			buffer_send->comando = string_new();
			strcpy(buffer_send->comando,"respuesta");
			buffer_send->comandoSize = strlen(buffer_send->comando) + 1;
			buffer_send->dataSize = length;
			buffer_send->data = bloque;
			enviar(sockFD, buffer_send);
			free(bloque);
			free(buffer_send);
			free(buffer_recv);
		}

		if (strcmp(result[0], "setBloque") == 0) {   //setBloque bloque [datos]
			log_info(log_nodo,"SET GET BLOQUE");
			int numBLoque = atoi(result[1]);
			setBloque(numBLoque, buffer_recv->data, buffer_recv->dataSize);
			free(buffer_send);
			free(buffer_recv);
		}

		if (strcmp(result[0], "getFileContent") == 0) { //getFileContent nombre
			log_info(log_nodo,"SOLICITUD GET FILE CONTENT");
			t_fileContent* fileContent;
			fileContent = getFileContent(result[1]);
			buffer_send->comando = string_new();
			strcpy(buffer_send->comando,"respuesta");
			buffer_send->comandoSize = strlen(buffer_send->comando) + 1;
			buffer_send->dataSize = fileContent->size;
			buffer_send->data = fileContent->contenido;
			enviar(sockFD, buffer_send);
			free(buffer_send->data);
			free(buffer_send->comando);
			free(buffer_send);
			free(buffer_recv);
		}

		if (strcmp(result[0], "borrarBloque") == 0) {   //setBloque bloque [datos]
			log_info(log_nodo,"SOLICITUD BORRAR BLOQUE");
			int numBLoque = atoi(result[1]);
			borrarBloque(numBLoque);
			free(buffer_send);
			free(buffer_recv);
		}

	} //cierra while(1)

} //cierra thread

//char** generate_fields(int size) {
//		char** result = malloc(size*sizeof(char*));
//		int i = 0;
//		for(i ; i < size; i++) {
//			result[i] = malloc(COMANDO_LENGTH);
//		}
//
//		return result;
//}
//
//void freeMemory(char** result) {
//	int i = 0;
//	for (i ; i < 2; i++) {
//		free(result[i]);
//	}
//	free(result);
//}
#include "commons/temporal.h"
#include "commons/log.h"
#include <semaphore.h>
pthread_mutex_t mutexPermisosScript;

void *map_conection_handler(void* ptr) {  //int bloque  char* nombreArchTemp

	int sockFD = *((int*) ptr);

	while (1) {

		mensaje_t* buffer_recv = malloc(sizeof(mensaje_t));
		mensaje_t* buffer_send = malloc(sizeof(mensaje_t));
		char** result;
		int numBloque;

		//recibir peticion de mapping
		if(recibir(sockFD, buffer_recv) == DESCONECTADO){
			log_info(log_nodo,"HILO MP DESCONECTADO");
			pthread_exit(NULL);
		}
		log_info(log_nodo,"RECIBIDA SOLICITUD MAPPING");

		/*****modificar la ruta, no siempre es map.sh, AÑADIR EN EL COMANDO EL NOMBRE DEL SCRIPT***********/


		result = string_split(buffer_recv->comando, " "); //pos 0 = numBloque  , pos 1 = nombreArchivoTemporal

		char *nombreScript = string_new();
		string_append_with_format(&nombreScript, "%s/%s", DIR_TEMP, result[2]);
		pthread_mutex_lock(&mutexPermisosScript);
		if (access(nombreScript, F_OK) == -1)
		{
			FILE* scriptFD = fopen(nombreScript, "w+");
			fwrite( buffer_recv->data, buffer_recv->dataSize, 1,scriptFD);
			fclose(scriptFD);
			char *permisoEjecucionScript = string_new();
			string_append_with_format(&permisoEjecucionScript, "chmod +x %s", nombreScript);
			system(permisoEjecucionScript);
		}
		pthread_mutex_unlock(&mutexPermisosScript);
		numBloque = atoi(result[3]); //paso el string "numBloque" a tipo int , pq mapping recibe int NumBloque

		int static inc = 0;
		char *archivoTemporal1 = string_new();
		string_append_with_format(&archivoTemporal1, "%s/%s%d.map", DIR_TEMP, temporal_get_string_time(), inc++);

		char *archivoTemporal2 = string_new();
		string_append_with_format(&archivoTemporal2, "%s/%s", DIR_TEMP, result[1]);

		sem_wait(&sMaps);
		int mapResult = mapping(nombreScript, numBloque, 	archivoTemporal1, archivoTemporal2);
		sem_post(&sMaps);

		buffer_send->dataSize = 1;
		buffer_send->data = "\0";


		if (mapResult == 1) {
			buffer_send->comando = strdup("mapFileResponse 1");
			log_info(log_nodo,"MAPPING REALIZADO EXITOSAMENTE");
		} else {
			buffer_send->comando = strdup("mapFileResponse 0");
			log_info(log_nodo,"MAPPING FALLIDO");
		}

		buffer_send->comandoSize = strlen("mapFileResponse X") + 1;

		//MUTEX ???
		if(enviar(sockFD, buffer_send) == DESCONECTADO){
			log_info(log_nodo,"HILO MP DESCONECTADO");
			pthread_exit(NULL);
		}
		//enviar(sockFD, buffer_send);
		//MUTEX


		free(buffer_send->comando);
		if(buffer_recv->comando){
			free(buffer_recv->comando);
		}
		if(buffer_recv->data){
			free(buffer_recv->data);
		}

		free(buffer_recv);
		free(buffer_send);

	} //while

} //cierra thread

void *reduce_conection_handler(void* ptr) {

	reduceArguments_t reduceArguments = *((reduceArguments_t*) ptr);

	while (1) {

		mensaje_t* buffer_recv = malloc(sizeof(mensaje_t));
		mensaje_t* buffer_send = malloc(sizeof(mensaje_t));
		char** result;
		char** archivosParaReduce;
//		int i = 2;

		if(recibir(*reduceArguments.sockJob, buffer_recv) == DESCONECTADO){
			log_info(log_nodo,"HILO RD DESCONECTADO");
			pthread_exit(NULL);
		}
		log_info(log_nodo,"RECIBIDA SOLICITUD REDUCE");
		//execReduce archiTemp nombreScript cantArchNodoLocal a1 a2 aN  {ipNodoRemoto puertoNodoRemoto cantArchivosNodoRemoto a1 a2 aN}
		result = string_split(buffer_recv->comando, " ");
		/*
		archivosParaReduce = malloc(sizeof(result));
		for (; i < sizeof(result); i++) {
			archivosParaReduce = strcat(archivosParaReduce, result[i]);
		}
		*/

		//reduceFile archivoReducido.txt 1 /tmp/archivoParaReducir.txt
		/*
		int espacio = 1;
		int cantCaracteresDemas = strlen(result[0]) + strlen(result[1]) + strlen(result[1]) + espacio * 2;
		archivosParaReduce = string_substring_from(buffer_recv->comando, cantCaracteresDemas);
		*/
		archivosParaReduce = string_n_split(buffer_recv->comando, 4, " ");
		char* ipNodoFallido = string_new();

		char *nombreScript = string_new();
		string_append_with_format(&nombreScript, "%s/%s", DIR_TEMP, result[2]);

		if (access(nombreScript, F_OK) == -1)
		{
			FILE* scriptFD = fopen(nombreScript, "w+");
			fwrite( buffer_recv->data, buffer_recv->dataSize, 1,scriptFD);
			fclose(scriptFD);
			char *permisoEjecucionScript = string_new();
			string_append_with_format(&permisoEjecucionScript, "chmod +x %s", nombreScript);
			system(permisoEjecucionScript);
		}


		char** filePath = string_split(result[1], "/");
		int j = 0;
		while(filePath[j+1] != NULL) {
			j++;
		};

		int reduceResult = reduce(nombreScript, archivosParaReduce[3],filePath[j], ipNodoFallido);

		if (reduceResult == 0) {
			buffer_send->comando = strdup("reduceFileResponse 1");
			buffer_send->dataSize = 1;
			buffer_send->data = "\0";
			log_info(log_nodo,"REDUCE REALIZADO EXITOSAMENTE");
		} else {
			buffer_send->comando = strdup("reduceFileResponse 0");
			//TODO Obtener IPs de Nodos que fallaron
			buffer_send->dataSize = strlen(ipNodoFallido) + 1;
			buffer_send->data = ipNodoFallido;
			log_info(log_nodo,"REDUCE FALLIDO");
		}

		buffer_send->comandoSize = strlen("reduceFileResponse X") + 1;

		if(enviar(*reduceArguments.sockJob, buffer_send) == DESCONECTADO){
			log_info(log_nodo,"HILO RD DESCONECTADO");
			pthread_exit(NULL);
		}



		free(ipNodoFallido);
		free(buffer_send->comando);
		if(buffer_recv->comando){
			free(buffer_recv->comando);
		}
		if(buffer_recv->data){
			free(buffer_recv->data);
		}

		if(reduceArguments.isFinalReduce) {
			filePath[j] = string_from_format("/%s", filePath[j]);
			t_fileContent* reduceFinal = getFileContent(filePath[j]);
			buffer_send->comando = string_from_format("reduceFinal %s", filePath[j]);
			buffer_send->comandoSize = strlen(buffer_send->comando) + 1;
			buffer_send->data = reduceFinal->contenido;
			buffer_send->dataSize = reduceFinal->size;
			enviar(*reduceArguments.sockFS, buffer_send);
		}

		free(buffer_recv);
		free(buffer_send);

	}

}

