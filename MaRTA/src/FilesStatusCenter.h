/*
 * FileStatusCenter.h
 *
 *  Created on: 21/5/2015
 *      Author: utnso
 */

#include <stdbool.h>
#ifndef FILESTATUSCENTER_H_
#define FILESTATUSCENTER_H_

#include "Utilities.h"
#include <commons/collections/list.h>
#include <commons/collections/dictionary.h>

//BlockState --> Keys
#define K_BlockState_state "BlockState_state"
#define K_BlockState_nroNodo "BlockState_nroNodo"
#define K_BlockState_nroBloque "BlockState_nroBloque"
#define K_BlockState_temporaryPath "BlockState_temporaryPath"
#define K_BlockState_puertoNodo "BlockState_puertoNodo"
#define K_BlockState_nodoDisponible "BlockState_nodoDisponible"

//fileState --> keys
#define K_FileState_arrayOfBlocksStates "FileState_arrayOfBlocksStates"

//fullData --> keys (data pedida al FS)
#define K_fullData_CantidadDeBloques "fullData_CantidadDeBloques"
#define K_fullData_CantidadDeCopias "fullData_CantidadDeCopias"
#define K_fullData_Data "fullData_Data"

//Copia de FullData --> keys (tabla que envia el FS)
#define K_Copia_IPNodo "Copia_IPNodo"
#define K_Copia_NroDeBloque "Copia_NroDeBloque"
#define K_Copia_PuertoNodo "Copia_PuertoNodo"
#define K_Copia_Estado "Copia_Estado"

//file_StatusData --> keys
#define K_file_StatusData_combinerMode "FileCombinerMode"
#define K_file_StatusData_Bloques "file_StatusData_Bloques"

//*******************************************************
// Inicializar
void initFilesStatusCenter();

// Agregar nuevas conexiones
void addFSConnection(int fs_socket);
void addNewConnection(int socket);

// FS
int getFSSocket();

// Varias
t_dictionary *addFileFullData(int sckt, char* path, Message *recvMessage,t_dictionary *file_StatusData);
void reloadFilaDeFileFullData(t_list *fullData, int nroDeBloqe,t_dictionary *file_StatusData);
void informarTareasPendientesDeMapping(char *path,int socket,t_dictionary *fileState);
//nodosData
void incrementarOperacionesEnProcesoEnNodo(char *IPnroNodo);
void decrementarOperacionesEnProcesoEnNodo(char *IPnroNodo);
int getCantidadDeOperacionesEnProcesoEnNodo(char *IPnroNodo);
void addTemporaryFilePathToNodoData(char *IPnroNodo,char* filePath);
void removeTemporaryFilePathToNodoData(char *IPnroNodo,char* filePath);
t_list *getNodoState_listaTemporales(t_dictionary *nodoState);

//filesToProcess
t_dictionary *crearNewFileForProcess(char *file_Path,_Bool *soportaCombiner,int jobSocket);//crea un fileState
void darDeBajaCopiaEnBloqueYNodo(char *IP_Nodo,t_dictionary *file_StatusData);
t_list* obtenerCopiasParaBloqueDeArchivo(int bloque,t_dictionary *file_StatusData);
void destruirFile_StatusData(int sckt, char *path);
bool* soportaCombiner(t_dictionary *file_StatusData);
bool todosArchivosDeJobReducidos(int jobSocket);
void agregarArchivoFinalizado(int jobSocket, char *path, char*ip, char*puerto, char*pathFinal);
t_list *obtenerListaParaReduceFinal(int jobSocket);

//filesStates
t_dictionary *getFileStateForPath(char *path,int socket);
void destruirFileState(char* path);
t_list* obtenerPathsTemporalesParaArchivo(char *IPnodoEnBlockState,t_dictionary *fileState);
int obtenerCantidadDePathsTemporalesEnNodo(char *IPnodoEnBlockState,t_dictionary *fileState);
t_list* obtenerNodosEnBlockStateArray(t_dictionary *fileState);
int obtenerCantidadDeNodosDiferentesEnBlockState(t_dictionary *fileState);
t_dictionary* obtenerNodoConMayorCantidadDeArchivosTemporales(t_dictionary *fileState);
int obtenerPosicionDeBloqueEnBlockStatesList(char *ipNodo,char *bloque,t_dictionary *fileState);
t_dictionary *obtenerBlockState(char *tempPath,t_dictionary *fileState);
void setBlockStatesListInReducingState(t_dictionary *fileState);
t_dictionary *crearFileState(int jobSocket,char *path, int cantidadDeBloques);

//fullDataTable
void agregarFullDataTable(t_list *table,char *path);
t_list *getCopyFullDataTable(char *path);

#endif /* FILESTATUSCENTER_H_ */
