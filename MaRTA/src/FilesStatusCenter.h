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

// Varias
void addNewFileForProcess(char *file_Path,_Bool *soportaCombiner,int jobSocket);
void addFileFullData(int sckt, char* path, t_dictionary *fullData);
void addTemporaryFilePathToNodoData(int nroNodo,char* filePath);
void incrementarOperacionesEnProcesoEnNodo(int nroNodo);
void changeFileBlockState(char* path,int nroBloque,blockState nuevoEstado);
int getCantidadDeOperacionesEnProcesoEnNodo(int nroNodo);
void darDeBajaCopiaEnBloqueYNodo(char*archivoParcial_Path,int skct,int nroBloque,int nroNodo);
t_dictionary* obtenerOtroNodoYBloqueParaArchParcial(char *archParcial);
#endif /* FILESTATUSCENTER_H_ */
