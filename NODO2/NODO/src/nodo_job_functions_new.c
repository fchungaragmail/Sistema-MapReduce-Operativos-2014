/*
 * nodo_job_functions_new.c
 *
 *  Created on: 11/6/2015
 *      Author: utnso
 */


#include "nodo_job_functions_new.h"
#include "nodo_new.h"


int mapping(char *script, int numeroBloque, char *archivoTemporal1, char* archivoTemporal2) {

		int p[2];
		pipe(p);

		//script  (cat)
		if(fork()==0)
		{
			close(p[1]);//cierro el extremo de escritura pues solo lee
			close(0);
			dup(p[0]);

			close(1);
			creat(archivoTemporal1, 0777);
			execlp(script,script,NULL);
		}


		//escribir bloque en STDIN del script
		if(fork()==0)
		{
			close(p[0]);//cierro el extremo de lectura pues solo escribe
			close(1);
			dup(p[1]);

			char *bloque = getBloque(numeroBloque);
			write(1, bloque, TAMANIO_BLOQUE);
		}

		//aplico sort
		if(fork()==0)
		{
			close(0);
			open(archivoTemporal1, O_RDWR);

			close(1);
			creat(archivoTemporal2, 0777);

			char *bloque = getBloque(numeroBloque);
			execlp("sort","sort",NULL);
		}

		return 0;

}



int reduce(char *script, t_list* listArchivos, char *archivoTemporal) {

	int p[2];
	pipe(p);

	//script  (cat)
	if(fork()==0) {
		close(p[1]);//cierro el extremo de escritura pues solo lee
		close(0);
		dup(p[0]);

		close(1);
		creat(archivoTemporal, 0777);
		execlp(script, script, NULL);
	}


	//escribir bloque en STDIN del script
	if(fork()==0) {
		close(p[0]);//cierro el extremo de lectura pues solo escribe
		close(1);
		dup(p[1]);

		write(1, listArchivos, TAMANIO_BLOQUE);
	}

	return 0;
}
