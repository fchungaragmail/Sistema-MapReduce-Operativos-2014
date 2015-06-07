/*
 * comandos.c
 *
 *  Created on: 14/5/2015
 *      Author: utnso
 */

#include "comandos.h"
#define MARTA "MaRTA"


void procesarComando(char** comando, void(*doComando)(void*));
void procesarComandoRemoto(mensaje_t* mensaje, Conexion_t* conexion);
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



void procesarComando(char** comando, void(*doComando)(void*))
{
	pthread_t tDoComando;
	char* message = string_from_format("Procesando comando: %s, "
					"con los argumentos: %s", comando[0], comando[1]);
	log_info(log, message);
	pthread_create(&tDoComando, NULL, (*doComando), comando[1]);
}


void procesarComandoRemoto(mensaje_t* mensaje, Conexion_t* conexion)
{
	bool dummy;
	if (procesarEntrada(mensaje->comando,&dummy) != 0)
	{
		//Procesarla como remoto
		//char** comando = string_n_split(mensaje->comando,2," ");
		nomb(mensaje->comando,conexion);
	}
}


int mover(char* argumentos){
	printf("Mover\n");
	return 0;
}


int borrar(char* argumentos){
	printf("Borrar\n");
	return 0;
}


int crearDir(char* argumentos){
	printf("Crear Directorio\n");
	return 0;
}


int importar(char* argumentos){
	printf("Importar archivo\n");
	return 0;
}


int exportar(char* argumentos){
	printf("Exportar archivo\n");
	return 0;
}


int md5(char* argumentos){
	printf("md5 de archivo\n");
	return 0;
}


int bloques(char* argumentos){
	printf("Mostrar bloques de un archivos\n");
	return 0;
}


int borrarBloque(char* argumentos){
	printf("Borrar un bloque de un archivo\n");
	return 0;
}


int copiarBloque(char* argumentos){
	printf("Copiar un bloque\n");
	return 0;
}


int agregarNodo(char* argumentos){
	printf("Agregar nodo\n");
	return 0;
}


int quitarNodo(char* argumentos){
	printf("Quitar nodo\n");
	return 0;
}

int nomb(char* argumentos, Conexion_t* conexion)
{
	char** tmp;
	tmp = string_split(argumentos, " ");

	if (strcmp(conexion->nombre,tmp[1]) == 0)
	{
		//El nodo ya existia
		log_info(log, "El nodo %s ya estaba identificado", conexion->nombre);
		return 0;
	}

	strcpy(conexion->nombre, tmp[1]);
	log_info(log, "Identificado el nodo %s", conexion->nombre);
	if (strcmp(conexion->nombre, "MaRTA") != 0)
	{
		nodosOnline++;
		if (nodosOnline == LISTA_NODOS)
			log_info(log, "Cantidad minima de nodos (%d) alcanzada.", LISTA_NODOS);
	}
	return 0;
}



