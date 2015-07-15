/*
 * persistencia.c
 *
 *  Created on: 9/7/2015
 *      Author: utnso
 */

#include "persistencia.h"


int persistirEstructuras();
int leerPersistencia();
int buscarSeccion(char* seccion);
Conexion_t* getConexionByNombre(char* nombre);
FILE* persistFile;
pthread_mutex_t mPersistFile;
sem_t sPersistencia;


//archivo	= nombre;dirPadre;estado;tamanio;nodo;nbloque//nodo;nbloque;
//dir		= nobmre;padre;
//conexion	= nombre;estado;totalBloques;(estadoBloques con 1 y 0)

int persistirEstructuras()
{
	while(true)
	{
		sem_wait(&sPersistencia);

		char* estructuras = string_new();

		//lista de archivos
		string_append_with_format(&estructuras, "%s\n", SECCION_LISTA_ARCHIVOS);
		pthread_mutex_lock(&mListaArchivos);
		for (int i=0;i<listaArchivos->elements_count;i++)
		{
			t_reg_archivo* archivo = list_get(listaArchivos,i);
			string_append_with_format(&estructuras, "%s;%d;%d;%"PRId64";",
					archivo->nombre,
					archivo->dirPadre,
					archivo->estado,
					archivo->tamanio);
			for (int j=0;j<archivo->bloques->elements_count;j++)
			{
				if (j != 0) string_append(&estructuras, "//");

				t_list* ubicaciones = list_get(archivo->bloques,j);
				for (int k=0;k<ubicaciones->elements_count;k++)
				{
					t_ubicacion_bloque* ubicacion = list_get(ubicaciones,k);
					string_append_with_format(&estructuras, "%s;%d;",
							ubicacion->nodo->nombre,
							ubicacion->bloque);
				}
			}
			string_append(&estructuras, "\n");
		}
		pthread_mutex_unlock(&mListaArchivos);


		string_append_with_format(&estructuras, "%s\n", SECCION_LISTA_DIRS);
		pthread_mutex_lock(&mListaDirs);
		for (int i=1;i<listaDirs->elements_count;i++)
		{
			t_reg_directorio* dir = list_get(listaDirs,i);
			string_append_with_format(&estructuras, "%s;%d;\n",
					dir->directorio,
					dir->padre);
		}
		pthread_mutex_unlock(&mListaDirs);


		string_append_with_format(&estructuras, "%s\n", SECCION_LISTA_CONEXIONES);
		pthread_mutex_lock(&mConexiones);
		for (int i=0;i<conexiones->elements_count;i++)
		{
			Conexion_t* conexion = list_get(conexiones,i);
			string_append_with_format(&estructuras, "%s;%d;%d;",
					conexion->nombre,
					conexion->estado,
					conexion->totalBloques);

			pthread_mutex_lock(&(conexion->mEstadoBloques));
			for (int k=0;k<conexion->totalBloques;k++)
			{
				string_append_with_format(&estructuras, "%d",
						conexion->estadoBloques[k]);
			}
			pthread_mutex_unlock(&(conexion->mEstadoBloques));
			string_append(&estructuras, "\n");
		}
		pthread_mutex_unlock(&mConexiones);

		pthread_mutex_lock(&mPersistFile);
		persistFile = fopen("./FileSystem.persist","w+");
		fprintf(persistFile,estructuras);
		fclose(persistFile);
		pthread_mutex_unlock(&mPersistFile);
	}

	return EXIT_SUCCESS;
}


