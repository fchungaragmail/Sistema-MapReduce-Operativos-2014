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
#include <fcntl.h>
#include <sys/mman.h>
#include <inttypes.h>
#include "recursosCompartidos.h"
#include "conexiones.h"

void initComandos();
void procesarComando(char** comando, void(*doComando)(void*));
int format(char* argumentos);
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
int espacioTotal();
void procesarComandoRemoto(argumentos_t* args);
void actualizarEstadoArchivos();


#endif /* COMANDOS_H_ */
