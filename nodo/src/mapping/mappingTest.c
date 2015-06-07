/*
 * mappingTest.c
 *
 *  Created on: 5/6/2015
 *      Author: daniel
 */
#include "mapping.c"
#include <stdlib.h>
/*
 * 1:nombre script
 * 2:numero bloque
 * 3:espacio de datos
 * 4:archivo temporal 1
 * 5:archivo temporal 2 definitivo
 */
int main(int argc, char **argv) {
	mapping(argv[1], atoi(argv[2]), argv[3], argv[4], argv[5]);
}


