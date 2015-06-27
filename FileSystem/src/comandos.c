/*
 * comandos.c
 *
 *  Created on: 14/5/2015
 *      Author: utnso
 */

#include "comandos.h"
#define MARTA "MaRTA"

void initComandos();
void procesarComando(char** comando, void(*doComando)(void*));
void procesarComandoRemoto(mensaje_t* mensaje, Conexion_t* conexion);
int mover(char* argumentos);
int borrar(char* argumentos);
int crearDir(char* argumentos);
int importar(char* argumentos);
int exportar(char* argumentos);
int md5(char* argumentos);
int bloques(char* argumentos);
int borrarBloque(char* argumentos);
int copiarBloque(char* argumentos);
int agregarNodo(char* argumentos);
int quitarNodo(char* argumentos);
int espacioTotal();
int nomb(char* argumentos, Conexion_t* conexion);
int dataFile(char* argumentos, Conexion_t* conexion);

int enviarBloque(enviarBloque_t* envio);


int16_t getDir(char* dir,int16_t padre);
int32_t get_file_size(const char * file_name);
int getNombreArchivo(char* ruta,char* nombre,int16_t *indexPadre);
int getArchivo(char* nombre,int16_t indexPadre, t_reg_archivo** archivo);
int getBloqueDisponible(Conexion_t* conexion);
int getCantidadBloquesDisponibles(Conexion_t* conexion);
int elegirNodos(int bloques, t_list* ubicaciones);
bool tieneMasEspacio(Conexion_t* nodo1,Conexion_t* nodo2);
bool esNodo(Conexion_t* conexion);
pthread_mutex_t mListaArchivos;
int nodosOnline;
pthread_mutex_t mNodosOnline;
pthread_mutex_t mElegirNodos;
FILE* logFile;
t_dictionary* comandosRemotos;


typedef struct {
	Conexion_t* nodo;
	int bloqueN;
} ubicacion_bloque;


void initComandos()
{
	nodosOnline = 0;
	pthread_mutex_init(&mElegirNodos, NULL);
	pthread_mutex_init(&mNodosOnline, NULL);
	comandosRemotos = dictionary_create();
	dictionary_put(comandosRemotos,"nombre",1);
	dictionary_put(comandosRemotos,"dataFile",2);
}


void procesarComando(char** comando, void(*doComando)(void*))
{
	pthread_t tDoComando;
	pthread_create(&tDoComando, NULL, (*doComando), comando[1]);
}


void procesarComandoRemoto(mensaje_t* mensaje, Conexion_t* conexion)
{
	bool dummy;
	if (procesarEntrada(mensaje->comando,&dummy) != 0)
	{
		char** comando = string_n_split(mensaje->comando,2," ");

		int eleccion = 0;
		eleccion = dictionary_get(comandosRemotos,comando[0]);
		switch (eleccion)
		{
		case 1:
		{
			//Identificar conexion
			nomb(comando[1],conexion);
			break;
		}
		case 2:
		{
			//Pedido de la tabla de bloques de un archivo
			dataFile(comando[1],conexion);
			break;
		}
		default:
			break;
		}

	}
}

int mover(char* argumentos){
	printf("Mover\n");
	return 0;
}


int borrar(char* argumentos){
	printf("Borrar\n");
	return 0;
}


int crearDir(char* argumentos){
	char** dirs = string_split(argumentos,"/");

	char* dir;
	int16_t padre = 0;
	int i = 0;
	dir = dirs[i];

	while (dir != NULL)
	{
		int index = getDir(dir,padre);
		if (index<0)
		{
			t_reg_directorio* directorio= malloc(sizeof(t_reg_directorio));
			strcpy(directorio->directorio, dir);
			directorio->padre = padre;
			padre = list_add(listaDirs, directorio);
			log_debug(logFile, "Directorio agregado: %s "
					"indice: %d padre: %d", dir, padre,directorio->padre);
		}else
		{
			padre = index;
		}
		i++;
		dir = dirs[i];
	}
	return 0;
}


