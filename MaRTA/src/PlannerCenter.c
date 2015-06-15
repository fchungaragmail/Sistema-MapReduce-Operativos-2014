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
#include <commons/string.h>
#include "Serializador.h"

// Variables Globales
int jobSocket;
int cantidadDePedidosAlFS;
t_list *nodosReduceList_Pedido1;

//-->PedidoRealizado
char *pedidoRealizado_Nodo;
char *pedidoRealizado_Bloque;
char *pedidoRealizado_Path;
StatusBlockState *pedidoRealizado_TipoPedido;
char *pedidoRealizado_PathArchTemporal;

// Funciones privadas
int obtenerIdParaComando(Message *recvMessage);
void planificar(Message *recvMessage,TypesMessages type);
void actualizarPedidoRealizado(char *bloque, char* ipnodo, char* path, char *pathTemporal, StatusBlockState tipo );
Message* obtenerProximoPedido(Message *recvMessage);
t_dictionary *obtenerCopiaConMenosCargaParaBloque(Message *recvMessage,char *path,int bloqueNro);
char* crearPathTemporal(char *path);
Message* armarMensajeParaEnvio(Message *recvMessage,void *stream,char *comando);
void actualizarTablas_RtaDeMapExitosa(Message *recvMessage);
Message *createFSrequest();
bool *obtenerEstadoDeReduce();

// Funciones publicas
void initPlannerCenter();
void processMessage(Message *recvMessage);

void initPlannerCenter()
{
	initFilesStatusCenter();
	jobSocket=0;
	nodosReduceList_Pedido1 = list_create();
	cantidadDePedidosAlFS=0;
}

