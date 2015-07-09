#include "Utils.h"

void FreeMensaje(mensaje_t* mensaje) {

	if (mensaje->comando != NULL) {
		free(mensaje->comando);
	}
	if (mensaje->data != NULL) {
		free(mensaje->data);
	}
	free(mensaje);
	mensaje = NULL;
}

mensaje_t* CreateMensaje(char* comandoStr, char* dataStr) {

	mensaje_t* mensaje = malloc(sizeof(mensaje_t));

	if (comandoStr != NULL) {
		mensaje->comando = strdup(comandoStr);
		mensaje->comandoSize = strlen(comandoStr) + 1;
	} else {
		mensaje->comando = NULL;
		mensaje->comandoSize = NULL;
	}

	if (dataStr != NULL) {
		mensaje->data = strdup(dataStr);
		mensaje->dataSize = strlen(dataStr) + 1;
	} else {
		mensaje->data = NULL;
		mensaje->dataSize = NULL;
	}

	return mensaje;
}

void FreeStringArray(char*** stringArray) {
	char** array = *stringArray;

	int i;
	for (i = 0; array[i] != NULL; ++i) {
		free(array[i]);
	}

	free(array);
}

void FreeHiloJobInfo(HiloJobInfo* hiloJobInfo) {

	if (hiloJobInfo->nombreArchivo != NULL) {
		free(hiloJobInfo->nombreArchivo);
	}
	if (hiloJobInfo->parametros != NULL) {
		free(hiloJobInfo->parametros);
	}
	if (hiloJobInfo->parametrosError != NULL) {
		free(hiloJobInfo->parametrosError);
	}
	free(hiloJobInfo);
	hiloJobInfo = NULL;
}
