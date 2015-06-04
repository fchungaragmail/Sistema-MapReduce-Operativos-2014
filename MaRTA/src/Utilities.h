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

/*
// ESTRUCTURAS
*/

typedef struct mensaje
{
	int16_t comandoSize;
	char* comando;
	int32_t dataSize; 	//Pongo long xq en un int no entraria el valor
	char* data;		//correspondiente a 20mb
}mensaje_t;

typedef struct _message {
	int sockfd;
	mensaje_t *mensaje;
}Message;

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
				UNINITIALIZED = -1, 	//sin estado
				IN_MAPPING = 1, 	//se esta mappeando, se espera respuesta del job
				MAPPED = 2, 		//archivo ya mappeado
				IN_REDUCING = 3,
				REDUCED = 4,
				TOTAL_ERROR = 5  	// fallo la operacion y el FS no tiene otros bloques disponibles para procesar
} statusBlock;


/*
// 	CONSTANTES
*/

//BlockState
#define K_BlockState_UninitializedPath "BlockState_UninitializedPath"
#define K_BlockState_UninitializedNodo "BlockState_UninitializedNodo"

//Copia de FullData(tabla que envia el FS)
#define K_Copia_DarDeBajaIPNodo "Copia_DarDeBajaIPNodo"

//Varias
#define K_PUERTO_LOCAL 6815
#define K_FS_IP -1 //DEFINIR
#define K_FS_PUERTO -1 //DEFINIR

#endif /* UTILITIES_H_ */
