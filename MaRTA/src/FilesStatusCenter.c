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
void addFileFullData(int sckt, char* path, Message *recvMessage);
void reloadFilaDeFileFullData(int sckt, char* path, t_list *fullData,int nroDeBloqe);
t_dictionary *getNodoState(char *ipNroNodo);
int getNodoState_OperacionesEnProceso(t_dictionary *nodoState);
t_list *getNodoState_listaTemporales(t_dictionary *nodoState);
void informarTareasPendientesDeMapping(char *path,int socket);
//nodosData
void incrementarOperacionesEnProcesoEnNodo(char *IPnroNodo);
void decrementarOperacionesEnProcesoEnNodo(char *IPnroNodo);
int getCantidadDeOperacionesEnProcesoEnNodo(char *IPnroNodo);
void addTemporaryFilePathToNodoData(char *IPnroNodo,char* filePath);
void removeTemporaryFilePathToNodoData(char *IPnroNodo,char* filePath);
//filesToProcess
void addNewFileForProcess(char *file_Path,_Bool *soportaCombiner,int jobSocket);//crea un fileState
void darDeBajaCopiaEnBloqueYNodo(char*path,int skct,char *IP_Nodo);
t_list* obtenerCopiasParaBloqueDeArchivo(int socket,int bloque, char *path);
void destruirFile_StatusData(int sckt, char *path);
bool* soportaCombiner(int sckt, char *path);
//filesStates
t_dictionary *getFileStateForPath(char *path,int socket);
t_dictionary *getFileState(char *path,int socket);
void destruirFileState(char* path);
t_list* obtenerPathsTemporalesParaArchivo(char *path,char *IPnodoEnBlockState, int socket);
int obtenerCantidadDePathsTemporalesEnNodo(char *path,char *IPnodoEnBlockState,int socket);
t_list* obtenerNodosEnBlockStateArray(char *path,int socket);
int obtenerCantidadDeNodosDiferentesEnBlockState(char *path,int socket);
t_dictionary* obtenerNodoConMayorCantidadDeArchivosTemporales(char *path,int socket);
bool* isPathInList(t_list *lista,char *path);
bool* isNodoInList(t_list *lista,char *IPnroDeNodo);
int obtenerPosicionDeBloqueEnBlockStatesList(char *path,char *ipNodo,char *bloque,int socket);
t_dictionary *obtenerBlockState(char *path,char *tempPath,int socket);
void setBlockStatesListInReducingState(char *path,int socket);
void agregarFileState(int jobSocket,char *path, int cantidadDeBloques);
//fullDataTable
void agregarFullDataTable(t_list *table,char *path);
t_list *getCopyFullDataTable(char *path);
//FUNCIONES AUXILIARES
char *crearKeyParaFileState(char*path,int socket);