int importar(char* argumentos){
	if (strcmp(argumentos,"/?") == 0 )
	{
		printf("import rutaDelArchivoLocal rutaDelArchivoEnMDFS\n");
		return 0;
	}

	char** tmp;
	tmp = string_split(argumentos, " ");
	//tmp[0]: ruta del archivo local
	//tmp[1]: ruta del archivo en MDFS

	if( access( tmp[0], F_OK ) == -1 )
	{
	    printf("El archivo %s no existe. \n", tmp[0]);
	    return 1;
	}

	int32_t tamanio = get_file_size(tmp[0]);

	//Busco que exista el dir en el FS
	int16_t indexPadre;
	char nombre[50];

	if (getNombreArchivo(tmp[1],nombre,&indexPadre) != EXIT_SUCCESS)
		return -1;

	int archivoDisk = open(tmp[0],O_RDONLY);
	void* archivoMap = mmap(NULL, tamanio, PROT_READ, MAP_SHARED, archivoDisk, 0);
	close(archivoDisk);

	int bloques = (tamanio / TAMANIO_BLOQUE) + 1;
	t_list* ubicacionesElegidas = list_create();
	if (elegirNodos(bloques, ubicacionesElegidas) != EXIT_SUCCESS)
	{
		return -1;
	}

	t_list* listaBloques = list_create();
	t_list* listaThreads = list_create();
	int32_t bytesEnviados = 0;
	while (bytesEnviados < tamanio)
	{
		for (int j = 0;j<bloques;j++)
		{
			t_list* ubicaciones = list_create();
			t_list* bloque = list_get(ubicacionesElegidas,j);
			//Asi lo copia en todos lados -> Diseniar un selector de nodo
			for (int i=0;i<bloque->elements_count;i++)
			{
				ubicacion_bloque* ubicacionElegida = list_get(bloque,i);
				Conexion_t* nodo = ubicacionElegida->nodo;

				enviarBloque_t* envio = malloc(sizeof(enviarBloque_t));

				envio->bloque = ubicacionElegida->bloqueN;

				t_ubicacion_bloque* ubicacion = malloc(sizeof(t_ubicacion_bloque));
				ubicacion->bloque = envio->bloque;
				strcpy(ubicacion->nodo,nodo->nombre);
				list_add(ubicaciones,ubicacion);

				envio->conexion = nodo;
				envio->archivoMap = archivoMap;
				envio->offset = bytesEnviados;
				envio->archivoSize = tamanio;

				pthread_t tEnvio;
				list_add(listaThreads, &(tEnvio));
				pthread_create(&tEnvio, NULL, enviarBloque, envio);
			}
			list_add_in_index(listaBloques,j,ubicaciones);
			bytesEnviados += TAMANIO_BLOQUE;
		}
	}

	for (int i=0;i<listaThreads->elements_count;i++)
	{
		pthread_join(*(pthread_t*)(list_get(listaThreads,i)),NULL);
	}

	t_reg_archivo* archivo = malloc(sizeof(t_reg_archivo));
	archivo->dirPadre = indexPadre;
	strcpy(archivo->nombre,nombre);
	archivo->tamanio = tamanio;
	pthread_mutex_t mBloques;
	pthread_mutex_init(&mBloques, NULL);
	archivo->mBloques = mBloques;
	archivo->bloques = listaBloques;
	archivo->estado = DISPONIBLE;

	pthread_mutex_lock(&mListaArchivos);
	list_add(listaArchivos,archivo);
	pthread_mutex_unlock(&mListaArchivos);
	log_debug(logFile, "Archivo agregado al FS:\n"
			"Nombre: %s\n"
			"Tamanio: %d\n"
			"DirPadre: %d\n"
			"Cantidad de Bloques: %d\n",
			archivo->nombre,archivo->tamanio,archivo->dirPadre,archivo->bloques->elements_count);
	return EXIT_SUCCESS;
}


int exportar(char* argumentos){
	printf("Exportar archivo\n");
	return 0;
}


int md5(char* argumentos){
	printf("md5 de archivo\n");
	return 0;
}


