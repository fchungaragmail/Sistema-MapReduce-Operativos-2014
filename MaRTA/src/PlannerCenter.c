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
#include <unistd.h>

// VARIABLES GLOBALES
int jobSocket;
char *filePathAProcesar;
t_list *nodosReduceList_Pedido1;
t_list *listaDeNodos_EnCasoDeFalloDeJob;
int pedidosDeFullDataRealizados;

//-->PedidoRealizado
char *ipNodoLocalDePedidoDeReduce;

// FUNCIONES PRIVADAS
int obtenerIdParaComando(Message *recvMessage);
void planificar(Message *recvMessage,TypesMessages type);
void actualizarPedidoRealizado(char *bloque, char* ipnodo, char* path, char *pathTemporal, StatusBlockState tipo );
Message* obtenerProximoPedido(Message *recvMessage);
t_dictionary *obtenerCopiaConMenosCargaParaBloque(Message *recvMessage,char *path,int bloqueNro);
char* crearPathTemporal(char *path);
Message* armarMensajeParaEnvio(Message *recvMessage,void *stream,char *comando);
void actualizarTablas_RtaDeMapExitosa(Message *recvMessage);
Message *createFSrequest();
bool *obtenerEstadoDeReduce(Message *msj);
void actualizarTablas_RtaDeMapFallo(Message *recvMessage);
void decrementarOperacionesEnReduceList();
void resetBlockStateConNodo(char *path,char *ipNodoCaido);
void actualizarTablas_ReduceFallo(char *path,Message *recvMessage);
void liberarMensaje(Message *msj);
void liberarNodosReduceList_Pedido1();
void liberarFileState_Y_nodoState(Message *recvMessage);
Message *crearMensajeAJobDeFinalizado(Message *msj);
void sacarCargasDeNodos_FalloDeJob();

// FUNCIONES PUBLICAS
void initPlannerCenter();
bool processMessage(Message *recvMessage);

void initPlannerCenter()
{
	initFilesStatusCenter();
	jobSocket=0;
	pedidosDeFullDataRealizados=0;
	nodosReduceList_Pedido1 = list_create();
	listaDeNodos_EnCasoDeFalloDeJob = list_create();
}

bool processMessage(Message *recvMessage)
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
			filePathAProcesar = filePath;
			bool *soportaCombiner = deserializeSoportaCombiner(recvMessage);
			//******************
			// --> se agrega un file_StatusData a filesToProcessPerJob
			addNewFileForProcess(filePath,soportaCombiner,recvMessage->sockfd);
			//******************
			planificar(recvMessage,K_Job_NewFileToProcess);

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
				free(tempPath);
				return true;
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
			printf("PlannerCenter : planificar Job_ReduceResponse\n");

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

				// Actualizar tablas !!
				//LIBERAR MEMORIA !!!!!!
				liberarNodosReduceList_Pedido1();
				liberarFileState_Y_nodoState(recvMessage);
				decrementarOperacionesEnProcesoEnNodo(ipNodoLocalDePedidoDeReduce);
				printf("archivo %s reducido con exito !\n",path);

				Message *jobMsj = crearMensajeAJobDeFinalizado(recvMessage);
				//enviar(jobMsj->sockfd,jobMsj->mensaje);
				return true;
			}

			//TERMINO 1ER PEDIDO
			if((*_response)&&(strcmp(reduceType,"reduceFileConCombiner-Pedido1")==0)){

				planificar(recvMessage,K_Job_ReduceResponse);
			}
			//FALLO PEDIDO
			if(!(*_response)){

				planificar(recvMessage,K_Job_ReduceResponse);
			}

			free(path);
			free(reduceType);
			free(_response);
			printf("***************\n");
			break;
		case K_Job_JobCaido:

			printf("el Job con socket %d se cayo, MaRTA no puede seguir procesando sus archivos \n",recvMessage->sockfd);
			liberarFileState_Y_nodoState(recvMessage);
			sacarCargasDeNodos_FalloDeJob();
			return true;

			break;
		default:

			printf("PlannerCenter: ERROR !! Comando no identificado !!\n");
			printf("***************\n");

			break;
	}

	liberarMensaje(recvMessage);
	return false;
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
	if(strcmp(comando,"JobCaido")==0){type = K_Job_JobCaido;}

	free(comando);
	return type;
}


