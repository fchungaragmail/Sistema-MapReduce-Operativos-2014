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
			//Al enviar un pedido tambien uso "incrementarOperacionesEnProcesoEnNodo(int nroNodo);"
			printf("***************\n");
			break;
		case K_Job_ChangeBlockState:
			printf("PlannerCenter : planificar Job_ChangeBlockState\n");
			//changeFileBlockState(int sckt,int nroNodo,int nroBloque,status nuevoEstado);
			//addTemporaryFilePathToNodoData(int nroNodo,char* filePath);
			//Job responde con la respuesta de la operacion que se le mando a hacer
			//y entonces cambio el estado del bloque y agrego el pathTemporal a nodosData
			//sigo con el envio siguiente

			//*Al concluir la rutina de Reduce le solicitará al FileSystem que copie
			//el archivo de resultado al MDFS y le notificará al Job que la operación
			//fue concluida ---> CONTROLAR SI ES EL ULTIMO BLOQUE !!!!
			printf("***************\n");
			break;
		case K_Job_OperationFailure:
			printf("PlannerCenter : planificar Job_OperationFailure\n");
			//operacion enviada a Job fallo
			//DEBO DAR DE BAJA LA DIRECCION QUE FALLO
			//debo RE-PLANIFICAR !!
			//CHECKEAR SI YA FALLARON TODAS LAS COPIAS DISPONIBLES
			//NO sigo con el envio siguiente y vuelvo a mandarlo, pero a otro NODO
			//Cuando no quedan mas copias, se saca el archivo de la lista "filesOrdersToProcess"
			//y se mete el fileState+path en la lista "filesOrdersFailed"
			printf("***************\n");
			break;
		default:
			printf("PlannerCenter: ERROR !! message no identificado !!\n");
			printf("***************\n");
			break;
	}

}

// Idea de planificaion : se hara map y reduce de los bloques en orden ascendente.
// Se procesara todo todos los archivos en paralelo o los mas que se puedan, sino lo habria un Job activo.
// Hago map de todos los bloques de un archivo y ahi lanzo el reduce.

// Habra un MAXIMO de envios por la red, hasta que no responda un envio, nose hara otro(Productor-Consumidor)
// Si vuelve con error, entonces se rePlanifica y se vuelve a mandar el mismo pero a otro NODO.
// Si NO hay otro NODO disponible entonces, se informa que el proceso del archivo quedo incompleto--> se envia a la lista de incompletos.
// Si la operacion enviada falla porque se cayo el Nodo o simplemente fallo entonces se RE-PLANIFICA, si se cae el Job NO hay nada mas que hacer.
