/*
 * Simulador.c
 *
 *  Created on: 6/6/2015
 *      Author: utnso
 */


#include "Simulador.h"
#include "Utilities.h"
#include <commons/string.h>

int nroDeLlamado = -1;

//FS
Message* simulacion_FS_DataFullResponse();

//Job
Message *simulacion_Job_newFileToProcess();
Message *simulacion_Job_mapResponse();
Message *simulacion_Job_reduceResponse();
Message *simulacion_Job_mapResponse_Fallo();

//Ambos
Message *simulacion_NewConnection(int sckt);
Message *simular();

Message *simular()
{
	nroDeLlamado = nroDeLlamado + 1;
	if(nroDeLlamado == 0){ return simulacion_NewConnection(K_Simulacion_ScktJob); }
	if(nroDeLlamado == 1){ return simulacion_Job_newFileToProcess(); }
	if(nroDeLlamado == 2){ return simulacion_FS_DataFullResponse(); }
	//************
	if(nroDeLlamado == 5){ return simulacion_Job_mapResponse_Fallo(); }
	if(nroDeLlamado == 6){ return simulacion_Job_mapResponse_Fallo(); }
	if(nroDeLlamado == 7){ return simulacion_Job_mapResponse_Fallo(); }
	if(nroDeLlamado == 8){

		return simulacion_FS_DataFullResponse();
	}

	if(nroDeLlamado < 11 ){ return simulacion_Job_mapResponse(); }
	//************
	if(nroDeLlamado == 12){
		printf("se llamo el reduceResponse");
		return simulacion_Job_reduceResponse();
	}


}
Message* simulacion_FS_DataFullResponse()
{
	//--> FS responde con tabla de archivo pedida
	//-Comando: "DataFileResponse rutaDelArchivo Respuesta cantidadDeBloques-nroDeCopias-sizeEstructura-"
	//-Data: estructura
	//estructura va a ser IPNodo-nroDeBloque-IPNodo-nroDeBloque-IPNodo-nroDeBloque...

	Message *fsResponse = malloc(sizeof(Message));

	//armo Comando
	char *comando = string_new();
	string_append(&comando,"DataFileResponse /user/juan/datos/temperatura2012.txt/ 1 4 3 24");//24 elementos tiene el *data
	fsResponse->mensaje = malloc(sizeof(mensaje_t));
	fsResponse->mensaje->comandoSize = strlen(comando);
	fsResponse->mensaje->comando=malloc(strlen(comando));
	strcpy(fsResponse->mensaje->comando,comando);

	//armo Data
	char *data1 = "195.456.2.5 01 196.422.1.1 76 192.456.8.9 55 ";
	char *data2 = "192.163.2.5 20 192.456.8.9 54 192.163.2.5 36 ";
	char *data3 = "195.456.2.5 99 192.163.2.5 85 192.163.2.5 77 ";
	char *data4 = "192.456.8.9 88 192.153.7.5 82 198.167.5.9 22";

	char *data = string_new();
	string_append(&data,data1);
	string_append(&data,data2);
	string_append(&data,data3);
	string_append(&data,data4);

	fsResponse->mensaje->dataSize = strlen(data);
	fsResponse->mensaje->data=malloc(strlen(data));
	fsResponse->mensaje->data=data;

	fsResponse->sockfd = K_Simulacion_ScktFS;

	return fsResponse;
}

Message *simulacion_Job_newFileToProcess()
{
		//-->Job le pasa a MaRTA su archivo a procesar (pueden ser muchos, en este caso seran varios envios)
		//*comando: "archivoAProcesar rutaDeArchivo soportaCombiner"
		//*data: NADA

		Message *jobMsj = malloc(sizeof(Message));
		jobMsj->mensaje= malloc(sizeof(mensaje_t));
		char *comando = string_new();
		string_append(&comando,"archivoAProcesar /user/juan/datos/temperatura2012.txt/ 0");

		jobMsj->mensaje->comandoSize = strlen(comando);
		jobMsj->mensaje->comando = malloc(strlen(comando));
		jobMsj->mensaje->comando = comando;

		jobMsj->mensaje->dataSize = 0;
		jobMsj->mensaje->data = malloc(0);
		jobMsj->mensaje->data = "";
		jobMsj->sockfd = K_Simulacion_ScktJob;
		return jobMsj;
}

Message *simulacion_Job_mapResponse()
{
	//-->Job responde a Marta con el resultado de la operacion de map
	//*comando : "mapFileResponse rutaArchivoTemporal Respuesta"
	//*data:NADA

	Message *jobMsj = malloc(sizeof(Message));
	jobMsj->mensaje  = malloc(sizeof(mensaje_t));
	char *comando = string_new();
	string_append(&comando,"mapFileResponse /user/juan/datos/temperatura2012.txt/-23:43:45:2345 1");
	jobMsj->mensaje->comandoSize = strlen(comando);
	jobMsj->mensaje->comando=malloc(strlen(comando));
	jobMsj->mensaje->comando = comando;

	jobMsj->mensaje->data = malloc(0);
	jobMsj->mensaje->dataSize = 0;
	jobMsj->mensaje->data = "";
	jobMsj->sockfd = K_Simulacion_ScktJob;
	return jobMsj;
}

Message *simulacion_Job_mapResponse_Fallo()
{
	//-->Job responde a Marta con el resultado de la operacion de map
	//*comando : "mapFileResponse rutaArchivoTemporal Respuesta"
	//*data:NADA

	Message *jobMsj = malloc(sizeof(Message));
	jobMsj->mensaje  = malloc(sizeof(mensaje_t));
	char *comando = string_new();
	string_append(&comando,"mapFileResponse /user/juan/datos/temperatura2012.txt/-23:43:45:2345 0");
	jobMsj->mensaje->comandoSize = strlen(comando);
	jobMsj->mensaje->comando=malloc(strlen(comando));
	jobMsj->mensaje->comando = comando;

	jobMsj->mensaje->data = malloc(0);
	jobMsj->mensaje->dataSize = 0;
	jobMsj->mensaje->data = "";
	jobMsj->sockfd = K_Simulacion_ScktJob;
	return jobMsj;
}
Message *simulacion_Job_reduceResponse()
{
	//-->Job responde a Marta con el resultado de la operacion de map
	//*comando : "mapFileResponse rutaArchivoTemporal Respuesta"
	//*data:NADA

	Message *jobMsj = malloc(sizeof(Message));
	jobMsj->mensaje  = malloc(sizeof(mensaje_t));
	char *comando = string_new();
	string_append(&comando,"reduceFileResponse /user/juan/datos/temperatura2012.txt/-23:43:45:2345 1");
	jobMsj->mensaje->comandoSize = strlen(comando);
	jobMsj->mensaje->comando=malloc(strlen(comando));
	jobMsj->mensaje->comando = comando;

	jobMsj->mensaje->data = malloc(0);
	jobMsj->mensaje->dataSize = 0;
	jobMsj->mensaje->data = "";
	jobMsj->sockfd = K_Simulacion_ScktJob;
	return jobMsj;
}

Message *simulacion_NewConnection(int sckt){

	Message *newConnection;
	newConnection=malloc(sizeof(Message));
	newConnection->mensaje=malloc(sizeof(*newConnection->mensaje));

	newConnection->mensaje->comandoSize=(strlen("newConnection")+1);
	newConnection->mensaje->comando=malloc(strlen("newConnection")+1);
	strcpy(newConnection->mensaje->comando,"newConnection");

	newConnection->mensaje->data = malloc(0);
	newConnection->mensaje->data="";
	newConnection->sockfd=sckt;
	return newConnection;
}
