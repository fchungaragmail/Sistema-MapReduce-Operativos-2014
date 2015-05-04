/*
 * consola.c
 *
 *  Created on: 2/5/2015
 *      Author: utnso
 */

#include "consola.h"
#include <string.h>
#include <commons/collections/dictionary.h>
#include <commons/string.h>

void initConsola();
void esperarComando();
void procesarEntrada(char[],bool*);
void mostrarAyuda();
void cerrarConsola();
t_dictionary *diccionarioComandos; //En este diccionario se guardan
							//todos los pares comando -> int
							//en initConsola se pueden ver todos

void execConsola()
{
	initConsola();
	char* menuInicial = "Bienvenido al FileSystem\n";
	system("clear");
	printf(menuInicial);
	esperarComando();
}

void esperarComando()
{
	char entrada[256];
	bool continuar;
	continuar = true;
	while(continuar)
	{
	scanf("%s", entrada);
	procesarEntrada(entrada,&continuar);
	}
}


void procesarEntrada(char entrada[],bool *continuar)
{
	char** comando = string_n_split(entrada,1," ");

	*continuar = true;
	int eleccion = dictionary_get(diccionarioComandos,comando[0]);
	switch (eleccion)
	{
	case 1:
	{
		mostrarAyuda();
		break;
	}
	case 2:
	{
		//Mover archivo
		break;
	}
	case 3:
	{
		//Borrar archivo
		break;
	}
	case 90:
	{
		cerrarConsola();
		*continuar = false;
		break;
	}
	default:
	{
		printf("Comando desconocido.\n");
		break;
	}
	}
}

void initConsola()
{
	//Aca vemos todos los comandos y sus integer
	//para el case de "procesarComandos()
	diccionarioComandos = dictionary_create();
	dictionary_put(diccionarioComandos,"ayuda",1);
	dictionary_put(diccionarioComandos,"salir",90);
	dictionary_put(diccionarioComandos,"mv",2);
	dictionary_put(diccionarioComandos,"rm",3);
}

void mostrarAyuda()
{
	printf("Comandos disponibles:\n"
			"ayuda	Muestra el menu de ayuda\n"
			"salir	Sale de la consola\n"
			"mv		Mover/renombrar arcivo. Simil UNIX"
			"rm		Borrar archivo. Simil UNIX");
}

void cerrarConsola()
{
	printf("Saliendo de la consola...\n");
	dictionary_destroy(diccionarioComandos);
}
