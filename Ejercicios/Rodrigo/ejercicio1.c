/*
 * ejercicio1.c
 *
 *  Created on: 11/4/2015
 *      Author: utnso
 */

#include <stdio.h>
#include <string.h>
#include <commons/string.h>

void string_concat_dinamyc(char* str1, char* str2, char** result) {
	*result = string_new();
	string_append(&*result, str1);
	string_append(&*result, str2);
}

int main() {

    char* nombre = "Rodrigo";
    char* saludo;
    string_concat_dinamyc("Hola ", nombre, &saludo);
	printf("%s", saludo);
	return 0;

}
