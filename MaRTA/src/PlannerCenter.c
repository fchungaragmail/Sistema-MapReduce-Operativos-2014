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
#include "Serializador.h"

// Constantes
#define MaxPedidosEnRed 5

//pedidoRealizado --> keys
#define K_PedidoRealizado_Nodo "PedidoRealizado_Nodo"
#define K_PedidoRealizado_Bloque "PedidoRealizado_Bloque"
#define K_PedidoRealizado_Path "PedidoRealizado_Path"
#define K_PedidoRealizado_TipoPedido "PedidoRealizado_TipoPedido" //map, reduce o pedido de tabla a fs
#define K_PedidoRealizado_PathArchTemporal "PedidoRealizado_PathArchTemporal"

// Variables Globales
t_dictionary* pedidoRealizado;

// Funciones privadas
int obtenerIdParaComando(Message *recvMessage);
Message* planificar(Message *recvMessage,TypesMessages type);
void actualizarPedidoRealizado(int bloque, char* ipnodo, char* path, char *pathTemporal, TypesPedidosRealizado tipo );
Message* obtenerProximoPedido(Message *recvMessage);
bool* obtenerRequestResponse(Message *recvMessage,TypesMessages type);
t_dictionary *obtenerCopiaDeConMenosCarga(Message *recvMessage,char *path,int bloqueNro);
char* crearPathTemporal(char *path);
Message* armarMensajeParaEnvio(Message *recvMessage,void *stream,char *comando);

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

			char* filePath = deserializeFilePath(recvMessage,K_Job_NewFileToProcess);
			bool *soportaCombiner = deserializeSoportaCombiner(recvMessage);
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

			bool *response = deserializeRequestResponse(recvMessage,K_FS_FileFullData);

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

			bool *_response = deserializeRequestResponse(recvMessage,K_Job_ReduceResponse);

			if(_response==true){
				Message *mensajePlanificado = planificar(recvMessage,K_Job_ReduceResponse);
				free(mensajePlanificado);


			}else{

				//QUE HACER SI FALLA EL REDUCE ???

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

int obtenerIdParaComando(Message *recvMessage)
{
	char *comando = deserializeComando(recvMessage);
	TypesMessages type;
	if(strcmp(comando,"newConnection")==0){type =  K_NewConnection;}
	if(strcmp(comando,"archivoAProcesar")==0){type = K_Job_NewFileToProcess;}
	if(strcmp(comando,"mapFileResponse")==0){type = K_Job_MapResponse;}
	if(strcmp(comando,"reduceFileResponse")==0){type = K_Job_ReduceResponse;}
	if(strcmp(comando,"DataFileResponse")==0){type = K_FS_FileFullData;}

	free(comando);
	return type;
}

//Planificacion
Message* planificar(Message *recvMessage,TypesMessages type)
{
	char *path = dictionary_get(pedidoRealizado,K_PedidoRealizado_Path);

	if(type == K_Job_NewFileToProcess){
		//pido al FS la tabla de direcciones del archivo
		//segun protocolo ---> -Comando: "DataFile rutaDelArchivo" /// -Data: Vacio

		Message *fsRequest = malloc(sizeof(Message));
		fsRequest->mensaje = malloc(sizeof(mensaje_t));

		//armo stream a mano
		char *stream = malloc(strlen("DataFile ") + strlen(path) + 1);
		char *comando = "DataFile ";
		int size,offset; size = 0; offset = 0;
		memcpy(stream,comando,size = strlen(comando));
		offset = offset + size;
		memcpy(stream+offset,path,strlen(path)+1);

		int fileSystemSocket = getFSSocket();

		fsRequest->sockfd = fileSystemSocket;
		fsRequest->mensaje->comandoSize = strlen(stream);
		fsRequest->mensaje->comando = stream;
		fsRequest->mensaje->dataSize = 0;
		fsRequest->mensaje->data = "";

		return fsRequest;
	}

	if(type==K_FS_FileFullData){

		//ACTUALIZO TABLAS
		char *path = deserializeFilePath(recvMessage,K_FS_FileFullData);
		t_dictionary *fullData = deserializarFullDataResponse(recvMessage);
		addFileFullData(recvMessage->sockfd, path, fullData);//se completa filesToProcess
															//y se crea un fileState
		//obtengo proximoPedido CON INFO ACTUALIZADA
		Message *sendMessage = obtenerProximoPedido(recvMessage);
		return sendMessage;
	}

	if(type==K_Job_MapResponse){

		//*data:sizeRutaArchivoTemporal-rutaArchivoTemporal-sizeRespuesta-Respuesta

		char *temporaryPath = deserializeFilePath(recvMessage,K_Job_MapResponse);
		bool *requestResponse = deserializeRequestResponse(recvMessage,K_Job_MapResponse);

		char *IPnroNodo =  dictionary_get(pedidoRealizado,K_PedidoRealizado_Nodo);
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
			decrementarOperacionesEnProcesoEnNodo(IPnroNodo);
			addTemporaryFilePathToNodoData(IPnroNodo,temporaryPath);

			//OBTENER PROXIMO PEDIDO !!!
			Message *sendMessage = obtenerProximoPedido(recvMessage);
			free(temporaryPath);
			free(requestResponse);
			return sendMessage;
		}

		if(requestResponse==false){//REPLANIFICAR

			//actualizar tablas y reenviar si existen copias

			//PONER -1 EN COPIAS
			darDeBajaCopiaEnBloqueYNodo(path,recvMessage->sockfd,nroBloque,IPnroNodo);
			decrementarOperacionesEnProcesoEnNodo(IPnroNodo);

			//OBTENER PROXIMO PEDIDO (se va a enviar devuelta el mismo, siempre y cuando haya copias disponibles)
			Message* sendMessage = obtenerProximoPedido(recvMessage);
			free(temporaryPath);
			free(requestResponse);
			return sendMessage;
		}
	}

	if(type==K_Job_ReduceResponse){
		//*data:sizeRutaArchivoTemporal-rutaArchivoTemporal-sizeRespuesta-Respuesta

		char *temporaryPath = deserializeFilePath(recvMessage,K_Job_ReduceResponse);
		bool *requestResponse = deserializeRequestResponse(recvMessage,K_Job_ReduceResponse);

		char *IPnroNodo =  dictionary_get(pedidoRealizado,K_PedidoRealizado_Nodo);
		int nroBloque =  dictionary_get(pedidoRealizado,K_PedidoRealizado_Bloque);


		//ACTUALIZAR TABLAS!!!!!!!!!!!!!!!!!!!!!!!!!!!

		if(requestResponse==false){
			//re planificar
			// ver que hacer si el reduce falla !!
		}
		if(requestResponse==true){

			//Reduce realizado co exito!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
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
			char *IPnroNodoLocal = obtenerNodoConMayorCantidadDeArchivosTemporales(path);

			int cantidadDeNodos = obtenerCantidadDeNodosDiferentesEnBlockState(path);
			t_list *nodosEnBlockState = obtenerNodosEnBlockStateArray(path);

			char *pathTemporalLocal = crearPathTemporal(path);

			// iniciar el serializado --> nroNodoLocal-pathTemporalLocal
			void *stream = createStream();
			addStringToStream(stream,IPnroNodoLocal);
			addStringToStream(stream,pathTemporalLocal);

			if(true == combinerMode){
				//planificar con combiner

			}

			if(false == combinerMode){
				//planificar sin combiner

				//****************************************************
				//PRIMERO PONER LOS PATH CORRESPONDIENTES AL NODO LOCAL

				t_list *pathsTemporalesParaNodoLocal = obtenerPathsTemporalesParaNodo(path, IPnroNodoLocal);
				int cantidadDePathsTempEnNodoLocal = obtenerCantidadDePathsTemporalesEnNodo(path,IPnroNodoLocal);
				int k;
				for(k=0;k<cantidadDePathsTempEnNodoLocal;k++){

					char *tempPath = list_get(pathsTemporalesParaNodoLocal,k);
					//agregar a el serializado "tempPath"
					addStringToStream(stream,tempPath);
				}
				//*****************************************************
				int i;
				for(i=0;i<cantidadDeNodos;i++){
					char *IPnodoEnBlockState = list_get(nodosEnBlockState,i);

					if( strcmp(IPnodoEnBlockState,IPnroNodoLocal) != 0 ){
						int cantidadDePathsTempEnNodo = obtenerCantidadDePathsTemporalesEnNodo(path,IPnodoEnBlockState);
						t_list *pathsTemporalesParaNodo = obtenerPathsTemporalesParaNodo(path, IPnodoEnBlockState);

						//agregar a el serializado --> "cantidadDePathsTempEnNodo"
						addIntToStream(stream,cantidadDePathsTempEnNodo,K_int16_t);

						int j;
						for(j=0;j<cantidadDePathsTempEnNodo;j++){

							char *tempPath = list_get(pathsTemporalesParaNodo,j);
							//agregar a el serializado "tempPath"
							addStringToStream(stream,tempPath);
						}
					}
				}
			}

			Message *sendMessage = armarMensajeParaEnvio(recvMessage,stream,"reduceFile");
			return sendMessage;
		}

		if(statusBlock == UNINITIALIZED){ //este bloque no esta procesado

			//obtengo conjunto (#nodo;#bloque) con menos carga de operaciones en #nodo
			t_dictionary *copiaConMenosCarga = obtenerCopiaDeConMenosCarga(recvMessage,path,i);

			char *IPnroDeNodo = dictionary_get(copiaConMenosCarga,K_Copia_IPNodo);
			if( strcmp(IPnroDeNodo,K_Copia_DarDeBajaIPNodo) == 0){//checkeo q haya copias disponibles

				//NO HAY COPIAS DISPONIBLES !!!!!
				//PREGUNTAR DEVUELTA AL FS ?
			}

			//armo pedido de map, segun protocolo es
			//*comando: "mapFile"
			//*data:sizeDireccionNodo-direccionNodo-nroDeBloque-...
			//...-sizeRutaArchivoTemporal-rutaArchivoTemporal

			char *path_with_temporal = crearPathTemporal(path);
			char *ipNodo = dictionary_get(copiaConMenosCarga,K_Copia_IPNodo);
			int nroDeBloque = dictionary_get(copiaConMenosCarga,K_Copia_NroDeBloque);

			void *stream = createStream();
			addStringToStream(stream,ipNodo);
			addIntToStream(stream,nroDeBloque,K_int16_t);
			addStringToStream(stream,path_with_temporal);
			Message *msjParaEnviar = armarMensajeParaEnvio(recvMessage,stream,"mapFile");

			//ACTUALIZAR PedidoRealizado, NODOState y BLOCKState
			actualizarPedidoRealizado(nroDeBloque, ipNodo, path, path_with_temporal, IN_MAPPING);
			incrementarOperacionesEnProcesoEnNodo(ipNodo);

			//actualizoBlockState
			dictionary_put(blockState,K_BlockState_state,IN_MAPPING);
			dictionary_put(blockState,K_BlockState_nroNodo,ipNodo);
			dictionary_put(blockState,K_BlockState_nroBloque,nroDeBloque);
			dictionary_put(blockState,K_BlockState_temporaryPath,path_with_temporal);

			return msjParaEnviar;
		}
	}
}

