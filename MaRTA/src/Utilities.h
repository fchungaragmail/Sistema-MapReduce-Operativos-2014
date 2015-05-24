/*
 * Message.h
 *
 *  Created on: 23/5/2015
 *      Author: utnso
 */

#ifndef UTILITIES_H_
#define UTILITIES_H_

struct _message {
	int sockfd;
	int head;
	char *messageData;
};
typedef struct _message Message;

#define 	HEAD_LENGHT 5

typedef enum
{
	K_Error = -1,
	K_NewConnection = 0,
	K_FSMessage = 1,
	K_JobMessage = 2

}TypesMessages;

#endif /* UTILITIES_H_ */