//Planificacion
void planificar(Message *recvMessage,TypesMessages type)
{

	if(type == K_Job_NewFileToProcess){
		//pido al FS la tabla de direcciones del archivo
		//segun protocolo ---> -Comando: "DataFile rutaDelArchivo" /// -Data: Vacio

		Message *fsRequest = createFSrequest(recvMessage);
		//enviar(fsRequest->sockfd,fsRequest->mensaje);
		liberarMensaje(fsRequest);
	}

	if(type==K_FS_FileFullData){

		//--> FS responde con tabla de archivo pedida
		//-Comando: "DataFileResponse rutaDelArchivo Respuesta cantidadDeBloques-nroDeCopias-sizeEstructura-"

		char *path = deserializeFilePath(recvMessage,K_FS_FileFullData);

		if(pedidosDeFullDataRealizados==0){
			//se completa filesToProcess y se crea un fileState
			addFileFullData(jobSocket, path,recvMessage);
		}
		if(pedidosDeFullDataRealizados>0){
			t_list *listaPadreDeBloques = deserializarFullDataResponse(recvMessage);
			reloadFileFullData(jobSocket,path,listaPadreDeBloques);
		}

		//Obtengo proximoPedido CON info actualizada
		Message *sendMessage = obtenerProximoPedido(recvMessage);
		//enviar(sendMessage->sockfd,sendMessage->mensaje);

		liberarMensaje(sendMessage);
		free(path);
	}

	if(type==K_Job_MapResponse){

		//*comando : "mapFileResponse rutaArchivoTemporal Respuesta"

		list_clean(listaDeNodos_EnCasoDeFalloDeJob);
		char *path = deserializeFilePath(recvMessage,K_Job_MapResponse);
		char *tempPath = deserializeTempFilePath(recvMessage,K_Job_MapResponse);
		t_dictionary *blockState = obtenerBlockState(path,tempPath,recvMessage->sockfd);

		char *nroDeBloque = dictionary_get(blockState,K_BlockState_nroBloque);
		char *ipNodo= dictionary_get(blockState,K_BlockState_nroNodo);

		int pos = obtenerPosicionDeBloqueEnBlockStatesList(path,ipNodo,nroDeBloque,recvMessage->sockfd);
		bool *requestResponse = deserializeRequestResponse(recvMessage,K_Job_MapResponse);

		if(*requestResponse){

			//actualizo tablas y obtengo prox pedido
			actualizarTablas_RtaDeMapExitosa(recvMessage);
			bool *todosMappeados = obtenerEstadoDeReduce(recvMessage);
			if(*todosMappeados){
				Message *sendMessage = obtenerProximoPedido(recvMessage);
				//enviar(sendMessage->sockfd,sendMessage->mensaje);
			}
			free(todosMappeados);
		}

		if(!(*requestResponse)){

			//REPLANIFICAR
			char *nroDeBloque = dictionary_get(blockState,K_BlockState_nroBloque);
			char *ipNodo= dictionary_get(blockState,K_BlockState_nroNodo);
			int pos = obtenerPosicionDeBloqueEnBlockStatesList(path,ipNodo,nroDeBloque,recvMessage->sockfd);
			printf("fallo pedido de map del archivo %s al nodo %s bloque %s en posicion %d\n",path,ipNodo,nroDeBloque,pos);
			printf("el nodo %s seguramente esta caido\n",ipNodo);

			actualizarTablas_RtaDeMapFallo(recvMessage);

			//OBTENER PROXIMO PEDIDO (se va a enviar devuelta el mismo, siempre y cuando haya copias disponibles)
			Message* sendMessage = obtenerProximoPedido(recvMessage);
			//enviar(sendMessage->sockfd,sendMessage->mensaje);
		}

		free(requestResponse);
	}

	if(type==K_Job_ReduceResponse){

		list_clean(listaDeNodos_EnCasoDeFalloDeJob);
		bool *requestResponse = deserializeRequestResponse(recvMessage,K_Job_ReduceResponse);
		char *path = deserializeFilePath(recvMessage,K_Job_ReduceResponse);

		if(*requestResponse){
			//PLANIFICAR reduceFileConCombiner-Pedido2 // ACTUALIZAR TABLAS !!!!
			//*comando: "reduceFileConCombiner-Pedido1 pathArchivo Respuesta"
			printf("reduce-Pedido1 realizado con exito \n");
			decrementarOperacionesEnReduceList();
			Message* sendMessage = obtenerProximoPedido(recvMessage);
			//enviar(sendMessage->sockfd,sendMessage->mensaje);
			liberarMensaje(sendMessage);
		}

		if(!(*requestResponse)){

			//*data: --> "IPnodo1 IPnodo2..."
			//Dar de baja en tabla los ips
			//actualizar Nodo State
			//actualizar el blockStatesList en K_UNINITIALIZED

			char *reduceType = deserializeComando(recvMessage);
			t_list *reduceResponse = deserializeFailedReduceResponse(recvMessage);
			Message* sendMessage;

			if(strcmp(reduceType,"reduceFileSinCombiner")==0){

				printf("reduceSinCombiner fallo \n");

				char *ipNodo = ipNodoLocalDePedidoDeReduce;
				decrementarOperacionesEnProcesoEnNodo(ipNodo);
				actualizarTablas_ReduceFallo(path,recvMessage);
				//VACIAR LISTA DE PEDIDOS!!!!
				sendMessage = obtenerProximoPedido(recvMessage);
			   //enviar(sendMessage->sockfd,sendMessage->mensaje);
			}

			if(strcmp(reduceType,"reduceFileConCombiner-Pedido1")==0){

				printf("reduce-Pedido1 fallo \n");
				decrementarOperacionesEnReduceList();
				actualizarTablas_ReduceFallo(path,recvMessage);
				sendMessage = obtenerProximoPedido(recvMessage);
			   //enviar(sendMessage->sockfd,sendMessage->mensaje);
				liberarNodosReduceList_Pedido1();

			}

			if(strcmp(reduceType,"reduceFileConCombiner-Pedido2")==0){

				printf("reduce-Pedido2 fallo \n");
				char *ipNodo = ipNodoLocalDePedidoDeReduce;
				decrementarOperacionesEnProcesoEnNodo(ipNodo);
				actualizarTablas_ReduceFallo(path,recvMessage);
				liberarNodosReduceList_Pedido1();

				sendMessage = obtenerProximoPedido(recvMessage);
				//enviar(sendMessage->sockfd,sendMessage->mensaje);
			}
			liberarMensaje(sendMessage);
		}

		free(requestResponse);
		free(path);
	}
}

