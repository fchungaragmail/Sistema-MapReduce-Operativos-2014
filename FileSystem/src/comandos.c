/*
 * comandos.c
 *
 *  Created on: 14/5/2015
 *      Author: utnso
 */

#include "comandos.h"

void initComandos();
void procesarComando(char** comando, void(*doComando)(void*));
void procesarComandoRemoto(argumentos_t* args);
int format(char* argumentos);
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

void actualizarEstadoArchivos();
int16_t getDir(char* dir,int16_t padre);
int32_t get_file_size(const char * file_name);
int getNombreArchivo(char* ruta,char* nombre,int16_t *indexPadre);
int getArchivo(char* nombre,int16_t indexPadre, t_reg_archivo** archivo);
int getBloqueDisponible(Conexion_t* conexion);
int getCantidadBloquesDisponibles(Conexion_t* conexion);
int elegirNodos(int bloques, t_list* ubicaciones);
bool tieneMasEspacio(Conexion_t* nodo1,Conexion_t* nodo2);
bool esNodo(Conexion_t* conexion);
bool estaDisponible(Conexion_t* conexion);
void formatNodo(Conexion_t* nodo);
pthread_mutex_t mListaArchivos;
pthread_mutex_t mListaDirs;
int nodosOnline;
pthread_mutex_t mNodosOnline;
pthread_mutex_t mElegirNodos;
FILE* logFile;
t_dictionary* comandosRemotos;


void initComandos()
{
	nodosOnline = 0;
	pthread_mutex_init(&mElegirNodos, NULL);
	pthread_mutex_init(&mNodosOnline, NULL);
	comandosRemotos = dictionary_create();
	dictionary_put(comandosRemotos,"nombre",1);
	dictionary_put(comandosRemotos,"DataFile",2);
	dictionary_put(comandosRemotos,"respuesta",3);
	dictionary_put(comandosRemotos,"archivoResultado",4);
}


void procesarComando(char** comando, void(*doComando)(void*))
{
	pthread_t tDoComando;
	pthread_create(&tDoComando, NULL, (*doComando), comando[1]);
	pthread_join(tDoComando, NULL);
	sem_post(&sPersistencia); //Persisto el resultado
}


void procesarComandoRemoto(argumentos_t* args)
{
	bool dummy;
	if (procesarEntrada(args->mensaje->comando,&dummy) != 0)
	{
		char** comando = string_n_split(args->mensaje->comando,2," ");

		int eleccion = 0;
		eleccion = dictionary_get(comandosRemotos,comando[0]);
		switch (eleccion)
		{
		case 1:
		{
			//Identificar conexion
			nomb(comando[1],args->conexion);
			//TODO: Destruir mensaje
			break;
		}
		case 2:
		{
			//Pedido de la tabla de bloques de un archivo
			dataFile(comando[1],args->conexion);
			//TODO: Destruir mensaje
			break;
		}
		case 3:
		{
			//Se recibe una respuesta de lo que sea
			sem_wait(&(args->conexion->respuestasR));
			args->conexion->respuestaSize = args->mensaje->dataSize;
			args->conexion->respuestaBuffer = malloc(args->mensaje->dataSize);
			memcpy(args->conexion->respuestaBuffer,
					args->mensaje->data,
					args->mensaje->dataSize);
			free(args->mensaje->data);
			free(args->mensaje->comando);
			free(args->mensaje);
			sem_post(&(args->conexion->respuestasP));
			//TODO: Destruir mensaje
			break;
		}
		case 4:
		{
			//
			dataFile(comando[1],args->conexion);
			break;
		}
		default:
			break;
		}

	}
	sem_post(&sPersistencia); //Persisto los cambios
}

int format(char* argumentos)
{
	pthread_mutex_lock(&mConexiones);
	t_list* nodos = list_filter(conexiones, esNodo);
	pthread_mutex_unlock(&mConexiones);

	for(int i=0;i<nodos->elements_count;i++)
	{
		Conexion_t* nodo = list_get(nodos,i);
		formatNodo(nodo);
	}

	pthread_mutex_lock(&mListaArchivos);
	for(int i=0;i<listaArchivos->elements_count;i++)
	{
		t_reg_archivo* archivo = list_get(listaArchivos,i);
		free(archivo);
	}
	list_clean(listaArchivos);
	pthread_mutex_unlock(&mListaArchivos);

	return EXIT_SUCCESS;
}

