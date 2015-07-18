/*
 * FileStatusCenter.c
 *
 *  Created on: 21/5/2015
 *      Author: utnso
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include <commons/collections/dictionary.h>
#include <commons/collections/list.h>
#include <commons/temporal.h>
#include "Utilities.h"
#include "FilesStatusCenter.h"
#include "VariablesGlobales.h"
//#include "MaRTA.c"

/*
//CONSTANTES
*/

//filesToProcess --> keys
#define K_filesToProcess_filesPerJob "filesPerJob"

//NodoState --> Keys
#define K_Nodo_OperacionesEnProceso "operacionesEnProceso"
#define K_Nodo_ArchivosTemporales "archivosTemporales"

t_dictionary *filesToProcess;
t_dictionary *filesStates;
t_dictionary *nodosData;

int fs_socket; //socket del FS

// Inicializar
void initFilesStatusCenter();
// Agregar nuevas conexiones
void addFSConnection(int fs_socket);
void addNewConnection(int socket);
// FS
int getFSSocket();
// Varias
t_dictionary *addFileFullData(int sckt, char* path, Message *recvMessage,t_dictionary *file_StatusData);
void reloadFilaDeFileFullData(t_list *fullData,int nroDeBloqe,t_dictionary *file_StatusData);
t_dictionary *getNodoState(char *ipNroNodo);
int *getNodoState_OperacionesEnProceso(t_dictionary *nodoState);
t_list *getNodoState_listaTemporales(t_dictionary *nodoState);
void informarTareasPendientesDeMapping(char *path,int socket,t_dictionary *fileState);
//nodosData
void incrementarOperacionesEnProcesoEnNodo(char *IPnroNodo);
void decrementarOperacionesEnProcesoEnNodo(char *IPnroNodo);
int getCantidadDeOperacionesEnProcesoEnNodo(char *IPnroNodo);
//filesToProcess
t_dictionary *crearNewFileForProcess(char *file_Path,_Bool *soportaCombiner,int jobSocket);//crea un fileState
void darDeBajaCopiaEnBloqueYNodo(char *IP_Nodo,t_dictionary *file_StatusData);
t_list* obtenerCopiasParaBloqueDeArchivo(int bloque,t_dictionary *file_StatusData);
bool* soportaCombiner(t_dictionary *file_StatusData);
bool todosArchivosDeJobReducidos(int jobSocket);
void agregarArchivoFinalizado(int jobSocket, char *path, char*ip, char*puerto, char*pathFinal);
t_list *obtenerListaParaReduceFinal(int jobSocket);
//filesStates
t_list* obtenerPathsTemporalesParaArchivo(char *IPnodoEnBlockState,t_dictionary *fileState);
int obtenerCantidadDePathsTemporalesEnNodo(char *IPnodoEnBlockState,t_dictionary *fileState);
t_list* obtenerNodosEnBlockStateArray(t_dictionary *fileState);
int obtenerCantidadDeNodosDiferentesEnBlockState(t_dictionary *fileState);
t_dictionary* obtenerNodoConMayorCantidadDeArchivosTemporales(t_dictionary *fileState);
bool* isPathInList(t_list *lista,char *path);
bool* isNodoInList(t_list *lista,char *IPnroDeNodo);
int obtenerPosicionDeBloqueEnBlockStatesList(char *ipNodo,char *bloque,t_dictionary *fileState);
t_dictionary *obtenerBlockState(char *tempPath,t_dictionary *fileState);
void setBlockStatesListInReducingState(t_dictionary *fileState);
t_dictionary *crearFileState(int jobSocket,char *path, int cantidadDeBloques);
//fullDataTable
void agregarFullDataTable(t_list *table,char *path);
t_list *getCopyFullDataTable(char *path);

void initFilesStatusCenter()
{
#ifdef K_SIMULACION
	fs_socket=3;
#else
	fs_socket=-1;
#endif
}

void addFSConnection(int fs_sckt)
{
	fs_socket = fs_sckt;
}

