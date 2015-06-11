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
#define K_file_StatusData_filePath "FilePath"
#define K_file_StatusData_combinerMode "FileCombinerMode"
#define K_file_StatusData_Bloques "file_StatusData_Bloques"
#define K_file_StatusData_CopiasSize "file_StatusData_Copias"
#define K_file_StatusData_BloquesSize "file_StatusData_BloquesSize"

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
void addFileFullData(int sckt, char* path,int nroDeBloques,int nroDeCopias, t_list *fullData);
//nodosData
void incrementarOperacionesEnProcesoEnNodo(char *IPnroNodo);
void decrementarOperacionesEnProcesoEnNodo(char *IPnroNodo);
int getCantidadDeOperacionesEnProcesoEnNodo(char *IPnroNodo);
void addTemporaryFilePathToNodoData(char *IPnroNodo,char* filePath);
//filesToProcess
void addNewFileForProcess(char *file_Path,_Bool *soportaCombiner,int jobSocket);//crea un fileState
void darDeBajaCopiaEnBloqueYNodo(char*path,int skct,int nroBloque,char *IP_nroNodo);
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
	char* skct_key = intToCharPtr(jobSocket);//VER COMO FUNCA ESTA FC
	t_dictionary *filesToProcessPerJob = dictionary_create();

	sem_wait(&semFilesToProcess);
	dictionary_put(filesToProcess, skct_key, filesToProcessPerJob);
	sem_post(&semFilesToProcess);

	free(skct_key);
}

void addNewFileForProcess(char *file_Path,_Bool *soportaCombiner,int jobSocket)
{
	char* dic_key = intToCharPtr(jobSocket);

	sem_wait(&semFilesToProcess);
	t_dictionary *filesToProcessPerJob = dictionary_get(filesToProcess,dic_key);
	sem_post(&semFilesToProcess);

	t_dictionary *file_StatusData = dictionary_create();
	dictionary_put(file_StatusData,K_file_StatusData_combinerMode,soportaCombiner);
	dictionary_put(file_StatusData,K_file_StatusData_filePath,file_Path);

	sem_wait(&semFilesToProcessPerJob);
	dictionary_put(filesToProcessPerJob,file_Path,file_StatusData);
	sem_post(&semFilesToProcessPerJob);

	free(dic_key);
}

void addFileFullData(int sckt, char* path,int nroDeBloques,int nroDeCopias, t_list *fullData)
{	// fullData me lo envia el FS
	// aca va la respuesta del FS
	// tiene la info de los bloques y nodos donde esta el archivo a procesar

	char *key = intToCharPtr(sckt);

	sem_wait(&semFilesToProcess);
	t_dictionary *filesToProcessPerJob = dictionary_get(filesToProcess,key);
	sem_post(&semFilesToProcess);

	sem_wait(&semFilesToProcessPerJob);
	t_dictionary *file_StatusData = dictionary_get(filesToProcessPerJob,path);
	sem_post(&semFilesToProcessPerJob);

	dictionary_put(file_StatusData,K_file_StatusData_BloquesSize,nroDeBloques);
	dictionary_put(file_StatusData,K_file_StatusData_CopiasSize,nroDeCopias);


	dictionary_put(file_StatusData,K_file_StatusData_Bloques,fullData);
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
	dictionary_put(fileState,K_FileState_size,nroDeBloques);

	sem_wait(&semFilesStates);
	dictionary_put(filesStates,path,fileState);
	sem_post(&semFilesStates);

	free(key);
}

