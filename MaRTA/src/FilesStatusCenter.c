/*
 * FileStatusCenter.c
 *
 *  Created on: 21/5/2015
 *      Author: utnso
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <commons/collections/dictionary.h>
#include "Utilities.h"

#define KFilePath "FilePath"
#define KFileCombinerMode "FileCombinerMode"
#define KFilePartsData "FileParts"
#define KPartsCount "PartsCount"

typedef struct partialFileData
{
   char 	 *partialName;
   char		 *nodo_IP;
   char		 *temporaryFileName; //el archivo temporal donde se va a guardar el rdo de la rutina, MaRTA asigna este nombre
   status 	 fileStatus;

} t_partialFileData;

t_dictionary *filesToProcess;
t_dictionary *filesStates;

int fs_socket; //socket del FS

// Inicializar
void initFilesStatusCenter();
// Agregar nuevas conexiones
void addFSConnection(int fs_socket);
void addNewConnection(int socket);
// Varias
void addNewFileForProcess(char *file_Path,_Bool *soportaCombiner,int jobSocket);
void addFileFullData(int sckt, char* path, t_dictionary *fullData);
void changeFileBlockState(char* path,int nroBloque,status nuevoEstado);

void initFilesStatusCenter()
{
	filesToProcess = dictionary_create();
	filesStates = dictionary_create();
	fs_socket=-1;
}

void addFSConnection(int fs_sckt)
{
		fs_socket = fs_sckt;
}
void addNewConnection(int jobSocket)
{
	char* skct_key = intToCharPtr(jobSocket);//VER COMO FUNCA ESTA FC
	t_dictionary *file_StatusData = dictionary_create();
	dictionary_put(filesToProcess, skct_key, file_StatusData);
	free(skct_key);
}

void addNewFileForProcess(char *file_Path,_Bool *soportaCombiner,int jobSocket)
{
	char* dic_key = intToCharPtr(jobSocket);//VER COMO FUNCA ESTA FC
	t_dictionary *file_StatusData = dictionary_get(filesToProcess,dic_key);

	dictionary_put(file_StatusData,KFileCombinerMode,soportaCombiner);
	dictionary_put(file_StatusData,KFilePath,file_Path);
	free(file_Path);
	free(soportaCombiner);
}

void addFileFullData(int sckt, char* path, t_dictionary *fullData)
{	// aca va la respuesta del FS
	// tiene la info de los bloques y nodos donde esta el archivo a procesar

	char *key = intToCharPtr(sckt);//VER COMO FUNCA ESTA FC
	t_dictionary *file_StatusData = dictionary_get(filesToProcess,key);

	int size = dictionary_get(fullData,"KCantidadDeBloques");
	dictionary_put(file_StatusData,"KCantidadDeBloques",size);

	t_dictionary *fileData = dictionary_get(fullData,"KFileData");
	dictionary_put(file_StatusData,"KFileData",fileData);

	//Construyo un "fileState"
	int intSize = charPtrToInt(size);
	t_dictionary *blocksStatesArray[intSize];
	int i;
	for(i=0;i<intSize;intSize++){

		t_dictionary *blockState = dictionary_create();
		dictionary_put(blockState,"State",UNINITIALIZED);
		dictionary_put(blockState,"NodoNumber",UNINITIALIZED);
		dictionary_put(blockState,"BlockNumber",UNINITIALIZED);
		blocksStatesArray[i]=blockState;
		free(blockState);
	}

	//Agrego "filesState" a "filesStates"
	t_dictionary *fileState = dictionary_create();
	dictionary_put(fileState,"array",blocksStatesArray);
	dictionary_put(fileState,"size",size);
	dictionary_put(filesStates,path,fileState);

	free(path);
	free(fullData);
	free(fileState);
	free(key);
	free(size);
	free(fileData);
	free(blocksStatesArray);
}

void changeFileBlockState(char *path,int nroBloque,status nuevoEstado)
{	//ESTO LO HAGO EN MI ESTRUCTURA "filesStates"
	//VER SI ES nroBloque o (nroBloque-1)!!!!!

	t_dictionary *fileState = dictionary_get(filesStates,path);
	int size = dictionary_get(fileState,"size");
	t_dictionary *array[size];
	array = dictionary_get(fileState,"array");

	t_dictionary *blockState = dictionary_create();
	blockState = array[nroBloque];
	dictionary_put(blockState,"State",nuevoEstado);

	free(path);
	free(blockState);
	free(fileState);
	free(array);
}