char* crearPathTemporal(char *path){
	char *temporal = temporal_get_string_time();
	char *path_with_temporal = malloc(strlen(path)+strlen("-")+strlen(temporal)+1);
	strcpy(path_with_temporal,path);
	strcpy(path_with_temporal+(strlen(path_with_temporal)),"-");
	strcpy(path_with_temporal+(strlen(path_with_temporal)),temporal);

	return path_with_temporal;
}

void actualizarPedidoRealizado(int bloque, char *ipNodo, char* path, char *pathTemporal, TypesPedidosRealizado tipo )
{
	//HACER FREE DE TODOS LOS ANTERIORES

	if(bloque != NULL){ dictionary_put(pedidoRealizado,K_PedidoRealizado_Bloque,bloque); }
	if(ipNodo != NULL){ dictionary_put(pedidoRealizado,K_PedidoRealizado_Nodo,ipNodo); }
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

		char *IPnroNodoCopia = dictionary_get(copia,K_Copia_IPNodo);
		char *IPnroNodoCopiaSiguiente = dictionary_get(copiaSiguiente,K_Copia_IPNodo);


		if((strcmp(IPnroNodoCopia,K_Copia_DarDeBajaIPNodo)!= 0) &&
				(strcmp(IPnroNodoCopiaSiguiente,K_Copia_DarDeBajaIPNodo)!=0) ){
			//ninguno de los nodos esta disponible

			copiaConMenosCarga = copia;

		}else if((strcmp(IPnroNodoCopia,K_Copia_DarDeBajaIPNodo)== 0)
					&& (strcmp(IPnroNodoCopiaSiguiente,K_Copia_DarDeBajaIPNodo)!=0)){

			copiaConMenosCarga = copia;

		}else if((strcmp(IPnroNodoCopia,K_Copia_DarDeBajaIPNodo)!=0) &&
					(strcmp(IPnroNodoCopiaSiguiente,K_Copia_DarDeBajaIPNodo)==0)){

			copiaConMenosCarga = copiaSiguiente;
		}else{

			int opsEnNodoCopia = getCantidadDeOperacionesEnProcesoEnNodo(IPnroNodoCopia);
			int opsEnNodoCopiaSig = getCantidadDeOperacionesEnProcesoEnNodo(IPnroNodoCopiaSiguiente);

			if(opsEnNodoCopia < opsEnNodoCopiaSig){
				copiaConMenosCarga = copia;
			}else{
				copiaConMenosCarga = copiaSiguiente;
			}
		}
	}

	return copiaConMenosCarga;
}

