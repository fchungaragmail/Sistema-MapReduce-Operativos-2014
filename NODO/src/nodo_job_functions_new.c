/*
 * nodo_job_functions_new.c
 *
 *  Created on: 11/6/2015
 *      Author: utnso
 */


#include "nodo_job_functions_new.h"
#include "nodo_new.h"

#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <sys/wait.h>


/*Ejecuta el script sobre el contenido del numeroBloque del espacioDatos, el resultado se almacena en archivoTemporal1
 * Luego ejecuta sort sobre archivoTemporal1 y lo almacenta en archivoTemporal2
 * */
int mapping(char *script, int numeroBloque, char* archivoTemporal) {

		int p[2];
		pipe(p);
		char* archivoTemporal1 = "Temporal.txt";

		//para escrbir el bloque en la tuberia
		if(fork()==0)
		{
			close(p[0]);
			char *bloque = getBloque(numeroBloque);
			write(p[1], bloque, TAMANIO_BLOQUE);
			exit (0);
		}

		//para aplicar el script
		if(fork()==0)
		{
			close(p[1]);

			//cambio la entrada standar por la tuberia
			close(0);
			dup(p[0]);

			//cambio la salida standar
			close(1);
			creat(archivoTemporal1, 0777);

			system(script);

		}

		wait(0);
		//para aplicar sort
		if(fork() == 0){
			close(0);
			open(archivoTemporal1, O_RDONLY);

			close(1);
			creat(archivoTemporal, 0777);

			system("sort");
		}

		return 0;

}



int reducing(char *script, char *listaArchivos, char* archivoTemporal2) {

		int p[2];
		pipe(p);

		//para escrbir el bloque en la tuberia
		if(fork()==0)
		{
//			close(p[0]);
//			char** result = string_split(listaArchivos, " ");
//			//conseguir los archivos
//			char* path = aparearArchivos(listaConseguida);
//			t_fileContent  *archivoTemporal = getFileContent(path);
//			write(p[1], archivoTemporal->contenido, archivoTemporal->size);
//			exit (EXIT_SUCCESS);
		}

		wait(0);
		//para aplicar el script
		if(fork()==0)
		{
			close(p[1]);

			//cambio la entrada standar por la tuberia
			close(0);
			dup(p[0]);

			//cambio la salida standar
			close(1);
			creat(archivoTemporal2, 0777);

			system(script);

		}

		return 0;

}