void changeFileBlockState(char *path,int nroBloque,StatusBlockState *nuevoEstado,char *temporaryPath)
{	//ESTO LO HAGO EN MI ESTRUCTURA "filesStates"
	//VER SI ES nroBloque o (nroBloque-1)!!!!!

	sem_wait(&semFilesStates);
	t_dictionary *fileState = dictionary_get(filesStates,path);
	sem_post(&semFilesStates);



	t_list *blocksStatesArray= dictionary_get(fileState,K_FileState_arrayOfBlocksStates);

	t_dictionary *blockState = list_get(blocksStatesArray,nroBloque);
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
		sem_wait(&semNodosData);
		t_dictionary *nodoState = dictionary_get(nodosData,IPnroNodo);
		sem_post(&semNodosData);

		sem_wait(&semNodoState);
		int operacionesEnProceso = dictionary_get(nodoState,K_Nodo_OperacionesEnProceso);
		sem_post(&semNodoState);

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
			sem_post(&semNodosData);
			t_dictionary *nodoState = dictionary_get(nodosData,IPnroNodo);
			sem_post(&semNodosData);

			sem_wait(&semNodoState);
			int operacionesEnProceso = dictionary_get(nodoState,K_Nodo_OperacionesEnProceso);
			operacionesEnProceso = operacionesEnProceso + 1;
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
		//busco el dic y le sumo una operacionProcesando
		sem_wait(&semNodosData);
		t_dictionary *nodoState = dictionary_get(nodosData,IPnroNodo);
		sem_post(&semNodosData);

		sem_wait(&semNodoState);
		int operacionesEnProceso = dictionary_get(nodoState,K_Nodo_OperacionesEnProceso);
		operacionesEnProceso = operacionesEnProceso - 1;
		dictionary_put(nodoState,K_Nodo_OperacionesEnProceso,operacionesEnProceso);
		sem_post(&semNodoState);
	}

}

void addTemporaryFilePathToNodoData(char *IPnroNodo,char* filePath)
{//el NodoState tiene que estar si o si creado, pues tiene que haber pasado por
 //"incrementarOperacionesEnProcesoEnNodo"

	sem_wait(&semNodosData);
	t_dictionary *nodoState = dictionary_get(nodosData,IPnroNodo);
	sem_post(&semNodosData);

	sem_wait(&semNodoState);
	t_list *listaTemporales = dictionary_get(nodoState,K_Nodo_ArchivosTemporales);
	list_add(listaTemporales,filePath);
	sem_post(&semNodoState);

}

void darDeBajaCopiaEnBloqueYNodo(char*path,int skct,int nroBloque,char *IP_Nodo )
{	//archivoParcial_Path seria algo como librazo-horaDeEnvio.txt
	//debo conseguir el char* librazo.txt --> supongamos que es char* _archivo;

	char *key = intToCharPtr(skct);
	sem_wait(&semFilesToProcess);
	t_dictionary *filesToProcessPerJob = dictionary_get(filesToProcess,key);
	sem_post(&semFilesToProcess);

	sem_wait(&semFilesToProcessPerJob);
	t_dictionary *file_StatusData = dictionary_get(filesToProcessPerJob,path);
	sem_post(&semFilesToProcessPerJob);

	int nroDeBloques = dictionary_get(file_StatusData,K_file_StatusData_BloquesSize);
	int nroDeCopias = dictionary_get(file_StatusData,K_file_StatusData_CopiasSize);

	t_dictionary *(*copiasArray)[nroDeBloques][nroDeCopias];
	copiasArray = dictionary_get(file_StatusData,K_file_StatusData_Bloques);
	int i;
	for(i=0;i<3;i++){

		t_dictionary *copiaDeBloque = (*copiasArray)[nroBloque][i];
		char *IPNodo = dictionary_get(copiaDeBloque,K_Copia_IPNodo);
		if( strcmp(IPNodo,IP_Nodo) == 0 ){

			dictionary_put(copiaDeBloque,K_Copia_IPNodo,K_Copia_DarDeBajaIPNodo);
			break;
		}
	}
	free(key);
}

t_dictionary *getFileStateForPath(char *path)
{
	t_dictionary *fileState;
	sem_wait(&semFilesStates);
	fileState = dictionary_get(filesStates,path);
	sem_post(&semFilesStates);
	return fileState;
}

