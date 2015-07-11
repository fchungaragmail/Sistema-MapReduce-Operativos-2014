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
#include <unistd.h>

char *deserializeFilePath(Message *recvMessage,TypesMessages type);
bool* deserializeSoportaCombiner(Message *recvMessage);
bool* deserializeRequestResponse(Message *recvMessage,TypesMessages type);
char* deserializeComando(Message *recvMessage);
t_list* deserializarFullDataResponse(Message *recvMessage);
char *deserializeTempFilePath(Message *recvMessage,TypesMessages type);
t_list *deserializeFailedReduceResponse(Message *recvMessage);
int deserializeNumeroDeBloque_PedidoDeCopias(Message *recvMessage);

char* createStream();
void addIntToStream(char *stream, int value,IntTypes type);
void addStringToStream(char **stream,char *value);

char *deserializeTempFilePath(Message *recvMessage,TypesMessages type)
{
	if(type == K_Job_MapResponse || type == K_Job_ReduceResponse){
			//Segun protocolo recvMessage->mensaje->data sera
			//*comando(map) : "mapFileResponse rutaArchivoTemporal Respuesta"
			//*comando(reduce) : "reduceFileResponse rutaArchivoTemporal Respuesta"
			//*data:NADA
			//necesito obtener "rutaArchivoTemporal"

			char *comandoStr = recvMessage->mensaje->comando;
			char **comandoArray = string_split(comandoStr," ");
			char *tempFilePath = malloc(strlen(comandoArray[1])+1);
			strcpy(tempFilePath,comandoArray[1]);

			free(comandoArray);
			return tempFilePath;
	}
}

