/*
 * PlannerCenter.c
 *
 *  Created on: 23/5/2015
 *      Author: utnso
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include "FilesStatusCenter.h"
#include "Utilities.h"

// Constantes
#define MaxPedidosEnRed 5

//pedidoRealizado --> keys
#define K_PedidoRealizado_Nodo "PedidoRealizado_Nodo"
#define K_PedidoRealizado_Bloque "PedidoRealizado_Bloque"
#define K_PedidoRealizado_Path "PedidoRealizado_Path"
#define K_PedidoRealizado_TipoPedido "PedidoRealizado_TipoPedido" //map, reduce o pedido de tabla a fs
#define K_PedidoRealizado_PathArchTemporal "PedidoRealizado_PathArchTemporal"

// Variables Globales
int pedidosEnRed;
t_dictionary* pedidoRealizado;

// Funciones privadas
int obtenerIdParaComando(Message *recvMessage);
char* obtenerFilePath(Message *recvMessage ,TypesMessages type);
char* obtenerPathDeTemporaryPath(char* path);
bool* obtenerSoportaCombiner(Message *recvMessage);
t_dictionary* procesarFullDataResponse(recvMessage);
bool redDisponible();
Message* planificar(Message *recvMessage,TypesMessages type);

// Funciones publicas
void initPlannerCenter();
void processMessage(Message *recvMessage);

void initPlannerCenter()
{
	initFilesStatusCenter();
	pedidosEnRed=0;
	pedidoRealizado=malloc(sizeof(t_dictionary));
}

bool redDisponible()
{
	if(MaxPedidosEnRed<pedidosEnRed){return true;}
	return false;
}
void processMessage(Message *recvMessage)
{
	int comandoId = obtenerIdParaComando(recvMessage);
	switch (comandoId) {
		case K_NewConnection:
			printf("PlannerCenter : planificar NewConnection\n");
			addNewConnection(recvMessage->sockfd);
			free(recvMessage);
			printf("***************\n");
			break;
		case K_Job_NewFileToProcess:

			printf("PlannerCenter : planificar Job_NewFileToProcess:\n");

			char* filePath = obtenerFilePath(*recvMessage,K_Job_NewFileToProcess);
			bool soportaCombiner = obtenerSoportaCombiner(Message *recvMessage);
			addNewFileForProcess(filePath,soportaCombiner,recvMessage->sockfd);

			int fileSystemSocket = getFSSocket();
			mensaje_t *fsRequest = createFSRequestForPath(filePath);//CONTEMPLAR QUE PASA SI EL FS NO TIENE UN BLOQUE DISPONIBLE
			enviar(fileSystemSocket,fsRequest);//pido al FS la tabla de direcciones del archivo

			dictionary_put(pedidoRealizado,K_PedidoRealizado_TipoPedido,K_Pedido_FileData);
			dictionary_put(pedidoRealizado,K_PedidoRealizado_Path,filePath);

			free(filePath);
			free(recvMessage);
			free(fsRequest);

			printf("***************\n");
			break;

		case K_FS_FileFullData:

			printf("PlannerCenter : planificar FS_FileFullData\n");
			//FS me responde con el pedido de datos que le hice.

			char *path = obtenerFilePath(recvMessage,K_FS_FileFullData);
			t_dictionary *fullData = procesarFullDataResponse(recvMessage);
			addFileFullData(recvMessage->sockfd, path, fullData);//se completa filesToProcess y se crea un fileState

			Message *planifiedMessage = malloc(sizeof(Message));
			planifiedMessage = planificar(recvMessage,K_FS_FileFullData);
			enviar(planifiedMessage->sockfd,planifiedMessage->mensaje);

			free(path);
			free(fullData);
			free(planifiedMessage);

			printf("***************\n");
			break;

		case K_Job_MapResponse:
			printf("PlannerCenter : planificar Job_MapResponse\n");
			//*data:sizeRutaArchivoTemporal-rutaArchivoTemporal-sizeRespuesta-Respuesta
			char *temporaryPath = obtenerFilePath(recvMessage,K_Job_MapResponse);
			char *path = obtenerPathDeTemporaryPath(temporaryPath);
			bool requestResponse = obtenerRequestResponse(recvMessage,K_Job_MapResponse);

			//OBTENER DE MIS "PEDIDOS REALIZADOS" el #bloque y #nodo
			int nroNodo,nroBloque;

			blockState nuevoEstado;
			if(requestResponse==true){nuevoEstado=MAPPED;}
			if(requestResponse==false){nuevoEstado=TEMPORAL_ERROR;}//REPLANIFICAR

			changeFileBlockState(path,nroBloque,nuevoEstado,temporaryPath);
			addTemporaryFilePathToNodoData(nroNodo,temporaryPath);

			planificar();

			free(temporaryPath);
			free(path);
			free(recvMessage);

			printf("***************\n");
			break;
		case K_Job_ReduceResponse:
			printf("PlannerCenter : planificar Job_ReduceResponse");
			//*data:sizeRutaArchivoTemporal-rutaArchivoTemporal-sizeRespuesta-Respuesta
			char *temporaryPath = obtenerFilePath(recvMessage,K_Job_ReduceResponse);
			char *path = obtenerPathDeTemporaryPath(temporaryPath);
			bool requestResponse = obtenerRequestResponse(recvMessage,K_Job_ReduceResponse);

			//OBTENER DE MIS "PEDIDOS REALIZADOS" el #bloque y #nodo
			int nroNodo,nroBloque;

			blockState nuevoEstado;
			if(requestResponse==true){
				nuevoEstado=REDUCED;
				printf("archivo %s reducido con exito !!!",path);
			}
			if(requestResponse==false){nuevoEstado=TEMPORAL_ERROR;}//REPLANIFICAR

			changeFileBlockState(path,nroBloque,nuevoEstado,temporaryPath);
			addTemporaryFilePathToNodoData(nroNodo,temporaryPath);

			planificar();

			free(temporaryPath);
			free(path);
			free(recvMessage);

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
			printf("PlannerCenter: ERROR !! Comando no identificado !!\n");
			printf("***************\n");
			break;
	}
}

char* obtenerPathDeTemporaryPath(char* path)
{
	char* temporaryPath;
	return temporaryPath;
}
int obtenerIdParaComando(Message *recvMessage)
{
	if(strcmp(recvMessage->mensaje->comando,"newConnection")==0){return K_NewConnection;}
	if(strcmp(recvMessage->mensaje->comando,"archivoAProcesar")==0){return K_Job_NewFileToProcess;}
	if(strcmp(recvMessage->mensaje->comando,"mapFileResponse")==0){return K_Job_MapResponse;}
	if(strcmp(recvMessage->mensaje->comando,"reduceFileResponse")==0){return K_Job_ReduceResponse;}
	if(strcmp(recvMessage->mensaje->comando,"DataFileResponse")==0){return K_FS_FileFullData;}
	return K_Unidentified;
}

//Job_NewFileToProcess - Job_MapResponse - FS_FileFullData

char* obtenerFilePath(Message *recvMessage, TypesMessages type)
{

	if(type==K_Job_NewFileToProcess){
		//Segun protocolo recvMessage->mensaje->data sera
		//*data: sizeRutaDeArchivo-rutaDeArchivo-sizeSoportaCombiner-soportaCombiner
		//necesito obtener "rutaDeArchivo"
		char *filePath;
		return filePath;
	}

	if(type==K_FS_FileFullData){
		//Tengo que ver como me pasa Juan el FullData
		//La estructura hasta el momento es:
		//-Data:sizeRutaDelArchivo-rutaDelArchivo-sizeCantidadDeBloques-cantidadDeBloques-sizeEstructura-estructura
		//Debo obtener "rutaDelArchivo"
		char *filePath;
		return filePath;
	}

	if(type==K_Job_MapResponse || type==K_Job_ReduceResponse ){
		//Segun protocolo recvMessage->mensaje->data sera
		//*data:sizeRutaArchivoTemporal-rutaArchivoTemporal-sizeRespuesta-Respuesta
		//necesito obtener "rutaArchivoTemporal"
		char *temporaryFilePath;
		return temporaryFilePath;
	}

	printf("PlannerCenter : ERROR se retorna nullFile ");//no deberia nunca llegar aca
	char *nullFile;
	return nullFile;
}

bool* obtenerSoportaCombiner(Message *recvMessage)
{	//Segun protocolo recvMessage->mensaje->data sera
	//*data: sizeRutaDeArchivo-rutaDeArchivo-sizeSoportaCombiner-soportaCombiner
	//necesito obtener "soportaCombiner"
	bool* soportaCombiner;
	return soportaCombiner;
}

//FS_FileFullData

t_dictionary* procesarFullDataResponse(Message *recvMessage)
{
	//debo obtener recvMessage->data que debe ser la matriz con las ubicaciones de
	//las particiones del archivo a procesar (#Nodo y #Bloque)
	t_dictionary *fullData;
	return fullData;
}

//Job_MapResponse

bool obtenerRequestResponse(Message *recvMessage,TypesMessages type)
{
	if(type==K_Job_MapResponse || type==K_Job_ReduceResponse){
		//segun protocolo el data sera
		//*data:sizeRutaArchivoTemporal-rutaArchivoTemporal-sizeRespuesta-Respuesta
		//debo obtener "Respuesta"
		bool requestResponse;
		return requestResponse;
	}

	//no deberia pasar nunca por aca
	printf("Planner Center : ERROR obtenerRequestResponse no pudo leer el valor");
	bool error = false;
	return error;
}

//Pedidos a FS

mensaje_t* createFSRequestForPath(char *filePath)
{
	//crear un mensaje_t con el comando "DataFile" y el data "filePath"
	mensaje_t *fsRequest = malloc(sizeof(mensaje_t));
	return fsRequest;
}

//Planificacion
Message* planificar(Message *recvMessage,TypesMessages type)
{
	bool redDisponible = redDisponible();//LA RED LA CONTROLA MARTA !! SACAR DE ACA ESTO
	// !!!!!!!!!!!!!!!!!!!!!!

	char *path = dictionary_get(pedidoRealizado,K_PedidoRealizado_Path);
	t_dictionary *fileState= malloc(sizeof(t_dictionary));
	t_dictionary *copias = malloc(sizeof(t_dictionary));
	fileState = getFileStateForPath(path);
	copias = obtenerCopiasParaArchivo(recvMessage->sockfd,path);

	//con fileState controlar cuantos archivos me quedan por procesar
	//
	if(type==K_FS_FileFullData){}
	if(type==K_Job_MapResponse){}
	if(type==K_Job_ReduceResponse){}

	//obtener de "copias" el nroDeNodo --> luego obtener "getCantidadDeOperacionesEnProcesoEnNodo"
	//ver cual tiene menos --> al que tiene menos se enviara el request
	//incrementar contador de nodo "incrementarOperacionesEnProcesoEnNodo"
	//actualizar fileState

	enviar(mensajePlanificado->sockfd,mensajePlanificado->mensaje);

}

// Idea de planificaion : se hara map y reduce de los bloques en orden ascendente.
// Se procesara todo todos los archivos en paralelo o los mas que se puedan, sino lo habria un Job activo.
// Hago map de todos los bloques de un archivo y ahi lanzo el reduce.

// Habra un MAXIMO de envios por la red, hasta que no responda un envio, nose hara otro(Productor-Consumidor)
// Si vuelve con error, entonces se rePlanifica y se vuelve a mandar el mismo pero a otro NODO.
// Si NO hay otro NODO disponible entonces, se informa que el proceso del archivo quedo incompleto--> se envia a la lista de incompletos.
// Si la operacion enviada falla porque se cayo el Nodo o simplemente fallo entonces se RE-PLANIFICA, si se cae el Job NO hay nada mas que hacer.
