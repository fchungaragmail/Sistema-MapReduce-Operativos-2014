/*
 ============================================================================
 Name        : Ejercicio_03-Commons.c
 Author      : 
 Version     :
 Copyright   : Your copyright notice
 Description : Hello World in C, Ansi-style
 ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <commons/string.h>
#include <commons/txt.h>
#include <commons/collections/list.h>
#include "tipos.h"
#include <commons/log.h>

const int LINE_SIZE =200;

void list(char*, t_list*);
bool isGreater(t_persona*,t_persona*);
void saveToFile(t_list*, FILE*);
bool isMajor(t_persona*);
t_log* logFile;


int main(void) {
	FILE* personasIN;
	FILE* personasOUT;
	char* line;
	t_list* personasList = list_create();


	personasIN = fopen("/home/utnso/workspace/tp-2015-1c-the-byteless/Ejercicios/Juan/Ejercicio_03-Commons/Personas.txt","r");
    personasOUT = fopen("/home/utnso/workspace/tp-2015-1c-the-byteless/Ejercicios/Juan/Ejercicio_03-Commons/PersonasNuevo.txt","w");
	logFile = log_create("/home/utnso/workspace/tp-2015-1c-the-byteless/Ejercicios/Juan/Ejercicio_03-Commons/log.txt",
						"Ejercicio 3",true,LOG_LEVEL_INFO);
    if (personasIN == NULL)
    {
        printf("No se pudo abrir el archivo");
    	exit(EXIT_FAILURE);
    }

	line = malloc(LINE_SIZE);


	while (fgets(line,LINE_SIZE,personasIN)!=NULL)
	{
		list(line, personasList);
	}

	list_sort(personasList, (isGreater));

	personasList = list_filter(personasList, (isMajor));

	saveToFile(personasList, personasOUT);

	free(line);
	fclose(personasIN);
	fclose(personasOUT);
	log_destroy(logFile);
	list_destroy(personasList);
	return EXIT_SUCCESS;
}

void list(char* line,t_list* lista)
{
	int credito;
	t_persona* persona = persona = malloc(sizeof(t_persona));
	char** array;

	array = string_split(line,";");
	strcpy(persona->region,array[0]);
	strcpy(persona->NomAp, array[1]);
	sscanf(array[2],"%d",&(persona->edad));
	sscanf(array[3],"%d",&(persona->Tel));
	sscanf(array[4],"%d",&(persona->DNI));
	sscanf(array[5],"%d",&credito);

	if (credito < 100)
	{
		log_info(logFile,string_substring_until(line,(strlen(line)-1)));
	}

	free(array);

	list_add(lista,persona);
}

bool isGreater(t_persona* persona1,t_persona* persona2)
{
	if (persona1->region == persona2->region)
	{
		if (persona1->edad >= persona2->edad)
		{
			return true;
		}else
		{
			return false;
		}
	}else
	{
		if (persona1->region > persona2->region)
		{
			return true;
		}else
		{
			return false;
		}

	}
}

void saveToFile(t_list* personasList,FILE* personasOUT)
{
	int i = 0;
	for(i=0;i<personasList->elements_count;i++)
	{
		fprintf(personasOUT,"%s|%d|%d|%s|%d\n",
				(((t_persona*)(list_get(personasList,i)))->region),
				(((t_persona*)(list_get(personasList,i)))->edad),
				(((t_persona*)(list_get(personasList,i)))->DNI),
				(((t_persona*)(list_get(personasList,i)))->NomAp),
				(((t_persona*)(list_get(personasList,i)))->Tel));
	}
}

bool isMajor(t_persona* persona)
{
	return (persona->edad >= 18);
}