char *deserializeFilePath(Message *recvMessage,TypesMessages type)
{
	char* filePath;

	if(type == K_Job_NewFileToProcess){
		//Segun protocolo recvMessage->mensaje->comando sera
		//*comando: "archivoAProcesar rutaDeArchivo soportaCombiner"
		//*data: NADA
		//necesito obtener "rutaDeArchivo"

		char *comandStr = recvMessage->mensaje->comando;
		char **comandoArray = string_split(comandStr," ");
		filePath = malloc(strlen(comandoArray[1])+1);
		strcpy(filePath,comandoArray[1]);

		free(comandoArray);
		return filePath;
	}

	if(type == K_FS_FileFullData){
		//Tengo que ver como me pasa Juan el FullData
		//-Comando: "DataFileResponse rutaDelArchivo Disponible"
		//-Data: Tabla
		//Debo obtener "rutaDelArchivo"

		char *comandoStr = recvMessage->mensaje->comando;
		char **comandoArray = string_split(comandoStr," ");
		filePath = malloc(strlen(comandoArray[1])+1);
		strcpy(filePath,comandoArray[1]);

		free(comandoArray);
		return filePath;
	}

	if(type == K_Job_MapResponse || type == K_Job_ReduceResponse){
		//Segun protocolo recvMessage->mensaje->data sera
		//*comando(map) : "mapFileResponse rutaArchivoTemporal Respuesta"
		//*comando(reduce) : "reduceFileResponse rutaArchivoTemporal Respuesta"
		//*data:NADA
		//necesito obtener "rutaArchivoTemporal"

		char *comandoStr = recvMessage->mensaje->comando;
		char **comandoArray = string_split(comandoStr," ");
		char *tempPath = comandoArray[1];
		char **tempPathSplit = string_split(tempPath,"-");
		filePath = malloc(strlen(tempPathSplit[0])+1);
		strcpy(filePath,tempPathSplit[0]);

		free(comandoArray);
		free(tempPathSplit);
		return filePath;
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
	if(strcmp(soportaCombinerStr,"0") == 0){ *soportaCombiner = false; }

	free(comandoArray);
	return soportaCombiner;
}

bool* deserializeRequestResponse(Message *recvMessage,TypesMessages type)
{
		bool* requestResponse;

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
			//-Comando: "DataFileResponse rutaDelArchivo Disponible"
			//-Data: Tabla
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
	char *comando = malloc(strlen((char*)(comandoArray[0]))+1);
	strcpy(comando,comandoArray[0]);

	free(comandoArray);
	return comando;
}
int fullData_obtenerCantidadDeBloqes(char **dataArray)
{
	int cantidadDeBloques=0;

	while(1){
		char *bloque = dataArray[cantidadDeBloques];
		if(bloque==NULL){
			break;
		}
		cantidadDeBloques++;
	}
	return cantidadDeBloques;

}

int fullData_obtenerCantidadDeCopias(char **copiasArray)
{
	int nroDeCopias = 0 ;

	while(1){
		char *elementoCopia = copiasArray[nroDeCopias];
		if(elementoCopia==NULL){
				break;
		}
			nroDeCopias++;
	}
	nroDeCopias = nroDeCopias - 1;
	nroDeCopias = (nroDeCopias/3);

	return nroDeCopias;
}

int deserializeNumeroDeBloque_PedidoDeCopias(Message *recvMessage){

	//--> FS responde con bloque de archivo pedido
	//-Comando: "DataFileResponse rutaDelArchivo Disponible"
	//-Data: Bloque
	// Ej: "0;Nodo1;3;Nodo8;2;Nodo2;45;"

	char *data = recvMessage->mensaje->data;
	char **copiasArray = string_split(data,";");
	char *ptrInt = copiasArray[0];
	int a = atoi(ptrInt);
	return a;

}
t_list *deserializarFullDataResponse(Message *recvMessage)
{
	// segun protocolo
	//-Comando: "DataFileResponse rutaDelArchivo Disponible"
	//-Data: Tabla
	/*// Ej: "0;Nodo1;3;Nodo8;2;Nodo2;45;
		1;Nodo2;1;Nodo1;2;Nodo3;10;"*/

	char *_comando = recvMessage->mensaje->comando;
	char **comando = string_split(_comando," ");
	char *data = recvMessage->mensaje->data;
	char **dataArray = string_split(data," ");//---> "\n" poner esto para el simulador!
	int cantidadDeBloques = fullData_obtenerCantidadDeBloqes(dataArray);

	t_list *listaPadreDeBloques = list_create();
	int i,j;

	for(i=0;i<cantidadDeBloques;i++){

		t_list *listaHijaDeCopias = list_create();
		char **copiasArray = string_split(dataArray[i],";");
		int nroDeCopias= fullData_obtenerCantidadDeCopias(copiasArray);
		int k = 1;
		for(j=0;j<nroDeCopias;j++){

			t_dictionary *dic = dictionary_create();
			char *ipNodo = copiasArray[k];
			char *puertoNodo = copiasArray[k+1];
			char *nroDeBloque = copiasArray[k+2];
			bool *estado = malloc(sizeof(bool));
			*estado = true;
			dictionary_put(dic,K_Copia_IPNodo,ipNodo);
			dictionary_put(dic,K_Copia_PuertoNodo,puertoNodo);
			dictionary_put(dic,K_Copia_NroDeBloque,nroDeBloque);
			dictionary_put(dic,K_Copia_Estado,estado);
			k=k+3;
			list_add(listaHijaDeCopias,dic);
		}
		list_add(listaPadreDeBloques,listaHijaDeCopias);
	}

	return listaPadreDeBloques;
}

t_list *deserializeFailedReduceResponse(Message *recvMessage)
{
	char *reduceType = deserializeComando(recvMessage);
	char *data = recvMessage->mensaje->data;
	char **dataSplit = string_split(data," ");
	t_list *listaP = list_create();

	if((strcmp(reduceType,"reduceFileConCombiner-Pedido1")==0)||
			(strcmp(reduceType,"reduceFileSinCombiner")==0)||
			(strcmp(reduceType,"reduceFileConCombiner-Pedido2")==0)){

		//*data: --> "IPnodo1 IPnodo2..."
		int i=0;
		while(1)
		{
			char *ipNodo = dataSplit[i];
			if(ipNodo==NULL){ break; }
			i++;
			list_add(listaP,ipNodo);
		}
	}

	return listaP;
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

void addStringToStream(char **stream, char *str)
{
	string_append(stream,str);
	string_append(stream," ");
}
