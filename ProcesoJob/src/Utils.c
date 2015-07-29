#include "Utils.h"

void FreeMensaje(mensaje_t* mensaje) {

	if (mensaje == NULL) {
		return;
	}

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
		mensaje->comando = strdup("\0");
		mensaje->comandoSize = 1;
	}

	if (dataStr != NULL) {
		mensaje->data = strdup(dataStr);
		mensaje->dataSize = strlen(dataStr) + 1;
	} else {
		mensaje->data = strdup("\0");
		mensaje->dataSize = 1;
	}

	return mensaje;
}

mensaje_t* CreateMensajeParaHilo(char* comandoStr, char* dataStr, int sizeData) {

	mensaje_t* mensaje = malloc(sizeof(mensaje_t));

	if (comandoStr != NULL) {
		mensaje->comando = strdup(comandoStr);
		mensaje->comandoSize = strlen(comandoStr) + 1;
	} else {
		mensaje->comando = strdup("\0");
		mensaje->comandoSize = 1;
	}

	if (dataStr != NULL) {
		mensaje->data = malloc(sizeData);
		memcpy(mensaje->data, dataStr, sizeData);
		mensaje->dataSize = sizeData;
	} else {
		mensaje->data = strdup("\0");
		mensaje->dataSize = 1;
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
	if (hiloJobInfo->archivoOriginal != NULL) {
		free(hiloJobInfo->archivoOriginal);
	}
	free(hiloJobInfo);
	hiloJobInfo = NULL;
}
