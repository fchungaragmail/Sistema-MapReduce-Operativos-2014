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

typedef enum
{
	TYPE_1 = "tipo1",
	TYPE_2 = "tipo2",
	TYPE_N = "tipoN"

}FS_TypesMessages;

#define 	HEAD_LENGHT 5

void listenToFS();
void listenToJobs();
void jobMessage(char *jobMessage);
//PLANIFICADOR
processFSMessage(char* message);
processJobMessage(char* message);

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
	while(1)
		{
			void *fsMessage;
		 	void *fsMessage = listenFS();

		 	//procesar data ...
		 	processFSMessage(fsMessage);
		}
}

void listenToJobs()
{
	while(1)
		{
			listenJobs();
		}
}

////////////////
//Planificador

processFSMessage(char* message)
{
	char *messageType = malloc(sizeof(HEAD_LENGHT));
	for (int i=0;i<HEAD_LENGHT;i++)
	{
		strcpy(messageType[i],message[i]);
	}

	switch (messageType) {
		case TYPE_1:
			//hacer tal cosa
			break;
		case TYPE_2:
			//hacer tal cosa
			break;
		case TYPE_N:
			//hacer tal cosa
			break;
		default:
			printf("ERROR !! FS Message no identificado !!");
			break;
	}

}

void jobMessage(char *jobMessage)
{
	//mensaje de cada job
}