int getFSSocket()
{
	return fs_socket;
}
void addNewConnection(int jobSocket)
{
	char* skct_key = intToCharPtr(jobSocket);
	t_dictionary *filesToProcessPerJob = dictionary_create();

	sem_wait(&semFilesToProcess);
	dictionary_put(filesToProcess, skct_key, filesToProcessPerJob);
	sem_post(&semFilesToProcess);

	free(skct_key);
}
t_dictionary* getFilesToProcessPerJob(int jobSocket)
{
	char* dic_key = intToCharPtr(jobSocket);

	sem_wait(&semFilesToProcess);
	t_dictionary *filesToProcessPerJob = dictionary_get(filesToProcess,dic_key);
	sem_post(&semFilesToProcess);

	free(dic_key);

	return filesToProcessPerJob;
}

t_dictionary *crearNewFileForProcess(char *file_Path,_Bool *soportaCombiner,int jobSocket)
{
	char *filePathDup = strdup(file_Path);
	t_dictionary *filesToProcessPerJob = getFilesToProcessPerJob(jobSocket);

	t_dictionary *file_StatusData = dictionary_create();
	dictionary_put(file_StatusData,K_file_StatusData_combinerMode,soportaCombiner);

	sem_wait(&semFilesToProcessPerJob);
	bool existeLista = dictionary_has_key(filesToProcessPerJob,"K_pathsEstados");

	if(!existeLista){
		t_list *pathsEstadosList = list_create();
		t_dictionary *estadoPath = dictionary_create();
		dictionary_put(estadoPath,"K_path",filePathDup);
		bool *finalizo = malloc(sizeof(bool));
		*finalizo = false;
		dictionary_put(estadoPath,"K_estado",finalizo);
		list_add(pathsEstadosList,estadoPath);

		//sem_wait(&semFilesToProcessPerJob);
		dictionary_put(filesToProcessPerJob,"K_pathsEstados",pathsEstadosList);
		//sem_post(&semFilesToProcessPerJob);

	}else{
		//sem_wait(&semFilesToProcessPerJob);
		t_list *pathsEstadosList =  dictionary_get(filesToProcessPerJob,"K_pathsEstados");
		//sem_post(&semFilesToProcessPerJob);

		t_dictionary *estadoPath = dictionary_create();
		dictionary_put(estadoPath,"K_path",filePathDup);
		bool *finalizo = malloc(sizeof(bool));
		*finalizo = false;
		dictionary_put(estadoPath,"K_estado",finalizo);
		list_add(pathsEstadosList,estadoPath);
	}

	sem_post(&semFilesToProcessPerJob);

	return file_StatusData;
}
void agregarArchivoFinalizado(int jobSocket, char *path, char*ip, char*puerto, char*pathFinal){

	t_dictionary *filesToProcessPerJob = getFilesToProcessPerJob(jobSocket);
	sem_wait(&semFilesToProcessPerJob);
	t_list *pathsEstados = dictionary_get(filesToProcessPerJob,"K_pathsEstados");
	int size = list_size(pathsEstados);
	int i;

	t_dictionary *ubicacionFinal = dictionary_create();
	char *_ip = malloc(strlen(ip)+1);
	char *_puerto = malloc(strlen(puerto)+1);
	char *_pathFinal = malloc(strlen(pathFinal)+1);
	strcpy(_ip,ip);
	strcpy(_puerto,puerto);
	strcpy(_pathFinal,pathFinal);

	dictionary_put(ubicacionFinal,"K_ip",_ip);
	dictionary_put(ubicacionFinal,"K_puerto",_puerto);
	dictionary_put(ubicacionFinal,"K_path",_pathFinal);

	for(i=0;i<size;i++){
		t_dictionary *estadoPath = list_get(pathsEstados,i);
		char *dicPath = dictionary_get(estadoPath,"K_path");

		if(strcmp(dicPath,path)==0){
			bool *estado = dictionary_get(estadoPath,"K_estado");
			*estado = true;
			dictionary_put(estadoPath,"K_UbicacionFinal",ubicacionFinal);
			break;
		}
	}
	sem_post(&semFilesToProcessPerJob);
}

