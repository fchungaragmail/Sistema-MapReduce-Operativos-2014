/*
 * FileStatusCenter.h
 *
 *  Created on: 21/5/2015
 *      Author: utnso
 */

#include <stdbool.h>
#ifndef FILESTATUSCENTER_H_
#define FILESTATUSCENTER_H_

#include <commons/collections/dictionary.h>

void initFilesStatusCenter();
////////////////// Para usar con Job
void addNewJobFilePath(char *fileToProcess_Path,_Bool soportaCombiner);
void removeJobFilePath(char *fileToRemove_Path);
////////////////// Para usar con FS
void addNodoDataForFile(char *filePath,t_dictionary *fileInfo);
void updateNodoLocationForPartialFile(char *filePath,char *partialFile, char *nodo_IP_update);

#endif /* FILESTATUSCENTER_H_ */
