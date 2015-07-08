/*
 * nodo_new.c
 *
 *  Created on: 11/6/2015
 *      Author: utnso
 */

#include "nodo_new.h"

//Global Variables
int PUERTO_FS;
char* IP_FS;
char* ARCHIVO_BIN;
char* DIR_TEMP;
char* NODO_NUEVO;
char* IP_NODO;
int PUERTO_NODO;

//Main Programm
int main() {

	int sockFD; //socket nodo servidor
	int sockAccept; //socket nodo servidor acepta conexiones
	int* sockFS = malloc(sizeof(int)); //socket nodo cliente para conectarse con FS
	int* sockForThread = malloc(sizeof(int)); //sockFS que paso por parametro al thread que queda hablando con FS
	socklen_t size;
	Sockaddr_in client_sock; //sockaddr que se usa en accept de nodo servidor
	pthread_t nodo_handler;
	pthread_t fs_handler;
	pthread_t map_handler;
	pthread_t reduce_handler;
	mensaje_t* buffer_shakehand = malloc(sizeof(mensaje_t));
	mensaje_t* buffer_send = malloc(sizeof(mensaje_t));

	getConfig();

	connectToFileSistem(sockFS);
	*sockForThread = sockFS;
	pthread_create(&fs_handler, NULL, fs_nodo_conection_handler, sockForThread);
	pthread_join(fs_handler, NULL);

	int value = initServer(&sockFD);
	if (value == -1) {
		printf("No se pudo crear socket que atiende conexiones");
		printf("***********************\n");
		return -1;
	}

	size = sizeof(struct sockaddr_in);

	while (1) {
		sockAccept = accept(sockFD, (struct sockaddr*) &client_sock, &size);
		if (sockAccept != 1) {
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
			pthread_create(&reduce_handler, NULL, reduce_conection_handler,
					sockAux);
		}

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

	if (bind(*sockFD, (struct sockaddr*) &my_sock, sizeof(my_sock)) == -1) {
		printf("Fallo al hacer el bind\n");
		printf("***********************\n");
		return -1;
	} else {
		printf("Bind realizado con exito\n");
		printf("***********************\n");
	}

	if (listen(*sockFD, 5) == -1) {
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

	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(PUERTO_FS);
	inet_aton("127.0.0.1", &(server_addr.sin_addr)); //la IP hay que pasarla tomandola del .config pero tiraba error
	bzero(&(server_addr.sin_zero), 8);

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


	string_append_with_format(&ipPuertoStr, "nombre %s:%d %d", IP_NODO, PUERTO_NODO, 1000000000);
	message->comando = ipPuertoStr;
	message->comandoSize = strlen(ipPuertoStr) + 1;
	message->data = NULL;
	message->dataSize = NULL;

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
		recibir(sockFD, buffer_recv);
		result = string_split(buffer_recv->comando, " ");

		if (strcmp(result[0], "getBloque") == 0) {

			char* bloque = malloc(TAMANIO_BLOQUE);
			bloque = getBloque(atoi(result[1]));  //getBloque bloque
			buffer_send->comandoSize = 1;
			buffer_send->comando = "\0";
			buffer_send->dataSize = sizeof(int32_t);
			buffer_send->data = bloque;
			enviar(sockFD, buffer_send);
			free(bloque);
			free(buffer_send);
			free(buffer_recv);
		}

		if (strcmp(result[0], "setBloque") == 0) {   //setBloque bloque [datos]
			setBloque(atoi(result[1]), buffer_recv->data);
			free(buffer_send);
			free(buffer_recv);
		}

		if (strcmp(result[0], "getFileContent") == 0) { //getFileContent nombre
			t_fileContent* fileContent;
			fileContent = getFileContent(result[1]);
			buffer_send->comandoSize = 1;
			buffer_send->comando = "\0";
			buffer_send->dataSize = fileContent->size;
			buffer_send->data = fileContent->contenido;
			enviar(sockFD, buffer_send);
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

void *map_conection_handler(void* ptr) {  //int bloque  char* nombreArchTemp

	int sockFD = *((int*) ptr);

	while (1) {

		mensaje_t* buffer_recv = malloc(sizeof(mensaje_t));
		mensaje_t* buffer_send = malloc(sizeof(mensaje_t));
		char** result;
		int numBloque;

		recibir(sockFD, buffer_recv);

		int scriptFD = open("/tmp/map.sh", O_CREAT, 0777);
		write(scriptFD, buffer_recv->data, sizeof(buffer_recv->data));

		result = string_split(buffer_recv->comando, " "); //pos 0 = numBloque  , pos 1 = nombreArchivoTemporal

		numBloque = atoi(result[0]); //paso el string "numBloque" a tipo int , pq mapping recibe int NumBloque

		int mapResult = mapping("./tmp/map.sh", numBloque, result[1]);
		if (mapResult == 0) {
			//informar a job que termino bien, send normal? o enviar y se pasa por data o comando?
		} else {
			//informar a job que no termino bien, misma pregunta que arriba
		}

		free(buffer_recv);
		free(buffer_send);

	} //while

} //cierra thread

void *reduce_conection_handler(void* ptr) {

	int sockFD = *((int*) ptr);

	while (1) {

		mensaje_t* buffer_recv = malloc(sizeof(mensaje_t));
		mensaje_t* buffer_send = malloc(sizeof(mensaje_t));
		char** result;
		char* archivosParaReduce;
		int i = 2;

		recibir(sockFD, buffer_recv);

		//execReduce archiTemp cantArchNodoLocal a1 a2 aN ipNodoRemoto puertoNodoRemoto a1 a2 aN
		result = string_split(buffer_recv->comando, " ");

		archivosParaReduce = malloc(sizeof(result));
		for (; i < sizeof(result); i++) {
			archivosParaReduce = strcat(archivosParaReduce, result[i]);
		}

		int reduceResult = reducing(buffer_recv->data, archivosParaReduce,
				result[1]);
		if (reduceResult == 0) {
			buffer_send->comandoSize = sizeof(reduceResult);
			buffer_send->comando = reduceResult;
			buffer_send->dataSize = 1;
			buffer_send->data = "\0";
		}
	}

}

