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
#include "Pruebas.h"
#include <commons/collections/dictionary.h>

#define ScriptPath "/home/utnso/Escritorio/tp-2015-1c-the-byteless/Ejercicios/Nico/CustomFunctions/scriptReal"

int main(void) {

	puts("!!!Hello World!!!"); /* prints !!!Hello World!!! */

	t_dictionary *fileState = dictionary_create();
	t_dictionary *blocksStatesArray[3];
	int i;
	for(i=0;i<3;i++){
		t_dictionary *blockState = dictionary_create();
		dictionary_put(blockState,"key",i);
		dictionary_put(blockState,"key2",8);
		char *str = malloc(strlen("hola")+1);
		strcpy(str,"hola");
		dictionary_put(blockState,"key3",str);
		blocksStatesArray[i] = blockState;
	}
	dictionary_put(fileState,"array",&blocksStatesArray);
	//**************
	 t_dictionary *(*blockStatesArray2)[3];
	 blockStatesArray2 = dictionary_get(fileState,"array");
	 for(i=0;i<3;i++){
	  t_dictionary *blockState2;
	  blockState2 = (*blockStatesArray2)[i];
	  int valor = dictionary_get(blockState2,"key");
	  int valor2 = dictionary_get(blockState2,"key2");
	  char *str2 = dictionary_get(blockState2,"key3");
	  printf("el valor es %s\n",str2);
	  printf("el valor2 es %d\n",valor2);
	  printf("el valor es %d\n",valor);
	}



	/*int sckt = 2;
	char *key = intToCharPtr(sckt);
	t_dictionary *dic = dictionary_create();
	dictionary_put(dic,key,7);
	free(key);
	int valor = dictionary_get(dic,"2");*/

	/*int a = 2;
	char *string = intToCharPtr(a);
	printf("a en string es %s",string);
	 */

	/*t_dictionary *dic = dictionary_create();
	int a = 2;
	printf("el valor de a es : %d ",a);
	dictionary_put(dic,"key",a);
	int b;
	b = dictionary_get(dic,"key");
	printf("el valor de b es : %d ",b);
	dictionary_destroy(dic);
	printf("dic destruido con exito");*/

	/*char *string = malloc(strlen("hola")+1);
	strcpy(string,"hola");
	printf("el string es %s",string);
	prueba(string);
	printf("el string luego es %s",string);*/

	//FILE *fp_arch_temp = fopen("scriptTestFile","w");
	//procesarScript(ScriptPath,"bloque de datos",fp_arch_temp);

	return EXIT_SUCCESS;
}



