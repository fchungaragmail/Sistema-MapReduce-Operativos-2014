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

//file_StatusData --> keys
#define K_file_StatusData_combinerMode "FileCombinerMode"
#define K_file_StatusData_Bloques "file_StatusData_Bloques"

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
void addFileFullData(int sckt, char* path, t_list *fullData);
void reloadFileFullData(int sckt, char* path, t_list *fullData);
t_dictionary *getNodoState(char *ipNroNodo);
int getNodoState_OperacionesEnProceso(t_dictionary *nodoState);
t_list *getNodoState_listaTemporales(t_dictionary *nodoState);
t_dictionary *getFileStateForPath(char *path);
t_dictionary *getFileState(char *path);
//nodosData
void incrementarOperacionesEnProcesoEnNodo(char *IPnroNodo);
void decrementarOperacionesEnProcesoEnNodo(char *IPnroNodo);
int getCantidadDeOperacionesEnProcesoEnNodo(char *IPnroNodo);
void addTemporaryFilePathToNodoData(char *IPnroNodo,char* filePath);
void removeTemporaryFilePathToNodoData(char *IPnroNodo,char* filePath);
//filesToProcess
void addNewFileForProcess(char *file_Path,_Bool *soportaCombiner,int jobSocket);//crea un fileState
void darDeBajaCopiaEnBloqueYNodo(char*path,int skct,char* nroBloque,char *IP_nroNodo,int indiceDeBloqe);
t_list* obtenerCopiasParaBloqueDeArchivo(int socket,int bloque, char *path);
int obtenerNumeroDeCopiasParaArchivo(int socket,char *path);
int obtenerNumeroDeBloquesParaArchivo(int socket,char *path);
void destruirFile_StatusData(int sckt, char *path);
bool* soportaCombiner(int sckt, char *path);
//filesStates
void changeFileBlockState(char* path,int nroBloque,StatusBlockState* nuevoEstado,char* temporaryPath);
t_dictionary *getFileStateForPath(char *path);
void destruirFileState(char* path);
t_list* obtenerPathsTemporalesParaNodo(char *path,char *IPnodoEnBlockState);
int obtenerCantidadDePathsTemporalesEnNodo(char *path,char *IPnodoEnBlockState);
t_list* obtenerNodosEnBlockStateArray(char *path);
int obtenerCantidadDeNodosDiferentesEnBlockState(char *path);
char* obtenerNodoConMayorCantidadDeArchivosTemporales(char *path);
bool* isPathInList(t_list *lista,char *path);
bool* isNodoInList(t_list *lista,char *IPnroDeNodo);
int obtenerPosicionDeBloqueEnBlockStatesList(char *path,char *ipNodo,char *bloque);
t_dictionary *obtenerBlockState(char *path,char *tempPath);
void setBlockStatesListInReducingState(char *path);

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

void addFileFullData(int sckt, char* path, t_list *fullData)
{	// fullData me lo envia el FS
	// aca va la respuesta del FS
	// tiene la info de los bloques y nodos donde esta el archivo a procesar

	t_dictionary *filesToProcessPerJob = getFilesToProcessPerJob(sckt);
	t_dictionary *file_StatusData = getFile_StatusData(filesToProcessPerJob,path);
	dictionary_put(file_StatusData,K_file_StatusData_Bloques,fullData);
	int nroDeBloques = list_size(fullData);

	//Construyo un "fileState"
	t_list *blocksStatesList = list_create();
	int i;
	for(i=0;i<nroDeBloques;i++){

		t_dictionary *blockState = dictionary_create();
		StatusBlockState *blkState = malloc(sizeof(StatusBlockState));
		*blkState = K_UNINITIALIZED;

		char *uninitializedNroDeBloqe = malloc(strlen(K_BlockState_UninitializedBlqe));
		char *uninitializedNodo = malloc(strlen(K_BlockState_UninitializedNodo));
		char *uninitializedPath = malloc(strlen(K_BlockState_UninitializedPath));

		strcpy(uninitializedNodo,K_BlockState_UninitializedNodo);
		strcpy(uninitializedPath,K_BlockState_UninitializedPath);
		strcpy(uninitializedNroDeBloqe,K_BlockState_UninitializedBlqe);

		dictionary_put(blockState,K_BlockState_state,blkState);
		dictionary_put(blockState,K_BlockState_nroBloque,uninitializedNroDeBloqe);
		dictionary_put(blockState,K_BlockState_nroNodo,K_BlockState_UninitializedNodo);
		dictionary_put(blockState,K_BlockState_temporaryPath,K_BlockState_UninitializedPath);

		list_add(blocksStatesList,blockState);
	}

	//Agrego "filesState" a "filesStates"
	t_dictionary *fileState = dictionary_create();
	dictionary_put(fileState,K_FileState_arrayOfBlocksStates,blocksStatesList);

	sem_wait(&semFilesStates);
	dictionary_put(filesStates,path,fileState);
	sem_post(&semFilesStates);
}