t_list *obtenerListaParaReduceFinal(int jobSocket){

	t_dictionary *filesToProcessPerJob = getFilesToProcessPerJob(jobSocket);
	sem_wait(&semFilesToProcessPerJob);
	t_list *pathsEstados = dictionary_get(filesToProcessPerJob,"K_pathsEstados");
	sem_post(&semFilesToProcessPerJob);
	return pathsEstados;
}
bool todosArchivosDeJobReducidos(int jobSocket){

	t_dictionary *filesToProcessPerJob = getFilesToProcessPerJob(jobSocket);
	sem_wait(&semFilesToProcessPerJob);
	t_list *pathsEstados = dictionary_get(filesToProcessPerJob,"K_pathsEstados");
	int size = list_size(pathsEstados);
	int i;
	bool todosReducidos = true;

	for(i=0;i<size;i++){
		t_dictionary *estadoPath = list_get(pathsEstados,i);
		bool *estado = dictionary_get(estadoPath,"K_estado");

		if(*estado == false){
			todosReducidos = false;
			sem_post(&semFilesToProcessPerJob);
			return todosReducidos;
		}
	}
	sem_post(&semFilesToProcessPerJob);
	return todosReducidos;
}
t_dictionary *addFileFullData(int jobSocket, char* path, Message *recvMessage,t_dictionary *file_StatusData)
{	// fullData me lo envia el FS
	// aca va la respuesta del FS
	// tiene la info de los bloques y nodos donde esta el archivo a procesar

	t_list *fullData;
	fullData = deserializarFullDataResponse(recvMessage);
	agregarFullDataTable(fullData,path);
	fullData = getCopyFullDataTable(path);

	dictionary_put(file_StatusData,K_file_StatusData_Bloques,fullData);
	int nroDeBloques = list_size(fullData);

	return crearFileState(jobSocket,path,nroDeBloques);

}

t_dictionary* crearFileState(int jobSocket,char *path, int cantidadDeBloques){
	//Construyo un "fileState"
	t_list *blocksStatesList = list_create();
	int i;
	for(i=0;i<cantidadDeBloques;i++){

		t_dictionary *blockState = dictionary_create();
		StatusBlockState *blkState = malloc(sizeof(StatusBlockState));
		*blkState = K_UNINITIALIZED;
		bool *disponible = malloc(sizeof(bool));
		*disponible = false;

		char *uninitializedNroDeBloqe = malloc(strlen(K_BlockState_UninitializedBlqe)+1);
		char *uninitializedIPNodo = malloc(strlen(K_BlockState_UninitializedIPNodo)+1);
		char *uninitializedPath = malloc(strlen(K_BlockState_UninitializedPath)+1);
		char *uninitializedPuertoNodo = malloc(strlen(K_BlockState_UninitializedPuertoNodo)+1);

		strcpy(uninitializedIPNodo,K_BlockState_UninitializedIPNodo);
		strcpy(uninitializedPath,K_BlockState_UninitializedPath);
		strcpy(uninitializedNroDeBloqe,K_BlockState_UninitializedBlqe);
		strcpy(uninitializedPuertoNodo,K_BlockState_UninitializedPuertoNodo);

		dictionary_put(blockState,K_BlockState_state,blkState);
		dictionary_put(blockState,K_BlockState_nroBloque,uninitializedNroDeBloqe);
		dictionary_put(blockState,K_BlockState_nroNodo,uninitializedIPNodo);
		dictionary_put(blockState,K_BlockState_temporaryPath,uninitializedPath);
		dictionary_put(blockState,K_BlockState_UninitializedPuertoNodo,uninitializedPuertoNodo);
		dictionary_put(blockState,K_BlockState_nodoDisponible,disponible);

		list_add(blocksStatesList,blockState);
	}

	//Agrego "filesState" a "filesStates"
	t_dictionary *fileState = dictionary_create();
	dictionary_put(fileState,K_FileState_arrayOfBlocksStates,blocksStatesList);

	return fileState;
}

