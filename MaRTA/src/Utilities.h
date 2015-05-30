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
	mensaje_t *mensaje;
};
typedef struct _message Message;

struct mensaje
{
	int comandoSize;
	char* comando;
	long dataSize; 	//Pongo long xq en un int no entraria el valor
	char* data;		//correspondiente a 20mb
}__attribute__((packed));
typedef struct mensaje mensaje_t;

typedef enum
{
	K_Unidentified = -1,

	K_NewConnection = 0,
	K_Job_NewFileToProcess = 1,
	K_Job_MapResponse = 2,
	K_Job_ReduceResponse = 3,

	K_FS_FileFullData = 5, //FS envia tabla con direccion de archivo en los distintos nodos
}TypesMessages;

typedef enum
{
	K_Pedido_Map = 0,
	K_Pedido_Reduce = 1,
	K_Pedido_FileData = 2,
}TypesPedidosRealizado;

typedef enum {
				UNINITIALIZED = 0, 	//sin estado
				IN_MAPPING = 1, 	//se esta mappeando, se espera respuesta del job
				MAPPED = 2, 		//archivo ya mappeado
				IN_REDUCING = 3,
				REDUCED = 4,
				TOTAL_ERROR = 5  	// fallo la operacion y el FS no tiene otros bloques disponibles para procesar
} statusBlock;

struct _blockData{
	int nroDeNodo;
	int nroDeBloque;
	char *archTemporal;
	statusBlock estado;
};
typedef struct _blockData BlockData;

/*
// 	CONSTANTES
*/

//BlockState
#define K_BlockState_UninitializedPath "BlockState_UninitializedPath"

//Varias
#define K_PUERTO_LOCAL 6796
#define K_FS_IP -1 //DEFINIR
#define K_FS_PUERTO -1 //DEFINIR

#endif /* UTILITIES_H_ */
