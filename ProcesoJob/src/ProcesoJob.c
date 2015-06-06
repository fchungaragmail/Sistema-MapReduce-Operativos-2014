#include "ProcesoJob.h"
#include <pthread.h>
#include <commons/collections/list.h>
#include <commons/error.h>
#include <arpa/inet.h>
#include <commons/config.h>
#include <commons/string.h>

int socketMartaFd = -1;
t_list* listaHilos;
pthread_t threadPedidosMartaHandler;
pthread_t threadProcesarHilos;
t_config* configuracion;

void* pedidosMartaHandler(void* arg) {

	listaHilos = list_create();
	HacerPedidoMarta();

	while (TRUE) {
		mensaje_t* mensajeMarta;
		recibir(socketMartaFd, mensajeMarta);

		char** comandoStr = string_split(mensajeMarta, " ");

		if (strncmp(comandoStr[0], "mapFile", 7) == 0) {

			/// TO DO , usar enums o defines
			////                    0        1       2     3                    4
			//// Ej del comando: mapFile  127.0.0.1 9999 12345 /user/juan/datos/temperatura2012.txt/-23:43:45:2345
			Sockaddr_in their_addr;

			their_addr.sin_family = AF_INET;
			their_addr.sin_port = htons(comandoStr[1]);
			inet_aton(comandoStr[2], &(their_addr.sin_addr));
			memset(&(their_addr.sin_zero), '\o', 8);

			HiloJob* hiloJob = malloc(sizeof(HiloJob));
			hiloJob->direccionNodo = their_addr;
			hiloJob->threadhilo = NULL;
			hiloJob->socketFd = -1;
			hiloJob->nroBloque = atoi(comandoStr[3]);
			hiloJob->nombreArchivo = string_duplicate(comandoStr[4]);

			list_add(listaHilos, (void*) hiloJob);
			CrearHiloMapper(hiloJob);

		} else if (strncmp(mensajeMarta->comando, "reduceFile", 10) == 0) {

		}

		free(comandoStr);

	}
	list_destroy(listaHilos);
}

void IniciarConfiguracion() {

	configuracion = config_create("config.ini");
	if (!configuracion) {
		error_show("Error leyendo archivo!\n");
		Terminar(EXIT_ERROR);
	}
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

int main(int argc, char* argv[]) {

	IniciarConfiguracion();
	IniciarConexionMarta();
	Terminar(EXIT_OK);
}
