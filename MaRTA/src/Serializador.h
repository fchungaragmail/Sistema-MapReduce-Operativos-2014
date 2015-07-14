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
#include <commons/collections/list.h>

char *deserializeFilePath(Message *recvMessage,TypesMessages type);
bool* deserializeSoportaCombiner(Message *recvMessage);
bool* deserializeRequestResponse(Message *recvMessage,TypesMessages type);
char* deserializeComando(Message *recvMessage);
t_list* deserializarFullDataResponse(Message *recvMessage);
char *deserializeTempFilePath(Message *recvMessage,TypesMessages type);
t_list *deserializeFailedReduceResponse(Message *recvMessage);
int deserializeNumeroDeBloque_PedidoDeCopias(Message *recvMessage);

void addIntToStream(char *stream, int value);

#endif /* SERIALIZADOR_H_ */
