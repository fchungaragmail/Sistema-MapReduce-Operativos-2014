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
#include <commons/temporal.h>
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
bool* obtenerSoportaCombiner(Message *recvMessage);
t_dictionary* procesarFullDataResponse(recvMessage);
bool redDisponible();
Message* planificar(Message *recvMessage,TypesMessages type);
void actualizarPedidoRealizado(int bloque, int nodo, char* path, char *pathTemporal, TypesPedidosRealizado tipo );
Message* obtenerProximoPedido(Message *recvMessage);

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
			mensaje_t *fsRequest = createFSRequestForPath(filePath);
			enviar(fileSystemSocket,fsRequest);//pido al FS la tabla de direcciones del archivo

			dictionary_put(pedidoRealizado,K_PedidoRealizado_TipoPedido,K_Pedido_FileData);
			dictionary_put(pedidoRealizado,K_PedidoRealizado_Path,filePath);

			free(filePath);
			free(fsRequest);

			printf("***************\n");
			break;

		case K_FS_FileFullData:

			printf("PlannerCenter : planificar FS_FileFullData\n");
			//FS me responde con el pedido de datos que le hice.

			//obtener del recvMessage el valor bool "Respuesta"
			//si es false entonces no esta disponible el archivo
			//QUE HACER ???????

			//ACTUALIZO TABLAS
			char *path = obtenerFilePath(recvMessage,K_FS_FileFullData);
			t_dictionary *fullData = procesarFullDataResponse(recvMessage);
			addFileFullData(recvMessage->sockfd, path, fullData);//se completa filesToProcess
																//y se crea un fileState
			//PLANIFICO CON INFO ACTUALIZADA
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

			Message *planifiedMessage = malloc(sizeof(Message));
			planificar(recvMessage,K_Job_MapResponse);
			enviar(planifiedMessage->sockfd,planifiedMessage->mensaje);

			free(planifiedMessage);

			printf("***************\n");
			break;
		case K_Job_ReduceResponse:
			printf("PlannerCenter : planificar Job_ReduceResponse");

			Message *planifiedMessage = malloc(sizeof(Message));
			planificar(recvMessage,K_Job_MapResponse);
			//ENVIAR ???
			//enviar(planifiedMessage->sockfd,planifiedMessage->mensaje);

			free(planifiedMessage);

			printf("***************\n");
			break;
		default:

			printf("PlannerCenter: ERROR !! Comando no identificado !!\n");
			printf("***************\n");

			break;
	}
	free(recvMessage);
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

	//con fileState controlar cuantos archivos me quedan por procesar

	if(type==K_FS_FileFullData){

		Message* sendMessage = malloc(sizeof(Message));
		sendMessage = obtenerProximoPedido(recvMessage);
		return sendMessage;
	}

	if(type==K_Job_MapResponse){
		//path//fileState//nroDeBloques//nroDeCopias//copias

		//*data:sizeRutaArchivoTemporal-rutaArchivoTemporal-sizeRespuesta-Respuesta

		char *temporaryPath = obtenerFilePath(recvMessage,K_Job_MapResponse);
		bool requestResponse = obtenerRequestResponse(recvMessage,K_Job_MapResponse);

		int nroNodo =  dictionary_get(pedidoRealizado,K_PedidoRealizado_Nodo);
		int nroBloque =  dictionary_get(pedidoRealizado,K_PedidoRealizado_Bloque);

		if(requestResponse==true){

			//actualizar tablas y realizar proximo envio

			t_dictionary *fileState = malloc(sizeof(t_dictionary));
			fileState = getFileStateForPath(path);

			int size = dictionary_get(fileState,K_FileState_size);
			t_dictionary *blockStatesArray[size];
			int i;
			for(i=0;i<size;i++){blockStatesArray[i]=malloc(sizeof(t_dictionary*));}
			blockStatesArray = dictionary_get(fileState,K_FileState_arrayOfBlocksStates);

			t_dictionary *blockState = malloc(sizeof(t_dictionary));
			blockState = blockStatesArray[nroBloque];

			//Actualizo blockState
			dictionary_put(blockState,K_BlockState_state,MAPPED);
			//Actualizo nodoState
			decrementarOperacionesEnProcesoEnNodo(nroNodo);
			addTemporaryFilePathToNodoData(nroNodo,temporaryPath);

			//OBTENER PROXIMO PEDIDO !!!
			Message* sendMessage = malloc(sizeof(Message));
			sendMessage = obtenerProximoPedido(recvMessage);
			return sendMessage;
		}

		if(requestResponse==false){//REPLANIFICAR

			//actualizar tablas y reenviar si existen copias

			//PONER -1 EN COPIAS
			darDeBajaCopiaEnBloqueYNodo(temporaryPath,recvMessage->sockfd,nroBloque,nroNodo);
			decrementarOperacionesEnProcesoEnNodo(nroNodo);

			//OBTENER PROXIMO PEDIDO !!!
			Message* sendMessage = malloc(sizeof(Message));
			sendMessage = obtenerProximoPedido(recvMessage);
			return sendMessage;
		}
	}

	if(type==K_Job_ReduceResponse){
		//path//fileState//nroDeBloques//nroDeCopias//copias
		//*data:sizeRutaArchivoTemporal-rutaArchivoTemporal-sizeRespuesta-Respuesta
		char *temporaryPath = obtenerFilePath(recvMessage,K_Job_ReduceResponse);
		bool requestResponse = obtenerRequestResponse(recvMessage,K_Job_ReduceResponse);

		int nroNodo =  dictionary_get(pedidoRealizado,K_PedidoRealizado_Nodo);
		int nroBloque =  dictionary_get(pedidoRealizado,K_PedidoRealizado_Bloque);

		if(requestResponse==true){

			decrementarOperacionesEnProcesoEnNodo(nroNodo);
			addTemporaryFilePathToNodoData(nroNodo,temporaryPath);
			printf("archivo %s reducido con exito !!!",path);
			free(temporaryPath);
			free(path);
		}

		if(requestResponse==false){
			//re planificar
			// ver que hacer si el reduce falla !!
			free(temporaryPath);
			free(path);
		}
	}
}

