/*
 * consola.c
 *
 *  Created on: 2/5/2015
 *      Author: utnso
 */

#include "consola.h"
#include <string.h>;

void execConsola()
{
	char entrada[256];
	char* menuInicial =
			"Bienvenido al FileSystem\n"
			"Escriba \"ayuda\" para visualizar los comandos disponibles.\n";
	system("clear");
	printf(menuInicial);

	scanf("%s", entrada);
	procesarEntrada(entrada);
	scanf("%s", entrada);
}


void procesarEntrada(char entrada[])
{
	t_comandos comandos;
	switch (comandos[entrada])
	{
	case 0:
	{
		printf("Menu de Ayuda.\n");
		break;
	}
	default:
	{
		printf("Comando desconocido.\n");
		break;
	}
	}
}
