/*
 * Utilities.c
 *
 *  Created on: 25/5/2015
 *      Author: utnso
 */

#include <stdio.h>
#include <stdlib.h>
#include "Utilities.h"

char* intToCharPtr(int x)
{

	char *ptr;
	ptr=malloc(2*sizeof(char));
	sprintf(ptr,"%d",x);
	return ptr;
}
