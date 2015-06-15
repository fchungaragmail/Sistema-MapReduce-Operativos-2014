//#include "getBloque.c"
#include "setBloque.c"
#include "getFileContent.c"
#include "mapping.c"

char* obtenerBloque(){
	int archivoDatos = open("201301hourly.txt", O_RDONLY);
	char *bloque = malloc(TAMANIO_BLOQUE);
	read(archivoDatos, bloque, TAMANIO_BLOQUE);
	return bloque;
}

int main(int argc, char **argv) {

	//getBloque
	char *bloque = getBloque("datos.bin", 0);
	//printf("%c\n", bloque[0]);

	//setBloque
	setBloque("datos.bin",0, obtenerBloque());

	//getFileContent
	t_fileContent *archivoTemporal = getFileContent("tmp/201301hourly.txt");
	//printf("tamaño archivo: %ld, %s", archivoTemporal->size, archivoTemporal->contenido);

	//mapping
	mapping("./mapper.sh", 0, "datos.bin", "tmp/archMapeado", "tmp/archOrdenado");

	//reducing


}