int leerPersistencia()
{
	pthread_mutex_init(&mPersistFile, NULL);
	sem_init(&sPersistencia, 0, 0);

	persistFile = fopen("./FileSystem.persist", "r");
	if (persistFile == NULL) return EXIT_SUCCESS;

	char linea[MAX_BUFF_SIZE];
	int ret = 0;
	int index = 1; //El 0 es el directorio raiz que esta por default

	if (buscarSeccion(SECCION_LISTA_DIRS) == EXIT_SUCCESS)
	{
		ret = fgets(linea,MAX_BUFF_SIZE,persistFile);
		while((ret != NULL) && (strstr(linea,"#") == NULL))
		{
			char** lDir = string_split(linea,";");

			t_reg_directorio* dir = malloc(sizeof(t_reg_directorio));
			strcpy(dir->directorio,lDir[0]);
			dir->padre = atoi(lDir[1]);
			list_add_in_index(listaDirs,index,dir);

			ret = fgets(linea,MAX_BUFF_SIZE,persistFile);
			index++;
		}
	}

	if (buscarSeccion(SECCION_LISTA_CONEXIONES) == EXIT_SUCCESS)
	{
		ret = fgets(linea,MAX_BUFF_SIZE,persistFile);
		while((ret != NULL) && (strstr(linea,"#") == NULL))
		{
			char** lConexion = string_n_split(linea, 4, ";");
			Conexion_t* conexion = malloc(sizeof(Conexion_t));

			strcpy(conexion->nombre, lConexion[0]);
			conexion->estado = NO_DISPONIBLE;
			conexion->totalBloques = atoi(lConexion[2]);
			conexion->estadoBloques = calloc(conexion->totalBloques,1);
			sem_init(&(conexion->respuestasR),0,1);
			sem_init(&(conexion->respuestasP),0,0);
			pthread_mutex_init(&(conexion->mSocket), NULL);
			pthread_mutex_init(&(conexion->mEstadoBloques), NULL);

			for (int i=0;i<conexion->totalBloques;i++)
			{
				if (lConexion[3][i] == '1')
				{
					conexion->estadoBloques[i] = true;
				}else
				{
					conexion->estadoBloques[i] = false;
				}
			}
			list_add(conexiones,conexion);

			ret = fgets(linea,MAX_BUFF_SIZE,persistFile);
		}
	}


	if (buscarSeccion(SECCION_LISTA_ARCHIVOS) == EXIT_SUCCESS)
	{
		ret = fgets(linea,MAX_BUFF_SIZE,persistFile);
		while((ret != NULL) && (strstr(linea,"#") == NULL))
		{
			char** lArchivo = string_n_split(linea,5,";");
			char** lBloques = string_split(lArchivo[4],"//");
			t_reg_archivo* archivo = malloc(sizeof(t_reg_archivo));

			strcpy(archivo->nombre,lArchivo[0]);
			archivo->dirPadre = atoi(lArchivo[1]);
			archivo->estado = NO_DISPONIBLE;
			archivo->tamanio = atoll(lArchivo[3]);
			pthread_mutex_init(&(archivo->mBloques),NULL);
			archivo->bloques = list_create();

			int i = 0;
			char** lUbicaciones;
			while(lBloques[i] != NULL)
			{
				t_list* ubicaciones = list_create();
				list_add_in_index(archivo->bloques, i, ubicaciones);
				lUbicaciones = string_n_split(lBloques[i],3,";");
				while (lUbicaciones[0] != NULL)
				{
					t_ubicacion_bloque* ubicacion = malloc(sizeof(t_ubicacion_bloque));
					ubicacion->nodo = getConexionByNombre(lUbicaciones[0]);
					ubicacion->bloque = atoi(lUbicaciones[1]);
					list_add(ubicaciones,ubicacion);

					if ((lUbicaciones[2] != NULL) && (strcmp(lUbicaciones[2],"\n") != 0))
					{
						lUbicaciones = string_n_split(lUbicaciones[3],3,";");
					}else
					{
						lUbicaciones[0] = NULL;
					}

				}
				i++;
			}
			list_add(listaArchivos,archivo);
			ret = fgets(linea,MAX_BUFF_SIZE,persistFile);
		}
	}


	return EXIT_SUCCESS;
}

int buscarSeccion(char* seccion)
{
	char linea[MAX_BUFF_SIZE];
	bool encontrado = false;
	fseek(persistFile,0,SEEK_SET);
	while (!encontrado)
	{
		if (fgets(linea,MAX_BUFF_SIZE,persistFile) == NULL)
		{
			return -1;
		}
		if (strstr(linea,seccion) != NULL)
			encontrado = true;
	}
	return EXIT_SUCCESS;
}


Conexion_t* getConexionByNombre(char* nombre)
{
	for (int i=0;i<conexiones->elements_count;i++)
	{
		Conexion_t* conexion = list_get(conexiones,i);
		if (strcmp(nombre,conexion->nombre) == 0)
		{
			return conexion;
		}
	}
	return NULL;
}
