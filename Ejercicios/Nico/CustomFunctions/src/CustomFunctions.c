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
#include <stdbool.h>
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

	//serializo
	char *request = "saraza";
	int8_t tipo=2;
	int8_t largo = 3;
	char* data = malloc(sizeof(int8_t)+sizeof(int8_t)+strlen(request)+/*1*/sizeof(int8_t));
	//NO AGREGAR "+1" xq sino incluyo el '/0' y si quiero pedir strlen(data) me lo va a cortar cuando
	//encuentre el '/0' y no va a tomar lo siguiente

	//en vez de char* data =... podria haber sido void* data =...
	bool *boolValue = malloc(sizeof(bool));
	*boolValue = true;
	int8_t bool8 = *boolValue;

	int size = 0, offset = 0;
	offset += size;
	memcpy(data + offset,&tipo, size = sizeof(int8_t));
	offset += size;
	memcpy(data + offset,&largo,size = sizeof(int8_t));
	offset += size;
	memcpy(data + offset,request,size = strlen(request) + 0);/*no agregar +1 !!!*/
	offset += size;
	memcpy(data + offset,&bool8,size = sizeof(int8_t));

	printf("Quedó %s\n:",(char *)data);
	printf("El largo de data* es %d\n:",strlen(data));
	//deserializo
	int8_t a,b;
	offset = 0; size = 0;
	memcpy(&a,data,size = sizeof(int8_t));
	offset = offset + size;
	memcpy(&b,data+offset,size = sizeof(int8_t));
	offset = offset + size;
	char *str = malloc(strlen("saraza")+1);
	memcpy(str,data+offset,size = (strlen("saraza")+0));
	str[strlen("saraza")+1] = '\0'; //agrego el '\0' a mano !!!
	offset = offset + size;
	int8_t boolRecibido;
	memcpy(&boolRecibido,data+offset,size=sizeof(int8_t));
	//int boolInt=boolRecibido;
	printf("el valor de a es : %d\n",a);
	printf("el valor de b es : %d\n",b);
	printf("el valor de str es : %s\n",str);
	printf("el valor de boolRecibido es : %d\n",boolRecibido);
	if(boolRecibido == 1) { printf("boolRecibido es true"); }
	if(boolRecibido == 0) { printf("boolRecibido es false"); }
	/*t_dictionary *fileState = dictionary_create();
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
	}*/

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