Message* obtenerProximoPedido(Message *recvMessage)
{
	int tipoComando = obtenerIdParaComando(recvMessage);
	char *path = deserializeFilePath(recvMessage,tipoComando);
	t_dictionary *fileState = getFileStateForPath(path,jobSocket);
	t_list *blockStateArray = dictionary_get(fileState,K_FileState_arrayOfBlocksStates);
	int size_fileState= list_size(blockStateArray);
	bool *todosMappeados = obtenerEstadoDeReduce(recvMessage);
	Message *msjParaEnviar;
	informarTareasPendientesDeMapping(path,jobSocket);

	if(*todosMappeados){
		//estan todos mappeados
		printf("estan todos mapeados ! \n");

		bool *tieneCombinerMode = soportaCombiner(recvMessage->sockfd,path);
		int cantidadDeNodos = obtenerCantidadDeNodosDiferentesEnBlockState(path,recvMessage->sockfd);
		t_list *nodosEnBlockState = obtenerNodosEnBlockStateArray(path,recvMessage->sockfd);
		char *finalStream;

		if(*tieneCombinerMode){

			//planificar con combiner
			char *comandoResponse = deserializeComando(recvMessage);

			if((strcmp(comandoResponse,"mapFileResponse")==0)){
			//1er pedido
			//***********
			//*comando: "reduceFileConCombiner-Pedido1 pathArchivo"
			//*data:      Nodo1 nombreArchTemp1 CantDeArchEnNodoAProcesar RAT1 RAT2 -etc...-
			//         ...Nodo2 nombreArchTemp2 CantDeArchEnNodoAProcesar RTA1 RAT2 -etc...
			//		   ...Nodo3 ....

				char *_path = deserializeFilePath(recvMessage,K_Job_MapResponse);
				char *command= createStream();
				char *stream = createStream();
				addStringToStream(&command,"reduceFileConCombiner-Pedido1");
				addStringToStream(&command,_path);

				int i;
				for(i=0;i<cantidadDeNodos;i++){

					t_dictionary *nodo= list_get(nodosEnBlockState,i);
					char *IPnodoEnBlockState  = dictionary_get(nodo,"ip");
					char *puertonodoEnBlockState  = dictionary_get(nodo,"puerto");
					t_list *pathsTemporalesParaNodo = obtenerPathsTemporalesParaArchivo(path, IPnodoEnBlockState,recvMessage->sockfd);
					int cantidadDePathsTempEnNodo = list_size(pathsTemporalesParaNodo);
					char *_pathTempo = crearPathTemporal(_path);

					//actualizar a tablas
					t_dictionary *reduceBlock = dictionary_create();
					dictionary_put(reduceBlock,"reduceBlock_tempPath",_pathTempo);
					dictionary_put(reduceBlock,"reduceBlock_ip",IPnodoEnBlockState);
					dictionary_put(reduceBlock,"reduceBlock_puerto",puertonodoEnBlockState);
					list_add(nodosReduceList_Pedido1,reduceBlock);
					list_add(listaDeNodos_EnCasoDeFalloDeJob,IPnodoEnBlockState);
					incrementarOperacionesEnProcesoEnNodo(IPnodoEnBlockState);

					//agregar a el serializado --> "Nodo1 nombreArchTemp1 CantDeArchEnNodoAProcesar"
					addStringToStream(&stream,IPnodoEnBlockState);
					addStringToStream(&stream,puertonodoEnBlockState);
					addStringToStream(&stream,_pathTempo);
					addIntToStream(stream,cantidadDePathsTempEnNodo,K_int16_t);

					int j;
					for(j=0;j<cantidadDePathsTempEnNodo;j++){

						char *tempPath = list_get(pathsTemporalesParaNodo,j);
						//agregar a el serializado "tempPath"
						addStringToStream(&stream,tempPath);
					}
					list_destroy(pathsTemporalesParaNodo);

				}
				printf("el Pedido1 de ReduceConCombiner es : %s\n",stream);
				msjParaEnviar = armarMensajeParaEnvio(recvMessage,stream,command);
				finalStream=stream;

				free(command);
			}

			//**************************************************************

			//ARMAR 2DO PEDIDO
			if((strcmp(comandoResponse,"reduceFileConCombiner-Pedido1")==0)){
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
				char *stream = createStream();
				addStringToStream(&comando,"reduceFileConCombiner-Pedido2");
				addStringToStream(&comando,pathTempo);
				free(_path);
				free(pathTempo);

				//******************************************
				//1ero NodoLocal --> tomo el 1ero porque si
				t_dictionary *reduceBlock = list_get(nodosReduceList_Pedido1,0);
				char *ip = dictionary_get(reduceBlock,"reduceBlock_ip");
				char *puerto = dictionary_get(reduceBlock,"reduceBlock_puerto");
				char *tempPath = dictionary_get(reduceBlock,"reduceBlock_tempPath");
				addStringToStream(&stream,ip);
				addStringToStream(&stream,puerto);
				addStringToStream(&stream,tempPath);
				ipNodoLocalDePedidoDeReduce = ip;
				incrementarOperacionesEnProcesoEnNodo(ip);
				list_add(listaDeNodos_EnCasoDeFalloDeJob,ip);
				//******************************************
				int size = list_size(nodosReduceList_Pedido1);
				int i;
				for(i=1;i<size;i++){
					t_dictionary *reduceBlock = list_get(nodosReduceList_Pedido1,i);
					char *ip = dictionary_get(reduceBlock,"reduceBlock_ip");
					char *puerto = dictionary_get(reduceBlock,"reduceBlock_puerto");
					char *tempPath = dictionary_get(reduceBlock,"reduceBlock_tempPath");
					addStringToStream(&stream,ip);
					addStringToStream(&stream,puerto);
					addStringToStream(&stream,tempPath);
					list_add(listaDeNodos_EnCasoDeFalloDeJob,ip);
				}
				printf("el Pedido2 de ReduceConCombiner es : %s\n",stream);
				finalStream=stream;
				msjParaEnviar = armarMensajeParaEnvio(recvMessage,stream,comando);

			}
			free(comandoResponse);
		}

		if(!(*tieneCombinerMode)){
			//planificar sin combiner

			//*comando: "reduceFileSinCombiner NombreArchTempFinal "
			//*data:	    NodoLocal  CantDeArchEnNodoLocalAProcesar RAT1 RAT2-...etc...-
			//           ...NodoRemoto1 CantDeArchEnNodoRemotoAProcesar RTA1 RAT2 RAT3 -etc...
			//	         ...NodoRemoto2-...."

			//****************************************************
			//PRIMERO PONER LOS PATH CORRESPONDIENTES AL NODO LOCAL

			t_dictionary *nodoLocal = obtenerNodoConMayorCantidadDeArchivosTemporales(path,recvMessage->sockfd);
			char *IPnroNodoLocal = dictionary_get(nodoLocal,"ip");
			char *puertoNodoLocal = dictionary_get(nodoLocal,"puerto");
			t_list *pathsTemporalesParaNodoLocal = obtenerPathsTemporalesParaArchivo(path, IPnroNodoLocal,recvMessage->sockfd);
			int cantidadDePathsTempEnNodoLocal = list_size(pathsTemporalesParaNodoLocal);
			list_add(listaDeNodos_EnCasoDeFalloDeJob,IPnroNodoLocal);

			char *stream = createStream();
			addStringToStream(&stream,IPnroNodoLocal);
			addStringToStream(&stream,puertoNodoLocal);
			addIntToStream(stream,cantidadDePathsTempEnNodoLocal,K_int16_t);

			int k;
			for(k=0;k<cantidadDePathsTempEnNodoLocal;k++){

				char *tempPath = list_get(pathsTemporalesParaNodoLocal,k);
				//agregar a el serializado "tempPath"
				addStringToStream(&stream,tempPath);
			}
			list_destroy(pathsTemporalesParaNodoLocal);
			//*****************************************************
			int i;
			for(i=0;i<cantidadDeNodos;i++){
				t_dictionary *nodo = list_get(nodosEnBlockState,i);
				char *IPnodoEnBlockState = dictionary_get(nodo,"ip");
				char *puertonodoEnBlockState = dictionary_get(nodo,"puerto");

				if( strcmp(IPnodoEnBlockState,IPnroNodoLocal) != 0 ){

					t_list *pathsTemporalesParaNodo = obtenerPathsTemporalesParaArchivo(path, IPnodoEnBlockState,recvMessage->sockfd);
					int cantidadDePathsTempEnNodo = list_size(pathsTemporalesParaNodo);
					list_add(listaDeNodos_EnCasoDeFalloDeJob,IPnodoEnBlockState);

					//agregar a el serializado --> "cantidadDePathsTempEnNodo"
					addStringToStream(&stream,IPnodoEnBlockState);
					addStringToStream(&stream,puertonodoEnBlockState);
					addIntToStream(stream,cantidadDePathsTempEnNodo,K_int16_t);

					int j;
					for(j=0;j<cantidadDePathsTempEnNodo;j++){

						char *tempPath = list_get(pathsTemporalesParaNodo,j);
						//agregar a el serializado "tempPath"
						addStringToStream(&stream,tempPath);
					}
					list_destroy(pathsTemporalesParaNodo);
				}
			}
			char *path = deserializeFilePath(recvMessage,K_Job_MapResponse);
			printf("el archivo %s no soporta Combiner\n",path);
			printf("el pedido de reduce es : %s\n",stream);

			//actualizar Tablas !!
			incrementarOperacionesEnProcesoEnNodo(IPnroNodoLocal);
			//setBlockStatesListInReducingState(path);
			ipNodoLocalDePedidoDeReduce = IPnroNodoLocal;
			finalStream=stream;
			msjParaEnviar = armarMensajeParaEnvio(recvMessage,finalStream,"reduceFile");
		}
	}

	if(!(*todosMappeados)){

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
					printf("no hay mas copias disponibles!!!\n");
					printf("hacer pedido FS_FileFullData aver si al FileSystem se le cargo nuevos nodos !\n");
					Message *fsRequest = createFSrequest(recvMessage);
					pedidosDeFullDataRealizados++;
					return fsRequest;
				}

				//armo pedido de map, segun protocolo es
				//*comando: "mapFile nombreArchivo"
				//*data:   direccionNodo puertoEscucha nroDeBloque rutaArchivoTemporal

				//char *path_with_temporal = crearPathTemporal(path);
				//*******************
				char *temporal = intToCharPtr(i);
				char *path_with_temporal = string_new();
				string_append(&path_with_temporal,path);
				string_append(&path_with_temporal,"-");
				string_append(&path_with_temporal,temporal);
				//******************
				char *ipNodo = dictionary_get(copiaConMenosCarga,K_Copia_IPNodo);
				char *puertoNodo = dictionary_get(copiaConMenosCarga,K_Copia_PuertoNodo);
				char *nroDeBloque = dictionary_get(copiaConMenosCarga,K_Copia_NroDeBloque);

				addStringToStream(&stream,ipNodo);
				addStringToStream(&stream,puertoNodo);
				addStringToStream(&stream,nroDeBloque);
				addStringToStream(&stream,path_with_temporal);

				//ACTUALIZAR NODOState y BLOCKState
				incrementarOperacionesEnProcesoEnNodo(ipNodo);

				//actualizoBlockState
				StatusBlockState *status = dictionary_get(blockState,K_BlockState_state);
				*status = K_IN_MAPPING;
				dictionary_clean(blockState);
				dictionary_put(blockState,K_BlockState_state,status);
				dictionary_put(blockState,K_BlockState_nroNodo,ipNodo);
				dictionary_put(blockState,K_BlockState_nroBloque,nroDeBloque);
				dictionary_put(blockState,K_BlockState_temporaryPath,path_with_temporal);
				dictionary_put(blockState,K_BlockState_puertoNodo,puertoNodo);

				list_add(listaDeNodos_EnCasoDeFalloDeJob,ipNodo);
			}
		}

		printf("se envia pedido de map, el stream es %s\n",stream);
		msjParaEnviar = armarMensajeParaEnvio(recvMessage,stream,"mapFile");
	}
	free(path);
	free(todosMappeados);
	return msjParaEnviar;
}

