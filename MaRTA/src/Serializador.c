/*
 * Serializador.c
 *
 *  Created on: 1/6/2015
 *      Author: utnso
 */


#include "Serializador.h"
#include <stdbool.h>
#include "FilesStatusCenter.h"
#include <commons/string.h>

char *deserializeFilePath(Message *recvMessage,TypesMessages type);
bool* deserializeSoportaCombiner(Message *recvMessage);
bool* deserializeRequestResponse(Message *recvMessage,TypesMessages type);
char* deserializeComando(Message *recvMessage);
t_dictionary* deserializarFullDataResponse(Message *recvMessage);

char* createStream();
void addIntToStream(char *stream, int value,IntTypes type);
void addBoolToStream(char *stream, bool value);
void addStringToStream(char *stream,char *value);

char *deserializeFilePath(Message *recvMessage,TypesMessages type)
{
	char* filePath;
	int offset,size;
	size = 0; offset = 0;

	if(type == K_Job_NewFileToProcess){
		//Segun protocolo recvMessage->mensaje->comando sera
		//*comando: "archivoAProcesar rutaDeArchivo soportaCombiner"
		//*data: NADA
		//necesito obtener "rutaDeArchivo"

		char *comandStr = recvMessage->mensaje->comando;
		char **comandoArray = string_split(comandStr," ");
		return comandoArray[1];
	}

	if(type == K_FS_FileFullData){
		//Tengo que ver como me pasa Juan el FullData
		//-Comando: "DataFileResponse rutaDelArchivo Respuesta cantidadDeBloques-nroDeCopias-sizeEstructura"
		//-Data: estructura
		//estructura va a ser IPNodoX-nroDeBloqueX-IPNodoY-nroDeBloqueY-IPNodoZ-nroDeBloqueZ...
		//Debo obtener "rutaDelArchivo"

		char *comandoStr = recvMessage->mensaje->comando;
		char **comandoArray = string_split(comandoStr," ");
		return comandoArray[1];
	}

	if(type == K_Job_MapResponse || type == K_Job_ReduceResponse){
		//Segun protocolo recvMessage->mensaje->data sera
		//*comando(map) : "mapFileResponse rutaArchivoTemporal Respuesta"
		//*comando(reduce) : "reduceFileResponse rutaArchivoTemporal Respuesta"
		//*data:NADA
		//necesito obtener "rutaArchivoTemporal"

		char *comandoStr = recvMessage->mensaje->comando;
		char **comandoArray = string_split(comandoStr," ");
		return comandoArray[1];
	}

	return filePath;
}

bool* deserializeSoportaCombiner(Message *recvMessage)
{
	//Segun protocolo recvMessage->mensaje->data sera
	//*comando: "archivoAProcesar rutaDeArchivo soportaCombiner"
	//*data: NADA
	//necesito obtener "soportaCombiner"

	char *comandoStr = recvMessage->mensaje->comando;
	char **comandoArray = string_split(comandoStr," ");
	char *soportaCombinerStr = comandoArray[2];

	bool* soportaCombiner = malloc(sizeof(bool));
	if(strcmp(soportaCombinerStr,"1") == 0){ *soportaCombiner = true; }
	if(strcmp(soportaCombinerStr,"1") != 0){ *soportaCombiner = false; }

	return soportaCombiner;

}

bool* deserializeRequestResponse(Message *recvMessage,TypesMessages type)
{
		bool* requestResponse = malloc(sizeof(bool));

		if(type==K_Job_MapResponse || type==K_Job_ReduceResponse){
			//segun protocolo el data sera
			//*comando(map) : "mapFileResponse rutaArchivoTemporal Respuesta"
			//*comando(reduce) : "reduceFileResponse rutaArchivoTemporal Respuesta"
			//*data:NADA
			//debo obtener "Respuesta"
			requestResponse = deserializeSoportaCombiner(recvMessage);
			return requestResponse;
		}

		if(type == K_FS_FileFullData){
			//segun protocolo
			//-Comando: "DataFileResponse rutaDelArchivo Respuesta cantidadDeBloques-nroDeCopias-sizeEstructura"
			//-Data: estructura
			//estructura va a ser IPNodoX-nroDeBloqueX-IPNodoY-nroDeBloqueY-IPNodoZ-nroDeBloqueZ...
			//debo obtener "Respuesta"
			requestResponse = deserializeSoportaCombiner(recvMessage);
			return requestResponse;
		}
}

char* deserializeComando(Message *recvMessage)
{
	//Segun protocolo el "comando" siempre es lo 1ero del stream
	char *comandoStr = recvMessage->mensaje->comando;
	char **comandoArray = string_split(comandoStr," ");
	char *soportaCombinerStr = comandoArray[2];

	return comandoArray[0];
}

t_dictionary* deserializarFullDataResponse(Message *recvMessage)
{
	// segun protocolo
	//-Comando: "DataFileResponse rutaDelArchivo Respuesta cantidadDeBloques-nroDeCopias-sizeEstructura"
	//-Data: estructura
	//estructura va a ser IPNodoX-nroDeBloqueX-IPNodoY-nroDeBloqueY-IPNodoZ-nroDeBloqueZ...
	//1ero fila 1,2,3,4,5....

	t_dictionary *fullDataDic = dictionary_create();

	char *_comando = recvMessage->mensaje->comando;
	char **comando = string_split(_comando," ");
	char *strCantDeBloques = comando[3];
	char *strNroDeCopias = comando[4];
	int16_t cantidadDeBloques = strtol(strCantDeBloques, (char **)NULL, 10);
	int16_t nroDeCopias = strtol(strNroDeCopias, (char **)NULL, 10);

	t_dictionary *dataDic = dictionary_create();
	t_dictionary *dataMatriz[cantidadDeBloques][nroDeCopias];
	char *data = recvMessage->mensaje->data;
	char **dataArray = string_split(data," ");
	int i,j;
	for(i=0;i<nroDeCopias;i++){
		for(j=0;j<(nroDeCopias*2);j=j+2){

			t_dictionary *dic = dictionary_create();
			char *ipNodo = dataArray[j];
			char *nroDeBloque = dataArray[j+1];
			dictionary_put(dic,K_Copia_IPNodo,ipNodo);
			dictionary_put(dic,K_Copia_NroDeBloque,nroDeBloque);
			dataMatriz[i][j] = dic;
		}
	}

	dictionary_put(fullDataDic,K_fullData_CantidadDeBloques,cantidadDeBloques);
	dictionary_put(fullDataDic,K_fullData_CantidadDeCopias,nroDeCopias);
	dictionary_put(fullDataDic,K_fullData_Data,dataDic);

	return fullDataDic;
}
char* createStream()
{
	char *stream = string_new();
	return stream;
}

void addIntToStream(char *stream, int value,IntTypes type)
{
	char *intValue = intToCharPtr(value);
	string_append(&stream," ");
	string_append(&stream,intValue);
}

void addBoolToStream(char *stream, bool value)
{
	if(value == true){ string_append(&stream," 1");};
	if(value == false){ string_append(&stream," 0");};
}

void addStringToStream(char *stream, char *str)
{
	string_append(&stream,str);
}