void formatNodo(Conexion_t* nodo)
{
	if (nodo->sockfd >= 0)
	{
		mensaje_t* mensaje = malloc(sizeof(mensaje_t));
		mensaje->comando = string_new();
		strcpy(mensaje->comando,"borrarBloque -1");
		mensaje->comandoSize = strlen(mensaje->comando) + 1;
		mensaje->dataSize = 0;

		pthread_mutex_lock(&(nodo->mSocket));
		enviar(nodo->sockfd,mensaje);
		pthread_mutex_unlock(&(nodo->mSocket));
	}

	pthread_mutex_lock(&(nodo->mEstadoBloques));
	for (int i=0;i<nodo->totalBloques;i++)
	{
		nodo->estadoBloques[i] = false;
	}
	pthread_mutex_unlock(&(nodo->mEstadoBloques));
}


int mover(char* argumentos){
	//POR AHORA SOLO MUEVE ARCHIVOS, FALTA DIRECTORIOS
	if (strcmp(argumentos,"/?") == 0 )
	{
		printf("mv origen destino\n");
		return 0;
	}

	char** rutas = string_split(argumentos," ");

	char nombre[50];
	int16_t indexPadre;
	t_reg_archivo* archivo;

	if (getNombreArchivo(rutas[0],nombre,&indexPadre) != EXIT_SUCCESS)
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

	char nombreDest[50];
	int16_t indexPadreDest;

	if (getNombreArchivo(rutas[1],nombreDest,&indexPadreDest) != EXIT_SUCCESS)
	{
		return -1;
	}

	strcpy(archivo->nombre,nombreDest);
	archivo->dirPadre = indexPadreDest;
	pthread_mutex_lock(&mLogFile);
	log_debug(logFile,	"Nombre del archivo: %s\n"
						"Directorio padre: %d",
						archivo->nombre,
						archivo->dirPadre);
	pthread_mutex_unlock(&mLogFile);
	return 0;
}


int borrar(char* argumentos){
	printf("Borrar\n");
	return 0;
}


int crearDir(char* argumentos){

	if (strcmp(argumentos,"/?") == 0 )
	{
		printf("mkdir directorioACrear\n");
		return 0;
	}

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
			pthread_mutex_lock(&mLogFile);
			log_debug(logFile, "Directorio agregado: %s "
					"indice: %d padre: %d", dir, padre,directorio->padre);
			pthread_mutex_unlock(&mLogFile);
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
		pthread_mutex_lock(&mLogFile);
		log_info(logFile, "El archivo %s no pudo ser importado al MDFS. "
				"Verifique que haya espacio disponible con el comando space", tmp[0]);
		pthread_mutex_unlock(&mLogFile);
		return -1;
	}

	t_list* listaBloques = list_create();
	t_list* listaThreads = list_create();
	int32_t bytesEnviados = 0;
	int sends = 0;
	while (bytesEnviados < tamanio)
	{
		for (int j = 0;j<bloques;j++)
		{
			t_list* ubicaciones = list_create();
			t_list* bloque = list_get(ubicacionesElegidas,j);
			//Asi lo copia en todos lados -> Diseniar un selector de nodo
			for (int i=0;i<bloque->elements_count;i++)
			{
				t_ubicacion_bloque* ubicacionElegida = list_get(bloque,i);
				Conexion_t* nodo = ubicacionElegida->nodo;

				enviarBloque_t* envio = malloc(sizeof(enviarBloque_t));
				envio->bloque = ubicacionElegida->bloque;
				envio->conexion = nodo;
				envio->archivoMap = archivoMap;
				envio->offset = bytesEnviados;
				envio->archivoSize = tamanio;

				list_add(ubicaciones,ubicacionElegida);

				pthread_t* tEnvio = malloc(sizeof(pthread_t));
				list_add(listaThreads, tEnvio);
				sends++;
				pthread_create(tEnvio, NULL, enviarBloque, envio);
			}
			list_add_in_index(listaBloques,j,ubicaciones);
			bytesEnviados += TAMANIO_BLOQUE;
		}
	}

	for (int i=0;i<listaThreads->elements_count;i++)
	{
		int ret = 0;
		pthread_t* tEnvio = list_get(listaThreads,i);
		pthread_join(*tEnvio,(void**)&ret);
		sends = sends - 1;
	}

	if (sends != 0)
	{
		pthread_mutex_lock(&mLogFile);
		log_info(logFile, "Error al importar el archivo. No se pudo enviar un bloque");
		pthread_mutex_unlock(&mLogFile);
		revertirAsignaciones(ubicacionesElegidas);
		return -1;
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
	pthread_mutex_lock(&mLogFile);
	log_debug(logFile, "Archivo agregado al FS:\n"
			"Nombre: %s\n"
			"Tamanio: %"PRId64"\n"
			"DirPadre: %d\n"
			"Cantidad de Bloques: %d\n",
			archivo->nombre,archivo->tamanio,archivo->dirPadre,archivo->bloques->elements_count);
	pthread_mutex_unlock(&mLogFile);
	return EXIT_SUCCESS;
}


