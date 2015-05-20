/*
 * MapFunctions.h
 *
 *  Created on: 20/5/2015
 *      Author: utnso
 */

#ifndef MAPFUNCTIONS_H_
#define MAPFUNCTIONS_H_

int mmapRead(char *filePath);
int mmapWrite(char* filePath, char* textToWrite);
int mmapConcatenateTextToFile(char* filePath,char *textToConcatenate);

#endif /* MAPFUNCTIONS_H_ */
