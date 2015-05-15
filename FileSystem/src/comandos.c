/*
 * comandos.c
 *
 *  Created on: 14/5/2015
 *      Author: utnso
 */

#include "comandos.h"

void procesarComando(char** comando, void(*doComando)(void*));




void procesarComando(char** comando, void(*doComando)(void*))
{
	pthread_t tDoComando;
	char* message = string_from_format("Procesando comando: %s, "
					"con los argumentos: %s", comando[0], comando[1]);
	log_info(log, message);
	pthread_create(&tDoComando, NULL, (*doComando), comando[1]);
}
