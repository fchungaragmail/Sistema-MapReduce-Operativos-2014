/*
 * execScript.h
 *
 *  Created on: 29/5/2015
 *      Author: utnso
 */

#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>

#define MESSAGE_LENGTH 256


int execScript() {

	int p[2];
	int readed_bytes;
	pid_t pid;
	char buffer[MESSAGE_LENGTH];

	pipe(p);

	pid = fork();

	if(pid == 0) {

		//HIJO
		close(p[1]); //cierro el lado de escritura, el hijo no lo usa, el hijo solo lee

		while((readed_bytes = read(p[0], buffer, MESSAGE_LENGTH)) > 0) {
			write(1, buffer, readed_bytes);
			//execv del script
		}
		close(p[0]); //cierro el lado de lectura ya que lei todo

	} else {

		close(p[0]); //cierro el lado de lectura, el padre no lee, solo escribe
		strcpy(buffer, "Aca mando script al hijo que lo va a ejecutar\n");
		write(p[1], buffer, sizeof(buffer));
		close(p[1]); //cierro lado de escritura ya que ya lo use y no lo voy a necesitar

	}

	waitpid(pid, NULL, 0);
	return 0;

}