int bloques(char* argumentos){
	if (strcmp(argumentos,"/?") == 0 )
	{
		printf("showb rutaDelArchivoEnMDFS\n");
		return 0;
	}

	char nombre[50];
	int16_t indexPadre;
	t_reg_archivo* archivo;

	if (getNombreArchivo(argumentos,nombre,&indexPadre) != EXIT_SUCCESS)
	{
		printf("Archivo no encontrado\n");
		return -1;
	} else
	{
		if (getArchivo(nombre,indexPadre, &archivo) != EXIT_SUCCESS)
		{
			printf("Archivo no encontrado\n");
			return -1;
		}
	}


	for (int i=0;i<archivo->bloques->elements_count;i++)
	{
		char* bloques = string_new();
		string_append_with_format(&bloques,"Bloque: %d", i);
		t_list* bloque = list_get(archivo->bloques, i);
		for (int j=0;j<bloque->elements_count;j++)
		{
			t_ubicacion_bloque* ubicacion = list_get(bloque,j);
			string_append_with_format(&bloques,"| Nodo: %s - Bloque: %d ",ubicacion->nodo,
					ubicacion->bloque);
		}
		string_append(&bloques,"\n");
		printf(bloques);
		free(bloques);
	}
	return 0;
}


int borrarBloque(char* argumentos){
	printf("Borrar un bloque de un archivo\n");
	return 0;
}


int copiarBloque(char* argumentos){
	printf("Copiar un bloque\n");
	return 0;
}


int agregarNodo(char* argumentos){
	printf("Agregar nodo\n");
	return 0;
}


int quitarNodo(char* argumentos){
	printf("Quitar nodo\n");
	return 0;
}

int nomb(char* argumentos, Conexion_t* conexion)
{
	if (strcmp(conexion->nombre,argumentos) == 0)
	{
		//El nodo ya existia
		log_info(logFile, "El nodo %s ya estaba identificado", conexion->nombre);
		return 0;
	}

	strcpy(conexion->nombre, argumentos);
	log_info(logFile, "Identificado el nodo %s", conexion->nombre);
	if (strcmp(conexion->nombre, "MaRTA") != 0)
	{
		pthread_mutex_lock(&mNodosOnline);
		nodosOnline++;
		pthread_mutex_unlock(&mNodosOnline);
		if (nodosOnline == LISTA_NODOS)
			log_info(logFile, "Cantidad minima de nodos (%d) alcanzada.", LISTA_NODOS);
	}
	return 0;
}


int dataFile(char* argumentos, Conexion_t* conexion)
{
	char nombre[50];
	int16_t indexPadre = 0;
	if (getNombreArchivo(argumentos,nombre,&indexPadre) != EXIT_SUCCESS)
	{
		return -1;
	}
	t_reg_archivo* archivo = malloc(sizeof(t_reg_archivo));
	if (getArchivo(nombre,indexPadre,&archivo) != EXIT_SUCCESS)
	{
		return -1;
	}

	if (archivo->estado == NO_DISPONIBLE)
	{
		mensaje_t* respuesta = malloc(sizeof(mensaje_t));
		respuesta->comando = string_new();
		string_append_with_format(&(respuesta->comando),
				"DataFileResponse %s noDisponible", argumentos);
		respuesta->comandoSize = strlen(respuesta->comando) + 1;
		memcpy(respuesta->data,NULL,0);
		respuesta->dataSize = 0;

		pthread_mutex_lock(&(conexion->mSocket));
		enviar(conexion->sockfd,respuesta);
		pthread_mutex_unlock(&(conexion->mSocket));

		log_info(logFile,"El archivo %s no se encuentra disponible",
				argumentos);
		return -1;
	}

	char* tabla = string_new();
	for(int i=0;i<archivo->bloques->elements_count;i++)
	{
		string_append_with_format(&tabla,"%d;",i);

		t_list* bloque = list_get(archivo->bloques,i);
		for(int j=0;j<bloque->elements_count;j++)
		{
			t_ubicacion_bloque* ubicacion = list_get(bloque,j);
			string_append_with_format(&tabla,"%s;%d;",ubicacion->nodo,ubicacion->bloque);
		}
		string_append(&tabla,"\n");
	}

	mensaje_t* respuesta = malloc(sizeof(mensaje_t));
	respuesta->comando = string_new();
	string_append_with_format(&(respuesta->comando),
			"DataFileResponse %s disponible", argumentos);
	respuesta->comandoSize = strlen(respuesta->comando) + 1;
	respuesta->data = tabla;
	respuesta->dataSize = strlen(respuesta->data) + 1;

	pthread_mutex_lock(&(conexion->mSocket));
	enviar(conexion->sockfd,respuesta);
	pthread_mutex_unlock(&(conexion->mSocket));

	return EXIT_SUCCESS;
}



