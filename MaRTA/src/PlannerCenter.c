/*
 * PlannerCenter.c
 *
 *  Created on: 23/5/2015
 *      Author: utnso
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#include "FilesStatusCenter.h"
#include "Utilities.h"
#include <commons/temporal.h>
#include <commons/string.h>
#include "Serializador.h"
#include <unistd.h>
#include "VariablesGlobales.h"
#include "protocolo.h"

// VARIABLES GLOBALES
int jobSocket;
char *filePathAProcesar;
t_list *nodosReduceList_Pedido1;
t_list *listaDeNodos_EnCasoDeFalloDeJob;
bool yaPediFullDataTable;

//-->PedidoRealizado
char *ipNodoLocalDePedidoDeReduce;

// FUNCIONES PRIVADAS
int obtenerIdParaComando(Message *recvMessage);
void planificar(Message *recvMessage,TypesMessages type);
void actualizarPedidoRealizado(char *bloque, char* ipnodo, char* path, char *pathTemporal, StatusBlockState tipo );
Message* obtenerProximoPedido(Message *recvMessage);
t_dictionary *obtenerCopiaConMenosCargaParaBloque(Message *recvMessage,char *path,int bloqueNro);
char* crearPathTemporal(char *path);
Message* armarMensajeParaEnvio(Message *recvMessage,char *stream,char *comando);
void actualizarTablas_RtaDeMapExitosa(Message *recvMessage);
Message *createFSrequest(Message *msj,int nroDeBloqe);
bool *obtenerEstadoDeMapping(Message *msj);
void actualizarTablas_RtaDeMapFallo(Message *recvMessage);
void decrementarOperacionesEnReduceList();
void resetBlockStateConNodo(char *path,char *ipNodoCaido);
void actualizarTablas_ReduceFallo(char *path,Message *recvMessage);
void liberarMensaje(Message *msj);
void liberarNodosReduceList_Pedido1();
void liberarFileState_Y_nodoState(Message *recvMessage);
Message *crearMensajeAJobDeFinalizado(Message *msj);
void sacarCargasDeNodos_FalloDeJob();
bool *obtenerEstanTodosDisponibles(Message *msj);
void liberarFileStatusData();

// FUNCIONES PUBLICAS
void initPlannerCenter();
bool processMessage(Message *recvMessage);

void initPlannerCenter()
{
	initFilesStatusCenter();
	jobSocket=0;
	yaPediFullDataTable=false;
	nodosReduceList_Pedido1 = list_create();
	listaDeNodos_EnCasoDeFalloDeJob = list_create();
}

bool processMessage(Message *recvMessage)
{
	int comandoId = obtenerIdParaComando(recvMessage);
	switch (comandoId) {
		case K_NewConnection:
			log_trace(logFile,"PlannerCenter : planificar NewConnection");
			addNewConnection(recvMessage->sockfd);
			jobSocket=recvMessage->sockfd;
			log_trace(logFile,"***************");
			break;
		case K_Job_NewFileToProcess:

			log_trace(logFile,"PlannerCenter : planificar Job_NewFileToProcess");
			char* filePath = deserializeFilePath(recvMessage,K_Job_NewFileToProcess);
			filePathAProcesar = filePath;
			bool *soportaCombiner = deserializeSoportaCombiner(recvMessage);
			//******************
			// --> se agrega un file_StatusData a filesToProcessPerJob
			addNewFileForProcess(filePath,soportaCombiner,recvMessage->sockfd);
			//******************
			planificar(recvMessage,K_Job_NewFileToProcess);

			log_trace(logFile,"***************");
			break;

		case K_FS_FileFullData:

			log_trace(logFile,"PlannerCenter : planificar FS_FileFullData");
			//FS me responde con el pedido de datos que le hice.

			bool *response = deserializeRequestResponse(recvMessage,K_FS_FileFullData);

			if(!(*response)){

				//si es false entonces no esta disponible el archivo en el FS
				char *tempPath = deserializeFilePath(recvMessage,K_FS_FileFullData);
				char *log = string_from_format("el pedido al FS del archivo %s fallo, MaRTA no puede continuar la operacion",tempPath);
				log_trace(logFile,log);
				free(log);
				liberarFileState_Y_nodoState(recvMessage);
				liberarFileStatusData();

				free(tempPath);
				return true;
			}

			if(*response){
				planificar(recvMessage,K_FS_FileFullData);
			}
			free(response);
			log_trace(logFile,"***************");
			break;

		case K_Job_MapResponse:

			log_trace(logFile,"PlannerCenter : planificar Job_MapResponse");
			planificar(recvMessage,K_Job_MapResponse);

			log_trace(logFile,"***************");
			break;
		case K_Job_ReduceResponse:

			log_trace(logFile,"PlannerCenter : planificar Job_ReduceResponse\n");
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
				liberarFileStatusData();
				char *log = string_from_format("archivo %s reducido con exito !\n",path);
				log_trace(logFile,log);
				free(log);

				Message *jobMsj = crearMensajeAJobDeFinalizado(recvMessage);
				enviar(jobMsj->sockfd,jobMsj->mensaje);
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
		case K_ProcesoCaido:

			/*int a = recvMessage->sockfd;
			char *mlog = string_from_format("el Job con socket %d se cayo, MaRTA no puede seguir procesando sus archivos",a);
			log_trace(logFile,mlog);
			free(mlog);*/
			//ME TIRA ERROR EN EL LOG_TRACE !!!!!

			liberarFileState_Y_nodoState(recvMessage);
			sacarCargasDeNodos_FalloDeJob();
			liberarFileStatusData();
			return true;

			break;
		default:

			log_trace(logFile,"PlannerCenter: ERROR !! Comando no identificado !!");
			return true;
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
	if(strcmp(comando,"ProcesoCaido")==0){type = K_ProcesoCaido;}

	free(comando);
	return type;
}