int exportar(char* argumentos){
	if (strcmp(argumentos,"/?") == 0 )
	{
		printf("export rutaDelArchivoEnMDFS rutaDelArchivoLocal\n");
		return 0;
	}

	char** tmp;
	tmp = string_split(argumentos, " ");
	//tmp[0]: ruta del archivo en MDFS
	//tmp[1]: ruta del archivo local

	t_reg_archivo* archivo;
	char nombre[50];
	int16_t indexPadre;

	if (getNombreArchivo(tmp[0],nombre,&indexPadre) != EXIT_SUCCESS)
	{
		pthread_mutex_lock(&mLogFile);
		log_error(logFile,"No se pudo abrir el archivo %s", tmp[0]);
		pthread_mutex_unlock(&mLogFile);
		return -1;
	} else
	{
		if (getArchivo(nombre,indexPadre, &archivo) != EXIT_SUCCESS)
		{
			pthread_mutex_lock(&mLogFile);
			log_error(logFile,"No se pudo abrir el archivo %s", tmp[0]);
			pthread_mutex_unlock(&mLogFile);
			return -1;
		}
	}
	if (archivo->estado == NO_DISPONIBLE)
	{
		pthread_mutex_lock(&mLogFile);
		log_error(logFile,"El archivo %s no se encientra disponible", tmp[0]);
		pthread_mutex_unlock(&mLogFile);
		return -1;
	}

	FILE* fp = fopen(tmp[1], "ab");
	if (fp==NULL)
	{
		pthread_mutex_lock(&mLogFile);
		log_error(logFile,"No se pudo abrir el archivo %s", tmp[1]);
		pthread_mutex_unlock(&mLogFile);
		return -1;
	}
	fclose(fp);

	for (int i=0;i<archivo->bloques->elements_count;i++)
	{
		t_list* bloque = list_get(archivo->bloques,i);
		for (int j=0;j<bloque->elements_count;j++)
		{
			t_ubicacion_bloque* ubicacion = list_get(bloque,j);
			if (ubicacion->nodo->estado == DISPONIBLE)
			{
				mensaje_t* mensaje = malloc(sizeof(mensaje_t));
				mensaje->comando = string_new();
				string_append_with_format(&(mensaje->comando),"getBloque %d",
						ubicacion->bloque);
				mensaje->comandoSize = strlen(mensaje->comando) + 1;
				mensaje->dataSize = 0;

				pthread_mutex_lock(&(ubicacion->nodo->mSocket));
				enviar(ubicacion->nodo->sockfd, mensaje);
				pthread_mutex_unlock(&(ubicacion->nodo->mSocket));


				sem_wait(&(ubicacion->nodo->respuestasP));

				fp = fopen(tmp[1], "ab");
				fwrite(ubicacion->nodo->respuestaBuffer, 1,
						ubicacion->nodo->respuestaSize, fp);

				fclose(fp);
				free(ubicacion->nodo->respuestaBuffer);
				ubicacion->nodo->respuestaSize = 0;

				sem_post(&(ubicacion->nodo->respuestasR));
				break;
			}
		}
	}

	pthread_mutex_lock(&mLogFile);
	log_info(logFile,"Archivo exportado con exito");
	pthread_mutex_unlock(&mLogFile);
	return 0;
}


