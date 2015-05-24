/*
 * FileStatusCenter.c
 *
 *  Created on: 21/5/2015
 *      Author: utnso
 */

#include <stdio.h>
#include <stdlib.h>

#include <commons/collections/dictionary.h>

#define KFilePath "FilePath"
#define KFileCombinerMode "FileCombinerMode"
#define KFilePartsData "FileParts"
#define KPartsCount "PartsCount"

typedef enum {
				UNINITIALIZED = 0, 	//sin estado
				IN_MAPPING = 1, 	//se esta mappeando, se espera respuesta del job
				MAPPED = 2, 		//archivo ya mappeado
				IN_REDUCING = 3,
				REDUCED = 4,
				TEMPORAL_ERROR = 5,//fallo la operacion de map o reduce
				TOTAL_ERROR = 6  	// fallo la operacion y el FS no tiene otros bloques disponibles para procesar
} status;

typedef struct partialFileData
{
   char 	 *partialName;
   char		 *nodo_IP;
   char		 *temporaryFileName; //el archivo temporal donde se va a guardar el rdo de la rutina, MaRTA asigna este nombre
   status 	 fileStatus;

} t_partialFileData;

t_dictionary *filesToProcess;

void initFilesStatusCenter();
void addNewFilePath(char *fileToProcess_Path,_Bool soportaCombiner);
void removeJobFilePath(char *fileToRemove_Path);
//////////////////
void addNodoDataForFile(char *filePath,t_dictionary *fileInfo);
void updateNodoLocationForPartialFile(char *filePath,char *partialFile, char *nodo_IP_update);

void initFilesStatusCenter()
{
	filesToProcess = dictionary_create();
}

void addNewJobFilePath(char *fileToProcess_Path,_Bool *soportaCombiner)
{
	t_dictionary *fileToProcessStatusData = dictionary_create();
	dictionary_put(fileToProcessStatusData, KFilePath, fileToProcess_Path);
	dictionary_put(fileToProcessStatusData, KFileCombinerMode,soportaCombiner);

	dictionary_put(filesToProcess,fileToProcess_Path,fileToProcessStatusData);
	//MaRTA y FilesStatusCenter deben compartir un array con los path de los archivos
}

void removeJobFilePath(char *fileToRemove_Path)
{
	dictionary_remove(filesToProcess,fileToRemove_Path);
}

/////////////////

// aca va la respuesta del FS
// tiene la info de los bloques y nodos donde esta el archivo a procesar

void addNodoDataForFile(char *filePath,t_dictionary *fileInfo)//fileInfo es un dic con 2 claves
																//1er clave es el nro de elementos
{																//2da clave el array con los elementos

	t_dictionary *fileToProcessStatusData = dictionary_get(filesToProcess,filePath);

	int elementsCount = dictionary_get(fileInfo,"KElementsCount");
	void *array = dictionary_get(fileInfo,"KElementsArray");//cada 2 posiciones tengo nodoIP y nombreParcial

	dictionary_put(fileToProcessStatusData,KPartsCount,elementsCount);

	t_partialFileData partsInfo[elementsCount]; //array lleno de partialFileData, N es length->filePath
/*
	int i=0;
	while(elementsCount>i){

		t_dictionary *partialFileData = *(t_dictionary *)array[i];
		void *nodo_IP = dictionary_get(partialFileData,"KNodo_IP");
		char *partialName = dictionary_get(partialFileData,"KPartialName"); //nombre del archivo parcial en el Nodo

		t_partialFileData *data;
		data = malloc(sizeof(t_partialFileData));
		strcpy(&data->nodo_IP,*nodo_IP);
		strcpy(&data->partialName,*partialName);
		strcpy(&data->temporaryFileName,"-1");
		(*data).fileStatus=UNINITIALIZED;

		partsInfo[i]=data;
		i++;
	}
*/
	dictionary_put(fileToProcessStatusData,KFilePartsData,partsInfo);


}

void updateNodoLocationForPartialFile(char *filePath,char *partialFile, char *nodo_IP_update)
//en el caso de que la rutina falle y le pida al FS otra ubicacion de ese arch. parcial
{
/*
	t_dictionary *fileToProcessStatusData = dictionary_get(filesToProcess,filePath);
	int elementsCount = dictionary_get(fileToProcessStatusData,KPartsCount);

	t_partialFileData partsInfo[elementsCount];
	partsInfo = dictionary_get(fileToProcessStatusData,KFilePartsData);

	int i=0;
	while(i<elementsCount)//busco el partial element por su PartialName y le cambio el Nodo_IP
	{
		t_partialFileData *data;
		data = malloc(sizeof(t_partialFileData));
		data = partsInfo[i];

		if(!strcmp((*data).partialName,partialFile)){
			strcpy(&data->nodo_IP,*nodo_IP_update);
			return;
		}
		i++;

	}
*/
}
