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
void addFileFullData(int sckt, char* path, Message *recvMessage);
void reloadFilaDeFileFullData(int sckt, char* path, t_list *fullData, int nroDeBloqe);
void informarTareasPendientesDeMapping(char *path,int socket);
//nodosData
void incrementarOperacionesEnProcesoEnNodo(char *IPnroNodo);
void decrementarOperacionesEnProcesoEnNodo(char *IPnroNodo);
int getCantidadDeOperacionesEnProcesoEnNodo(char *IPnroNodo);
void addTemporaryFilePathToNodoData(char *IPnroNodo,char* filePath);
void removeTemporaryFilePathToNodoData(char *IPnroNodo,char* filePath);
t_list *getNodoState_listaTemporales(t_dictionary *nodoState);

//filesToProcess
void addNewFileForProcess(char *file_Path,_Bool *soportaCombiner,int jobSocket);//crea un fileState
void darDeBajaCopiaEnBloqueYNodo(char*path,int skct,char *IP_Nodo);
t_list* obtenerCopiasParaBloqueDeArchivo(int socket,int bloque,char *path);
void destruirFile_StatusData(int sckt, char *path);
bool* soportaCombiner(int sckt, char *path);

//filesStates
t_dictionary *getFileStateForPath(char *path,int socket);
void destruirFileState(char* path);
t_list* obtenerPathsTemporalesParaArchivo(char *path,char *IPnodoEnBlockState, int socket);
int obtenerCantidadDePathsTemporalesEnNodo(char *path,char *IPnodoEnBlockState,int socket);
t_list* obtenerNodosEnBlockStateArray(char *path,int socket);
int obtenerCantidadDeNodosDiferentesEnBlockState(char *path,int socket);
t_dictionary* obtenerNodoConMayorCantidadDeArchivosTemporales(char *path,int socket);
int obtenerPosicionDeBloqueEnBlockStatesList(char *path,char *ipNodo,char *bloque,int socket);
t_dictionary *obtenerBlockState(char *path,char *tempPath,int socket);
void setBlockStatesListInReducingState(char *path,int socket);
void agregarFileState(int jobSocket,char *path, int cantidadDeBloques);

//fullDataTable
void agregarFullDataTable(t_list *table,char *path);
t_list *getCopyFullDataTable(char *path);

#endif /* FILESTATUSCENTER_H_ */
