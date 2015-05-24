/*
 * Message.h
 *
 *  Created on: 23/5/2015
 *      Author: utnso
 */

#ifndef UTILITIES_H_
#define UTILITIES_H_

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
	K_FSMessage = 1,
	K_JobMessage = 2

}TypesMessages;

/*
// 	CONSTANTES
*/

#define 	HEAD_LENGHT 5

#endif /* UTILITIES_H_ */