t_list* obtenerCopiasParaBloqueDeArchivo(int socket,int bloque ,char *path)
{
	char *key = intToCharPtr(socket);
	sem_wait(&semFilesToProcess);
	t_dictionary *filesToProcessPerJob = dictionary_get(filesToProcess,key);
	sem_post(&semFilesToProcess);

	sem_wait(&semFilesToProcessPerJob);
	t_dictionary *file_StatusData = dictionary_get(filesToProcessPerJob,path);
	sem_post(&semFilesToProcessPerJob);

	int nroDeCopias = obtenerNumeroDeCopiasParaArchivo(socket,path);
	int nroDeBloques = obtenerNumeroDeBloquesParaArchivo(socket,path);
	int i,j;

	//t_dictionary *(*bloquesArray)[nroDeBloques][nroDeCopias];
	t_list *blocksList = dictionary_get(file_StatusData,K_file_StatusData_Bloques);
	t_list *listaDeCopias = list_get(blocksList,bloque);

	t_list *copiasParaBloque = list_create();

	for(i=0;i<nroDeCopias;i++){
		list_get(blocksList,bloque);
		t_dictionary *copia = list_get(listaDeCopias,i);
		char *ip = dictionary_get(copia,K_Copia_IPNodo);
		char *b = dictionary_get(copia,K_Copia_NroDeBloque);
		list_add(copiasParaBloque, copia);
	}

	free(key);

	return copiasParaBloque;
}

int obtenerNumeroDeCopiasParaArchivo(int socket,char *path)
{
	char *key = intToCharPtr(socket);
	sem_wait(&semFilesToProcess);
	t_dictionary *filesToProcessPerJob = dictionary_get(filesToProcess,key);
	sem_post(&semFilesToProcess);

	sem_wait(&semFilesToProcessPerJob);
	t_dictionary *file_StatusData = dictionary_get(filesToProcessPerJob,path);
	sem_post(&semFilesToProcessPerJob);

	int nroDeCopias = dictionary_get(file_StatusData,K_file_StatusData_CopiasSize);

	free(key);

	return nroDeCopias;
}

int obtenerNumeroDeBloquesParaArchivo(int socket,char *path)
{
	char *key = intToCharPtr(socket);
	sem_wait(&semFilesToProcess);
	t_dictionary *filesToProcessPerJob = dictionary_get(filesToProcess,key);
	sem_post(&semFilesToProcess);

	sem_wait(&semFilesToProcessPerJob);
	t_dictionary *file_StatusData = dictionary_get(filesToProcessPerJob,path);
	sem_post(&semFilesToProcessPerJob);

	int nroDeBloques = dictionary_get(file_StatusData,K_file_StatusData_BloquesSize);

	free(key);

	return nroDeBloques;
}

void destruirFile_StatusData(int sckt, char *path)
{
	char *socket = intToCharPtr(sckt);
	sem_wait(&semFilesToProcess);
	t_dictionary *filesToProcessPerJob = dictionary_get(filesToProcess,socket);
	sem_post(&semFilesToProcess);

	sem_wait(&semFilesToProcessPerJob);
	t_dictionary *file_StatusData = dictionary_get(filesToProcessPerJob,path);
	sem_post(&semFilesToProcessPerJob);

	free(file_StatusData); //SE LIBERAN TODOS LOS SUB-DICCIONARIOS QUE TIENE ???
}
void destruirFileState(char* path)
{
	sem_wait(&semFilesStates);
	t_dictionary *fileState = dictionary_get(filesStates,path);
	sem_post(&semFilesStates);

	free(fileState); //SE LIBERAN TODOS LOS SUB-DICCIONARIOS QUE TIENE ???
}