char* crearPathTemporal(char *path){

	usleep(10000);// sino me tira todos los "temporal_get_string_time()" iguales
	char *temporal = temporal_get_string_time();
	char *path_with_temporal = string_new();
	string_append(&path_with_temporal,path);
	string_append(&path_with_temporal,"-");
	string_append(&path_with_temporal,temporal);
	return path_with_temporal;
}
bool *obtenerEstadoDeReduce(Message *msj)
{
	int tipoDeCommando = obtenerIdParaComando(msj);
	char *path = deserializeFilePath(msj,tipoDeCommando);
	t_dictionary *fileState = getFileStateForPath(path,jobSocket);
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
			break;
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

	char *path = deserializeFilePath(recvMessage,K_Job_MapResponse);
	char *temporaryPath = deserializeTempFilePath(recvMessage,K_Job_MapResponse);
	t_dictionary *blockState = obtenerBlockState(path,temporaryPath,recvMessage->sockfd);
	char *nroBloque = dictionary_get(blockState,K_BlockState_nroBloque);
	char *IPnroNodo= dictionary_get(blockState,K_BlockState_nroNodo);
	int pos = obtenerPosicionDeBloqueEnBlockStatesList(path,IPnroNodo,nroBloque,recvMessage->sockfd);

	//******************************************
	//Actualizo blockState
	StatusBlockState *status = dictionary_get(blockState,K_BlockState_state);
	*status = K_MAPPED;

	//******************************************
	//Actualizo nodoState
	decrementarOperacionesEnProcesoEnNodo(IPnroNodo);
	addTemporaryFilePathToNodoData(IPnroNodo,temporaryPath);

	informarTareasPendientesDeMapping(path,jobSocket);
	free(path);
	free(temporaryPath);
}

