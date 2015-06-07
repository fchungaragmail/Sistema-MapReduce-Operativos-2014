/*
 * Pruebas.c
 *
 *  Created on: 29/5/2015
 *      Author: utnso
 */

#include "Pruebas.h"

void prueba(char *string);

void prueba(char *string)
{
	free(string);
}

char* intToCharPtr(int x)
{
	char *ptr;
	ptr=malloc(2*sizeof(char));
	sprintf(ptr,"%d",x);

	return ptr;
}
