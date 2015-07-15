#ifndef LECTURAYESCRITURA_H_
#define LECTURAYESCRITURA_H_

#define TAM_HEADER 8//ID|SIZE
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <stdint.h>

typedef struct{
	int32_t id;
	int32_t size;
	char* data;
}t_header;

//Prototypes
int32_t enviar_paquete(int32_t enlace,t_header header_a_enviar);
int32_t recibirPaquete(int32_t enlace, t_header* header_a_recibir);
int32_t escribir_socket (int32_t nuevo_socket, char *datos, size_t longitud);
int32_t leer_socket (int32_t nuevo_socket, char *buffer, size_t size);

#endif /* LECTURAYESCRITURA_H_ */
