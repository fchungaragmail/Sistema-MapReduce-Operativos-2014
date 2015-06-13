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
		int f = open ("mi_fichero", O_CREAT|O_WRONLY,0644);
		close(f);

		//para escrbir el bloque en la tuberia
		if(fork()==0)
		{
			close(p[0]);
			char *bloque = getBloque(espacioDatos, numeroBloque);
			write(p[1], bloque, TAMANIO_BLOQUE);
			exit (0);
		}

		//para aplicar el script
		if(fork()==0)
		{
			close(p[1]);
			char *b = malloc(100);
			read(p[0], b, 100);
			close(1);
			creat("temporal", 0777);
			printf("%s", b);
			/*close(p[1]);

			//cambio la entrada standar por la tuberia
			close(0);
			dup(p[0]);

			//cambio la salida standar
			close(1);
			creat(archivoTemporal1, 0777);

			system("./mapper.sh");
			*/
			exit(0);
		}
/*
		//sleep(5);
		//para aplicar sort
		if(fork() == 0){
			close(0);
			open(archivoTemporal1, O_RDONLY);

			close(1);
			creat(archivoTemporal2, 0777);

			system("sort");
			exit(0);
		}
*/
		sleep(234);
		return 0;

}


