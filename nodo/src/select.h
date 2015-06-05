/*
 * select.h
 *
 *  Created on: 1/6/2015
 *      Author: utnso
 */

#ifndef SELECT_H_
#define SELECT_H_

#include "conection_threads.h"


void listen_conections() {

	pthread_t fs_nodo_handler;
	pthread_t map_handler;
	pthread_t reduce_handler;
	fd_set readSocks;
	socklen_t length;
	struct sockaddr_in client_sock;
	int sockServerFD, maxSockFD, acceptSockFD, selectReturnFD;

	sockServerFD = initServer();

	FD_ZERO(&readSocks);
	FD_SET(sockServerFD, &readSocks);

	maxSockFD = sockServerFD + 50;

	while(1) {

		selectReturnFD = select(maxSockFD + 1, &readSocks, NULL, NULL, NULL);
		if (selectReturnFD < 0) {
			printf("Error en select\n");
		}
		length = sizeof(client_sock);
		if (FD_ISSET(sockServerFD, &readSocks)) {
			acceptSockFD = accept(sockServerFD, (struct sockaddr*) &client_sock, &length);
		}
		if (acceptSockFD < 0 ) {
			printf("Error de Accept\n");
		}
		FD_SET(acceptSockFD, &readSocks);
		if(maxSockFD < acceptSockFD) {
			maxSockFD = acceptSockFD;
		}
	}




}

#endif /* SELECT_H_ */
