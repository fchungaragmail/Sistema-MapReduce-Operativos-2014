/*
 * nodoCliente.c
 *
 *  Created on: 21/5/2015
 *      Author: utnso
 */

#include <string.h>
#include <stdio.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <pthread.h>


#define PORT 8000

int main() {


	int socketCli;
	struct sockaddr_in dir;
	socklen_t dirsize;
	char bufferRecive[200];

	dir.sin_family = AF_INET;
	dir.sin_port = htons(PORT);
	dir.sin_addr.s_addr = inet_addr("127.0.0.1");

	dirsize = sizeof(struct sockaddr);

	socketCli = socket(AF_INET, SOCK_STREAM, 0);

	connect(socketCli, &dir, &dirsize);

	recv(socketCli, bufferRecive, 140, 0);




	return 0;
}

