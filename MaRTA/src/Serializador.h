/*
 * Serializador.h
 *
 *  Created on: 1/6/2015
 *      Author: utnso
 */

#ifndef SERIALIZADOR_H_
#define SERIALIZADOR_H_

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include "Utilities.h"
#include <commons/collections/dictionary.h>

typedef enum{
	K_int16_t = 1,
	K_int32_t = 2,
}IntTypes;

char *deserializeFilePath(Message *recvMessage,TypesMessages type);
bool* deserializeSoportaCombiner(Message *recvMessage);
bool* deserializeRequestResponse(Message *recvMessage,TypesMessages type);
char* deserializeComando(Message *recvMessage);
t_dictionary* deserializarFullDataResponse(Message *recvMessage);

char* createStream();
void addIntToStream(char *stream, int value,IntTypes type);
void addBoolToStream(char *stream, bool value);
void addStringToStream(char *stream,char *value);

#endif /* SERIALIZADOR_H_ */
