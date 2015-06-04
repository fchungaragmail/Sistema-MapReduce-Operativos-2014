/*
 * Serializador.c
 *
 *  Created on: 1/6/2015
 *      Author: utnso
 */


#include "Serializador.h"
#include <stdbool.h>
#include "FilesStatusCenter.h"

char *deserializeFilePath(Message *recvMessage,TypesMessages type);
bool* deserializeSoportaCombiner(Message *recvMessage);
bool* deserializeRequestResponse(Message *recvMessage,TypesMessages type);
char* deserializeComando(Message *recvMessage);
t_dictionary* deserializarFullDataResponse(Message *recvMessage);

void* createStream();
void addIntToStream(void *stream, int value,IntTypes type);
void addBoolToStream(void *stream, bool value);
void addStringToStream(void *stream,char *value);

char *deserializeFilePath(Message *recvMessage,TypesMessages type)
{
	char* filePath;
	int offset,size;
	size = 0; offset = 0;

	if(type == K_Job_NewFileToProcess){
		//Segun protocolo recvMessage->mensaje->data sera
		//*data: sizeRutaDeArchivo-rutaDeArchivo-soportaCombiner
		//necesito obtener "rutaDeArchivo"
		void *data = recvMessage->mensaje->data;
		int16_t sizeRutaDeArchivo;
		memcpy(&sizeRutaDeArchivo,data,size = sizeof(int16_t));
		offset = offset + size;
		filePath = malloc(sizeRutaDeArchivo + 1);
		memcpy(filePath,data+offset,size = (sizeRutaDeArchivo + 1));
		return filePath;
	}

	if(type == K_FS_FileFullData){
		//Tengo que ver como me pasa Juan el FullData
		//La estructura hasta el momento es:
		//-Data:sizeRutaDelArchivo-rutaDelArchivo-sizeCantidadDeBloques-cantidadDeBloques-sizeEstructura-estructura
		//Debo obtener "rutaDelArchivo"
		void *data = recvMessage->mensaje->data;
		int16_t sizeRutaDeArchivo;
		memcpy(&sizeRutaDeArchivo,data,size = sizeof(int16_t));
		offset = offset + size;
		filePath = malloc(sizeRutaDeArchivo + 1);
		memcpy(filePath,data+offset,size = (sizeRutaDeArchivo + 1));
		return filePath;
	}

	if(type == K_Job_MapResponse || type == K_Job_ReduceResponse){
		//Segun protocolo recvMessage->mensaje->data sera
		//*data:sizeRutaArchivoTemporal-rutaArchivoTemporal-Respuesta
		//necesito obtener "rutaArchivoTemporal"
		void *data = recvMessage->mensaje->data;
		int16_t sizeRutaDeArchivo;
		memcpy(&sizeRutaDeArchivo,data,size = sizeof(int16_t));
		offset = offset + size;
		filePath = malloc(sizeRutaDeArchivo + 1);
		memcpy(filePath,data+offset,size = (sizeRutaDeArchivo + 1));
		return filePath;
	}

	return filePath;
}

bool* deserializeSoportaCombiner(Message *recvMessage)
{
		//Segun protocolo recvMessage->mensaje->data sera
		//*data: sizeRutaDeArchivo-rutaDeArchivo-soportaCombiner
		//necesito obtener "soportaCombiner"
	void *data = recvMessage->mensaje->data;
	int sizeRutaDeArchivo;
	memcpy(&sizeRutaDeArchivo,data,sizeRutaDeArchivo = sizeof(int16_t));
	int offset,size; offset = 0; size = 0;
	offset = sizeof(int16_t) + sizeRutaDeArchivo;
	int8_t _soportaCombiner;
	memcpy(&_soportaCombiner,data,sizeof(int8_t));

	bool* soportaCombiner = malloc(sizeof(bool));
	if(_soportaCombiner == 1){ *soportaCombiner = true; }
	if(_soportaCombiner == 0){ *soportaCombiner = false; }

	return soportaCombiner;

}

