/*
 ============================================================================
 Name        : Ejercicio_01.c
 Author      : 
 Version     :
 Copyright   : Your copyright notice
 Description : Hello World in C, Ansi-style
 ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char* string_concat(const char* string1, const char* string2);
void string_concat_dinamyc(const char*, const char*, char**);

int main(void) {
	char* nombre = "Ritchie";
	char* saludo = string_concat("Hola ", nombre);
	printf("%s", saludo);
	free(saludo);
	return EXIT_SUCCESS;
}

char* string_concat(const char* string1, const char* string2)
{
	char* sResult;
	sResult = malloc(strlen(string1)+strlen(string2)+1);
	strcat(sResult,string1);
	strcat(sResult, " ");
	strcat(sResult,string2);
	return &sResult;
}