int16_t getDir(char* dir,int16_t padre)
{
	int16_t ret = -1;
	for (int i=0;i<listaDirs->elements_count;i++)
	{
		t_reg_directorio* dirListado = list_get(listaDirs,i);
		if ((strcmp(dirListado->directorio,dir) == 0) &&
				(dirListado->padre == padre) ){
			ret = i;
			break;
		}

	}
	return ret;
}


int enviarBloque(enviarBloque_t* envio)
{
	//crear el mensaje
	mensaje_t* mensaje = malloc(sizeof(mensaje_t));
	mensaje->comando = string_new();
	string_append(&(mensaje->comando), "setBloque ");
	string_append(&(mensaje->comando), string_itoa(envio->bloque));
	mensaje->comandoSize = strlen(mensaje->comando) + 1;
	mensaje->data = envio->archivoMap + envio->offset;//Desde donde se envia

	//Calculo hasta donde tengo que enviar
	div_t result = div(envio->archivoSize-envio->offset,TAMANIO_BLOQUE);
	if ((result.quot == 0) && (result.rem < TAMANIO_BLOQUE))
	{
		mensaje->dataSize = result.rem;
	}else
	{
		mensaje->dataSize = TAMANIO_BLOQUE;
	}

	pthread_mutex_lock(&(envio->conexion->mSocket));
	enviar(envio->conexion->sockfd,mensaje);
	pthread_mutex_unlock(&(envio->conexion->mSocket));
	return 1;
}

int32_t get_file_size(const char * file_name)
{
    struct stat sb;
    stat (file_name, & sb);
    return sb.st_size;
}

int getNombreArchivo(char* ruta,char* nombre,int16_t* indexPadre)
{
	*indexPadre = 0;
	char nombrePadre[256];
	strcpy(nombrePadre,"");
	int i = 1;

	char** directorios = string_split(ruta, "/");
	strcpy(nombre,directorios[0]);

	while (directorios[i] != NULL)
	{
		strcpy(nombrePadre,directorios[i-1]);
		*indexPadre = getDir(nombrePadre,*indexPadre);
		if (*indexPadre < 0 )
		{
			log_error(logFile, "Directorio no encontrado %s", nombrePadre);
			return -1;
		}
		strcpy(nombre,directorios[i]);
		i++;
	}
	return EXIT_SUCCESS;
}

int getArchivo(char* nombre,int16_t indexPadre, t_reg_archivo** archivo)
{
	for (int i=0;i<listaArchivos->elements_count;i++)
	{
		t_reg_archivo* archivoTmp = list_get(listaArchivos, i);
		if ((strcmp(archivoTmp->nombre,nombre) == 0)
			&& (archivoTmp->dirPadre == indexPadre))
		{
			*archivo = archivoTmp;
			return EXIT_SUCCESS;
		}
	}
	return -1;
}

int getBloqueDisponible(Conexion_t* conexion)
{
	for (int i=0;i<BLOQUES_NODO;i++)
	{
		if (conexion->estadoBloques[i] == false)
			return i;
	}
	return -1;
}

