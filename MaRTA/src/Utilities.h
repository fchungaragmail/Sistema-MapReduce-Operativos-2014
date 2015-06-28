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
	K_ProcesoCaido = 4,
	K_FS_FileFullData = 5, //FS envia tabla con direccion de archivo en los distintos nodos
}TypesMessages;

typedef enum
{
	K_Pedido_Map = 0,
	K_Pedido_Reduce = 1,
	K_Pedido_FileData = 2,
}TypesPedidosRealizado;

// Keys --> StatusBlock
typedef enum
{
	K_UNINITIALIZED = 1, 	//sin estado
	K_IN_MAPPING =2,//se esta mappeando, se espera respuesta del job
	K_MAPPED = 3, 		//archivo ya mappeado
	K_IN_REDUCING = 4,
	K_REDUCED = 5,
	K_TOTAL_ERROR = 6  	// fallo la operacion y el FS no tiene otros bloques disponibles para procesar
}StatusBlockState;

/*
// 	CONSTANTES
*/

//BlockState
#define K_BlockState_UninitializedPath "BlockState_UninitializedPath"
#define K_BlockState_UninitializedIPNodo "BlockState_UninitializedIPNodo"
#define K_BlockState_UninitializedBlqe "BlockState_UninitializedBlqe"
#define K_BlockState_UninitializedPuertoNodo"BlockState_UninitializedPuertoNodo"

//Copia de FullData(tabla que envia el FS)
#define K_Copia_DarDeBajaIPNodo "Copia_DarDeBajaIPNodo"

// HiloDic --> keys
#define K_HiloDic_Sem "HiloDic_Sem"
#define K_HiloDic_Path "HiloDic_Path"
#define K_HiloDic_JobSocket "HiloDic_JobSocket"
#define K_HiloDic_PedidosQueue "PedidosQueue"

//Varias
#define K_PUERTO_LOCAL 6851
#define K_FS_IP -1 //DEFINIR
#define K_FS_PUERTO -1 //DEFINIR

//Simulacion
#define K_Simulacion_ScktJob 	5
#define K_Simulacion_ScktFS 	3

#endif /* UTILITIES_H_ */