Message* obtenerProximoPedido(Message *recvMessage)
{
	char *path = dictionary_get(pedidoRealizado,K_PedidoRealizado_Path);
	t_dictionary *fileState= malloc(sizeof(t_dictionary));
	fileState = getFileStateForPath(path);

	int nroDeCopias = obtenerNumeroDeCopiasParaArchivo(recvMessage->sockfd,path);
	int nroDeBloques = obtenerNumeroDeBloquesParaArchivo(recvMessage->sockfd,path);
	t_dictionary copias[nroDeCopias];

	int k;
	for(k=0;k<nroDeCopias;k++){ copias[k] = malloc(sizeof(t_dictionary*)); }

	copias = obtenerCopiasParaBloqueDeArchivo(recvMessage->sockfd,path);


	int size_fileState= dictionary_get(fileState,K_FileState_size);
	t_dictionary *blockStateArray[size_fileState];
	int i;
	for(i=0;i<size_fileState;i++){blockStateArray[i]=malloc(sizeof(t_dictionary));}
	blockStateArray = dictionary_get(fileState,K_FileState_arrayOfBlocksStates);

	for(i=0;i<size_fileState;i++){

		t_dictionary *blockState = malloc(sizeof(t_dictionary));
		blockState = blockStateArray[i];
		int statusBlock = dictionary_get(blockState,K_BlockState_state);

		if(statusBlock == MAPPED && i==(size_fileState-1)){
			//estan todos mappeados , inculuidos el ultimo
			//Actualizar estados y luego
			//HACER REDUCE !!! --> usar fileState para ver las ubicaciones
			//hacer reduce en el nodo que contenga mas archivos mappeados


		};

		if(statusBlock == UNINITIALIZED){ //este bloque no esta procesado


			t_dictionary *copiaConMenosCarga = malloc(sizeof(t_dictionary));
			int j;
			for(j=0;i<(nroDeCopias-1);j++){//obtengo nodo con menos carga de operaciones

				t_dictionary *copia = malloc(sizeof(t_dictionary));
				t_dictionary *copiaSiguiente = malloc(sizeof(t_dictionary));

				copia = copias[j];
				copiaSiguiente = copias[j+1];

				int nroNodoCopia = dictionary_get(copia,"nroDeNodo");
				int nroNodoCopiaSiguiente = dictionary_get(copia,"nroDeNodo");


				if(nroNodoCopia == -1 && nroNodoCopiaSiguiente == -1){
					//ninguno de los nodos esta disponible

					t_dictionary *dic = malloc(sizeof(t_dictionary));
					dictionary_put(dic,"nroDeNodo",-1);
					copiaConMenosCarga = dic;

				}else if(nroNodoCopia != -1 && nroNodoCopiaSiguiente == -1){

					copiaConMenosCarga = nroNodoCopia;

				}else if(nroNodoCopia == -1 && nroNodoCopiaSiguiente != -1){

					copiaConMenosCarga = nroNodoCopiaSiguiente;
				}else{

					int opsEnNodoCopia = getCantidadDeOperacionesEnProcesoEnNodo(nroNodoCopia);
					int opsEnNodoCopiaSig = getCantidadDeOperacionesEnProcesoEnNodo(nroNodoCopiaSiguiente);

					if(opsEnNodoCopia < opsEnNodoCopia){
						copiaConMenosCarga = copia;
					}else{
						copiaConMenosCarga = copiaSiguiente;
					}
				}

				free(copia);
				free(copiaSiguiente);
			}
			//ya tengo el nodo con menos carga

			int nroDeNodo = dictionary_get(copiaConMenosCarga,"nroDeNodo");
			if(nroDeNodo == -1){

				//NO HAY COPIAS DISPONIBLES !!!!!

			}
			//armo pedido de map, segun protocolo es
			//*comando: "mapFile"
			//*data:sizeDireccionNodo-direccionNodo-sizeNroDeBloque-nroDeBloque-
			//-sizeRutaArchivoTemporal-rutaArchivoTemporal

			//"copiaConMenosCarga" tiene el nroDeNodo y nroDeBloque
			//en "path" tengo el path a concatenar con la hora actual
			char *temporal = temporal_get_string_time();
			char *path_with_temporal = malloc(strlen(temporal)+strlen(path)+1);
			strcpy(path_with_temporal,path);
			strcat(path_with_temporal,temporal);
			//YA TENGO EL ARCHIVO PARA ENVIAR

			//ACTUALIZAR PedidoRealizado, NODOState y BLOCKState
			//actualizarPedidoRealizado(bloque, nodo, path, pathTemporal, IN_MAPPING);
			//incrementarOperacionesEnProcesoEnNodo(int nroNodo);

			//actualizoBlockState
			dictionary_put(blockState,K_BlockState_state,IN_MAPPING);
			//dictionary_put(blockState,K_BlockState_nroNodo,/*copiaConMenosCarga.nodo*/);
			//dictionary_put(blockState,K_BlockState_nroBloque,/*copiaConMenosCarga.bloque*/);
			dictionary_put(blockState,K_BlockState_temporaryPath,path_with_temporal);

			Message *sendMessage;

			free(path);
			free(fileState);
			free(copiaConMenosCarga);
			for(k=0;k<nroDeCopias;k++){ free(copias[k]); }
			for(i=0;i<size_fileState;i++){ free(blockStateArray[i]); }

			return sendMessage;
		}
	}
}


