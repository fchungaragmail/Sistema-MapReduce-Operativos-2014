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
#endif /* FILESTATUSCENTER_H_ */
