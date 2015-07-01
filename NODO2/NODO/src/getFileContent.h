/*
 * getFileContent.h
 *
 *  Created on: 29/6/2015
 *      Author: daniel
 */

#ifndef GETFILECONTENT_H_
#define GETFILECONTENT_H_
#include <stdlib.h>
#include "commons/string.h"
#include <stdio.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>

typedef struct{
	char* contenido;
	long int size;
}t_fileContent;

t_fileContent *getFileContent(char *archivoTemporal);

#endif /* GETFILECONTENT_H_ */
