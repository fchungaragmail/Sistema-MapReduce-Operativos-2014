/*
 * ejercicio3.c
 *
 *  Created on: 4/5/2015
 *      Author: utnso
 */
#include <stdio.h>
#include <stdlib.h>
#include <commons/string.h>
#include <commons/collections/list.h>
#include "estructura.h"


//Headers
void prepareToList(char*,t_list*);
void guardarEnFile(t_list*, t_list*, FILE**);
bool comparator(t_persona*, t_persona*);
bool menorEdad(t_persona*);
void closeFiles(FILE*, FILE*);
void destroyLists(t_list*, t_list*);


//Main
int main() {

	FILE* personas;
	FILE* personasOUT;
	char* line;
	t_list* listPersonas;
	t_list* listMenores;


	//Creo lista de personas
	listPersonas = list_create();
	listMenores = list_create();

	//Abro archivos
	personas = fopen("/home/utnso/Escritorio/gitRepo/tp-2015-1c-the-byteless/Ejercicios/Rodrigo/ejercicio3/Personas.txt", "r");
	if (personas == NULL) {
		printf("Error al abrir archivo");
		exit(EXIT_FAILURE);
	}
	personasOUT = fopen("/home/utnso/Escritorio/gitRepo/tp-2015-1c-the-byteless/Ejercicios/Rodrigo/ejercicio3/PersonasNEW.txt", "w");
	//aca va log

	line = malloc(150);
	while(fgets(line, 150, personas) != NULL) {
		prepareToList(line, listPersonas);
	}

	list_sort(listPersonas, (comparator));
	listMenores = list_filter(listPersonas, (menorEdad));

	guardarEnFile(listPersonas, listMenores, personasOUT);

	closeFiles(personas, personasOUT);
	destroyLists(listPersonas, listMenores);
	free(line);


	return EXIT_SUCCESS;

}


// Functions

void prepareToList(char* line, t_list* list) {

	char** array;
	t_persona* persona = malloc(sizeof(t_persona));
	array = string_split(line, ";");

	strcpy(persona->region, array[0]);
	strcpy(persona->apeNom, array[1]);
	sscanf(array[2],"%d", &(persona->edad));
	sscanf(array[3], "%d", &(persona->DNI));
	sscanf(array[4], "%d", &(persona->numTel));
	sscanf(array[5], "%d", &(persona->credito));

	//falta hacer log

	list_add(list, persona);

	free(array);

}

bool comparator(t_persona* persona1, t_persona* persona2) {

	if (persona1->region == persona2->region) {
		return persona1->edad >= persona2->edad;
	}
	else {
		return persona1->region >= persona2->region;
	}

}

bool menorEdad(t_persona* persona) {

	return persona->edad < 18;
}


void guardarEnFile(t_list* list, t_list* listMenores, FILE** fileOUT) {

	int i = 0;
	while (list != NULL) {

		fprintf(fileOUT, "%s | %s | %d | %d | %d | %d \n",
				((t_persona*)list_get(list,i))->region,
				((t_persona*)list_get(list,i))->apeNom,
				((t_persona*)list_get(list,i))->edad,
				((t_persona*)list_get(list,i))->DNI,
				((t_persona*)list_get(list,i))->numTel,
				((t_persona*)list_get(list,i))->credito);
	}

	while (listMenores != NULL) {

		fprintf(fileOUT, "Personas Menores de Edad: \n");
		fprintf(fileOUT, "%s\n%d",
				((t_persona*)list_get(listMenores,i))->apeNom,
				((t_persona*)list_get(listMenores,i))->edad);
	}

}

void closeFiles(FILE* a, FILE* b) {

	fclose(a);
	fclose(b);
}

void destroyLists(t_list* a, t_list* b) {

	list_destroy(a);
	list_destroy(b);
}

