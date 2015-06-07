/*
 * comandos.h
 *
 *  Created on: 14/5/2015
 *      Author: utnso
 */

#ifndef COMANDOS_H_
#define COMANDOS_H_

#include <pthread.h>
#include <commons/string.h>
#include <unistd.h>
#include <sys/stat.h>
#include "recursosCompartidos.h"
#include "protocolo.h"


void procesarComando(char** comando, void(*doComando)(void*));
int mover(char* argumentos);
int borrar(char* argumentos);
int crearDir(char* argumentos);
int importar(char* argumentos);
int exportar(char* argumentos);
int md5(char* argumentos);
int bloques(char* argumentos);
int borrarBloque(char* argumentos);
int copiarBloque(char* argumentos);
int agregarNodo(char* argumentos);
int quitarNodo(char* argumentos);
int nomb(char* argumentos, Conexion_t* conexion);
void procesarComandoRemoto(mensaje_t* mensaje, Conexion_t* conexion);


#endif /* COMANDOS_H_ */