void actualizarTablas_RtaDeMapFallo(Message *recvMessage){

	//actualizar tablas y reenviar si existen copias
	char *path = deserializeFilePath(recvMessage,K_Job_MapResponse);
	char *tempPath = deserializeTempFilePath(recvMessage,K_Job_MapResponse);
	t_dictionary *blockState = obtenerBlockState(path,tempPath,recvMessage->sockfd);
	char *nroDeBloque = dictionary_get(blockState,K_BlockState_nroBloque);
	char *ipNodo= dictionary_get(blockState,K_BlockState_nroNodo);
	int pos = obtenerPosicionDeBloqueEnBlockStatesList(path,ipNodo,nroDeBloque,recvMessage->sockfd);

	//PONER -1 EN COPIAS
	darDeBajaCopiaEnBloqueYNodo(path,recvMessage->sockfd,ipNodo);
	decrementarOperacionesEnProcesoEnNodo(ipNodo);

	StatusBlockState *status = dictionary_get(blockState,K_BlockState_state);
	*status = K_UNINITIALIZED;

	informarTareasPendientesDeMapping(path,jobSocket);
	free(path);
	free(tempPath);
}

void decrementarOperacionesEnReduceList()
{
	int size = list_size(nodosReduceList_Pedido1);
	int i;
	for(i=0;i<size;i++){
		t_dictionary *reduceBlock = list_get(nodosReduceList_Pedido1,i);
		char *ipNodo = dictionary_get(reduceBlock,"reduceBlock_ip");
		decrementarOperacionesEnProcesoEnNodo(ipNodo);
	}
}

