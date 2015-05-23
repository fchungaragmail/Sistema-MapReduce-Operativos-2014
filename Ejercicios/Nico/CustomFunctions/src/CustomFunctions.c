/*
 ============================================================================
 Name        : CustomFunctions.c
 Author      : Nicolas Buzzano
 Version     :
 Copyright   : Your copyright notice
 Description : Hello World in C, Ansi-style
 ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <sys/types.h>

#include "MapFunctions.h"
#include "ScriptFunctions.h"

#define ScriptPath "/home/utnso/Escritorio/tp-2015-1c-the-byteless/Ejercicios/Nico/CustomFunctions/scriptReal"

int main(void) {
	puts("!!!Hello World!!!"); /* prints !!!Hello World!!! */

	FILE *fp_arch_temp = fopen("scriptTestFile","w");
	procesarScript(ScriptPath,"bloque de datos",fp_arch_temp);

	return EXIT_SUCCESS;
}