void processMessage(Message *recvMessage)
{
	int comandoId = obtenerIdParaComando(recvMessage);
	switch (comandoId) {
		case K_NewConnection:
			printf("PlannerCenter : planificar NewConnection\n");
			addNewConnection(recvMessage->sockfd);
			jobSocket=recvMessage->sockfd;
			printf("***************\n");
			break;
		case K_Job_NewFileToProcess:

			printf("PlannerCenter : planificar Job_NewFileToProcess:\n");

			char* filePath = deserializeFilePath(recvMessage,K_Job_NewFileToProcess);
			bool *soportaCombiner = deserializeSoportaCombiner(recvMessage);
			//******************
			// --> se agrega un file_StatusData a filesToProcessPerJob
			addNewFileForProcess(filePath,soportaCombiner,recvMessage->sockfd);
			//******************
			planificar(recvMessage,K_Job_NewFileToProcess);


			free(filePath);

			printf("***************\n");
			break;

		case K_FS_FileFullData:

			printf("PlannerCenter : planificar FS_FileFullData\n");
			//FS me responde con el pedido de datos que le hice.

			bool *response = deserializeRequestResponse(recvMessage,K_FS_FileFullData);

			if(!(*response)){
				//si es false entonces no esta disponible el archivo en el FS
				char *tempPath = deserializeFilePath(recvMessage,K_Job_MapResponse);
				printf("el pedido al FS del archivo %s fallo, MaRTA no puede continuar la operacion\n",tempPath);
			}

			if(*response){
				planificar(recvMessage,K_FS_FileFullData);
			}
			free(response);

			printf("***************\n");
			break;

		case K_Job_MapResponse:

			printf("PlannerCenter : planificar Job_MapResponse\n");
			planificar(recvMessage,K_Job_MapResponse);


			printf("***************\n");
			break;
		case K_Job_ReduceResponse:
			printf("PlannerCenter : planificar Job_ReduceResponse");

			//tipos de rtas
			//**************
			//*comando: "reduceFileConCombiner-Pedido1 pathArchivo Respuesta"
			//*comando: "reduceFileConCombiner-Pedido2 NombreArchTempFinal Respuesta"
			//*comando: "reduceFileSinCombiner NombreArchTempFinal Respuesta"

			bool *_response = deserializeRequestResponse(recvMessage,K_Job_ReduceResponse);
			char *path = deserializeFilePath(recvMessage,K_Job_ReduceResponse);
			char *reduceType = deserializeComando(recvMessage);

			//FINALIZADO
			if((*_response)&&((strcmp(reduceType,"reduceFileConCombiner-Pedido2")==0)||(strcmp(reduceType,"reduceFileSinCombiner")==0))){

				//Reduce realizado con exito // Actualizar tablas !!
				decrementarOperacionesEnProcesoEnNodo(pedidoRealizado_Nodo);
				printf("archivo %s reducido con exito !\n",path);
			}

			//TERMINO 1ER PEDIDO
			if((*_response)&&(strcmp(reduceType,"reduceFileConCombiner-Pedido1")==0)){

				planificar(recvMessage,K_Job_ReduceResponse);
			}
			//FALLO PEDIDO
			if(!(*_response)){

				planificar(recvMessage,K_Job_ReduceResponse);
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
	if(strcmp(comando,"DataFileResponse")==0){type = K_FS_FileFullData;}

	if(strcmp(comando,"reduceFileConCombiner-Pedido1")==0){type = K_Job_ReduceResponse;}
	if(strcmp(comando,"reduceFileConCombiner-Pedido2")==0){type = K_Job_ReduceResponse;}
	if(strcmp(comando,"reduceFileSinCombiner")==0){type = K_Job_ReduceResponse;}

	free(comando);
	return type;
}

//Planificacion
void planificar(Message *recvMessage,TypesMessages type)
{

	if(type == K_Job_NewFileToProcess){
		//pido al FS la tabla de direcciones del archivo
		//segun protocolo ---> -Comando: "DataFile rutaDelArchivo" /// -Data: Vacio

		pedidoRealizado_Path = deserializeFilePath(recvMessage,K_Job_NewFileToProcess);
		Message *fsRequest = createFSrequest();
		//enviar(fsRequest->sockfd,fsRequest->mensaje);

	}

	if(type==K_FS_FileFullData){

		//--> FS responde con tabla de archivo pedida
		//-Comando: "DataFileResponse rutaDelArchivo Respuesta cantidadDeBloques-nroDeCopias-sizeEstructura-"

		//ACTUALIZO TABLAS
		char *path = deserializeFilePath(recvMessage,K_FS_FileFullData);
		int nroDeBloques = deserializarFullDataResponse_nroDeBloques(recvMessage);
		int nroDeCopias = deserializarFullDataResponse_nroDeCopias(recvMessage);
		t_list *listaPadreDeBloques = deserializarFullDataResponse(recvMessage);
		pedidoRealizado_Path=path;

		if(cantidadDePedidosAlFS==0){
			//se completa filesToProcess y se crea un fileState
			addFileFullData(jobSocket, path,nroDeBloques,nroDeCopias,listaPadreDeBloques);
		}

		if(cantidadDePedidosAlFS>0){
			//reload fullData
			reloadFileFullData(jobSocket, path,nroDeBloques,nroDeCopias,listaPadreDeBloques);
		}

		//Obtengo proximoPedido CON info actualizada
		Message *sendMessage = obtenerProximoPedido(recvMessage);

		//enviar(sendMessage->sockfd,sendMessage->mensaje);
	}

	if(type==K_Job_MapResponse){

		//*data:sizeRutaArchivoTemporal-rutaArchivoTemporal-Respuesta
		char *path = deserializeFilePath(recvMessage,K_Job_MapResponse);
		char *tempPath = deserializeTempFilePath(recvMessage,K_Job_MapResponse);
		t_dictionary *blockState = obtenerBlockState(path,tempPath);
		pedidoRealizado_Bloque = dictionary_get(blockState,K_BlockState_nroBloque);
		pedidoRealizado_Nodo= dictionary_get(blockState,K_BlockState_nroNodo);
		pedidoRealizado_Path=path;
		pedidoRealizado_PathArchTemporal = tempPath;

		int pos = obtenerPosicionDeBloqueEnBlockStatesList(path,pedidoRealizado_Nodo,pedidoRealizado_Bloque);
		bool *requestResponse = deserializeRequestResponse(recvMessage,K_Job_MapResponse);

		if(*requestResponse){

			//actualizo tablas y obtengo prox pedido
			printf("pedido de map realizado con exito, se mapeo la parte nro %d \n",pos);
			actualizarTablas_RtaDeMapExitosa(recvMessage);

			bool *todosMappeados = obtenerEstadoDeReduce();
			if(*todosMappeados){
				Message *sendMessage = obtenerProximoPedido(recvMessage);
			}
			//enviar(sendMessage->sockfd,sendMessage->mensaje);
		}

		if(!(*requestResponse)){
			//REPLANIFICAR
			printf("fallo pedido de map del archivo %s al nodo %s bloque %s \n",pedidoRealizado_Path,pedidoRealizado_Nodo,pedidoRealizado_Bloque);

			//actualizar tablas y reenviar si existen copias
			char *IPnroNodo =  pedidoRealizado_Nodo;
			char *nroBloque =  pedidoRealizado_Bloque;
			//PONER -1 EN COPIAS
			darDeBajaCopiaEnBloqueYNodo(path,recvMessage->sockfd,nroBloque,IPnroNodo,pos);
			decrementarOperacionesEnProcesoEnNodo(IPnroNodo);

			t_dictionary *blockState = obtenerBlockState(path,tempPath);
			StatusBlockState *status = dictionary_get(blockState,K_BlockState_state);
			*status = K_UNINITIALIZED;

			//OBTENER PROXIMO PEDIDO (se va a enviar devuelta el mismo, siempre y cuando haya copias disponibles)
			Message* sendMessage = obtenerProximoPedido(recvMessage);
			free(requestResponse);
			//enviar(sendMessage->sockfd,sendMessage->mensaje);
		}
	}

	if(type==K_Job_ReduceResponse){

		bool *requestResponse = deserializeRequestResponse(recvMessage,K_Job_ReduceResponse);
		char *path = deserializeFilePath(recvMessage,K_Job_ReduceResponse);

		if(*requestResponse){
			//*comando: "reduceFileConCombiner-Pedido1 pathArchivo Respuesta"
			//PLANIFICAR reduceFileConCombiner-Pedido2 // ACTUALIZAR TABLAS !!!!
			int size = list_size(nodosReduceList_Pedido1);
			int i;
			for(i=0;i<size;i++){
				char *ipNodo = list_get(nodosReduceList_Pedido1,i);
				decrementarOperacionesEnProcesoEnNodo(ipNodo);
			}

			Message* sendMessage = obtenerProximoPedido(recvMessage);
			free(requestResponse);
			//enviar(sendMessage->sockfd,sendMessage->mensaje);
		}

		if(!(*requestResponse)){

			//ACTUALIZAR TABLAS !!!!
			char *reduceType = deserializeComando(recvMessage);
			t_list *reduceResponse = deserializeFailedReduceResponse(recvMessage);

			if(strcmp(reduceType,"reduceFileSinCombiner")==0){
				//*data: --> "IPnodo1 IPnodo2..."
				// reduceResponse --> keys --> "listaNodos" "ipNodo"

				//Dar de baja en tabla los ips
				//actualizar Nodo State
				//actualizar el blockStatesList en K_UNINITIALIZED
				char *ipNodo = pedidoRealizado_Nodo;
				decrementarOperacionesEnProcesoEnNodo(ipNodo);

				t_dictionary *fileState = getFileState(path);
				t_list *blockStatesList = dictionary_get(fileState,K_FileState_arrayOfBlocksStates);
				int listSize = list_size(blockStatesList);

				t_list *nodosCaidos =deserializeFailedReduceResponse(recvMessage);
				int sizeNodosCaidos = list_size(nodosCaidos);
				int i,j;
				for(i=0;i<sizeNodosCaidos;i++){
					char *ipNodoCaido = list_get(nodosCaidos,i);
					//darDeBajaCopiaEnBloqueYNodo(ipNodoCaido); --> descomentar !!!!!!!!!
					j=0;
					for(j=0;j<listSize;j++){
						t_dictionary *blockState = list_get(blockStatesList,i);
						char *ip = dictionary_get(blockState,K_BlockState_nroNodo);
						if(strcmp(ip,ipNodoCaido)==0){
							bool *state = dictionary_get(blockState,K_BlockState_state);
							*state = K_UNINITIALIZED;
						}
					}
				}
			}

			if(strcmp(reduceType,"reduceFileConCombiner-Pedido1")==0){
			//*data: --> "IPnodo1 IPnodo2..."
			// reduceResponse --> keys --> "listaNodos" "ipNodo"

			//Dar de baja en tabla los ips
			//actualizar Nodo State
			//actualizar el blockStatesList en K_UNINITIALIZED

				int size = list_size(nodosReduceList_Pedido1);
				int i,j;
				for(i=0;i<size;i++){
					char *ipNodo = list_get(nodosReduceList_Pedido1,i);
					decrementarOperacionesEnProcesoEnNodo(ipNodo);
				}

				t_dictionary *fileState = getFileState(path);
				t_list *blockStatesList = dictionary_get(fileState,K_FileState_arrayOfBlocksStates);
				int listSize = list_size(blockStatesList);

				t_list *nodosCaidos =deserializeFailedReduceResponse(recvMessage);
				int sizeNodosCaidos = list_size(nodosCaidos);
				for(i=0;i<sizeNodosCaidos;i++){
					char *ipNodoCaido = list_get(nodosCaidos,i);
					//darDeBajaCopiaEnBloqueYNodo(ipNodoCaido); --> descomentar !!!!!!!!!
					j=0;
					for(j=0;j<listSize;j++){
						t_dictionary *blockState = list_get(blockStatesList,i);
						char *ip = dictionary_get(blockState,K_BlockState_nroNodo);
						if(strcmp(ip,ipNodoCaido)==0){
							bool *state = dictionary_get(blockState,K_BlockState_state);
							*state = K_UNINITIALIZED;
						}
					}
				}
				Message* sendMessage = obtenerProximoPedido(recvMessage);
				free(requestResponse);
			   //enviar(sendMessage->sockfd,sendMessage->mensaje);
			}

			if(strcmp(reduceType,"reduceFileConCombiner-Pedido2")){
			//*data: --> "Nodo1 Nodo2..."
			// reduceResponse --> keys --> "tempPath" "ipNodo"

				//Dar de baja en tabla los ips
				//actualizar Nodo State
				//actualizar el blockStatesList en K_UNINITIALIZED
				char *ipNodo = pedidoRealizado_Nodo;
				decrementarOperacionesEnProcesoEnNodo(ipNodo);

				t_dictionary *fileState = getFileState(path);
				t_list *blockStatesList = dictionary_get(fileState,K_FileState_arrayOfBlocksStates);
				int listSize = list_size(blockStatesList);

				t_list *nodosCaidos =deserializeFailedReduceResponse(recvMessage);
				int sizeNodosCaidos = list_size(nodosCaidos);
				int i,j;
				for(i=0;i<sizeNodosCaidos;i++){
					char *ipNodoCaido = list_get(nodosCaidos,i);
					//darDeBajaCopiaEnBloqueYNodo(ipNodoCaido); --> descomentar !!!!!!!!!
					j=0;
					for(j=0;j<listSize;j++){
						t_dictionary *blockState = list_get(blockStatesList,i);
						char *ip = dictionary_get(blockState,K_BlockState_nroNodo);
						if(strcmp(ip,ipNodoCaido)==0){
							bool *state = dictionary_get(blockState,K_BlockState_state);
							*state = K_UNINITIALIZED;
						}
					}
				}
			}
		}
	}
}

Message* obtenerProximoPedido(Message *recvMessage)
{
	char *path = pedidoRealizado_Path;
	t_dictionary *fileState = getFileStateForPath(path);
	t_list *blockStateArray = dictionary_get(fileState,K_FileState_arrayOfBlocksStates);
	int size_fileState= list_size(blockStateArray);
	bool *todosMappeados = obtenerEstadoDeReduce();


	if(*todosMappeados){
		//estan todos mappeados
		printf("estan todos mapeados ! \n");

		//Actualizo estados

		//HACER REDUCE !!! --> usar fileState para ver las ubicaciones
		//VER SI SOPORTA COMBINER O NO
		//hacer reduce en el nodo que contenga mas archivos mappeados

		bool *tieneCombinerMode = soportaCombiner(recvMessage->sockfd,path);
		char *IPnroNodoLocal = obtenerNodoConMayorCantidadDeArchivosTemporales(path);
		int cantidadDeNodos = obtenerCantidadDeNodosDiferentesEnBlockState(path);
		t_list *nodosEnBlockState = obtenerNodosEnBlockStateArray(path);

		// iniciar el serializado --> nroNodoLocal-pathTemporalLocal
		char *stream = createStream();
		addStringToStream(&stream,IPnroNodoLocal);

		if(*tieneCombinerMode){

			//planificar con combiner
			char *comando = deserializeComando(recvMessage);

			if((strcmp(comando,"mapFileResponse")==0)){
			//1er pedido
			//***********
			//*comando: "reduceFileConCombiner-Pedido1 pathArchivo"
			//*data:      Nodo1 nombreArchTemp1 CantDeArchEnNodoAProcesar RAT1 RAT2 -etc...-
			//         ...Nodo2 nombreArchTemp2 CantDeArchEnNodoAProcesar RTA1 RAT2 -etc...
			//		   ...Nodo3 ....

				char *_path = deserializeFilePath(recvMessage,K_Job_MapResponse);
				char *comando = createStream();
				addStringToStream(&comando,"reduceFileConCombiner-Pedido1");
				addStringToStream(&comando,_path);

				int i;
				for(i=0;i<cantidadDeNodos;i++){

					char *IPnodoEnBlockState = list_get(nodosEnBlockState,i);
					t_list *pathsTemporalesParaNodo = obtenerPathsTemporalesParaNodo(path, IPnodoEnBlockState);
					int cantidadDePathsTempEnNodo = list_size(pathsTemporalesParaNodo);
					char *_pathTempo = crearPathTemporal(_path);

					//actualizar a tablas
					t_dictionary *reduceBlock = dictionary_create();
					dictionary_put(reduceBlock,"tempPath",_pathTempo);
					dictionary_put(reduceBlock,"ip",IPnodoEnBlockState);
					incrementarOperacionesEnProcesoEnNodo(IPnodoEnBlockState);
					list_add(nodosReduceList_Pedido1,IPnodoEnBlockState);

					//agregar a el serializado --> "Nodo1 nombreArchTemp1 CantDeArchEnNodoAProcesar"
					addStringToStream(&stream,IPnodoEnBlockState);
					addStringToStream(&stream,_pathTempo);
					addIntToStream(stream,cantidadDePathsTempEnNodo,K_int16_t);

					int j;
					for(j=0;j<cantidadDePathsTempEnNodo;j++){

						char *tempPath = list_get(pathsTemporalesParaNodo,j);
						//agregar a el serializado "tempPath"
						addStringToStream(&stream,tempPath);
					}
				}
				Message *sendMessage = armarMensajeParaEnvio(recvMessage,stream,comando);
				return sendMessage;
			}
			//**************************************************************
			//ARMAR 2DO PEDIDO
			if((strcmp(comando,"reduceFileConCombiner-Pedido1")==0)){
			//*****************
			//2do pedido
			//***********
			//*comando: "reduceFileConCombiner-Pedido2 nombreArchTempFinal"
			//*data:      NodoLocal nombreArchTempLocal
			//         ...NodoRemoto1 nombreArchTemp1...
			//		   ...NodoRemoto2 nombreArchTemp2...
			//	       ...etc...
				char *_path = deserializeFilePath(recvMessage,K_Job_MapResponse);
				char *pathTempo = crearPathTemporal(_path);
				char *comando = createStream();
				addStringToStream(&comando,"reduceFileConCombiner-Pedido2");
				addStringToStream(&comando,pathTempo);

				char *stream = createStream();
				//******************************************
				//1ero NodoLocal
				t_dictionary *reduceBlock = list_get(nodosReduceList_Pedido1,0);
				char *ip = dictionary_get(reduceBlock,"ip");
				char *tempPath = dictionary_get(reduceBlock,"tempPath");
				addStringToStream(&stream,ip);
				addStringToStream(&stream,tempPath);
				pedidoRealizado_Nodo = ip;
				//******************************************
				int size = list_size(nodosReduceList_Pedido1);
				int i;
				for(i=1;i<size;i++){
					t_dictionary *reduceBlock = list_get(nodosReduceList_Pedido1,i);
					char *ip = dictionary_get(reduceBlock,"ip");
					char *tempPath = dictionary_get(reduceBlock,"tempPath");
					addStringToStream(&stream,ip);
					addStringToStream(&stream,tempPath);
				}
				Message *sendMessage = armarMensajeParaEnvio(recvMessage,stream,comando);
				return sendMessage;
			}
		}

		if(!(*tieneCombinerMode)){
			//planificar sin combiner

			//*comando: "reduceFileSinCombiner NombreArchTempFinal "
			//*data:	    NodoLocal  CantDeArchEnNodoLocalAProcesar RAT1 RAT2-...etc...-
			//           ...NodoRemoto1 CantDeArchEnNodoRemotoAProcesar RTA1 RAT2 RAT3 -etc...
			//	         ...NodoRemoto2-...."

			//****************************************************
			//PRIMERO PONER LOS PATH CORRESPONDIENTES AL NODO LOCAL

			t_list *pathsTemporalesParaNodoLocal = obtenerPathsTemporalesParaNodo(path, IPnroNodoLocal);
			int cantidadDePathsTempEnNodoLocal = list_size(pathsTemporalesParaNodoLocal);
			addIntToStream(stream,cantidadDePathsTempEnNodoLocal,K_int16_t);

			int k;
			for(k=0;k<cantidadDePathsTempEnNodoLocal;k++){

				char *tempPath = list_get(pathsTemporalesParaNodoLocal,k);
				//agregar a el serializado "tempPath"
				addStringToStream(&stream,tempPath);
			}
			//*****************************************************
			int i;
			for(i=0;i<cantidadDeNodos;i++){
				char *IPnodoEnBlockState = list_get(nodosEnBlockState,i);

				if( strcmp(IPnodoEnBlockState,IPnroNodoLocal) != 0 ){

					t_list *pathsTemporalesParaNodo = obtenerPathsTemporalesParaNodo(path, IPnodoEnBlockState);
					int cantidadDePathsTempEnNodo = list_size(pathsTemporalesParaNodo);

					//agregar a el serializado --> "cantidadDePathsTempEnNodo"
					addStringToStream(&stream,IPnodoEnBlockState);
					addIntToStream(stream,cantidadDePathsTempEnNodo,K_int16_t);

					int j;
					for(j=0;j<cantidadDePathsTempEnNodo;j++){

						char *tempPath = list_get(pathsTemporalesParaNodo,j);
						//agregar a el serializado "tempPath"
						addStringToStream(&stream,tempPath);
					}
				}
			}
			printf("el archivo %s no soporta Combiner\n",pedidoRealizado_Path);
			printf("el pedido de reduce es : %s\n",stream);
			//actualizar Tablas !!
			incrementarOperacionesEnProcesoEnNodo(IPnroNodoLocal);
			//setBlockStatesListInReducingState(path);

			pedidoRealizado_Nodo = IPnroNodoLocal;

		}

		Message *sendMessage = armarMensajeParaEnvio(recvMessage,stream,"reduceFile");
		return sendMessage;
	}

	if(!(*todosMappeados)){

		char *path = pedidoRealizado_Path;
		t_dictionary *fileState = getFileStateForPath(path);
		t_list *blockStateArray = dictionary_get(fileState,K_FileState_arrayOfBlocksStates);
		int size_fileState= list_size(blockStateArray);
		char *stream = createStream();

		int i;
		for(i=0;i<size_fileState;i++){

			t_dictionary *blockState = list_get(blockStateArray,i);
			StatusBlockState *status = dictionary_get(blockState,K_BlockState_state);

			if((*status)==K_UNINITIALIZED){
				//obtengo conjunto (#ipNodo;#nroBloque) con menos carga de operaciones para bloque en pos "i"
				t_dictionary *copiaConMenosCarga = obtenerCopiaConMenosCargaParaBloque(recvMessage,path,i);
				char *IPnroDeNodo = dictionary_get(copiaConMenosCarga,K_Copia_IPNodo);

				if( strcmp(IPnroDeNodo,K_Copia_DarDeBajaIPNodo) == 0){//checkeo q haya copias disponibles

					//NO HAY COPIAS DISPONIBLES !!!!!
					if(cantidadDePedidosAlFS==0){
						printf("no hay mas copias disponibles!!!\n");
						printf("hacer pedido FS_FileFullData aver si al FileSystem se le cargo nuevos nodos !\n");
						Message *fsRequest = createFSrequest();
						cantidadDePedidosAlFS++;
						return fsRequest;
					}else{
						printf("no hay mas copias disponibles!!!\n");
						printf("ya se le hizo un pedido al FS previamente, MaRTA no puede continuar con la operacion \n");
						//HACER UN RETURN ACA !!!
					}
				}

				//armo pedido de map, segun protocolo es

				//-->MaRTA le dice al Job que haga una rutina de mapping
				//*comando: "mapFile nombreArchivo"
				//*data:   direccionNodo puertoEscucha nroDeBloque rutaArchivoTemporal
				//		...direccionNodo puertoEscucha nroDeBloque rutaArchivoTemporal...
				//		...direccionNodo puertoEscucha nroDeBloque rutaArchivoTemporal...

				//char *path_with_temporal = crearPathTemporal(path);
				//*******************
				char *temporal = intToCharPtr(i);
				char *path_with_temporal = string_new();
				string_append(&path_with_temporal,path);
				string_append(&path_with_temporal,"-");
				string_append(&path_with_temporal,temporal);
				//******************
				char *ipNodo = dictionary_get(copiaConMenosCarga,K_Copia_IPNodo);
				char *nroDeBloque = dictionary_get(copiaConMenosCarga,K_Copia_NroDeBloque);

				addStringToStream(&stream,ipNodo);
				addStringToStream(&stream,nroDeBloque);
				addStringToStream(&stream,path_with_temporal);

				printf("la copia con menos carga tiene nroDeBloque %s \n",nroDeBloque);

				//ACTUALIZAR PedidoRealizado, NODOState y BLOCKState
				incrementarOperacionesEnProcesoEnNodo(ipNodo);

				//actualizoBlockState
				StatusBlockState *status = malloc(sizeof(StatusBlockState));
				*status = K_IN_MAPPING;

				dictionary_clean(blockState);
				dictionary_put(blockState,K_BlockState_state,status);
				dictionary_put(blockState,K_BlockState_nroNodo,ipNodo);
				dictionary_put(blockState,K_BlockState_nroBloque,nroDeBloque);
				dictionary_put(blockState,K_BlockState_temporaryPath,path_with_temporal);

			}
		}

		printf("se envia pedido de map, el stream es %s\n",stream);
		Message *msjParaEnviar = armarMensajeParaEnvio(recvMessage,stream,"mapFile");
		return msjParaEnviar;
	}
}

char* crearPathTemporal(char *path){
	char *temporal = temporal_get_string_time();
	char *path_with_temporal = string_new();
	string_append(&path_with_temporal,path);
	string_append(&path_with_temporal,"-");
	string_append(&path_with_temporal,temporal);
	return path_with_temporal;
}
bool *obtenerEstadoDeReduce()
{
	char *path = pedidoRealizado_Path;
	t_dictionary *fileState = getFileStateForPath(path);
	t_list *blockStateArray = dictionary_get(fileState,K_FileState_arrayOfBlocksStates);
	int size_fileState= list_size(blockStateArray);

	bool *todosMappeados = malloc(sizeof(bool));
	*todosMappeados = true;
	int i;
	for(i=0;i<size_fileState;i++){
		t_dictionary *blockState = list_get(blockStateArray,i);
		StatusBlockState *statusBlock = dictionary_get(blockState,K_BlockState_state);
		if((*statusBlock)!= K_MAPPED){
			*todosMappeados = false;
		}
	}
	return todosMappeados;
}

t_dictionary *obtenerCopiaConMenosCargaParaBloque(Message *recvMessage,char *path,int bloqueNro){

	t_list *copias = obtenerCopiasParaBloqueDeArchivo(jobSocket,bloqueNro,path);
	int nroDeCopias = list_size(copias);

	t_dictionary *copiaConMenosCarga = list_get(copias,0);
	int j;
	for(j=1;j<(nroDeCopias);j++){

		//obtengo nodo con menos carga de operaciones

		t_dictionary *copia = list_get(copias,j);

		char *IPnroNodoCopia = dictionary_get(copia,K_Copia_IPNodo);
		char *IPnroNodoCopiaPivot = dictionary_get(copiaConMenosCarga,K_Copia_IPNodo);

		if((strcmp(IPnroNodoCopia,K_Copia_DarDeBajaIPNodo) == 0) &&
				(strcmp(IPnroNodoCopiaPivot,K_Copia_DarDeBajaIPNodo)==0) ){
			//ninguno de los nodos esta disponible, dejo la copia ya asignada

		}else if((strcmp(IPnroNodoCopia,K_Copia_DarDeBajaIPNodo)== 0)
					&& (strcmp(IPnroNodoCopiaPivot,K_Copia_DarDeBajaIPNodo)!=0)){

			//dejo la copia pivot

		}else if((strcmp(IPnroNodoCopia,K_Copia_DarDeBajaIPNodo)!=0) &&
					(strcmp(IPnroNodoCopiaPivot,K_Copia_DarDeBajaIPNodo)==0)){

			copiaConMenosCarga = copia;
		}else{

			int opsEnNodoCopia = getCantidadDeOperacionesEnProcesoEnNodo(IPnroNodoCopia);
			int opsEnNodoCopiaPivot = getCantidadDeOperacionesEnProcesoEnNodo(IPnroNodoCopiaPivot);

			if(opsEnNodoCopia < opsEnNodoCopiaPivot){
				copiaConMenosCarga = copia;
			}else{
				//la copiaPivot es la que tiene menos carga. La dejo.
			}
		}
	}

	return copiaConMenosCarga;
}

void actualizarTablas_RtaDeMapExitosa(Message *recvMessage)
{
	//*data:sizeRutaArchivoTemporal-rutaArchivoTemporal-Respuesta

	//*comando: "reduceFileConCombiner-Pedido1 pathArchivo"
				//*comando: "reduceFileConCombiner-Pedido2 pathArchivo"
				//*comando: "reduceFileSinCombiner NombreArchTempFinal"

	char *temporaryPath = pedidoRealizado_PathArchTemporal;
	char *IPnroNodo =  pedidoRealizado_Nodo;
	char *nroBloque =  pedidoRealizado_Bloque;
	int pos = obtenerPosicionDeBloqueEnBlockStatesList(pedidoRealizado_Path,pedidoRealizado_Nodo,pedidoRealizado_Bloque);

	t_dictionary *fileState = getFileStateForPath(pedidoRealizado_Path);
	t_list *blockStatesList = dictionary_get(fileState,K_FileState_arrayOfBlocksStates);
	t_dictionary *blockState = list_get(blockStatesList,pos);

	//******************************************
	//Actualizo blockState
	char *ptrNodo = malloc(strlen(IPnroNodo));
	ptrNodo=pedidoRealizado_Nodo;
	char *ptrBlq = malloc(strlen(nroBloque));
	ptrBlq=pedidoRealizado_Bloque;
	char *ptrPathTempo = malloc(strlen(temporaryPath));
	ptrPathTempo=pedidoRealizado_PathArchTemporal;

	StatusBlockState *state = malloc(sizeof(StatusBlockState));
	*state=K_MAPPED;
	dictionary_destroy(blockState);

	t_dictionary *newBlckState = dictionary_create();
	dictionary_put(newBlckState,K_BlockState_nroBloque,ptrBlq);
	dictionary_put(newBlckState,K_BlockState_nroNodo,ptrNodo);
	dictionary_put(newBlckState,K_BlockState_temporaryPath,ptrPathTempo);
	dictionary_put(newBlckState,K_BlockState_state,state);

	list_remove(blockStatesList,pos);
	list_add_in_index(blockStatesList,pos,newBlckState);
	//******************************************
	//Actualizo nodoState
	decrementarOperacionesEnProcesoEnNodo(IPnroNodo);
	addTemporaryFilePathToNodoData(IPnroNodo,temporaryPath);

}

Message *createFSrequest(){

	char *path = pedidoRealizado_Path;
	Message *fsRequest = malloc(sizeof(Message));
	fsRequest->mensaje = malloc(sizeof(mensaje_t));

	//armo stream a mano
	char *stream = string_new();
	string_append(&stream,"DataFile ");
	string_append(&stream,path);

	fsRequest->sockfd = getFSSocket();
	fsRequest->mensaje->comandoSize = (int16_t)strlen(stream);
	fsRequest->mensaje->comando = stream;
	fsRequest->mensaje->dataSize = 0;
	fsRequest->mensaje->data = "";
	return fsRequest;
}
Message* armarMensajeParaEnvio(Message *recvMessage,void *stream,char *comando)
{
	Message *msjParaEnvio = malloc(sizeof(Message));
	msjParaEnvio->mensaje = malloc(sizeof(mensaje_t));
	msjParaEnvio->mensaje->comando = malloc(strlen(comando));
	msjParaEnvio->mensaje->data = malloc(strlen(stream));

	msjParaEnvio->sockfd = recvMessage->sockfd;
	msjParaEnvio->mensaje->comandoSize = (int16_t)strlen(comando);
	msjParaEnvio->mensaje->comando = comando;
	msjParaEnvio->mensaje->dataSize = (int32_t)strlen(stream);
	msjParaEnvio->mensaje->data = stream;

	return msjParaEnvio;
}

// Idea de planificaion : se hara map y reduce de los bloques en orden ascendente.
// Hago map de todos los bloques de un archivo y ahi lanzo el reduce.
// Habra un MAXIMO de envios por la red, hasta que no responda un envio, nose hara otro(Productor-Consumidor)
// Si vuelve con error, entonces se rePlanifica y se vuelve a mandar el mismo pero a otro NODO.
// Si NO hay otro NODO disponible entonces, se informa que el proceso del archivo quedo incompleto--> no se que hacer luego.
// Si la operacion enviada falla porque se cayo el Nodo o simplemente fallo entonces se RE-PLANIFICA, si se cae el Job NO hay nada mas que hacer.
