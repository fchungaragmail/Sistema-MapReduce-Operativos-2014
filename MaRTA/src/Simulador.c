/*
 * Simulador.c
 *
 *  Created on: 6/6/2015
 *      Author: utnso
 */


#include "Simulador.h"

Message* simulacion_FS_DataFullResponse();

Message* simulacion_FS_DataFullResponse()
{
	//--> FS responde con tabla de archivo pedida
	//-Comando: "DataFileResponse rutaDelArchivo Respuesta"
	//-Data: cantidadDeBloques-nroDeCopias-sizeEstructura-estructura
	//estructura va a ser IPNodo-nroDeBloque-IPNodo-nroDeBloque-IPNodo-nroDeBloque...

	Message *fsResponse = malloc(sizeof(Message));

	//armo Comando
	int16_t comandoSize = (strlen("DataFileResponse /user/juan/datos/temperatura2012.txt/ 1"))+1;
	strcpy(fsResponse->mensaje->comando,"DataFileResponse /user/juan/datos/temperatura2012.txt/ 1");

	//armo Data --> matriz de 4x4
	int i,j; t_dictionary *matriz[4][4];

	char *IP1 = "192.163.2.5";
	char *IP2 = "192.456.8.9";
	char *IP3 = "195.456.2.5";
	char *IP4 = "196.422.1.1";

	for(i=0;i<4;i++){

		char *ip;
		if(i==0){ ip=IP1; };
		if(i==1){ ip=IP2; };
		if(i==2){ ip=IP3; };
		if(i==3){ ip=IP4; };

		for(j=0;j<4;j++){

				t_dictionary *dic = malloc(sizeof(t_dictionary));

			}

	}


}