void reloadFilaDeFileFullData(t_list *CopiasfullData,int nroDeBloqe,t_dictionary *file_StatusData)
{
	//--> FS responde con bloque de archivo pedido
	//-Comando: "DataFileResponse rutaDelArchivo nroDeSocket Disponible"
	//-Data: Bloque
	// Ej: "0;Nodo1;3;Nodo8;2;Nodo2;45;"

	t_list *listaPadre = dictionary_get(file_StatusData,K_file_StatusData_Bloques);
	t_list *copiasAReemplazar = list_get(listaPadre,nroDeBloqe);
	int size = list_size(copiasAReemplazar);
	int n = list_size(listaPadre);

	int i;
	for(i=0;i<size;i++){

		t_dictionary *dicCopia = list_get(copiasAReemplazar,i);
		char *ip = dictionary_get(dicCopia,K_Copia_IPNodo);
		char *nroBloqe = dictionary_get(dicCopia,K_Copia_NroDeBloque);
		char *puerto = dictionary_get(dicCopia,K_Copia_PuertoNodo);
		bool *estado = dictionary_get(dicCopia,K_Copia_Estado);
		free(ip);free(puerto);free(nroBloqe);free(estado);
		dictionary_destroy(dicCopia);

	}

	list_clean(copiasAReemplazar);

	int j;
	t_list *listaH = list_get(CopiasfullData,0);
	size = list_size(listaH);
	for(j=0;j<size;j++){
		t_dictionary *copia = list_get(listaH,j);
		list_add(copiasAReemplazar,copia);
	}
}

int getCantidadDeOperacionesEnProcesoEnNodo(char *IPnroNodo)
{
	sem_wait(&semNodosData);
	bool hasKey = dictionary_has_key(nodosData,IPnroNodo);


	if(hasKey){
		//el nodo existe
		t_dictionary *nodoState = getNodoState(IPnroNodo);
		int *operacionesEnProceso = getNodoState_OperacionesEnProceso(nodoState);
		int valor = *operacionesEnProceso;
		sem_post(&semNodosData);
		return valor;
	}
	//else --> al nodo no se le asigno nada aun, entonces no tiene operacionesEnProceso
	sem_post(&semNodosData);
	return 0;
}

void incrementarOperacionesEnProcesoEnNodo(char *IPnroNodo)
{
	char *_ipDup = strdup(IPnroNodo);

	//se llama cuando envio un pedido de map o reduce al Job
	sem_wait(&semNodosData);
	bool hasKey = dictionary_has_key(nodosData,IPnroNodo);

		if(hasKey){
			//busco el dic y le sumo una operacionProcesando
			t_dictionary *nodoState = getNodoState(IPnroNodo);
			int *operacionesEnProceso = getNodoState_OperacionesEnProceso(nodoState);
			*operacionesEnProceso = *operacionesEnProceso + 1;

			char *log = string_from_format("el nodo %s esta procesando %d pedidos",IPnroNodo,*operacionesEnProceso);
			log_debug(logFile,log);free(log);

		}else{
			char *_ipDup = strdup(IPnroNodo);
			//creo un nuevo dic y le sumo una operacionProcesando
			t_dictionary *nodoState = dictionary_create();
			int *operacionesEnProceso = malloc(sizeof(int));
			*operacionesEnProceso = 1;
			dictionary_put(nodoState,K_Nodo_OperacionesEnProceso,operacionesEnProceso);
			dictionary_put(nodosData,_ipDup,nodoState);

			char *log = string_from_format("el nodo %s esta procesando %d pedidos",_ipDup,*operacionesEnProceso);
			log_debug(logFile,log);free(log);
		}
	sem_post(&semNodosData);
}

void decrementarOperacionesEnProcesoEnNodo(char *IPnroNodo)
{
	//se llama cuando envio un pedido de map o reduce al Job
	sem_wait(&semNodosData);
	bool hasKey = dictionary_has_key(nodosData,IPnroNodo);

	if(hasKey){
		//busco el dic y le resto una operacionProcesando
		t_dictionary *nodoState = getNodoState(IPnroNodo);
		int *operacionesEnProceso = getNodoState_OperacionesEnProceso(nodoState);

		//*operacionesEnProceso = *operacionesEnProceso - 1;
		*operacionesEnProceso = 0;

		char *log = string_from_format("el nodo %s esta procesando %d pedidos",IPnroNodo,*operacionesEnProceso);
		log_debug(logFile,log);free(log);
	}
	sem_post(&semNodosData);
}

