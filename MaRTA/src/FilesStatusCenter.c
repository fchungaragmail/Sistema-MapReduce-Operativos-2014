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

/*
//CONSTANTES
*/

//filesToProcess --> keys
#define K_filesToProcess_filesPerJob "filesPerJob"

//file_StatusData --> keys
#define K_file_StatusData_filePath "FilePath"
#define K_file_StatusData_combinerMode "FileCombinerMode"
#define K_file_StatusData_Bloques "file_StatusData_Bloques"
#define K_file_StatusData_BloquesSize "file_StatusData_BloquesSize"

//NodoState --> Keys
#define K_Nodo_OperacionesEnProceso "operacionesEnProceso"
#define K_Nodo_ArchivosTemporales "archivosTemporales"

//BlockState --> Keys
#define K_BlockState_state "BlockState_state"
#define K_BlockState_nroNodo "BlockState_nroNodo"
#define K_BlockState_nroBloque "BlockState_nroBloque"
#define K_BlockState_temporaryPath "BlockState_temporaryPath"

//fileState --> keys
#define K_FileState_size "FileState_size"
#define K_FileState_arrayOfBlocksStates "FileState_arrayOfBlocksStates"

t_dictionary *filesToProcess;
t_dictionary *filesStates;
t_dictionary *nodosData;
t_list *filesOrdersToProcess;
t_list *filesOrdersFailed;

int fs_socket; //socket del FS

// Inicializar
void initFilesStatusCenter();
// Agregar nuevas conexiones
void addFSConnection(int fs_socket);
void addNewConnection(int socket);
// FS
int getFSSocket();
// Varias
void addFileFullData(int sckt, char* path, t_dictionary *fullData);
//nodosData
void incrementarOperacionesEnProcesoEnNodo(int nroNodo);
int getCantidadDeOperacionesEnProcesoEnNodo(int nroNodo);
void addTemporaryFilePathToNodoData(int nroNodo,char* filePath);
//filesToProcess
void addNewFileForProcess(char *file_Path,_Bool *soportaCombiner,int jobSocket);//crea un fileState
void darDeBajaCopiaEnBloqueYNodo(char*archivoParcial_Path,int skct,int nroBloque,int nroNodo);
t_dictionary* obtenerCopiasParaArchivo(int socket,char *path);
//filesStates
void changeFileBlockState(char* path,int nroBloque,blockState nuevoEstado,char* temporaryPath);
t_dictionary *getFileStateForPath(char *path);

void initFilesStatusCenter()
{
	filesToProcess = dictionary_create();
	filesStates = dictionary_create();
	filesOrdersToProcess=list_create();
	filesOrdersFailed=list_create();
	nodosData = dictionary_create();
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
	dictionary_put(filesToProcess, skct_key, filesToProcessPerJob);

	free(filesToProcessPerJob);
	free(skct_key);
}

void addNewFileForProcess(char *file_Path,_Bool *soportaCombiner,int jobSocket)
{
	char* dic_key = intToCharPtr(jobSocket);//VER COMO FUNCA ESTA FC
	t_dictionary *filesToProcessPerJob = dictionary_get(filesToProcess,dic_key);
	t_dictionary *file_StatusData = dictionary_create();

	dictionary_put(file_StatusData,K_file_StatusData_combinerMode,soportaCombiner);
	dictionary_put(file_StatusData,K_file_StatusData_filePath,file_Path);
	dictionary_put(filesToProcessPerJob,file_Path,file_StatusData);
	list_add(filesOrdersToProcess,file_Path);

	free(file_Path);
	free(soportaCombiner);
	free(file_StatusData);
	free(filesToProcessPerJob);
}

void addFileFullData(int sckt, char* path, t_dictionary *fullData)
{	//fullData me lo envia el FS
	// aca va la respuesta del FS
	// tiene la info de los bloques y nodos donde esta el archivo a procesar

	char *key = intToCharPtr(sckt);//VER COMO FUNCA ESTA FC
	t_dictionary *filesToProcessPerJob = dictionary_get(filesToProcess,key);
	t_dictionary *file_StatusData = dictionary_get(filesToProcessPerJob,path);

	int size = dictionary_get(fullData,"KCantidadDeBloques");
	dictionary_put(file_StatusData,K_file_StatusData_BloquesSize,size);

	t_dictionary *fileData = dictionary_get(fullData,"KFileData");
	dictionary_put(file_StatusData,K_file_StatusData_Bloques,fileData);

	//Construyo un "fileState"
	int intSize = charPtrToInt(size);
	t_dictionary *blocksStatesArray[intSize];
	int i;
	for(i=0;i<intSize;intSize++){

		t_dictionary *blockState = dictionary_create();
		dictionary_put(blockState,K_BlockState_state,UNINITIALIZED);
		dictionary_put(blockState,K_BlockState_nroNodo,UNINITIALIZED);
		dictionary_put(blockState,K_BlockState_nroBloque,UNINITIALIZED);
		blocksStatesArray[i]=blockState;
		free(blockState);
	}

	//Agrego "filesState" a "filesStates"
	t_dictionary *fileState = dictionary_create();
	dictionary_put(fileState,K_FileState_arrayOfBlocksStates,blocksStatesArray);
	dictionary_put(fileState,K_FileState_size,size);
	dictionary_put(filesStates,path,fileState);

	free(path);
	free(fullData);
	free(fileState);
	free(key);
	free(fileData);
	free(blocksStatesArray);
}

