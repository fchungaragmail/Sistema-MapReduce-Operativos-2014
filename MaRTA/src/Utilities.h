/*
 * Message.h
 *
 *  Created on: 23/5/2015
 *      Author: utnso
 */

#ifndef UTILITIES_H_
#define UTILITIES_H_
/*
//Funciones auxiliares
*/

char* intToCharPtr(int x);
int charPtrToInt(char* ptr);

/*
// ESTRUCTURAS
*/
struct _message {
	int sockfd;
	int head;
	char *messageData;
};
typedef struct _message Message;

typedef struct mensaje
{
	int comandoSize;
	char* comando;
	long dataSize; 	//Pongo long xq en un int no entraria el valor
	char* data;		//correspondiente a 20mb
} mensaje_t;

typedef enum
{
	K_Error = -1,
	K_NewConnection = 0,
	K_Job_NewFileToProcess = 1,//Job envia path de su archivo y modo combiner
	K_Job_ChangeBlockState = 3,
	K_Job_OperationFailure =4,

	K_FS_FileFullData = 2, //FS envia tabla con direccion de archivo en los distintos nodos
	K_FSMessage = 999,

}TypesMessages;

typedef enum {
				UNINITIALIZED = 0, 	//sin estado
				IN_MAPPING = 1, 	//se esta mappeando, se espera respuesta del job
				MAPPED = 2, 		//archivo ya mappeado
				IN_REDUCING = 3,
				REDUCED = 4,
				TEMPORAL_ERROR = 5,//fallo la operacion de map o reduce
				TOTAL_ERROR = 6  	// fallo la operacion y el FS no tiene otros bloques disponibles para procesar
} status;

struct _blockData{
	int nroDeNodo;
	int nroDeBloque;
	char *archTemporal;
	status estado;
};
typedef struct _blockData BlockData;

/*
// 	CONSTANTES
*/

#define 	HEAD_LENGHT 5

#define K_PUERTO_LOCAL 6782
#define K_FS_IP -1 //DEFINIR
#define K_FS_PUERTO -1 //DEFINIR

#endif /* UTILITIES_H_ */
