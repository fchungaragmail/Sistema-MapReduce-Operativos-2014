/*
 * protocolo.c
 *
 *  Created on: 23/5/2015
 *      Author: utnso
 */

#include <protocolo.h>


int recibir(int socket, mensaje_t* mensaje);
int enviar(int socket, mensaje_t* mensaje);
int sendall(int s, void *buf, int32_t len);

int recibir(int socket, mensaje_t* mensaje){

	int estado = 0;

	estado = recv(socket, &(mensaje->comandoSize), sizeof(int16_t),MSG_WAITALL);
	if (estado <= 0) return DESCONECTADO;

	mensaje->comando = malloc(mensaje->comandoSize);
	if (mensaje->comandoSize != 0)
		recv(socket, mensaje->comando, mensaje->comandoSize,MSG_WAITALL);

	mensaje->dataSize = 0;
	recv(socket, &(mensaje->dataSize), sizeof(int32_t),MSG_WAITALL);
	mensaje->data = malloc(mensaje->dataSize);
	if (mensaje->dataSize != 0)
		recv(socket, mensaje->data, mensaje->dataSize,MSG_WAITALL);

	return CONECTADO;
}

int enviar(int socket, mensaje_t* mensaje)
{
	if (sendall(socket, &(mensaje->comandoSize), sizeof(int16_t)) < 0)
		return -1;
	if (sendall(socket, mensaje->comando, mensaje->comandoSize) < 0)
		return -1;
	if (sendall(socket, &(mensaje->dataSize), sizeof(int32_t)) < 0)
		return -1;
	if (sendall(socket, mensaje->data, mensaje->dataSize) < 0)
		return -1;

	return EXIT_SUCCESS;
}

int sendall(int s, void* buf, int32_t len)
{
    int total = 0;        // how many bytes we've sent
    int bytesleft = len; // how many we have left to send
    int n = 0;

    while(total < len) {
        n = send(s, buf+total, bytesleft, 0);
        if (n == -1) { break; }
        total += n;
        bytesleft -= n;
    }

    //*len = total; Devolveria el total de enviados, no nos interesa y nos rompe
    // la retrocompatibilidad

    return n==-1?-1:0; // return -1 on failure, 0 on success
}
