/*
 * getFileContentTest.c
 *
 *  Created on: 5/6/2015
 *      Author: daniel
 */
#include "getFileContent.c"
/*
  * 1:nombre del archivo temporal
 */
int main(int argc, char **argv) {
	t_fileContent *fileContent = getFileContent(argv[1]);
	write(1, fileContent->contenido, fileContent->size);
}


