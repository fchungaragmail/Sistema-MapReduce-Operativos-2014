
#include"LecturaYEscritura.h"
#include <stdint.h>

/*
* Lee datos del socket. Supone que se le pasa un buffer con hueco suficiente para los datos. Devuelve el numero de bytes leidos o
* 0 si se cierra fichero o -1 si hay error.
*/
int32_t leer_socket (int32_t nuevo_socket, char *buffer, size_t size)
{
	size_t leido = 0;
	size_t aux = 0;

	/*Comprobacion de que los parametros de entrada son correctos*/

	if ((nuevo_socket == -1) || (buffer == NULL) || (size < 1))
		return -1;

	/* Mientras no hayamos leido todos los datos solicitados*/
	while (leido < size){
		aux = recv(nuevo_socket, buffer + leido, size - leido, 0);
		if (aux > 0){
			/*
			* En caso de leer datos, se incrementa la variable que contiene los datos leidos hasta el momento
			*/
			leido = leido + aux;
		}
		else{
			/*
			* Si read devuelve 0, es que se cerro el socket. Se devuelven los caracteres leidos hasta ese momento
			*/
			if (aux == 0)
			{
				return leido;
			}
			if (aux == -1){
				/*
				* En caso de error:
				* EINTR se produce hubo una  interrupcion del sistema antes de leer ningun dato. No
				* es un error realmente.
				* EGAIN significa que el socket no esta disponible por el  momento.
				* Ambos errores se tratan con una espera de 100 microsegundos y se vuelve a intentar.
				* El resto de los posibles errores provocan que salgamos de la funcion con error.
				*/

				//Mover la logica de señales al Sig_handler

				switch (errno){
					case EINTR:
					case EAGAIN:
						usleep (100);
						break;
					default:
						return -1;
				}
			}
		}
	}
	/*
	* Se devuelve el total de los caracteres leidos
	*/
	return leido;
}

/*
* Escribe dato en el socket cliente. Devuelve numero de bytes escritos, o -1 si hay error.
*/
int32_t escribir_socket (int32_t nuevo_socket, char *datos, size_t longitud)
{
	size_t escrito = 0;
	size_t aux = 0;

	/*
	* Comprobacion de los parametros de entrada
	*/
	if ((nuevo_socket == -1) || (datos == NULL) || (longitud < 1))
		return -1;

	/*
	* Bucle hasta que hayamos escrito todos los caracteres que se indicaron.
	*/
	while (escrito < longitud){

		aux = send(nuevo_socket, datos + escrito, longitud - escrito, 0);

		if (aux > 0){
			/*
			* Si se consiguio escribir caracteres, se actualiza la variable escrito
			*/
			escrito = escrito + aux;
		}
		else
		{
			/*
			* Si se cerro el socket, devolvemos el numero de caracteres leidos.
			* Si hubo un error, devuelve -1
			*/
			if (aux == 0){
				return escrito;
			}
			else
			{
				return -1;
			}
		}
	}

	/*
	* Devolvemos el total de caracteres leidos
	*/
	return escrito;
}