void darDeBajaCopiaEnBloqueYNodo(char *IP_Nodo,t_dictionary *file_StatusData)
{	//archivoParcial_Path seria algo como librazo-horaDeEnvio.txt
	//debo conseguir el char* librazo.txt --> supongamos que es char* _archivo;

	t_list *bloquesList = dictionary_get(file_StatusData,K_file_StatusData_Bloques);
	int bloqesSize = list_size(bloquesList);
	int i,j;
	for(i=0;i<bloqesSize;i++){

		t_list *copiasList = list_get(bloquesList,i);
		int copiasSize = list_size(copiasList);

		for(j=0;j<copiasSize;j++){
			t_dictionary *copiaDeBloque = list_get(copiasList,j);
			char *IPNodo = dictionary_get(copiaDeBloque,K_Copia_IPNodo);

			if( strcmp(IPNodo,IP_Nodo) == 0 ){
				bool *disponible = dictionary_get(copiaDeBloque,K_Copia_Estado);
				*disponible = false;
			}
		}
	}
}

t_list* obtenerCopiasParaBloqueDeArchivo(int bloque ,t_dictionary *file_StatusData)
{
	t_list *blocksList =dictionary_get(file_StatusData,K_file_StatusData_Bloques);
	t_list *listaDeCopias = list_get(blocksList,bloque);

	return listaDeCopias;
}

bool* soportaCombiner(t_dictionary *file_StatusData)
{
	bool *soportaCombiner = dictionary_get(file_StatusData,K_file_StatusData_combinerMode);
	return soportaCombiner;
}

t_list* obtenerPathsTemporalesParaArchivo(char *IPnodoEnBlockState,t_dictionary *fileState){

	//t_dictionary* fileState = getFileState(path,socket);
	t_list *blockStateList = dictionary_get(fileState,K_FileState_arrayOfBlocksStates);
	int size_fileState = list_size(blockStateList);
	t_list *pathsTemporalesEnNodo = list_create();

	int i;
	for(i=0;i<size_fileState;i++){
		t_dictionary *blockState = list_get(blockStateList,i);
		char *IPnroDeNodo = dictionary_get(blockState,K_BlockState_nroNodo);
		if( strcmp(IPnroDeNodo,IPnodoEnBlockState) == 0 ){
			char *pathTemp = dictionary_get(blockState,K_BlockState_temporaryPath);
			list_add(pathsTemporalesEnNodo,pathTemp);
		}
	}
	return pathsTemporalesEnNodo;
}

int obtenerCantidadDeNodosDiferentesEnBlockState(t_dictionary *fileState){

	t_list *nodosDiferentesEnBlockState = obtenerNodosEnBlockStateArray(fileState);
	int size = list_size(nodosDiferentesEnBlockState);
	list_destroy(nodosDiferentesEnBlockState);
	return size;
}

int obtenerCantidadDePathsTemporalesEnNodo(char *IPnodoEnBlockState,t_dictionary *fileState){

	t_list *listaPathsTemporalesEnArchivo= obtenerPathsTemporalesParaArchivo(IPnodoEnBlockState,fileState);
	int size = list_size(listaPathsTemporalesEnArchivo);
	list_destroy(listaPathsTemporalesEnArchivo);
	return size;
}
t_list* obtenerNodosEnBlockStateArray(t_dictionary *fileState){

	t_list *blocksStatesList = dictionary_get(fileState,K_FileState_arrayOfBlocksStates);
	int sizeBlocksList = list_size(blocksStatesList);

	int i;
	t_list *nodosEnBlockStateArray = list_create();
	for(i=0;i<sizeBlocksList;i++){

		t_dictionary *blockState = list_get(blocksStatesList,i);
		char *IPnroDeNodo = dictionary_get(blockState,K_BlockState_nroNodo);
		char *puertoNodo = dictionary_get(blockState,K_BlockState_puertoNodo);
		bool *estaEnLista = isNodoInList(nodosEnBlockStateArray,IPnroDeNodo);

		if(!(*estaEnLista)){
			t_dictionary *nodo = dictionary_create();
			dictionary_put(nodo,"ip",IPnroDeNodo);
			dictionary_put(nodo,"puerto",puertoNodo);
			list_add(nodosEnBlockStateArray,nodo);
		}
		free(estaEnLista);
	}
	return nodosEnBlockStateArray;
}