void revertirAsignaciones(t_list* ubicaciones)
{
	for (int i=0;i<ubicaciones->elements_count;i++)
	{
		t_list* bloque = list_get(ubicaciones,i);
		for (int j=0;j<bloque->elements_count;j++)
		{
			ubicacion_bloque* ubicacion = list_get(bloque,j);

			pthread_mutex_lock(&(ubicacion->nodo->mSocket));
			ubicacion->nodo->estadoBloques[ubicacion->bloqueN] = false;
			//devuelvo a NO EN USO a los bloques seleccionados
			pthread_mutex_unlock(&(ubicacion->nodo->mSocket));
			free(ubicacion);
		}
		list_destroy(bloque);
	}
}


int elegirNodos(int bloques, t_list* ubicaciones)
{
	pthread_mutex_lock(&mElegirNodos);

	for (int i=0;i<bloques;i++)
	{
		pthread_mutex_lock(&mConexiones);
		t_list* nodos = list_filter(conexiones, esNodo);
		pthread_mutex_unlock(&mConexiones);
		if (nodos->elements_count < LISTA_NODOS)
		{
			revertirAsignaciones(ubicaciones);
			return-1; //No hay suficientes nodos
		}

		list_sort(nodos,tieneMasEspacio);

		t_list* bloque = list_create();
		list_add_in_index(ubicaciones, i, bloque);

		for (int j=0;j<LISTA_NODOS;j++)
		{
			Conexion_t* nodo = list_get(nodos,j);
			int bloqueDisponible = getBloqueDisponible(nodo);
			if (bloqueDisponible == -1)
			{
				revertirAsignaciones(ubicaciones);
				return -1;//De los tres primeros nodos, alguno no tiene bloques disponibles
			}

			pthread_mutex_lock(&(nodo->mEstadoBloques));
			nodo->estadoBloques[bloqueDisponible] = true; //Lo marco en uso
			pthread_mutex_unlock(&(nodo->mEstadoBloques));

			ubicacion_bloque* ubicacion = malloc(sizeof(ubicacion_bloque));
			ubicacion->bloqueN = bloqueDisponible;
			ubicacion->nodo = nodo;

			list_add(bloque,ubicacion);
		}
	}
	pthread_mutex_unlock(&mElegirNodos);
	return EXIT_SUCCESS;
}


int getCantidadBloquesDisponibles(Conexion_t* conexion)
{
	int bloquesDisponibles = 0;
	for (int i=0;i<BLOQUES_NODO;i++)
	{
		if (conexion->estadoBloques[i] == false)
			bloquesDisponibles++;
	}
	return bloquesDisponibles;
}

bool tieneMasEspacio(Conexion_t* nodo1,Conexion_t* nodo2)
{
	if (getCantidadBloquesDisponibles(nodo1) >= getCantidadBloquesDisponibles(nodo2))
		return true;
	return false;
}

bool esNodo(Conexion_t* conexion)
{
	if (strcmp(conexion->nombre,"MaRTA") == 0)
		return false;
	return true;
}

int espacioTotal()
{
	float espacioTotal = 0;
	float espacioDisponible = 0;

	pthread_mutex_lock(&mConexiones);
	t_list* nodos = list_filter(conexiones, esNodo);
	pthread_mutex_unlock(&mConexiones);

	for (int i=0;i<nodos->elements_count;i++)
	{
		Conexion_t* nodo = list_get(nodos,i);

		espacioTotal += BLOQUES_NODO * TAMANIO_BLOQUE/1024/1024;
		pthread_mutex_lock(&(nodo->mEstadoBloques));
		espacioDisponible = espacioDisponible + (getCantidadBloquesDisponibles(nodo) * TAMANIO_BLOQUE/1024/1024);
		pthread_mutex_unlock(&(nodo->mEstadoBloques));
	}

	list_destroy(nodos);

	espacioTotal = (espacioTotal/1024); //Lo paso a GB
	espacioDisponible = (espacioDisponible/1024); //Lo paso a GB

	printf(	"Espacio Total : %.2f GB.\n"
			"Espacio disponible: %.2f GB.\n",
			espacioTotal,
			espacioDisponible);
	return EXIT_SUCCESS;
}
