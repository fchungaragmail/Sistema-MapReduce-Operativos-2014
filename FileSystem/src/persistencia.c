/*
 * persistencia.c
 *
 *  Created on: 9/7/2015
 *      Author: utnso
 */

#include "persistencia.h"


int persistirEstructuras();
int leerPersistencia();
FILE* persistFile;


//archivo	= nombre;dirPadre;estado;bloque;nodo;nbloque//bloque;nodo;nbloque;
//dir		= nobmre;padre;
//conexion	= nombre;estado;totalBloques;(estadoBloques con 1 y 0)

int persistirEstructuras()
{
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
			string_append_with_format(&estructuras,"%d;",j);

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

	persistFile = fopen("./FileSistem.persist","w+");
	fprintf(persistFile,estructuras);
	fclose(persistFile);
	return EXIT_SUCCESS;
}


int leerPersistencia()
{
	return EXIT_SUCCESS;
}