Message* armarMensajeParaEnvio(Message *recvMessage,void *stream,char *comando)
{
	Message *msjParaEnvio = malloc(sizeof(Message));
	msjParaEnvio->mensaje = malloc(sizeof(mensaje_t));
	msjParaEnvio->mensaje->comando = malloc(strlen(comando));
	msjParaEnvio->mensaje->data = malloc(strlen(stream)+1);

	msjParaEnvio->sockfd = recvMessage->sockfd;
	msjParaEnvio->mensaje->comandoSize = strlen(comando);
	msjParaEnvio->mensaje->comando = comando;
	msjParaEnvio->mensaje->dataSize = strlen(stream);
	msjParaEnvio->mensaje->data = stream;

	return msjParaEnvio;
}

// Idea de planificaion : se hara map y reduce de los bloques en orden ascendente.
// Hago map de todos los bloques de un archivo y ahi lanzo el reduce.
// Habra un MAXIMO de envios por la red, hasta que no responda un envio, nose hara otro(Productor-Consumidor)
// Si vuelve con error, entonces se rePlanifica y se vuelve a mandar el mismo pero a otro NODO.
// Si NO hay otro NODO disponible entonces, se informa que el proceso del archivo quedo incompleto--> no se que hacer luego.
// Si la operacion enviada falla porque se cayo el Nodo o simplemente fallo entonces se RE-PLANIFICA, si se cae el Job NO hay nada mas que hacer.