void changeFileBlockState(char *path,int nroBloque,blockState nuevoEstado,char *temporaryPath)
{	//ESTO LO HAGO EN MI ESTRUCTURA "filesStates"
	//VER SI ES nroBloque o (nroBloque-1)!!!!!

	t_dictionary *fileState = dictionary_get(filesStates,path);
	int size = dictionary_get(fileState,K_FileState_size);

	t_dictionary *blocksStatesArray[size];
	int i;
	for(i=0;i<size;i++){ blocksStatesArray[i]=malloc(sizeof(t_dictionary)); }
	blocksStatesArray = dictionary_get(fileState,K_FileState_arrayOfBlocksStates);

	t_dictionary *blockState = dictionary_create();
	blockState = blocksStatesArray[nroBloque];
	int estadoBloque = nuevoEstado;
	dictionary_put(blockState,K_BlockState_state,estadoBloque);

	dictionary_put(blockState,K_BlockState_temporaryPath,temporaryPath);

	free(path);
	free(blockState);
	free(fileState);
	free(blocksStatesArray);
	free(temporaryPath);
}

int getCantidadDeOperacionesEnProcesoEnNodo(int nroNodo)
{
	char *key_nroDeNodo = intToCharPtr(nroNodo);
	bool hasKey = dictionary_has_key(nodosData,key_nroDeNodo);

	if(hasKey == true){
		//el nodo existe
		t_dictionary *nodoState = dictionary_get(nodosData,key_nroDeNodo);
		int operacionesEnProceso = dictionary_get(nodoState,K_Nodo_OperacionesEnProceso);
		return operacionesEnProceso;
	}
	//else --> al nodo no se le asigno nada aun, entonces no tiene operacionesEnProceso
	return 0;
}

void incrementarOperacionesEnProcesoEnNodo(int nroNodo)
{//se llama cuando envio un pedido de map o reduce al Job
	char *key_nroDeNodo = intToCharPtr(nroNodo);
	bool hasKey = dictionary_has_key(nodosData,key_nroDeNodo);

		if(hasKey == true){
			//busco el dic y le sumo una operacionProcesando
			t_dictionary *nodoState = dictionary_get(nodosData,key_nroDeNodo);
			int operacionesEnProceso = dictionary_get(nodoState,K_Nodo_OperacionesEnProceso);
			operacionesEnProceso = operacionesEnProceso + 1;
			dictionary_put(nodoState,K_Nodo_OperacionesEnProceso,operacionesEnProceso);
			free(nodoState);
		}else{
			//creo un nuevo dic y le sumo una operacionProcesando
			t_dictionary *nodoState = dictionary_create();
			int operacionesEnProceso = 1;
			dictionary_put(nodoState,K_Nodo_OperacionesEnProceso,operacionesEnProceso);

			//creo una lista de archivosTemporales
			t_list *archivosTemporales = list_create();
			dictionary_put(nodoState,K_Nodo_ArchivosTemporales,archivosTemporales);

			//agrego a nodosData
			dictionary_put(nodosData,key_nroDeNodo,nodoState);

			free(archivosTemporales);
			free(nodoState);
		}

	free(key_nroDeNodo);
}
void addTemporaryFilePathToNodoData(int nroNodo,char* filePath)
{//el NodoState tiene que estar si o si creado, pues tiene que haber pasado por
 //"incrementarOperacionesEnProcesoEnNodo"

	char *key_nroDeNodo = intToCharPtr(nroNodo);
	t_dictionary *nodoState = dictionary_get(nodosData,key_nroDeNodo);
	t_list *listaTemporales = dictionary_get(nodoState,K_Nodo_ArchivosTemporales);
	list_add(listaTemporales,filePath);

	free(listaTemporales);
	free(nodoState);
	free(key_nroDeNodo);
	free(filePath);
}

void darDeBajaCopiaEnBloqueYNodo(char*archivoParcial_Path,int skct,int nroBloque,int nroNodo )
{	//archivoParcial_Path seria algo como librazo-horaDeEnvio.txt
	//debo conseguir el char* librazo.txt --> supongamos que es char* _archivo;
	char* _archivo;

	char *key = intToCharPtr(skct);
	t_dictionary *filesToProcessPerJob = dictionary_get(filesToProcess,key);
	t_dictionary *file_StatusData =dictionary_get(filesToProcessPerJob,_archivo);
	int size = dictionary_get(file_StatusData,K_file_StatusData_BloquesSize);

	t_dictionary *copiasArray[size][3];
	int i,j;
	for(i=0;i<size;i++){
		for(j=0;j<3;j++){
			copiasArray[i][j]=malloc(sizeof(t_dictionary));
		}
	}

	for(i=0;i<3;i++){

		t_dictionary *copiaDeBloque = copiasArray[nroBloque][i];
		int numeroNodo = dictionary_get(copiaDeBloque,"nroNodo");
		if(numeroNodo==nroNodo){

			dictionary_put(copiaDeBloque,"nroNodo",-1);
			free(copiaDeBloque);
			break;
		}
	}

	free(_archivo);
	free(archivoParcial_Path);
	free(key);
	free(filesToProcessPerJob);
	free(file_StatusData);
	free(copiasArray);
}

t_dictionary *getFileStateForPath(char *path)
{
	t_dictionary *fileState = malloc(sizeof(t_dictionary));
	fileState = dictionary_get(filesStates,path);
	return fileState;
}

t_dictionary* obtenerCopiasParaArchivo(int socket,char *path)
{
	//IMPLEMENTAR
	//Devuelvo un dic con copia 1,2,3 de filesToProcess
}