void reloadFileFullData(int sckt, char* path, t_list *fullData)
{
	t_dictionary *filesToProcessPerJob =getFilesToProcessPerJob(sckt);
	t_dictionary *file_StatusData = getFile_StatusData(filesToProcessPerJob,path);

	bool *combinerMode = malloc(sizeof(bool));
	combinerMode = dictionary_get(file_StatusData,K_file_StatusData_combinerMode);

	dictionary_clean(file_StatusData);
	dictionary_put(file_StatusData ,K_file_StatusData_Bloques,fullData);
	dictionary_put(file_StatusData ,K_file_StatusData_combinerMode,combinerMode);
}

void changeFileBlockState(char *path,int nroBloque,StatusBlockState *nuevoEstado,char *temporaryPath)
{	//ESTO LO HAGO EN MI ESTRUCTURA "filesStates"
	//VER SI ES nroBloque o (nroBloque-1)!!!!!

	t_dictionary *fileState = getFileState(path);
	t_list *blocksStatesArray= dictionary_get(fileState,K_FileState_arrayOfBlocksStates);
	t_dictionary *blockState = list_get(blocksStatesArray,nroBloque);

	//ESTO NO FUNCA !!!!!
	dictionary_put(blockState,K_BlockState_state,nuevoEstado);
	dictionary_put(blockState,K_BlockState_temporaryPath,temporaryPath);
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
{//se llama cuando envio un pedido de map o reduce al Job

	sem_wait(&semNodosData);
	bool hasKey = dictionary_has_key(nodosData,IPnroNodo);
	sem_post(&semNodosData);

		if(hasKey){
			//busco el dic y le sumo una operacionProcesando
			t_dictionary *nodoState = getNodoState(IPnroNodo);

			int operacionesEnProceso = getNodoState_OperacionesEnProceso(nodoState);
			//VER SI ESTO FUNCA !!!!!
			operacionesEnProceso = operacionesEnProceso + 1;
			sem_wait(&semNodoState);
			dictionary_put(nodoState,K_Nodo_OperacionesEnProceso,operacionesEnProceso);
			sem_post(&semNodoState);

		}else{
			//creo un nuevo dic y le sumo una operacionProcesando
			t_dictionary *nodoState = dictionary_create();
			int operacionesEnProceso = 1;

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
		//VERIFICAR SI ESTO FUNCA !!!!!!!!!!!!!!!!!!
		t_dictionary *nodoState = getNodoState(IPnroNodo);

		int operacionesEnProceso = getNodoState_OperacionesEnProceso(nodoState);
		operacionesEnProceso = operacionesEnProceso - 1;
		sem_wait(&semNodoState);
		dictionary_put(nodoState,K_Nodo_OperacionesEnProceso,operacionesEnProceso);
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

void darDeBajaCopiaEnBloqueYNodo(char*path,int skct,char *nroBloque,char *IP_Nodo,int indiceDeBloqe )
{	//archivoParcial_Path seria algo como librazo-horaDeEnvio.txt
	//debo conseguir el char* librazo.txt --> supongamos que es char* _archivo;

	t_dictionary *filesToProcessPerJob = getFilesToProcessPerJob(skct);
	t_dictionary *file_StatusData = getFile_StatusData(filesToProcessPerJob,path);

	t_list *copiasArray = dictionary_get(file_StatusData,K_file_StatusData_Bloques);
	t_list *copiasPorBloqueList = list_get(copiasArray,indiceDeBloqe);
	int listaSize = list_size(copiasPorBloqueList);
	int i;
	for(i=0;i<listaSize;i++){

		t_dictionary *copiaDeBloque = list_get(copiasPorBloqueList,i);
		char *IPNodo = dictionary_get(copiaDeBloque,K_Copia_IPNodo);
		if( strcmp(IPNodo,IP_Nodo) == 0 ){

			char *nroDeBloqe = dictionary_get(copiaDeBloque,K_Copia_NroDeBloque);
			char *newNroDeBloqe = malloc(strlen(nroDeBloqe));
			strcpy(newNroDeBloqe,nroDeBloqe);

			t_dictionary *newCopiaDeBloqe = dictionary_create();
			dictionary_put(newCopiaDeBloqe,K_Copia_IPNodo,K_Copia_DarDeBajaIPNodo);
			dictionary_put(newCopiaDeBloqe,K_Copia_NroDeBloque,newNroDeBloqe);

			dictionary_destroy(copiaDeBloque);
			list_remove(copiasPorBloqueList,i);
			list_add_in_index(copiasPorBloqueList,i,newCopiaDeBloqe);

			break;
		}
	}
}

t_list* obtenerCopiasParaBloqueDeArchivo(int socket,int bloque ,char *path)
{
	t_dictionary *filesToProcessPerJob = getFilesToProcessPerJob(socket);
	t_dictionary *file_StatusData = getFile_StatusData(filesToProcessPerJob,path);

	t_list *blocksList = dictionary_get(file_StatusData,K_file_StatusData_Bloques);
	t_list *listaDeCopias = list_get(blocksList,bloque);
	int nroDeBloques = list_size(blocksList);
	int nroDeCopias = list_size(listaDeCopias);
	int i,j;

	t_list *copiasParaBloque = list_create();

	for(i=0;i<nroDeCopias;i++){
		list_get(blocksList,bloque);
		t_dictionary *copia = list_get(listaDeCopias,i);
		list_add(copiasParaBloque, copia);
	}

	return copiasParaBloque;
}

int obtenerNumeroDeCopiasParaArchivo(int socket,char *path)
{
	/*t_dictionary *filesToProcessPerJob = getFilesToProcessPerJob(socket) ;
	t_dictionary *file_StatusData = getFile_StatusData(filesToProcessPerJob,path);
	t_list *listaDeBloqes = dictionary_get(file_StatusData,K_file_StatusData_Bloques);
	t_list *listaDeCopias =
	int nroDeCopias = list_;

	return nroDeCopias;*/
	return -1;
}

int obtenerNumeroDeBloquesParaArchivo(int socket,char *path)
{
	char *key = intToCharPtr(socket);
	t_dictionary *filesToProcessPerJob = getFilesToProcessPerJob(socket);
	t_dictionary *file_StatusData = getFile_StatusData(filesToProcessPerJob,path);
	t_list *listaDeBloqes = dictionary_get(file_StatusData,K_file_StatusData_Bloques);
	int nroDeBloques = list_size(listaDeBloqes);

	return nroDeBloques;
}

void destruirFile_StatusData(int sckt, char *path)
{
	t_dictionary *filesToProcessPerJob = getFilesToProcessPerJob(sckt);
	t_dictionary *file_StatusData = getFile_StatusData(filesToProcessPerJob,path);
	free(file_StatusData); //SE LIBERAN TODOS LOS SUB-DICCIONARIOS QUE TIENE ???
}
void destruirFileState(char* path)
{
	t_dictionary *fileState = getFileState(path);
	free(fileState); //SE LIBERAN TODOS LOS SUB-DICCIONARIOS QUE TIENE ???
}

bool* soportaCombiner(int sckt, char *path)
{
	t_dictionary *filesToProcessPerJob = getFilesToProcessPerJob(sckt);
	t_dictionary *file_StatusData = getFile_StatusData(filesToProcessPerJob,path);
	bool *soportaCombiner = dictionary_get(file_StatusData,K_file_StatusData_combinerMode);

	return soportaCombiner;
}

t_list* obtenerPathsTemporalesParaNodo(char *path,char *IPnodoEnBlockState){

	t_dictionary* fileState = getFileState(path);
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

int obtenerCantidadDeNodosDiferentesEnBlockState(char *path){

	t_list *nodosDiferentesEnBlockState = obtenerNodosEnBlockStateArray(path);
	int size = list_size(nodosDiferentesEnBlockState);
	return size;
}

int obtenerCantidadDePathsTemporalesEnNodo(char *path,char *IPnodoEnBlockState){

	t_list *listaPathsTemporalesParaNodo = obtenerPathsTemporalesParaNodo(path,IPnodoEnBlockState);
	int size = list_size(listaPathsTemporalesParaNodo);
	list_destroy(listaPathsTemporalesParaNodo);
	return size;
}
t_list* obtenerNodosEnBlockStateArray(char *path){

	t_dictionary* fileState = getFileState(path);
	t_list *blocksStatesList = dictionary_get(fileState,K_FileState_arrayOfBlocksStates);
	int sizeBlocksList = list_size(blocksStatesList);

	int i;
	t_list *nodosEnBlockStateArray = list_create();
	for(i=0;i<sizeBlocksList;i++){

		t_dictionary *blockState = list_get(blocksStatesList,i);
		char *IPnroDeNodo = dictionary_get(blockState,K_BlockState_nroNodo);
		bool *estaEnLista = isNodoInList(nodosEnBlockStateArray,IPnroDeNodo);
		if(!(*estaEnLista)){
			list_add(nodosEnBlockStateArray,IPnroDeNodo);
		}
		free(estaEnLista);
	}
	return nodosEnBlockStateArray;
}

char* obtenerNodoConMayorCantidadDeArchivosTemporales(char *path){

	t_list *nodosEnBlockStateArray = obtenerNodosEnBlockStateArray(path);
	int size = list_size(nodosEnBlockStateArray);

	char *ipNodoPivot= list_get(nodosEnBlockStateArray,0);

	int i;
	for(i=1;i<(size);i++){
		char *ipNroDeNodo = list_get(nodosEnBlockStateArray,i);

		t_list *pathsTemporalesParaNodoPivot = obtenerPathsTemporalesParaNodo(path,ipNodoPivot);
		t_list *pathsTemporalesParaNodo = obtenerPathsTemporalesParaNodo(path,ipNroDeNodo);

		int cantDePathsTempEnNodo = list_size(pathsTemporalesParaNodo);
		int cantDePathsTempEnNodoPivot = list_size(pathsTemporalesParaNodoPivot);

		if(cantDePathsTempEnNodo < pathsTemporalesParaNodoPivot){
			//dejo el nodo pivot
		}else{
			ipNodoPivot = ipNroDeNodo;
		}
	}

	list_destroy(nodosEnBlockStateArray);
	return ipNodoPivot;
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
		char *IPnroDeNodoEnLista= list_get(lista,i);
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

t_dictionary *getFileStateForPath(char *path)
{
	t_dictionary *fileState;
	sem_wait(&semFilesStates);
	fileState = dictionary_get(filesStates,path);
	sem_post(&semFilesStates);
	return fileState;
}

t_dictionary *getFileState(char *path)
{
	sem_wait(&semFilesStates);
	t_dictionary *fileState = dictionary_get(filesStates,path);
	sem_post(&semFilesStates);

	return fileState;
}

int obtenerPosicionDeBloqueEnBlockStatesList(char *path,char *ipNodo,char *bloque)
{
	t_dictionary *fileState = getFileState(path);
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
}

t_dictionary *obtenerBlockState(char *path,char *tempPath)
{
	t_dictionary *fileState = getFileState(path);
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

void setBlockStatesListInReducingState(char *path)
{
	t_dictionary *fileState = getFileState(path);
	t_list *blockStatesList = dictionary_get(fileState,K_FileState_arrayOfBlocksStates);
	int size = list_size(blockStatesList);
	int i;
	for(i=0;i<size;i++){
		t_dictionary *blockState = list_get(blockStatesList,i);
		StatusBlockState *state = dictionary_get(blockState,K_BlockState_state);
		*state = K_IN_REDUCING;
	}
}