void liberarMensaje(Message *msj)
{
	return;
	free(msj->mensaje->comando);
	free(msj->mensaje->data);
	free(msj->mensaje);
	free(msj);
}
Message *createFSrequest(Message *msj){

	int tipoDeCommando = obtenerIdParaComando(msj);
	char *path = deserializeFilePath(msj,tipoDeCommando);
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
	fsRequest->mensaje->data = malloc(strlen(""));
	strcpy(fsRequest->mensaje->data,"");

	return fsRequest;
}

Message *crearMensajeAJobDeFinalizado(Message *msj){

	int tipoDeCommando = obtenerIdParaComando(msj);
	char *path = deserializeFilePath(msj,tipoDeCommando);
	Message *fsRequest = malloc(sizeof(Message));
	fsRequest->mensaje = malloc(sizeof(mensaje_t));

	//armo stream a mano
	char *stream = string_new();
	string_append(&stream,"FileSuccess ");
	string_append(&stream,path);

	fsRequest->sockfd = jobSocket;
	fsRequest->mensaje->comandoSize = (int16_t)strlen(stream);
	fsRequest->mensaje->comando = stream;
	fsRequest->mensaje->dataSize = 0;
	fsRequest->mensaje->data = malloc(strlen(""));
	strcpy(fsRequest->mensaje->data,"");

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


void actualizarTablas_ReduceFallo(char *path,Message *recvMessage)
{
	t_list *nodosCaidos =deserializeFailedReduceResponse(recvMessage);
	int sizeNodosCaidos = list_size(nodosCaidos);
	int i;
	for(i=0;i<sizeNodosCaidos;i++){

		char *ipNodoCaido = list_get(nodosCaidos,i);
		printf("el pedido de reduce al nodo %s fallo\n",ipNodoCaido);
		printf("seguramente el nodo %s esta caido\n",ipNodoCaido);
		resetBlockStateConNodo(path,ipNodoCaido);
		darDeBajaCopiaEnBloqueYNodo(path,recvMessage->sockfd,ipNodoCaido);
	}

	for(i=0;i<sizeNodosCaidos;i++){
			char *ipNodoCaido = list_get(nodosCaidos,i);
			free(ipNodoCaido);
	}
	list_destroy(nodosCaidos);
}
void resetBlockStateConNodo(char *path,char *ipNodoCaido)
{
	t_dictionary *fileState = getFileState(path,jobSocket);
	t_list *blockStatesList = dictionary_get(fileState,K_FileState_arrayOfBlocksStates);
	int listSize = list_size(blockStatesList);
	int j;
	for(j=0;j<listSize;j++){
		t_dictionary *blockState = list_get(blockStatesList,j);
		char *ip = dictionary_get(blockState,K_BlockState_nroNodo);
		if(strcmp(ip,ipNodoCaido)==0){
		bool *state = dictionary_get(blockState,K_BlockState_state);
		*state = K_UNINITIALIZED;
		}
	}
}

void liberarNodosReduceList_Pedido1()
{
	int i;
	int size = list_size(nodosReduceList_Pedido1);
	for(i=0;i<size;i++){
		t_dictionary *reduceBlock = list_get(nodosReduceList_Pedido1,i);
		char *tempPath = dictionary_get(reduceBlock,"reduceBlock_tempPath");
		//NO LIBERO EL IP PORQE ESTA ASOCIADO A UN BLOCK_STATE TMB
		free(tempPath);
		dictionary_destroy(reduceBlock);
	}
	list_clean(nodosReduceList_Pedido1);
}

void liberarFileState_Y_nodoState(Message *recvMessage)
{
	//*comando: "reduceFileConCombiner-Pedido2 NombreArchTempFinal Respuesta"
	//*comando: "reduceFileSinCombiner NombreArchTempFinal Respuesta"

	char *path = filePathAProcesar;

	//LIBERO NODO_STATE
	//IMPLEMENTAR !!!!!!!!!!!

	//LIBERO FILE_STATE
	t_dictionary *fileState = getFileState(path,jobSocket);
	t_list *blocksStatesList = dictionary_get(fileState,K_FileState_arrayOfBlocksStates);
	int size = list_size(blocksStatesList);
	int i;
	for(i=0;i<size;i++){

		t_dictionary *blockState = list_get(blocksStatesList,i);
		//LIBERO SOLO archTempo y state --> los otros pueden estar siendo apuntados por otras estructuras
		char *archTempo = dictionary_get(blockState,K_BlockState_temporaryPath);
		StatusBlockState *status = dictionary_get(blockState,K_BlockState_state);
		free(status);
		free(archTempo);
		dictionary_destroy(blockState);
	}
	list_destroy(blocksStatesList);

}

void sacarCargasDeNodos_FalloDeJob(){

	int size = list_size(listaDeNodos_EnCasoDeFalloDeJob);
	int i;
	for(i=0;i<size;i++){
		char *ip = list_get(listaDeNodos_EnCasoDeFalloDeJob,i);
		decrementarOperacionesEnProcesoEnNodo(ip);
		informarTareasPendientesDeMapping(filePathAProcesar,jobSocket);
	}
	list_destroy(listaDeNodos_EnCasoDeFalloDeJob);
}
