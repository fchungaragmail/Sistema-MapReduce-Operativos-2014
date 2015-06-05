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
#include "../getBloque/getBloque.c"

/*Ejecuta el script sobre el contenido del numeroBloque del espacioDatos, el resultado se almacena en archivoTemporal1
 * Luego ejecuta sort sobre archivoTemporal1 y lo almacenta en archivoTemporal2
 * */
int mapping(char *script, int numeroBloque, char* espacioDatos, char *archivoTemporal1, char* archivoTemporal2) {

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

			char *bloque = getBloque(espacioDatos, numeroBloque);
			write(1, bloque, TAMANIO_BLOQUE);
		}

		//aplico sort
		if(fork()==0)
		{
			close(0);
			open(archivoTemporal1, O_RDWR);

			close(1);
			creat(archivoTemporal2, 0777);

			char *bloque = getBloque(espacioDatos, numeroBloque);
			execlp("sort","sort",NULL);
		}

		return 0;

}