t_dictionary* obtenerNodoConMayorCantidadDeArchivosTemporales(t_dictionary *fileState){

	t_list *nodosEnBlockStateArray = obtenerNodosEnBlockStateArray(fileState);
	int size = list_size(nodosEnBlockStateArray);

	t_dictionary *nodoPivot = list_get(nodosEnBlockStateArray,0);

	int i;
	for(i=1;i<(size);i++){

		t_dictionary *nodo = list_get(nodosEnBlockStateArray,i);
		char *ipNroDeNodo = dictionary_get(nodo,"ip");
		char *ipNodoPivot = dictionary_get(nodoPivot,"ip");

		t_list *pathsTemporalesParaNodoPivot = obtenerPathsTemporalesParaArchivo(ipNodoPivot,fileState);
		t_list *pathsTemporalesParaNodo = obtenerPathsTemporalesParaArchivo(ipNroDeNodo,fileState);

		int cantDePathsTempEnNodo = list_size(pathsTemporalesParaNodo);
		int cantDePathsTempEnNodoPivot = list_size(pathsTemporalesParaNodoPivot);

		if(cantDePathsTempEnNodo < pathsTemporalesParaNodoPivot){
			//dejo el nodo pivot
		}else{
			nodoPivot = nodo;
		}
	}

	list_destroy(nodosEnBlockStateArray);
	return nodoPivot;
}

bool* isPathInList(t_list *lista,char *path)
{
	int listSize = list_size(lista);
	int i;
	bool *response = malloc(sizeof(bool));
	response = false;
	for(i=0;i<listSize;i++){
		char *temporaryPathInList = list_get(lista,i);
		if(strcmp(path,temporaryPathInList) == 0){
			*response = true;
			return response;
		}
	}
	return response;
}

bool* isNodoInList(t_list *lista,char *IPnroDeNodo)
{
	int listSize = list_size(lista);
	int i;
	bool *response = malloc(sizeof(bool));
	*response = false;
	for(i=0;i<listSize;i++){
		t_dictionary *nodo= list_get(lista,i);
		char *IPnroDeNodoEnLista = dictionary_get(nodo,"ip");
		if( strcmp(IPnroDeNodo,IPnroDeNodoEnLista) == 0 ){
			*response = true;
			return response;
		}
	}
	return response;
}

t_dictionary *getNodoState(char *ipNroNodo)
{
	t_dictionary *nodoState = dictionary_get(nodosData,ipNroNodo);
	return nodoState;
}

int *getNodoState_OperacionesEnProceso(t_dictionary *nodoState)
{
	int *operacionesEnProceso = dictionary_get(nodoState,K_Nodo_OperacionesEnProceso);
	return operacionesEnProceso;
}

t_list *getNodoState_listaTemporales(t_dictionary *nodoState)
{
	//TODO ya no la uso
	t_list *listaTemporales = dictionary_get(nodoState,K_Nodo_ArchivosTemporales);
	return listaTemporales;
}

int obtenerPosicionDeBloqueEnBlockStatesList(char *ipNodo,char *bloque,t_dictionary *fileState)
{
	t_list *lista = dictionary_get(fileState,K_FileState_arrayOfBlocksStates);
	int size = list_size(lista);
	int i;
	for(i=0;i<size;i++){

		t_dictionary *blockState = list_get(lista,i);
		char *_ipNodo= dictionary_get(blockState,K_BlockState_nroNodo);
		char *_bloque = dictionary_get(blockState,K_BlockState_nroBloque);
		if((strcmp(_ipNodo,ipNodo)==0)&&(strcmp(bloque,_bloque)==0)){
			return i;
		}
	}
	return -1;
}

