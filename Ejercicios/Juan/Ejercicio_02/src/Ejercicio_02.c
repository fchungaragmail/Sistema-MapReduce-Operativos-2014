/*
 ============================================================================
 Name        : Ejercicio_02.c
 Author      : 
 Version     :
 Copyright   : Your copyright notice
 Description : Hello World in C, Ansi-style
 ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void string_concat_dinamyc(const char*, const char*, char**);

int main(void) {

	char* nombre = "Ritchie";
	char* saludo;
	string_concat_dinamyc("Hola ", nombre, &saludo);
	printf("%s",saludo);
	free(saludo);
	return EXIT_SUCCESS;
}

void string_concat_dinamyc(const char* s1, const char*s2, char** sResultado)
{
	*sResultado = malloc(sizeof(s1)+sizeof(s2)+2);
	strcat(*sResultado,s1);
	strcat(*sResultado, " ");
	strcat(*sResultado, s2);
}
