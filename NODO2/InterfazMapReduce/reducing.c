/*
 * mapping.c
 *
 *  Created on: 4/6/2015
 *      Author: daniel
 */

#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include "getFileContent.c"
#include <sys/wait.h>


//aplica el script sobre archivoTemporal1 y lo guarda en archivoTemporal2
int reducing(char *script, char *archivoTemporal1, char* archivoTemporal2) {

		int p[2];
		pipe(p);

		//para escrbir el bloque en la tuberia
		if(fork()==0)
		{
			close(p[0]);
			t_fileContent  *archivoTemporal = getFileContent(archivoTemporal1);
			write(p[1], archivoTemporal->contenido, archivoTemporal->size);
			exit (EXIT_SUCCESS);
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


