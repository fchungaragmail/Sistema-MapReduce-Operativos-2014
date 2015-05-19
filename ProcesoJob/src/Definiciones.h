/*
 * Definiciones.h
 *
 *  Created on: 18/5/2015
 *      Author: utnso
 */

#ifndef SRC_DEFINICIONES_H_
#define SRC_DEFINICIONES_H_

#include <netinet/in.h>

#define EXIT_ERROR -1
#define EXIT_OK 1

typedef struct {
	short int sin_family;
	unsigned short int sin_port;
	struct in_addr sin_addr;
	unsigned char sin_zero[8];
} Sockaddr_in ;


#endif /* SRC_DEFINICIONES_H_ */