void actualizarPedidoRealizado(int bloque, int nodo, char* path, char *pathTemporal, TypesPedidosRealizado tipo )
{
	if(bloque != NULL){ dictionary_put(pedidoRealizado,K_PedidoRealizado_Bloque,bloque); }
	if(nodo != NULL){ dictionary_put(pedidoRealizado,K_PedidoRealizado_Nodo,nodo); }
	if(path != NULL){ dictionary_put(pedidoRealizado,K_PedidoRealizado_Path,path);}
	if(pathTemporal != NULL){ dictionary_put( pedidoRealizado,K_PedidoRealizado_PathArchTemporal,pathTemporal); }
	if(tipo != NULL){ dictionary_put(pedidoRealizado,K_PedidoRealizado_TipoPedido,tipo); }
}

// Idea de planificaion : se hara map y reduce de los bloques en orden ascendente.
// Hago map de todos los bloques de un archivo y ahi lanzo el reduce.
// Habra un MAXIMO de envios por la red, hasta que no responda un envio, nose hara otro(Productor-Consumidor)
// Si vuelve con error, entonces se rePlanifica y se vuelve a mandar el mismo pero a otro NODO.
// Si NO hay otro NODO disponible entonces, se informa que el proceso del archivo quedo incompleto--> no se que hacer luego.
// Si la operacion enviada falla porque se cayo el Nodo o simplemente fallo entonces se RE-PLANIFICA, si se cae el Job NO hay nada mas que hacer.
