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
mensaje_t* createFSRequestForPath(char *path);
bool* obtenerSoportaCombiner(Message *recvMessage);
t_dictionary* procesarFullDataResponse(Message *recvMessage);
Message* planificar(Message *recvMessage,TypesMessages type);
void actualizarPedidoRealizado(int bloque, int nodo, char* path, char *pathTemporal, TypesPedidosRealizado tipo );
Message* obtenerProximoPedido(Message *recvMessage);
bool* obtenerRequestResponse(Message *recvMessage,TypesMessages type);
t_dictionary *obtenerCopiaDeConMenosCarga(Message *recvMessage,char *path,int bloqueNro);
char* obtenerPathTemporal(char *path);

// Funciones publicas
void initPlannerCenter();
void processMessage(Message *recvMessage);

void initPlannerCenter()
{
	initFilesStatusCenter();
	pedidoRealizado=malloc(sizeof(t_dictionary));
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

			char* filePath = obtenerFilePath(recvMessage,K_Job_NewFileToProcess);
			bool *soportaCombiner = obtenerSoportaCombiner(recvMessage);
			addNewFileForProcess(filePath,soportaCombiner,recvMessage->sockfd);	//se agrega un file_StatusData a
																				//filesToProcessPerJob
			actualizarPedidoRealizado(NULL,NULL,filePath,NULL,K_Pedido_FileData);
			Message *sendMessage = planificar(recvMessage,K_Job_NewFileToProcess);
			enviar(sendMessage->sockfd,sendMessage->mensaje);

			free(filePath);

			printf("***************\n");
			break;

		case K_FS_FileFullData:

			printf("PlannerCenter : planificar FS_FileFullData\n");
			//FS me responde con el pedido de datos que le hice.

			bool *response = obtenerRequestResponse(recvMessage,K_FS_FileFullData);

			if(response == false){
				//si es false entonces no esta disponible el archivo
				//QUE HACER ???????
			}

			if(response == true ){

				Message *planifiedMessage = planificar(recvMessage,K_FS_FileFullData);
				enviar(planifiedMessage->sockfd,planifiedMessage->mensaje);
			}
			free(response);

			printf("***************\n");
			break;

		case K_Job_MapResponse:
			printf("PlannerCenter : planificar Job_MapResponse\n");

			Message *planifiedMsj = planificar(recvMessage,K_Job_MapResponse);
			enviar(planifiedMsj->sockfd,planifiedMsj->mensaje);

			printf("***************\n");
			break;
		case K_Job_ReduceResponse:
			printf("PlannerCenter : planificar Job_ReduceResponse");

			bool *_response = obtenerRequestResponse(recvMessage,K_Job_ReduceResponse);

			if(_response==true){
				Message *mensajePlanificado = planificar(recvMessage,K_Job_ReduceResponse);
				free(mensajePlanificado);


			}else{
				Message *mensajePlanificado = planificar(recvMessage,K_Job_MapResponse);
				enviar(mensajePlanificado->sockfd,mensajePlanificado->mensaje);
			}
			free(_response);

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

bool* obtenerRequestResponse(Message *recvMessage,TypesMessages type)
{
	bool* requestResponse = malloc(sizeof(bool));

	if(type==K_Job_MapResponse || type==K_Job_ReduceResponse){
		//segun protocolo el data sera
		//*data:sizeRutaArchivoTemporal-rutaArchivoTemporal-sizeRespuesta-Respuesta
		//debo obtener "Respuesta"
		return requestResponse;
	}

	if(type == K_FS_FileFullData){
		//segun protocolo --> Comando: "DataFileResponse"
		//Si existe el archivo --> Data:sizeRutaDelArchivo-rutaDelArchivo-sizeRespuesta-Respuesta-.......
		//debo obtener "Respuesta"
		return requestResponse;
	}

	//no deberia pasar nunca por aca
	printf("Planner Center : ERROR obtenerRequestResponse no pudo leer el valor");
	requestResponse = false;
	return requestResponse;
}

//Pedidos a FS

mensaje_t* createFSRequestForPath(char *filePath)
{
	//crear un mensaje_t con el comando "DataFile" y el data "filePath"
	//segun protocolo --> Comando: "DataFile" // Data: sizeRutaDelArchivo-rutaDelArchivo

	mensaje_t *fsRequest = malloc(sizeof(mensaje_t));
	return fsRequest;
}

//Planificacion
Message* planificar(Message *recvMessage,TypesMessages type)
{
	char *path = dictionary_get(pedidoRealizado,K_PedidoRealizado_Path);

	if(type == K_Job_NewFileToProcess){

		//pido al FS la tabla de direcciones del archivo
		int fileSystemSocket = getFSSocket();
		mensaje_t *fsRequest = createFSRequestForPath(path);
		Message *sendMessage = malloc(sizeof(Message));
		sendMessage->mensaje = fsRequest;
		sendMessage->sockfd = fileSystemSocket;
		return sendMessage;
	}

	if(type==K_FS_FileFullData){

		//ACTUALIZO TABLAS
		char *path = obtenerFilePath(recvMessage,K_FS_FileFullData);
		t_dictionary *fullData = procesarFullDataResponse(recvMessage);
		addFileFullData(recvMessage->sockfd, path, fullData);//se completa filesToProcess
															//y se crea un fileState
		//obtengo proximoPedido CON INFO ACTUALIZADA
		Message *sendMessage = obtenerProximoPedido(recvMessage);
		return sendMessage;
	}

	if(type==K_Job_MapResponse){
		//*data:sizeRutaArchivoTemporal-rutaArchivoTemporal-sizeRespuesta-Respuesta

		char *temporaryPath = obtenerFilePath(recvMessage,K_Job_MapResponse);
		bool *requestResponse = obtenerRequestResponse(recvMessage,K_Job_MapResponse);

		int nroNodo =  dictionary_get(pedidoRealizado,K_PedidoRealizado_Nodo);
		int nroBloque =  dictionary_get(pedidoRealizado,K_PedidoRealizado_Bloque);

		if(requestResponse==true){

			//actualizar tablas y realizar proximo envio

			t_dictionary *fileState = getFileStateForPath(path);
			int nroDeBloques = dictionary_get(fileState,K_FileState_size);

			t_dictionary *(*blockStatesArray)[nroDeBloques];
			blockStatesArray = dictionary_get(fileState,K_FileState_arrayOfBlocksStates);

			t_dictionary *blockState = (*blockStatesArray)[nroBloque];

			//Actualizo blockState
			dictionary_put(blockState,K_BlockState_state,MAPPED);
			//Actualizo nodoState
			decrementarOperacionesEnProcesoEnNodo(nroNodo);
			addTemporaryFilePathToNodoData(nroNodo,temporaryPath);

			//OBTENER PROXIMO PEDIDO !!!
			Message *sendMessage = obtenerProximoPedido(recvMessage);
			return sendMessage;
		}

		if(requestResponse==false){//REPLANIFICAR

			//actualizar tablas y reenviar si existen copias

			//PONER -1 EN COPIAS
			darDeBajaCopiaEnBloqueYNodo(path,recvMessage->sockfd,nroBloque,nroNodo);
			decrementarOperacionesEnProcesoEnNodo(nroNodo);

			//OBTENER PROXIMO PEDIDO (se va a enviar devuelta el mismo, siempre y cuando haya copias disponibles)
			Message* sendMessage = obtenerProximoPedido(recvMessage);
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

		if(requestResponse==false){
			//re planificar
			// ver que hacer si el reduce falla !!
		}
	}

	Message *error; //no deberia nunca llegar aca
	return error;
}

Message* obtenerProximoPedido(Message *recvMessage)
{
	char *path = dictionary_get(pedidoRealizado,K_PedidoRealizado_Path);
	t_dictionary *fileState = getFileStateForPath(path);

	int size_fileState= dictionary_get(fileState,K_FileState_size);
	t_dictionary *(*blockStateArray)[size_fileState];
	blockStateArray = dictionary_get(fileState,K_FileState_arrayOfBlocksStates);

	int i;
	for(i=0;i<size_fileState;i++){//itero en fileState hasta encontrar el siguiente a mappear

		t_dictionary *blockState = (*blockStateArray)[i];
		int statusBlock = dictionary_get(blockState,K_BlockState_state);

		if(statusBlock == MAPPED && i==(size_fileState-1)){
			//estan todos mappeados , inculuidos el ultimo

			//Actualizo estados
			actualizarPedidoRealizado(NULL,NULL,NULL,NULL,IN_REDUCING);

			//HACER REDUCE !!! --> usar fileState para ver las ubicaciones
			//VER SI SOPORTA COMBINER O NO
			//hacer reduce en el nodo que contenga mas archivos mappeados

			bool *combinerMode = soportaCombiner(recvMessage->sockfd,path);
			int nroNodoLocal = obtenerNodoConMayorCantidadDeArchivosTemporales(path);

			int cantidadDeNodos = obtenerCantidadDeNodosDiferentesEnBlockState(path);
			t_list *nodosEnBlockState = obtenerNodosEnBlockStateArray(path);

			char *pathTemporal = obtenerPathTemporal(path);

			// iniciar el serializado --> nroNodoLocal-pathTemporal
			if(true == combinerMode){
				//planificar con combiner

			}

			if(false == combinerMode){
				//planificar sin combiner
				int i;
				for(i=0;i<cantidadDeNodos;i++){
					int nodoEnBlockState = list_get(nodosEnBlockState,i);
					int cantidadDePathsTempEnNodo = obtenerCantidadDePathsTemporalesEnNodo(path,nodoEnBlockState);
					t_list *pathsTemporalesParaNodo = obtenerPathsTemporalesParaNodo(path, nodoEnBlockState);

					//agregar a el serializado --> "cantidadDePathsTempEnNodo"
					int j;
					for(j=0;j<cantidadDePathsTempEnNodo;j++){

						char *tempPath = list_get(pathsTemporalesParaNodo,j);
						//agregar a el serializado "tempPath"
					}
				}
			}
			Message *sendMessage;

			return sendMessage;
		}

		if(statusBlock == UNINITIALIZED){ //este bloque no esta procesado

			//obtengo conjunto (#nodo;#bloque) con menos carga de operaciones en #nodo
			t_dictionary *copiaConMenosCarga = obtenerCopiaDeConMenosCarga(recvMessage,path,i);

			int nroDeNodo = dictionary_get(copiaConMenosCarga,"nroDeNodo");
			int nroDeBloque = dictionary_get(copiaConMenosCarga,"nroDeBloque");
			if(nroDeNodo == -1){//checkeo q haya copias disponibles

				//NO HAY COPIAS DISPONIBLES !!!!!
				//PREGUNTAR DEVUELTA AL FS ?
			}

			//armo pedido de map, segun protocolo es
			//*comando: "mapFile"
			//*data:sizeDireccionNodo-direccionNodo-sizeNroDeBloque-nroDeBloque-...
			//...-sizeRutaArchivoTemporal-rutaArchivoTemporal

			//"copiaConMenosCarga" tiene el nroDeNodo y nroDeBloque
			//en "path" tengo el path a concatenar con la hora actual
			char *path_with_temporal = obtenerPathTemporal(path);

			//YA TENGO EL ARCHIVO PARA ENVIAR

			//ACTUALIZAR PedidoRealizado, NODOState y BLOCKState
			actualizarPedidoRealizado(nroDeBloque, nroDeNodo, path, path_with_temporal, IN_MAPPING);
			incrementarOperacionesEnProcesoEnNodo(nroDeNodo);

			//actualizoBlockState
			dictionary_put(blockState,K_BlockState_state,IN_MAPPING);
			dictionary_put(blockState,K_BlockState_nroNodo,nroDeNodo);
			dictionary_put(blockState,K_BlockState_nroBloque,nroDeBloque);
			dictionary_put(blockState,K_BlockState_temporaryPath,path_with_temporal);

			Message *sendMessage;

			return sendMessage;
		}
	}
}

char* obtenerPathTemporal(char *path){
	char *temporal = temporal_get_string_time();
	char *path_with_temporal = malloc(strlen(temporal)+strlen(path)+1);
	strcpy(path_with_temporal,path);
	strcat(path_with_temporal,temporal);
	return path_with_temporal;
}

void actualizarPedidoRealizado(int bloque, int nodo, char* path, char *pathTemporal, TypesPedidosRealizado tipo )
{
	//HACER FREE DE TODOS LOS ANTERIORES

	if(bloque != NULL){ dictionary_put(pedidoRealizado,K_PedidoRealizado_Bloque,bloque); }
	if(nodo != NULL){ dictionary_put(pedidoRealizado,K_PedidoRealizado_Nodo,nodo); }
	if(path != NULL){ dictionary_put(pedidoRealizado,K_PedidoRealizado_Path,path);}
	if(pathTemporal != NULL){ dictionary_put( pedidoRealizado,K_PedidoRealizado_PathArchTemporal,pathTemporal); }
	if(tipo != NULL){
		int _tipo = tipo;
		dictionary_put(pedidoRealizado,K_PedidoRealizado_TipoPedido,_tipo);
	}
}

t_dictionary *obtenerCopiaDeConMenosCarga(Message *recvMessage,char *path,int bloqueNro){

	int nroDeCopias = obtenerNumeroDeCopiasParaArchivo(recvMessage->sockfd,path);
	t_dictionary *(*copias)[nroDeCopias];

	copias = obtenerCopiasParaBloqueDeArchivo(recvMessage->sockfd,bloqueNro,path);
	t_dictionary *copiaConMenosCarga;
	int j;
	for(j=0;j<(nroDeCopias-1);j++){

		//obtengo nodo con menos carga de operaciones

		t_dictionary *copia = (*copias)[j];
		t_dictionary *copiaSiguiente = (*copias)[j+1];

		int nroNodoCopia = dictionary_get(copia,"nroDeNodo");
		int nroNodoCopiaSiguiente = dictionary_get(copiaSiguiente,"nroDeNodo");


		if(nroNodoCopia == -1 && nroNodoCopiaSiguiente == -1){
			//ninguno de los nodos esta disponible

			copiaConMenosCarga = copia;

		}else if(nroNodoCopia != -1 && nroNodoCopiaSiguiente == -1){

			copiaConMenosCarga = copia;

		}else if(nroNodoCopia == -1 && nroNodoCopiaSiguiente != -1){

			copiaConMenosCarga = copiaSiguiente;
		}else{

			int opsEnNodoCopia = getCantidadDeOperacionesEnProcesoEnNodo(nroNodoCopia);
			int opsEnNodoCopiaSig = getCantidadDeOperacionesEnProcesoEnNodo(nroNodoCopiaSiguiente);

			if(opsEnNodoCopia < opsEnNodoCopiaSig){
				copiaConMenosCarga = copia;
			}else{
				copiaConMenosCarga = copiaSiguiente;
			}
		}
	}

	return copiaConMenosCarga;
}

// Idea de planificaion : se hara map y reduce de los bloques en orden ascendente.
// Hago map de todos los bloques de un archivo y ahi lanzo el reduce.
// Habra un MAXIMO de envios por la red, hasta que no responda un envio, nose hara otro(Productor-Consumidor)
// Si vuelve con error, entonces se rePlanifica y se vuelve a mandar el mismo pero a otro NODO.
// Si NO hay otro NODO disponible entonces, se informa que el proceso del archivo quedo incompleto--> no se que hacer luego.
// Si la operacion enviada falla porque se cayo el Nodo o simplemente fallo entonces se RE-PLANIFICA, si se cae el Job NO hay nada mas que hacer.
