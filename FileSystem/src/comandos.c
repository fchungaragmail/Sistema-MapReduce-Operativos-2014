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

int getDir(char* dir,int16_t padre);

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

	char** dirs = string_split(argumentos,"/");

	char* dir;
	int16_t padre = 0;
	int i = 0;
	dir = dirs[i];

	while (dir != NULL)
	{
		int index = getDir(dir,padre);
		if (index<0)
		{
			t_reg_directorio* directorio= malloc(sizeof(t_reg_directorio));
			strcpy(directorio->directorio, dir);
			directorio->padre = padre;
			padre = list_add(listaDirs, directorio);
		}else
		{
			padre = index;
		}
		i++;
		dir = dirs[i];
	}
	return 0;
}


int importar(char* argumentos){
	printf("Importar archivo\n");

	char** tmp;
	tmp = string_split(argumentos, " ");
	//tmp[0]: ruta del archivo local
	//tmp[1]: ruta del archivo en MDFS

	if( access( tmp[0], F_OK ) == -1 )
	{
	    printf("El archivo %s no existe. \n");
	    return 1;
	}

	//Habria que chequear que el archivo no exista ya en el FS



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

	free(tmp);
	return 0;
}

int getDir(char* dir,int16_t padre)
{
	int ret = -1;
	int length = list_size(listaDirs);
	for (int i=0;i<length;i++)
	{
		t_reg_directorio* dirListado = list_get(listaDirs,i);

		if ((strcmp(dirListado->directorio,dir) == 0) &&
				(dirListado->padre == padre) ){
			ret = i;
			break;
		}

	}
	return ret;
}