void initFilesStatusCenter()
{
	fs_socket=-1;

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

t_dictionary* getFile_StatusData(t_dictionary *filesToProcessPerJob,char *path)
{
	sem_wait(&semFilesToProcessPerJob);
	t_dictionary *file_StatusData = dictionary_get(filesToProcessPerJob,path);
	sem_post(&semFilesToProcessPerJob);
	return file_StatusData;
}

void addNewFileForProcess(char *file_Path,_Bool *soportaCombiner,int jobSocket)
{
	t_dictionary *filesToProcessPerJob = getFilesToProcessPerJob(jobSocket);

	t_dictionary *file_StatusData = dictionary_create();
	dictionary_put(file_StatusData,K_file_StatusData_combinerMode,soportaCombiner);

	sem_wait(&semFilesToProcessPerJob);
	dictionary_put(filesToProcessPerJob,file_Path,file_StatusData);
	sem_post(&semFilesToProcessPerJob);
}

void addFileFullData(int jobSocket, char* path, Message *recvMessage)
{	// fullData me lo envia el FS
	// aca va la respuesta del FS
	// tiene la info de los bloques y nodos donde esta el archivo a procesar

	t_list *fullData;
	fullData = deserializarFullDataResponse(recvMessage);
	agregarFullDataTable(fullData,path);
	fullData = getCopyFullDataTable(path);

	t_dictionary *filesToProcessPerJob = getFilesToProcessPerJob(jobSocket);
	t_dictionary *file_StatusData = getFile_StatusData(filesToProcessPerJob,path);
	dictionary_put(file_StatusData,K_file_StatusData_Bloques,fullData);
	int nroDeBloques = list_size(fullData);

	agregarFileState(jobSocket,path,nroDeBloques);

}

void agregarFileState(int jobSocket,char *path, int cantidadDeBloques){
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

	char *key = crearKeyParaFileState(path,jobSocket);
	sem_wait(&semFilesStates);
	dictionary_put(filesStates,key,fileState);
	sem_post(&semFilesStates);
	free(key);
}

void reloadFilaDeFileFullData(int sckt, char* path, t_list *CopiasfullData,int nroDeBloqe)
{
	//--> FS responde con bloque de archivo pedido
	//-Comando: "DataFileResponse rutaDelArchivo Disponible"
	//-Data: Bloque
	// Ej: "0;Nodo1;3;Nodo8;2;Nodo2;45;"

	t_dictionary *filesToProcessPerJob =getFilesToProcessPerJob(sckt);
	t_dictionary *file_StatusData = getFile_StatusData(filesToProcessPerJob,path);
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
	sem_post(&semNodosData);

	if(hasKey){
		//el nodo existe
		t_dictionary *nodoState = getNodoState(IPnroNodo);
		int operacionesEnProceso = getNodoState_OperacionesEnProceso(nodoState);

		return operacionesEnProceso;
	}
	//else --> al nodo no se le asigno nada aun, entonces no tiene operacionesEnProceso
	return 0;
}

void incrementarOperacionesEnProcesoEnNodo(char *IPnroNodo)
{
	//se llama cuando envio un pedido de map o reduce al Job
	sem_wait(&semNodosData);
	bool hasKey = dictionary_has_key(nodosData,IPnroNodo);
	sem_post(&semNodosData);

		if(hasKey){
			//busco el dic y le sumo una operacionProcesando
			t_dictionary *nodoState = getNodoState(IPnroNodo);

			int operacionesEnProceso = getNodoState_OperacionesEnProceso(nodoState);
			t_list *tempPathsList = getNodoState_listaTemporales(nodoState);
			operacionesEnProceso = operacionesEnProceso + 1;

			char *log = string_from_format("el nodo %s esta procesando %d pedidos",IPnroNodo,operacionesEnProceso);
			log_trace(logFile,log);free(log);

			sem_wait(&semNodoState);
			dictionary_clean(nodoState);
			dictionary_put(nodoState,K_Nodo_ArchivosTemporales,tempPathsList);
			dictionary_put(nodoState,K_Nodo_OperacionesEnProceso,operacionesEnProceso);
			sem_post(&semNodoState);

		}else{
			//creo un nuevo dic y le sumo una operacionProcesando
			t_dictionary *nodoState = dictionary_create();
			int operacionesEnProceso = 1;

			char *log = string_from_format("el nodo %s esta procesando %d pedidos",IPnroNodo,operacionesEnProceso);
			log_trace(logFile,log);free(log);

			sem_wait(&semNodoState);
			dictionary_put(nodoState,K_Nodo_OperacionesEnProceso,operacionesEnProceso);
			//creo una lista de archivosTemporales
			t_list *archivosTemporales = list_create();
			dictionary_put(nodoState,K_Nodo_ArchivosTemporales,archivosTemporales);
			sem_post(&semNodoState);

			//agrego a nodosData
			sem_wait(&semNodosData);
			dictionary_put(nodosData,IPnroNodo,nodoState);
			sem_post(&semNodosData);
		}
}

void decrementarOperacionesEnProcesoEnNodo(char *IPnroNodo)
{
	//se llama cuando envio un pedido de map o reduce al Job
	sem_wait(&semNodosData);
	bool hasKey = dictionary_has_key(nodosData,IPnroNodo);
	sem_post(&semNodosData);

	if(hasKey){
		//busco el dic y le resto una operacionProcesando
		t_dictionary *nodoState = getNodoState(IPnroNodo);

		int operacionesEnProceso = getNodoState_OperacionesEnProceso(nodoState);
		t_list *tempPathsList = getNodoState_listaTemporales(nodoState);
		operacionesEnProceso = operacionesEnProceso - 1;

		char *log = string_from_format("el nodo %s esta procesando %d pedidos",IPnroNodo,operacionesEnProceso);
		log_trace(logFile,log);free(log);


		sem_wait(&semNodoState);
		dictionary_clean(nodoState);
		dictionary_put(nodoState,K_Nodo_OperacionesEnProceso,operacionesEnProceso);
		dictionary_put(nodoState,K_Nodo_ArchivosTemporales,tempPathsList);
		sem_post(&semNodoState);
	}
}
void removeTemporaryFilePathToNodoData(char *IPnroNodo,char* filePath){
	t_dictionary *nodoState = getNodoState(IPnroNodo);
	t_list *listaTemporales = getNodoState_listaTemporales(nodoState);
	int size = list_size(listaTemporales);
	int i;

	for(i=0;i<size;i++){
		char *tempPath = list_get(listaTemporales,i);

		if(strcmp(tempPath,filePath)==0){
			list_remove(listaTemporales,i);
		}
	}
}
void addTemporaryFilePathToNodoData(char *IPnroNodo,char* filePath)
{//el NodoState tiene que estar si o si creado, pues tiene que haber pasado por
 //"incrementarOperacionesEnProcesoEnNodo"

	t_dictionary *nodoState = getNodoState(IPnroNodo);
	t_list *listaTemporales = getNodoState_listaTemporales(nodoState);

	sem_wait(&semNodoState);
	list_add(listaTemporales,filePath);
	sem_post(&semNodoState);

}

void darDeBajaCopiaEnBloqueYNodo(char*path,int skct,char *IP_Nodo)
{	//archivoParcial_Path seria algo como librazo-horaDeEnvio.txt
	//debo conseguir el char* librazo.txt --> supongamos que es char* _archivo;

	t_dictionary *filesToProcessPerJob = getFilesToProcessPerJob(skct);
	t_dictionary *file_StatusData = getFile_StatusData(filesToProcessPerJob,path);

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

t_list* obtenerCopiasParaBloqueDeArchivo(int socket,int bloque ,char *path)
{
	t_dictionary *filesToProcessPerJob = getFilesToProcessPerJob(socket);
	t_dictionary *file_StatusData = getFile_StatusData(filesToProcessPerJob,path);

	t_list *blocksList =dictionary_get(file_StatusData,K_file_StatusData_Bloques);
	t_list *listaDeCopias = list_get(blocksList,bloque);

	return listaDeCopias;
}

bool* soportaCombiner(int sckt, char *path)
{
	t_dictionary *filesToProcessPerJob = getFilesToProcessPerJob(sckt);
	t_dictionary *file_StatusData = getFile_StatusData(filesToProcessPerJob,path);
	bool *soportaCombiner = dictionary_get(file_StatusData,K_file_StatusData_combinerMode);

	return soportaCombiner;
}

t_list* obtenerPathsTemporalesParaArchivo(char *path,char *IPnodoEnBlockState, int socket){

	t_dictionary* fileState = getFileState(path,socket);
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

int obtenerCantidadDeNodosDiferentesEnBlockState(char *path,int socket){

	t_list *nodosDiferentesEnBlockState = obtenerNodosEnBlockStateArray(path,socket);
	int size = list_size(nodosDiferentesEnBlockState);
	list_destroy(nodosDiferentesEnBlockState);
	return size;
}

int obtenerCantidadDePathsTemporalesEnNodo(char *path,char *IPnodoEnBlockState,int socket){

	t_list *listaPathsTemporalesEnArchivo= obtenerPathsTemporalesParaArchivo(path,IPnodoEnBlockState,socket);
	int size = list_size(listaPathsTemporalesEnArchivo);
	list_destroy(listaPathsTemporalesEnArchivo);
	return size;
}
t_list* obtenerNodosEnBlockStateArray(char *path,int socket){

	t_dictionary* fileState = getFileState(path,socket);
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

t_dictionary* obtenerNodoConMayorCantidadDeArchivosTemporales(char *path,int socket){

	t_list *nodosEnBlockStateArray = obtenerNodosEnBlockStateArray(path,socket);
	int size = list_size(nodosEnBlockStateArray);

	t_dictionary *nodoPivot = list_get(nodosEnBlockStateArray,0);

	int i;
	for(i=1;i<(size);i++){

		t_dictionary *nodo = list_get(nodosEnBlockStateArray,i);
		char *ipNroDeNodo = dictionary_get(nodo,"ip");
		char *ipNodoPivot = dictionary_get(nodoPivot,"ip");

		t_list *pathsTemporalesParaNodoPivot = obtenerPathsTemporalesParaArchivo(path,ipNodoPivot,socket);
		t_list *pathsTemporalesParaNodo = obtenerPathsTemporalesParaArchivo(path,ipNroDeNodo,socket);

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
	sem_wait(&semNodosData);
	t_dictionary *nodoState = dictionary_get(nodosData,ipNroNodo);
	sem_post(&semNodosData);

	return nodoState;
}

int getNodoState_OperacionesEnProceso(t_dictionary *nodoState)
{
	sem_wait(&semNodoState);
	int operacionesEnProceso = dictionary_get(nodoState,K_Nodo_OperacionesEnProceso);
	sem_post(&semNodoState);

	return operacionesEnProceso;
}

t_list *getNodoState_listaTemporales(t_dictionary *nodoState)
{
	sem_wait(&semNodoState);
	t_list *listaTemporales = dictionary_get(nodoState,K_Nodo_ArchivosTemporales);
	sem_post(&semNodoState);

	return listaTemporales;
}

t_dictionary *getFileStateForPath(char *path,int socket)
{
	char *key = crearKeyParaFileState(path,socket);
	t_dictionary *fileState;
	sem_wait(&semFilesStates);
	fileState = dictionary_get(filesStates,key);
	sem_post(&semFilesStates);

	free(key);
	return fileState;
}

t_dictionary *getFileState(char *path,int socket)
{
	char *key = crearKeyParaFileState(path,socket);
	sem_wait(&semFilesStates);
	t_dictionary *fileState = dictionary_get(filesStates,key);
	sem_post(&semFilesStates);

	free(key);
	return fileState;
}

int obtenerPosicionDeBloqueEnBlockStatesList(char *path,char *ipNodo,char *bloque,int socket)
{
	t_dictionary *fileState = getFileState(path,socket);
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

t_dictionary *obtenerBlockState(char *path,char *tempPath,int socket)
{
	t_dictionary *fileState = getFileState(path,socket);
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

void setBlockStatesListInReducingState(char *path,int socket)
{
	t_dictionary *fileState = getFileState(path,socket);
	t_list *blockStatesList = dictionary_get(fileState,K_FileState_arrayOfBlocksStates);
	int size = list_size(blockStatesList);
	int i;
	for(i=0;i<size;i++){
		t_dictionary *blockState = list_get(blockStatesList,i);
		StatusBlockState *state = dictionary_get(blockState,K_BlockState_state);
		*state = K_IN_REDUCING;
	}
}

void informarTareasPendientesDeMapping(char *path,int socket)
{
	t_dictionary *fileState = getFileState(path,socket);
	t_list *listBlocksStates = dictionary_get(fileState,K_FileState_arrayOfBlocksStates);
	int size = list_size(listBlocksStates);
	int i,count=size;
	for(i=0;i<size;i++){
		t_dictionary *blockState = list_get(listBlocksStates,i);
		StatusBlockState *state = dictionary_get(blockState,K_BlockState_state);
		if((*state)==K_MAPPED){count--;}
	}

	char *log = string_from_format("el job %d para el archivo %s debe mappear %d bloques",socket,path,count);
	log_trace(logFile,log); free(log);
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
			int k = 1;
			for(j=0;j<nroDeCopias;j++){

				t_dictionary *dicPivot = list_get(listaDeCopias,j);
				t_dictionary *dic = dictionary_create();
				char *ipNodoPivot = dictionary_get(dicPivot,K_Copia_IPNodo);
				char *puertoNodoPivot = dictionary_get(dicPivot,K_Copia_PuertoNodo);
				char *nroDeBloquePivot = dictionary_get(dicPivot,K_Copia_NroDeBloque);
				bool *estado = malloc(sizeof(bool));
				*estado = true;

				char *ipNodo = malloc(strlen(ipNodoPivot));
				strcpy(ipNodo,ipNodoPivot);
				char *puertoNodo = malloc(strlen(puertoNodoPivot));
				strcpy(puertoNodo,puertoNodoPivot);
				char *nroDeBloque = malloc(strlen(nroDeBloquePivot));
				strcpy(nroDeBloque,nroDeBloquePivot);

				dictionary_put(dic,K_Copia_IPNodo,ipNodo);
				dictionary_put(dic,K_Copia_PuertoNodo,puertoNodo);
				dictionary_put(dic,K_Copia_NroDeBloque,nroDeBloque);
				dictionary_put(dic,K_Copia_Estado,estado);
				k=k+3;
				list_add(listaHijaDeCopias,dic);
			}
			list_add(listaPadreDeBloques,listaHijaDeCopias);
		}

	sem_post(&semFullDataTables);

	return listaPadreDeBloques;
}


char *crearKeyParaFileState(char*path,int socket){

	char *key = string_new();
	string_append(&key,path);
	char *sckt = intToCharPtr(socket);
	string_append(&key,sckt);
	return key;
}