bool* soportaCombiner(int sckt, char *path)
{
	char *key = intToCharPtr(sckt);

	sem_wait(&semFilesToProcess);
	t_dictionary *filesToProcessPerJob = dictionary_get(filesToProcess,key);
	sem_post(&semFilesToProcess);

	sem_wait(&semFilesToProcessPerJob);
	t_dictionary *file_StatusData  = dictionary_get(filesToProcessPerJob,path);
	sem_post(&semFilesToProcessPerJob);

	bool *soportaCombiner = dictionary_get(file_StatusData,K_file_StatusData_combinerMode);
	return soportaCombiner;

}

t_list* obtenerPathsTemporalesParaNodo(char *path,char *IPnodoEnBlockState){

	sem_wait(&semFilesStates);
	t_dictionary* fileState = dictionary_get(filesStates,path);
	sem_post(&semFilesStates);

	int size_fileState = dictionary_get(fileState,K_FileState_size);
	t_dictionary *(*blockStateArray)[size_fileState];
	blockStateArray = dictionary_get(fileState,K_FileState_arrayOfBlocksStates);
	int i;
	t_list *pathsTemporalesEnNodo = list_create();

	for(i=0;i<size_fileState;i++){
		t_dictionary *blockState = (*blockStateArray)[i];
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

	//
	t_list *litaPathsTemporalesParaNodo = obtenerPathsTemporalesParaNodo(path,IPnodoEnBlockState);
	int size = list_size(litaPathsTemporalesParaNodo);
	return size;
}
t_list* obtenerNodosEnBlockStateArray(char *path){

	sem_wait(&semFilesStates);
	t_dictionary* fileState = dictionary_get(filesStates,path);
	sem_post(&semFilesStates);

	int sizeBlocksArray = dictionary_get(fileState,K_FileState_size);
	t_list *blockStateArray = dictionary_get(fileState,K_FileState_arrayOfBlocksStates);

	int i;
	t_list *nodosEnBlockStateArray = list_create();
	for(i=0;i<sizeBlocksArray;i++){

		t_dictionary *blockState = list_get(blockStateArray,i);
		char *IPnroDeNodo = dictionary_get(blockState,K_BlockState_nroNodo);
		bool *estaEnLista = isNodoInList(nodosEnBlockStateArray,IPnroDeNodo);
		if(!estaEnLista){
			list_add(nodosEnBlockStateArray,IPnroDeNodo);
		}
		free(estaEnLista);
	}
	return nodosEnBlockStateArray;
}

char* obtenerNodoConMayorCantidadDeArchivosTemporales(char *path){

	t_list *nodosEnBlockStateArray = obtenerNodosEnBlockStateArray(path);
	int size = list_size(nodosEnBlockStateArray);
	char *nodoConMayorCantidadDeArchTemp;

	int i;
	for(i=0;i<(size-1);i++){
		char *IPnroDeNodo = list_get(nodosEnBlockStateArray,i);
		char *IPnroDeNodoSiguiente = list_get(nodosEnBlockStateArray,i+1);

		t_list *pathsTemporalesParaNodo = obtenerPathsTemporalesParaNodo(path,IPnroDeNodo);
		t_list *pathsTemporalesParaNodoSig = obtenerPathsTemporalesParaNodo(path,IPnroDeNodoSiguiente);

		int cantDePathsTempEnNodo = list_size(pathsTemporalesParaNodo);
		int cantDePathsTempEnNodoSiguiente = list_size(pathsTemporalesParaNodoSig);

		if(cantDePathsTempEnNodo < cantDePathsTempEnNodoSiguiente){
			nodoConMayorCantidadDeArchTemp = IPnroDeNodoSiguiente;
		}else{
			nodoConMayorCantidadDeArchTemp = IPnroDeNodo;
		}
	}
	return nodoConMayorCantidadDeArchTemp;
}

bool* isPathInList(t_list *lista,char *path)
{
	int listSize = list_size(lista);
	int i;
	bool *response = malloc(sizeof(bool));
	response = false;
	for(i=0;i<listSize;i++){
		char *temporaryPathInList = list_get(lista,i);
		if(strcmp(path,temporaryPathInList) == 0){ return true; }
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
