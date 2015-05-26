/*
 * PlannerCenter.c
 *
 *  Created on: 23/5/2015
 *      Author: utnso
 */

#include <stdio.h>
#include <stdlib.h>

#include "FilesStatusCenter.h"
#include "Utilities.h"

void processMessage(Message *recvMessage)
{
	switch ((*recvMessage).head) {
		case K_NewConnection:
			printf("PlannerCenter : planificar NewConnection\n");
			addNewConnection((*recvMessage).sockfd);
			printf("***************\n");
			break;
		case K_Job_NewFileToProcess:
			printf("PlannerCenter : planificar Job_NewFileToProcess:\n");
			//addNewFileForProcess(char *file_Path,_Bool *soportaCombiner,int jobSocket);
			//mandar mensaje a FS para que me responda con las direcciones de los nodos
			printf("***************\n");
			break;
		case K_FS_FileFullData:
			printf("PlannerCenter : planificar FS_FileFullData\n");
			//addFileFullData(int sckt, char* path, t_dictionary *fullData);
			//FS responde con la info de los bloques y nodos donde esta el archivo a procesar
			//** A cont. debo PLANIFICAR!!! y empezar a decirle que arch. mapp/reduce al Job
			printf("***************\n");
			break;
		case K_Job_ChangeBlockState:
			printf("PlannerCenter : planificar Job_ChangeBlockState\n");
			//changeFileBlockState(int sckt,int nroNodo,int nroBloque,status nuevoEstado);
			//Job responde con la respuesta de la operacion que se le mando a hacer
			//y entonces cambio el estado del bloque
			//sigo con el envio siguiente

			//*Al concluir la rutina de Reduce le solicitará al FileSystem que copie
			//el archivo de resultado al MDFS y le notificará al Job que la operación
			//fue concluida ---> CONTROLAR SI ES EL ULTIMO BLOQUE !!!!
			printf("***************\n");
			break;
		case K_Job_OperationFailure:
			printf("PlannerCenter : planificar Job_OperationFailure\n");
			//operacion enviada a Job fallo
			//debo RE-PLANIFICAR !!
			//CHECKEAR SI YA FALLARON TODAS LAS COPIAS DISPONIBLES
			//NO sigo con el envio siguiente y vuelvo a mandarlo, pero a otro NODO
			printf("***************\n");
			break;
		default:
			printf("PlannerCenter: ERROR !! message no identificado !!\n");
			printf("***************\n");
			break;
	}

}

// Idea de planificaion : se hara map y reduce de los bloques en orden ascendente.
// Se procesara todo un archivo y luego se pasara al siguiente.
// Cuando me llega el OK de un map, entonces ahi encolo el pedido de Reduce
// Se hara una lista en la cual, se encolaran los pedidos.
// Habra un MAXIMO de envios por la red, hasta que no responda un envio, nose hara otro(Productor-Consumidor)
// Si vuelve con error, entonces se rePlanifica y se vuelve a mandar el mismo pero a otro NODO.
// Si no hay otro NODO disponible entonces, se informa que el proceso del archivo quedo incompleto.