//Planificacion
void planificar(Message *recvMessage,TypesMessages type)
{

	if(type == K_Job_NewFileToProcess){
		//pido al FS la tabla de direcciones del archivo
		//segun protocolo ---> -Comando: "DataFile rutaDelArchivo" /// -Data: Vacio

		char *path = deserializeFilePath(recvMessage,K_Job_NewFileToProcess);

		if(dictionary_has_key(fullDataTables,path)){

			yaPediFullDataTable=true;//lo cuento como si ya hubiera hecho el pedido
			t_list *fullData = getCopyFullDataTable(path);
			int cantidadDeBloqes = list_size(fullData);

			t_dictionary *filesToProcessPerJob = getFilesToProcessPerJob(jobSocket);
			t_dictionary *file_StatusData = getFile_StatusData(filesToProcessPerJob,path);
			dictionary_put(file_StatusData,K_file_StatusData_Bloques,fullData);
			agregarFileState(jobSocket,path,cantidadDeBloqes);

			Message *sendMessage = obtenerProximoPedido(recvMessage);
			enviar(sendMessage->sockfd,sendMessage->mensaje);
		}

		if(!(dictionary_has_key(fullDataTables,path))){
			Message *fsRequest = createFSrequest(recvMessage,-1);
			enviar(fsRequest->sockfd,fsRequest->mensaje);
			liberarMensaje(fsRequest);
		}
	}

	if(type==K_FS_FileFullData){

		//--> FS responde con tabla de archivo pedida
		//-Comando: "DataFileResponse rutaDelArchivo Respuesta cantidadDeBloques-nroDeCopias-sizeEstructura-"

		char *path = deserializeFilePath(recvMessage,K_FS_FileFullData);

		if(yaPediFullDataTable==false){
			//se completa filesToProcess y se crea un fileState
			addFileFullData(jobSocket, path,recvMessage);
		}
		if(yaPediFullDataTable){

			t_list *fileDeCopias = deserializarFullDataResponse(recvMessage);
			int nroDeCopia = deserializeNumeroDeBloque_PedidoDeCopias(recvMessage);
			reloadFilaDeFileFullData(jobSocket,path,fileDeCopias,nroDeCopia);
		}

		//Obtengo proximoPedido CON info actualizada
		Message *sendMessage = obtenerProximoPedido(recvMessage);
		enviar(sendMessage->sockfd,sendMessage->mensaje);

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
			bool *todosMappeados = obtenerEstadoDeMapping(recvMessage);
			bool *nodosTodosDisp = obtenerEstanTodosDisponibles(recvMessage);
			if(*todosMappeados && *nodosTodosDisp){
				Message *sendMessage = obtenerProximoPedido(recvMessage);
				enviar(sendMessage->sockfd,sendMessage->mensaje);
			}
			free(todosMappeados);
			free(nodosTodosDisp);
		}

		if(!(*requestResponse)){

			//REPLANIFICAR
			char *nroDeBloque = dictionary_get(blockState,K_BlockState_nroBloque);
			char *ipNodo= dictionary_get(blockState,K_BlockState_nroNodo);
			int pos = obtenerPosicionDeBloqueEnBlockStatesList(path,ipNodo,nroDeBloque,recvMessage->sockfd);

			char *log = string_from_format("fallo pedido de map del archivo %s al nodo %s bloque %s en posicion %d",path,ipNodo,nroDeBloque,pos);
			char *log2 = string_from_format("el nodo %s seguramente esta caido",ipNodo);
			log_trace(logFile,log);
			log_trace(logFile,log2);
			free(log);
			free(log2);

			actualizarTablas_RtaDeMapFallo(recvMessage);

			//OBTENER PROXIMO PEDIDO (se va a enviar devuelta el mismo, siempre y cuando haya copias disponibles)
			Message* sendMessage = obtenerProximoPedido(recvMessage);
			enviar(sendMessage->sockfd,sendMessage->mensaje);
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
			log_trace(logFile,"reduce-Pedido1 realizado con exito");
			decrementarOperacionesEnReduceList();
			Message* sendMessage = obtenerProximoPedido(recvMessage);
			enviar(sendMessage->sockfd,sendMessage->mensaje);
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

				log_trace(logFile,"reduceSinCombiner fallo");
				char *ipNodo = ipNodoLocalDePedidoDeReduce;
				decrementarOperacionesEnProcesoEnNodo(ipNodo);
				actualizarTablas_ReduceFallo(path,recvMessage);
				sendMessage = obtenerProximoPedido(recvMessage);
			   enviar(sendMessage->sockfd,sendMessage->mensaje);
			}

			if(strcmp(reduceType,"reduceFileConCombiner-Pedido1")==0){

				log_trace(logFile,"reduce-Pedido1 fallo");
				decrementarOperacionesEnReduceList();
				actualizarTablas_ReduceFallo(path,recvMessage);
				sendMessage = obtenerProximoPedido(recvMessage);
			   enviar(sendMessage->sockfd,sendMessage->mensaje);
				liberarNodosReduceList_Pedido1();

			}

			if(strcmp(reduceType,"reduceFileConCombiner-Pedido2")==0){

				log_trace(logFile,"reduce-Pedido2 fallo");
				char *ipNodo = ipNodoLocalDePedidoDeReduce;
				decrementarOperacionesEnProcesoEnNodo(ipNodo);
				actualizarTablas_ReduceFallo(path,recvMessage);
				liberarNodosReduceList_Pedido1();

				sendMessage = obtenerProximoPedido(recvMessage);
				enviar(sendMessage->sockfd,sendMessage->mensaje);
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
	bool *todosMappeados = obtenerEstadoDeMapping(recvMessage);
	bool *nodosTodosDisp = obtenerEstanTodosDisponibles(recvMessage);
	Message *msjParaEnviar;
	informarTareasPendientesDeMapping(path,jobSocket);

	if(*todosMappeados && *nodosTodosDisp){
		//estan todos mappeados y disponibles

		char *log = string_from_format("el archivo %s con socket %d esta mappeado!",filePathAProcesar,jobSocket);
		log_trace(logFile,log);
		free(log);

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
				string_append(&command,"reduceFileConCombiner-Pedido1 ");
				string_append(&command,_path);

				bool firstTime = true;
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

					if(!firstTime){ string_append(&stream," "); }
					if(firstTime){ firstTime = false; }
					//agregar a el serializado --> "Nodo1 nombreArchTemp1 CantDeArchEnNodoAProcesar"
					string_append(&stream,IPnodoEnBlockState);
					string_append(&stream," "); string_append(&stream,puertonodoEnBlockState);
					string_append(&stream," "); string_append(&stream,_pathTempo);
					addIntToStream(stream,cantidadDePathsTempEnNodo,K_int16_t);

					int j;
					for(j=0;j<cantidadDePathsTempEnNodo;j++){

						char *tempPath = list_get(pathsTemporalesParaNodo,j);
						//agregar a el serializado "tempPath"
						string_append(&stream," "); string_append(&stream,tempPath);
					}
					list_destroy(pathsTemporalesParaNodo);

				}

				char *log = string_from_format("el Pedido1 (%s - %d) de ReduceConCombiner es : %s",filePathAProcesar,jobSocket,stream);
				log_trace(logFile,log); free(log);

				msjParaEnviar = armarMensajeParaEnvio(recvMessage,stream,command);
				finalStream=stream;
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
				string_append(&comando,"reduceFileConCombiner-Pedido2");string_append(&comando," ");
				string_append(&comando,pathTempo);
				free(_path);
				free(pathTempo);

				//******************************************
				//1ero NodoLocal --> tomo el 1ero porque si
				t_dictionary *reduceBlock = list_get(nodosReduceList_Pedido1,0);
				char *ip = dictionary_get(reduceBlock,"reduceBlock_ip");
				char *puerto = dictionary_get(reduceBlock,"reduceBlock_puerto");
				char *tempPath = dictionary_get(reduceBlock,"reduceBlock_tempPath");
				string_append(&stream,ip);string_append(&stream," ");
				string_append(&stream,puerto);string_append(&stream," ");
				string_append(&stream,"1 ");
				string_append(&stream,tempPath);
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
					string_append(&stream," "); string_append(&stream,ip);
					string_append(&stream," "); string_append(&stream,puerto);
					string_append(&stream," 1");
					string_append(&stream," "); string_append(&stream,tempPath);
					list_add(listaDeNodos_EnCasoDeFalloDeJob,ip);
				}
				char *log = string_from_format("el Pedido2 (%s - %d) de ReduceConCombiner es : %s",filePathAProcesar,jobSocket,stream);
				log_trace(logFile,log); free(log);

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
			string_append(&stream,IPnroNodoLocal);string_append(&stream," ");
			string_append(&stream,puertoNodoLocal);
			addIntToStream(stream,cantidadDePathsTempEnNodoLocal,K_int16_t);

			int k;
			for(k=0;k<cantidadDePathsTempEnNodoLocal;k++){

				char *tempPath = list_get(pathsTemporalesParaNodoLocal,k);
				//agregar a el serializado "tempPath"
				string_append(&stream," "); string_append(&stream,tempPath);
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
					string_append(&stream," "); string_append(&stream,IPnodoEnBlockState);
					string_append(&stream," "); string_append(&stream,puertonodoEnBlockState);
					addIntToStream(stream,cantidadDePathsTempEnNodo,K_int16_t);

					int j;
					for(j=0;j<cantidadDePathsTempEnNodo;j++){

						char *tempPath = list_get(pathsTemporalesParaNodo,j);
						//agregar a el serializado "tempPath"
						string_append(&stream," "); string_append(&stream,tempPath);
					}
					list_destroy(pathsTemporalesParaNodo);
				}
			}
			char *path = deserializeFilePath(recvMessage,K_Job_MapResponse);

			char *log = string_from_format("el archivo %s - sckt %d no soporta Combiner",path,jobSocket);
			char *log2 = string_from_format("el pedido de reduce (arch %s - sckt %d)es : %s",filePathAProcesar,jobSocket,stream);
			log_trace(logFile,log);log_trace(logFile,log2);
			free(log);free(log2);

			//actualizar Tablas !!
			incrementarOperacionesEnProcesoEnNodo(IPnroNodoLocal);
			//setBlockStatesListInReducingState(path);
			ipNodoLocalDePedidoDeReduce = IPnroNodoLocal;
			finalStream=stream;

			char *comando = createStream();
			char *tempPathFinal = crearPathTemporal(path);
			string_append(&comando,"reduceFile ");
			string_append(&comando,tempPathFinal);

			msjParaEnviar = armarMensajeParaEnvio(recvMessage,finalStream,comando);
		}
	}

	if(!(*todosMappeados) || !(*nodosTodosDisp)){

		char *stream = createStream();
		int i;
		bool firstTime = true;

		for(i=0;i<size_fileState;i++){

			t_dictionary *blockState = list_get(blockStateArray,i);
			StatusBlockState *status = dictionary_get(blockState,K_BlockState_state);
			bool *disponible = dictionary_get(blockState,K_BlockState_nodoDisponible);

			if((*status)==K_UNINITIALIZED || (*disponible)==false){
				//obtengo conjunto (#ipNodo;#nroBloque) con menos carga de operaciones para bloque en pos "i"
				t_dictionary *copiaConMenosCarga = obtenerCopiaConMenosCargaParaBloque(recvMessage,path,i);
				bool *_disponible = dictionary_get(copiaConMenosCarga,K_Copia_Estado);

				if((*_disponible) == false){//checkeo q haya copias disponibles

					//NO HAY COPIAS DISPONIBLES !!!!!
					log_trace(logFile,"no hay mas copias disponibles!!!");
					char *log = string_from_format("averiguo si FileSystem tiene nuevos nodos !, (%d - %s) pedido de bloqe %d",i,jobSocket,filePathAProcesar);
					log_trace(logFile,log);free(log);

					Message *fsRequest = createFSrequest(recvMessage,i);
					yaPediFullDataTable = true;
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
				bool *disponible =  dictionary_get(copiaConMenosCarga,K_Copia_Estado);

				if(firstTime==false){ string_append(&stream," "); }
				if(firstTime==true){ firstTime = false; }
				string_append(&stream,ipNodo);
				string_append(&stream," "); string_append(&stream,puertoNodo);
				string_append(&stream," "); string_append(&stream,nroDeBloque);
				string_append(&stream," "); string_append(&stream,path_with_temporal);

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
				dictionary_put(blockState,K_BlockState_nodoDisponible,disponible);

				list_add(listaDeNodos_EnCasoDeFalloDeJob,ipNodo);
			}
		}

		char *command = createStream();
		string_append(&command,"mapFile ");
		string_append(&command,path);

		char *log = string_from_format("se envia pedido de map, el stream es %s\n",stream);
		log_trace(logFile,log); free(log);
		msjParaEnviar = armarMensajeParaEnvio(recvMessage,stream,command);
	}
	free(path);
	free(todosMappeados);
	free(nodosTodosDisp);
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
bool *obtenerEstadoDeMapping(Message *msj)
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

bool *obtenerEstanTodosDisponibles(Message *msj)
{
	int tipoDeCommando = obtenerIdParaComando(msj);
	char *path = deserializeFilePath(msj,tipoDeCommando);
	t_dictionary *fileState = getFileStateForPath(path,jobSocket);
	t_list *blockStateArray = dictionary_get(fileState,K_FileState_arrayOfBlocksStates);
	int size_fileState= list_size(blockStateArray);

	bool *todosDisp = malloc(sizeof(bool));
	*todosDisp = true;

	int i;
	for(i=0;i<size_fileState;i++){
		t_dictionary *blockState = list_get(blockStateArray,i);
		bool *disponible = dictionary_get(blockState,K_BlockState_nodoDisponible);
		if((*disponible)==false){
			*todosDisp = false;
			return todosDisp;
		}
	}
	return todosDisp;
}
t_dictionary *obtenerCopiaConMenosCargaParaBloque(Message *recvMessage,char *path,int bloqueNro){

	t_list *copias = obtenerCopiasParaBloqueDeArchivo(jobSocket,bloqueNro,path);
	int nroDeCopias = list_size(copias);

	t_dictionary *copiaConMenosCarga = list_get(copias,0);
	int j;
	for(j=1;j<(nroDeCopias);j++){

		//obtengo nodo con menos carga de operaciones

		t_dictionary *copia = list_get(copias,j);

		bool *nodoCopiaEstado = dictionary_get(copia,K_Copia_Estado);
		bool *nodoCopiaPivotEstado = dictionary_get(copiaConMenosCarga,K_Copia_Estado);
		char *IPnroNodoCopia = dictionary_get(copia,K_Copia_IPNodo);
		char *IPnroNodoCopiaPivot = dictionary_get(copiaConMenosCarga,K_Copia_IPNodo);

		if((*nodoCopiaEstado) == false &&
				((*nodoCopiaPivotEstado)== false )){
			//ninguno de los nodos esta disponible, dejo la copia ya asignada

		}else if(((*nodoCopiaEstado)== false)
					&& ((*nodoCopiaPivotEstado) == true)){

			//dejo la copia pivot

		}else if(((*nodoCopiaEstado)== true) && ((*nodoCopiaPivotEstado)==false)){

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

	//PONER IP NO DISPONIBLE
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
Message *createFSrequest(Message *msj,int nroDeBloqe){

	int tipoDeCommando = obtenerIdParaComando(msj);
	char *path = deserializeFilePath(msj,tipoDeCommando);
	Message *fsRequest = malloc(sizeof(Message));
	fsRequest->mensaje = malloc(sizeof(mensaje_t));

	//armo stream a mano
	char *stream = string_new();
	string_append(&stream,"DataFile ");
	string_append(&stream,path);

	if(nroDeBloqe != -1){

		char *blqStr = intToCharPtr(nroDeBloqe);
		string_append(&stream," ");
		string_append(&stream,blqStr);

	}
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

Message* armarMensajeParaEnvio(Message *recvMessage,char *stream,char *comando)
{
	Message *msjParaEnvio = malloc(sizeof(Message));
	msjParaEnvio->mensaje = malloc(sizeof(mensaje_t));

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
		char *log = string_from_format("el pedido de reduce al nodo %s fallo, debe estar caido\n",ipNodoCaido);
		log_trace(logFile,log);	free(log);

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

void liberarFileStatusData()
{
	t_dictionary *fileToProcessPerJob = getFilesToProcessPerJob(jobSocket);
	t_dictionary *file_StatusData = getFile_StatusData(fileToProcessPerJob,filePathAProcesar);
	t_list *fullDataTable = dictionary_get(file_StatusData,K_file_StatusData_Bloques);

	int i,j;
	int cantDeCopias = list_size(fullDataTable);

	for(i=0;i<cantDeCopias;i++){
		t_list *copias = list_get(fullDataTable,i);
		int copiasSize = list_size(copias);

		for(j=0;j<copiasSize;j++){
			t_dictionary *dicCopia = list_get(copias,j);
			char *ip = dictionary_get(dicCopia,K_Copia_IPNodo);
			char *nroBloqe = dictionary_get(dicCopia,K_Copia_NroDeBloque);
			char *puerto = dictionary_get(dicCopia,K_Copia_PuertoNodo);
			bool *estado = dictionary_get(dicCopia,K_Copia_Estado);
			free(ip);free(puerto);free(nroBloqe);free(estado);
			dictionary_destroy(dicCopia);
		}
		list_destroy(copias);
	}
}
