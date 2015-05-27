/*
 * comandos.c
 *
 *  Created on: 14/5/2015
 *      Author: utnso
 */

#include "comandos.h"

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
int nombre(char* comando, void* conexion);



void procesarComando(char** comando, void(*doComando)(void*))
{
	pthread_t tDoComando;
	char* message = string_from_format("Procesando comando: %s, "
					"con los argumentos: %s", comando[0], comando[1]);
	log_info(log, message);
	pthread_create(&tDoComando, NULL, (*doComando), comando[1]);
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

int nombre(char* comando, void* conexion)
{
	char** lineasComando = string_n_split(comando,2," ");
	//strcpy(conexion->nombre, lineasComando[1]);
	if ((strcmp(lineasComando[1], "MaRTA")) != 0)
	{
		return 1;
	}else
	{
		return 0;
	}
}