bool* deserializeRequestResponse(Message *recvMessage,TypesMessages type)
{
		bool* requestResponse = malloc(sizeof(bool));

		if(type==K_Job_MapResponse || type==K_Job_ReduceResponse){
			//segun protocolo el data sera
			//*data:sizeRutaArchivoTemporal-rutaArchivoTemporal-Respuesta
			//debo obtener "Respuesta"
			requestResponse = deserializeSoportaCombiner(recvMessage);
			return requestResponse;
		}

		if(type == K_FS_FileFullData){
			//segun protocolo --> Comando: "DataFileResponse"
			//Si existe el archivo --> Data:sizeRutaDelArchivo-rutaDelArchivo-Respuesta-.......
			//debo obtener "Respuesta"
			requestResponse = deserializeSoportaCombiner(recvMessage);
			return requestResponse;
		}
}

char* deserializeComando(Message *recvMessage)
{
	int16_t size = recvMessage->mensaje->comandoSize;
	char *comando = malloc(size+1);
	void *head_comando = recvMessage->mensaje->comando;
	memcpy(comando,head_comando,size);
	return comando;
}

t_dictionary* deserializarFullDataResponse(Message *recvMessage)
{
	// segun protocolo
	//-Data:sizeRutaDelArchivo-rutaDelArchivo-Respuesta-cantidadDeBloques-nroDeCopias-sizeEstructura-estructura

	t_dictionary *fullDataDic = dictionary_create();
	void *data = recvMessage->mensaje->data;

	int16_t sizeRutaDelArchivo;
	memcpy(&sizeRutaDelArchivo,data,sizeof(int16_t));

	int offset = sizeof(int16_t) + sizeRutaDelArchivo;
	int size;

	int16_t cantidadDeBloques;
	int16_t nroDeCopias;

	memcpy(&cantidadDeBloques,data+offset,size = sizeof(int16_t));
	offset = offset + size;
	memcpy(&nroDeCopias,data+offset,size = sizeof(int16_t));
	offset = offset + size;

	dictionary_put(fullDataDic,K_fullData_CantidadDeBloques,cantidadDeBloques);
	dictionary_put(fullDataDic,K_fullData_CantidadDeCopias,nroDeCopias);

	//FATAL SABER COMO ME VA A LLEGAR LA "estructura"

	return fullDataDic;
}
void* createStream()
{
	void *stream = malloc(sizeof(char));
	return stream;
}

void addIntToStream(void *stream, int value,IntTypes type)
{
	void *intStream;

	if(type == K_int16_t){
		intStream = malloc(sizeof(int16_t));
		int16_t streamInt = value;
		intStream = &streamInt;
	}
	if(type == K_int32_t){
		intStream = malloc(sizeof(int32_t));
		int32_t streamInt = value;
		intStream = &streamInt;
	}

	int offset = strlen(stream);
	stream = realloc(stream,strlen(stream)+sizeof(*intStream));
	memcpy(stream + offset ,intStream,sizeof(*intStream));

}

void addBoolToStream(void *stream, bool value)
{
	int8_t bool8 = value;

	int offset = strlen(stream);
	stream = realloc(stream,strlen(stream)+sizeof(int8_t));
	memcpy(stream + offset ,bool8,sizeof(int8_t));

}

void addStringToStream(void *stream, char *str)
{
	//se agrega al stream un --> ...-stringSize-string-...
	addIntToStream(stream,strlen(str),K_int16_t);

	int offset = strlen(stream);
	stream = realloc(stream,strlen(stream)+sizeof(str));
	memcpy(stream + offset ,str,sizeof(str));

}

/*
 //serializo
char *request = "saraza";
int tipo=2;
int largo = 3;
char* data = malloc(sizeof(int)+sizeof(int)+strlen(request)+1);
int size = 0, offset = 0;
offset += size;
memcpy(data + offset,&tipo, size = sizeof(int));
offset += size;
memcpy(data + offset,&largo,size = sizeof(int));
offset += size;
memcpy(data + offset,request,size = strlen(request) + 1);
printf("Quedó %s\n:",(char *)data);

//deserializo
int a,b;
offset = 0; size = 0;
memcpy(&a,data,size = sizeof(int));
offset = offset + size;
memcpy(&b,data+offset,size = sizeof(int));
offset = offset + size;
char *str = malloc(strlen("saraza")+1);
memcpy(str,data+offset,size = (strlen("saraza")+1));
offset = offset + size;

printf("el valor de a es : %d\n",a);
printf("el valor de b es : %d\n",b);
printf("el valor de str es : %s\n",str);
*/
