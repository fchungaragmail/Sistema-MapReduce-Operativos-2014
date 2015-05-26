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
	char myChar = (char) x;

	char *ptr;
	ptr=malloc(2*sizeof(char));
	ptr[0]=myChar;
	ptr[1]='\0';

	return ptr;
	//printf("el int x es : %d",x);
	//printf("/// el char* ptr es : %s",ptr);
}

int charPtrToInt(char* ptr)
{
	char myChar;
	myChar=ptr[0];
	int x = (int)myChar;
	return x;

}
