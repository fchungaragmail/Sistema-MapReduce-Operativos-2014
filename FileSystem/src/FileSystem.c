/*
 ============================================================================
 Name        : FileSystem.c
 Author      : The ByteLess
 Version     :
 Copyright   : 
 Description : Hello World in C, Ansi-style
 ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include "estructuras.h"
#include "consola.h"


int main(void) {
	pthread_t tConsola;

	pthread_create(&tConsola, NULL, execConsola, NULL);

	pthread_join(tConsola,NULL);
	return EXIT_SUCCESS;
}

