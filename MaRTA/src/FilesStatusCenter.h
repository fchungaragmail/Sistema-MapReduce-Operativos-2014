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
#include <commons/collections/dictionary.h>

//BlockState --> Keys
#define K_BlockState_state "BlockState_state"
#define K_BlockState_nroNodo "BlockState_nroNodo"
#define K_BlockState_nroBloque "BlockState_nroBloque"
#define K_BlockState_temporaryPath "BlockState_temporaryPath"

//fileState --> keys
#define K_FileState_size "FileState_size"
#define K_FileState_arrayOfBlocksStates "FileState_arrayOfBlocksStates"

//*******************************************************

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
void decrementarOperacionesEnProcesoEnNodo(int nroNodo);
int getCantidadDeOperacionesEnProcesoEnNodo(int nroNodo);
void addTemporaryFilePathToNodoData(int nroNodo,char* filePath);

//filesToProcess
void addNewFileForProcess(char *file_Path,_Bool *soportaCombiner,int jobSocket);//crea un fileState
void darDeBajaCopiaEnBloqueYNodo(char*archivoParcial_Path,int skct,int nroBloque,int nroNodo);
t_dictionary* obtenerCopiasParaArchivo(int socket,char *path);
int obtenerNumeroDeCopiasParaArchivo(int socket,char *path);
int obtenerNumeroDeBloquesParaArchivo(int socket,char *path);

//filesStates
void changeFileBlockState(char* path,int nroBloque,blockState nuevoEstado,char* temporaryPath);
t_dictionary *getFileStateForPath(char *path);
#endif /* FILESTATUSCENTER_H_ */