t_dictionary *obtenerBlockState(char *tempPath,t_dictionary *fileState)
{
	t_list *blockStatesList = dictionary_get(fileState,K_FileState_arrayOfBlocksStates);
	int size = list_size(blockStatesList);
	t_dictionary *blockState;

	int i;
	for(i=0;i<size;i++){
		t_dictionary *_blockState = list_get(blockStatesList,i);
		char *_tempPath = dictionary_get(_blockState,K_BlockState_temporaryPath);
		if(strcmp(_tempPath,tempPath)==0){
			blockState = _blockState ;
		}
	}
	return blockState;
}

void setBlockStatesListInReducingState(t_dictionary *fileState)
{
	t_list *blockStatesList = dictionary_get(fileState,K_FileState_arrayOfBlocksStates);
	int size = list_size(blockStatesList);
	int i;
	for(i=0;i<size;i++){
		t_dictionary *blockState = list_get(blockStatesList,i);
		StatusBlockState *state = dictionary_get(blockState,K_BlockState_state);
		*state = K_IN_REDUCING;
	}
}

void informarTareasPendientesDeMapping(char *path,int socket,t_dictionary *fileState)
{
	t_list *listBlocksStates = dictionary_get(fileState,K_FileState_arrayOfBlocksStates);
	int size = list_size(listBlocksStates);
	int i,count=size;
	for(i=0;i<size;i++){
		t_dictionary *blockState = list_get(listBlocksStates,i);
		StatusBlockState *state = dictionary_get(blockState,K_BlockState_state);
		if((*state)==K_MAPPED){count--;}
	}
	pthread_mutex_lock(&mutexLog);
	char *log = string_from_format("el job %d para el archivo %s debe mappear %d bloques",socket,path,count);
	log_debug(logFile,log); free(log);
	pthread_mutex_unlock(&mutexLog);
}

//fullDataTable
void agregarFullDataTable(t_list *table,char *path){

	sem_wait(&semFullDataTables);
	dictionary_put(fullDataTables,path,table);
	sem_post(&semFullDataTables);
}
t_list *getCopyFullDataTable(char *path){

	sem_wait(&semFullDataTables);

	t_list *fullDataTable = dictionary_get(fullDataTables,path);
	int cantidadDeBloques = list_size(fullDataTable);

	t_list *listaPadreDeBloques = list_create();
		int i,j;

		for(i=0;i<cantidadDeBloques;i++){

			t_list *listaHijaDeCopias = list_create();
			t_list *listaDeCopias = list_get(fullDataTable,i);
			int nroDeCopias= list_size(listaDeCopias);

			for(j=0;j<nroDeCopias;j++){

				t_dictionary *dicPivot = list_get(listaDeCopias,j);
				t_dictionary *dic = dictionary_create();
				char *ipNodoPivot = dictionary_get(dicPivot,K_Copia_IPNodo);
				char *puertoNodoPivot = dictionary_get(dicPivot,K_Copia_PuertoNodo);
				char *nroDeBloquePivot = dictionary_get(dicPivot,K_Copia_NroDeBloque);
				bool *estado = malloc(sizeof(bool));
				*estado = true;

				char *ipNodo = malloc(strlen(ipNodoPivot)+1);
				strcpy(ipNodo,ipNodoPivot);
				char *puertoNodo = malloc(strlen(puertoNodoPivot)+1);
				strcpy(puertoNodo,puertoNodoPivot);
				char *nroDeBloque = malloc(strlen(nroDeBloquePivot)+1);
				strcpy(nroDeBloque,nroDeBloquePivot);

				dictionary_put(dic,K_Copia_IPNodo,ipNodo);
				dictionary_put(dic,K_Copia_PuertoNodo,puertoNodo);
				dictionary_put(dic,K_Copia_NroDeBloque,nroDeBloque);
				dictionary_put(dic,K_Copia_Estado,estado);
				list_add(listaHijaDeCopias,dic);
			}
			list_add(listaPadreDeBloques,listaHijaDeCopias);
		}

	sem_post(&semFullDataTables);

	return listaPadreDeBloques;
}
