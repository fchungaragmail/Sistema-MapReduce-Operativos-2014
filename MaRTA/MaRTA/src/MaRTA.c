/*
 ============================================================================
 Name        : MaRTA.c
 Author      : Nicolas Buzzano
 Version     :
 Copyright   : Your copyright notice
 Description : Hello World in C, Ansi-style
 ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>

#include "FilesStatusCenter.h"
#include "ConexionCenter.h"

int main(void) {

	puts("MaRTA al ataque !!!");

	initFilesStatusCenter();
	initConexiones();

	pthread_t hiloFS;
	pthread_t hiloJobs;

	pthread_create(&hiloFS, NULL, listenToFS(),NULL);
	pthread_create(&hiloJobs, NULL, listenToJobs(),NULL);

	pthread_join(hiloFS,NULL);
	pthread_join(hiloJobs,NULL);

	closeServidores();

	//cada vez que un Job se conecte la direccion que MaRTA le pase al FilesStatusCenter
	//tambien debe quedarselo MaRTA

	return EXIT_SUCCESS;
}

void listenToFS()
{
	listenToFS();

}

void listenToJobs()
{
	listenJobs();
}