int md5(char* argumentos){
	if (strcmp(argumentos,"/?") == 0 )
	{
		printf("md5 rutaDelArchivoEnMDFS\n");
		return 0;
	}

	srand(time(NULL));
	int r = rand();
	char* rutaLocal = string_new();
	string_append_with_format(&rutaLocal,"/tmp/%d",r);

	char* export = string_from_format("%s %s", argumentos, rutaLocal);

	if (exportar(export) != 0)
	{
		return -1;
	}

	char* cmd = string_new();
	strcpy(cmd,"md5sum ");
	string_append(&cmd, rutaLocal);
	char path[1035];
	FILE* fp = popen(cmd, "r");
	if (fp == NULL) {
		printf("Failed to run command\n" );
		exit(1);
	}
	while (fgets(path, sizeof(path)-1, fp) != NULL) {
		printf("%s", path);
	}

	free(rutaLocal);
	free(cmd);
	free(export);
	pclose(fp);

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
			string_append_with_format(&bloques,"| Nodo: %s - Bloque: %d ",ubicacion->nodo->nombre,
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
	char** args = string_split(argumentos, " ");

	pthread_mutex_lock(&mConexiones);
	for (int i=0;i<conexiones->elements_count;i++)
	{
		Conexion_t* nodo = list_get(conexiones,i);
		if (nodo==conexion) continue;
		if (strcmp(args[0], nodo->nombre) == 0)
		{
			nodo->sockfd = conexion->sockfd;
			nodo->estado = DISPONIBLE;

			for (int k=0;k<conexiones->elements_count;k++)
			{
				Conexion_t* nodoAQuitar = list_get(conexiones,k);
				if (nodoAQuitar==conexion) list_remove(conexiones,k);
			}
			free(conexion->estadoBloques);
			free(conexion);
			pthread_mutex_unlock(&mConexiones);

			pthread_mutex_lock(&mLogFile);
			log_info(logFile, "Reconectado con %s", nodo->nombre);
			pthread_mutex_unlock(&mLogFile);

			actualizarEstadoArchivos();
			return 0;
		}
	}
	pthread_mutex_unlock(&mConexiones);


	strcpy(conexion->nombre, args[0]);
	pthread_mutex_lock(&mLogFile);
	log_info(logFile, "Identificado %s", conexion->nombre);
	pthread_mutex_unlock(&mLogFile);
	if (strcmp(conexion->nombre, "MaRTA") != 0)
	{
		conexion->totalBloques = strtoll(args[1], NULL, 10) / TAMANIO_BLOQUE;
		conexion->estadoBloques = calloc(conexion->totalBloques, 1);


		pthread_mutex_lock(&mNodosOnline);
		nodosOnline++;
		pthread_mutex_unlock(&mNodosOnline);
		if (nodosOnline == LISTA_NODOS)
		{
			pthread_mutex_lock(&mLogFile);
			log_info(logFile, "Cantidad minima de nodos (%d) alcanzada.", LISTA_NODOS);
			pthread_mutex_unlock(&mLogFile);
		}
	}
	return 0;
}


int dataFile(char* argumentos, Conexion_t* conexion)
{
	char** args = string_split(argumentos," ");
	char nombre[50];
	int16_t indexPadre = 0;
	if (getNombreArchivo(args[0],nombre,&indexPadre) != EXIT_SUCCESS)
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
				"DataFileResponse %s noDisponible", args[0]);
		respuesta->comandoSize = strlen(respuesta->comando) + 1;
		memcpy(respuesta->data,NULL,0);
		respuesta->dataSize = 0;

		pthread_mutex_lock(&(conexion->mSocket));
		enviar(conexion->sockfd,respuesta);
		pthread_mutex_unlock(&(conexion->mSocket));

		pthread_mutex_lock(&mLogFile);
		log_info(logFile,"El archivo %s no se encuentra disponible",
				args[0]);
		pthread_mutex_unlock(&mLogFile);
		return -1;
	}

	bool unBloque = false;
	int nBloque;
	if (args[1] != NULL)
	{
		nBloque = atoi(args[1]);
		unBloque = true;
	}

	char* tabla = string_new();
	for(int i=0;i<archivo->bloques->elements_count;i++)
	{
		if (unBloque) i = nBloque;

		string_append_with_format(&tabla,"%d;",i);

		t_list* bloque = list_get(archivo->bloques,i);
		for(int j=0;j<bloque->elements_count;j++)
		{
			t_ubicacion_bloque* ubicacion = list_get(bloque,j);
			if (ubicacion->nodo->estado == DISPONIBLE)
			{
				string_append_with_format(&tabla,"%s;%d;",ubicacion->nodo->nombre,ubicacion->bloque);
			}
		}
		string_append(&tabla,"\n");
		if (unBloque) break;
	}

	mensaje_t* respuesta = malloc(sizeof(mensaje_t));
	respuesta->comando = string_new();
	string_append_with_format(&(respuesta->comando),
			"DataFileResponse %s Disponible", argumentos);
	respuesta->comandoSize = strlen(respuesta->comando) + 1;
	respuesta->data = tabla;
	respuesta->dataSize = strlen(respuesta->data) + 1;

	pthread_mutex_lock(&(conexion->mSocket));
	enviar(conexion->sockfd,respuesta);
	pthread_mutex_unlock(&(conexion->mSocket));

	pthread_mutex_lock(&mLogFile);
	log_info(logFile,"Enviada la tabla de bloques a MaRTA del archivo: %s",
			archivo->nombre);
	pthread_mutex_unlock(&mLogFile);

	return EXIT_SUCCESS;
}


