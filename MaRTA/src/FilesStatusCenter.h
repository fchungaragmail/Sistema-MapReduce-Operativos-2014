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

//fileState --> keys
#define K_FileState_size "FileState_size"
#define K_FileState_arrayOfBlocksStates "FileState_arrayOfBlocksStates"

//fullData --> keys (data pedida al FS)
#define K_fullData_CantidadDeBloques "fullData_CantidadDeBloques"
#define K_fullData_CantidadDeCopias "fullData_CantidadDeCopias"
#define K_fullData_Data "fullData_Data"

//Copia de FullData --> keys (tabla que envia el FS)
#define K_Copia_IPNodo "Copia_IPNodo"
#define K_Copia_NroDeBloque "Copia_NroDeBloque"

//*******************************************************

// Inicializar
void initFilesStatusCenter();

// Agregar nuevas conexiones
void addFSConnection(int fs_socket);
void addNewConnection(int socket);

// FS
int getFSSocket();

// Varias
void addFileFullData(int sckt, char* path,int nroDeBloques,int nroDeCopias, t_list *fullData);
void reloadFileFullData(int sckt, char* path,int nroDeBloques,int nroDeCopias, t_list *fullData);
//nodosData
void incrementarOperacionesEnProcesoEnNodo(char *IPnroNodo);
void decrementarOperacionesEnProcesoEnNodo(char *IPnroNodo);
int getCantidadDeOperacionesEnProcesoEnNodo(char *IPnroNodo);
void addTemporaryFilePathToNodoData(char *IPnroNodo,char* filePath);
void removeTemporaryFilePathToNodoData(char *IPnroNodo,char* filePath);

//filesToProcess
void addNewFileForProcess(char *file_Path,_Bool *soportaCombiner,int jobSocket);//crea un fileState
void darDeBajaCopiaEnBloqueYNodo(char*path,int skct,char *nroBloque,char *IP_Nodo,int indiceDeBloqe);
t_list* obtenerCopiasParaBloqueDeArchivo(int socket,int bloque,char *path);
int obtenerNumeroDeCopiasParaArchivo(int socket,char *path);
int obtenerNumeroDeBloquesParaArchivo(int socket,char *path);
void destruirFile_StatusData(int sckt, char *path);
bool* soportaCombiner(int sckt, char *path);

//filesStates
void changeFileBlockState(char* path,int nroBloque,StatusBlockState *nuevoEstado,char* temporaryPath);
t_dictionary *getFileStateForPath(char *path);
void destruirFileState(char* path);
t_list* obtenerPathsTemporalesParaNodo(char *path,char *IPnodoEnBlockState);
int obtenerCantidadDePathsTemporalesEnNodo(char *path,char *IPnodoEnBlockState);
t_list* obtenerNodosEnBlockStateArray(char *path);
int obtenerCantidadDeNodosDiferentesEnBlockState(char *path);
char* obtenerNodoConMayorCantidadDeArchivosTemporales(char *path);
int obtenerPosicionDeBloqueEnBlockStatesList(char *path,char *ipNodo,char *bloque);
t_dictionary *obtenerBlockState(char *path,char *tempPath);
void setBlockStatesListInReducingState(char *path);
#endif /* FILESTATUSCENTER_H_ */