void actualizarEstadoArchivos()
{
	char* actualizaciones = string_new();
	pthread_mutex_lock(&mListaArchivos);
	for (int i = 0;i<listaArchivos->elements_count;i++)
	{
		t_reg_archivo* archivo = list_get(listaArchivos,i);
		int bloquesDisponibles = 0;
		for (int j=0;j<archivo->bloques->elements_count;j++)
		{
			t_list* bloque = list_get(archivo->bloques,j);
			for (int k=0;k<bloque->elements_count;k++)
			{
				t_ubicacion_bloque* ubicacion = list_get(bloque,k);
				if (ubicacion->nodo->estado == DISPONIBLE)
				{
					bloquesDisponibles++;
					break;
				}
			}
		}
		if ((archivo->estado == DISPONIBLE) &&
				(bloquesDisponibles < archivo->bloques->elements_count))
				{
					string_append_with_format(&actualizaciones,
							"El archivo %s ya no se encuentra disponible.\n",
							archivo->nombre);
					archivo->estado = NO_DISPONIBLE;
					continue;
				}
		if ((archivo->estado == NO_DISPONIBLE) &&
				(bloquesDisponibles == archivo->bloques->elements_count))
				{
					string_append_with_format(&actualizaciones,
							"El archivo %s se encuentra disponible nuevamente.\n",
							archivo->nombre);
					archivo->estado = DISPONIBLE;
					continue;
				}
	}
	pthread_mutex_unlock(&mListaArchivos);

	if (strcmp("",actualizaciones) != 0)
	{
		pthread_mutex_lock(&mLogFile);
		log_info(logFile, actualizaciones);
		pthread_mutex_unlock(&mLogFile);
	}
	free(actualizaciones);
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
			pthread_mutex_lock(&mLogFile);
			log_error(logFile, "Directorio no encontrado %s", nombrePadre);
			pthread_mutex_unlock(&mLogFile);
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
	for (int i=0;i<conexion->totalBloques;i++)
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
			t_ubicacion_bloque* ubicacion = list_get(bloque,j);

			pthread_mutex_lock(&(ubicacion->nodo->mEstadoBloques));
			ubicacion->nodo->estadoBloques[ubicacion->bloque] = false;
			//devuelvo a NO EN USO a los bloques seleccionados
			pthread_mutex_unlock(&(ubicacion->nodo->mEstadoBloques));
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
		nodos = list_filter(nodos, estaDisponible);
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
				pthread_mutex_unlock(&mElegirNodos);
				return -1;//De los tres primeros nodos, alguno no tiene bloques disponibles
			}

			pthread_mutex_lock(&(nodo->mEstadoBloques));
			nodo->estadoBloques[bloqueDisponible] = true; //Lo marco en uso
			pthread_mutex_unlock(&(nodo->mEstadoBloques));

			t_ubicacion_bloque* ubicacion = malloc(sizeof(t_ubicacion_bloque));
			ubicacion->bloque = bloqueDisponible;
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
	for (int i=0;i<conexion->totalBloques;i++)
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

bool estaDisponible(Conexion_t* conexion)
{
	if (conexion->estado == DISPONIBLE)
	{
		return true;
	}
	return false;
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
		if (nodo->estado == NO_DISPONIBLE) continue;

		espacioTotal += nodo->totalBloques * TAMANIO_BLOQUE/1024/1024;
		pthread_mutex_lock(&(nodo->mEstadoBloques));
		espacioDisponible = espacioDisponible + (getCantidadBloquesDisponibles(nodo) * TAMANIO_BLOQUE/1024/1024);
		pthread_mutex_unlock(&(nodo->mEstadoBloques));
	}

	pthread_mutex_lock(&mConexiones);
	list_destroy(nodos);
	pthread_mutex_unlock(&mConexiones);

	espacioTotal = (espacioTotal/1024); //Lo paso a GB
	espacioDisponible = (espacioDisponible/1024); //Lo paso a GB

	printf(	"Espacio Total : %.2f GB.\n"
			"Espacio disponible: %.2f GB.\n",
			espacioTotal,
			espacioDisponible);
	return EXIT_SUCCESS;
}
