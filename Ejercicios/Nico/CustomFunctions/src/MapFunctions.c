/*
 * MapFunctions.c
 *
 *  Created on: 20/5/2015
 *      Author: utnso
 */

#include <stdio.h>
#include <fcntl.h>
#include <ctype.h>

#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>

int mmapConcatenateTextToFile(char* filePath,char *textToConcatenate)
{
    int fd;
    int *map;  /* mmapped array of int's */

    fd = open(filePath, O_RDONLY);
    if (fd == -1) {
	perror("Error opening file for reading");
	exit(EXIT_FAILURE);
    }

    off_t fsize;
    fsize = lseek(fd, 0, SEEK_END);

    map = mmap(0, fsize, PROT_READ, MAP_SHARED, fd, 0);	//fsize dice hasta donde se lee, podria tener la longitud de un registro
    if (map == MAP_FAILED) {							//el 0 del ultimo argumento indica el offset a partir de donde se lee
	close(fd);
	perror("Error mmapping the file");
	exit(EXIT_FAILURE);
    }

   printf("%s\n",map); //LEE TTODO EL ARCHIVO, SIN DISTINGUIR ENTRE SALTOS DE LINEA

 //*****************
   char* dest;
   memcpy(dest, map, fsize);
   strcat(dest, textToConcatenate);
//*****************

   printf("%s\n",dest);
   // Don't forget to free the mmapped memory
       if (munmap(map, fsize) == -1)
       {
           close(fd);
           perror("Error un-mmapping the file");
           exit(EXIT_FAILURE);
       }

    close(fd);

    mmapWrite(filePath,dest);
   return 0;
}
int mmapRead(char* filePath)
{
    int fd;
    int *map;  /* mmapped array of int's */

    fd = open(filePath, O_RDONLY);
    if (fd == -1) {
	perror("Error opening file for reading");
	exit(EXIT_FAILURE);
    }

    off_t fsize;
    fsize = lseek(fd, 0, SEEK_END);

    map = mmap(0, fsize, PROT_READ, MAP_SHARED, fd, 0);	//fsize dice hasta donde se lee, podria tener la longitud de un registro
    if (map == MAP_FAILED) {							//el 0 del ultimo argumento indica el offset a partir de donde se lee
	close(fd);
	perror("Error mmapping the file");
	exit(EXIT_FAILURE);
    }

   printf("%s\n",map); //LEE TTODO EL ARCHIVO, SIN DISTINGUIR ENTRE SALTOS DE LINEA

   // Don't forget to free the mmapped memory
       if (munmap(map, fsize) == -1)
       {
           close(fd);
           perror("Error un-mmapping the file");
           exit(EXIT_FAILURE);
       }

    close(fd);

   return 0;
}
int mmapWrite(char* filePath, char* textToWrite)
{

	int fd = open(filePath, O_RDWR | O_CREAT | O_TRUNC, (mode_t)0600);

	    if (fd == -1)
	    {
	        perror("Error opening file for writing");
	        exit(EXIT_FAILURE);
	    }

	    // Stretch the file size to the size of the (mmapped) array of char

	    size_t textsize = strlen(textToWrite) + 1; // + \0 null character

	    if (lseek(fd, textsize-1, SEEK_SET) == -1)
	    {
	        close(fd);
	        perror("Error calling lseek() to 'stretch' the file");
	        exit(EXIT_FAILURE);
	    }

	    /* Something needs to be written at the end of the file to
	     * have the file actually have the new size.
	     * Just writing an empty string at the current file position will do.
	     *
	     * Note:
	     *  - The current position in the file is at the end of the stretched
	     *    file due to the call to lseek().
	     *  - An empty string is actually a single '\0' character, so a zero-byte
	     *    will be written at the last byte of the file.
	     */

	    if (write(fd, "", 1) == -1)
	    {
	        close(fd);
	        perror("Error writing last byte of the file");
	        exit(EXIT_FAILURE);
	    }


	    // Now the file is ready to be mmapped.
	    char *map = mmap(0, textsize, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
	    if (map == MAP_FAILED)
	    {
	        close(fd);
	        perror("Error mmapping the file");
	        exit(EXIT_FAILURE);
	    }

	    int pos=0;
	           	while(pos<textsize)
	           	{

	           		printf("Writing character %c at %zu\n", textToWrite[pos], pos);
	           		              map[pos] = textToWrite[pos];

	           		           pos++;

	           	}

	    // Write it now to disk
	    if (msync(map, textsize, MS_SYNC) == -1)
	    {
	        perror("Could not sync the file to disk");
	    }

	    // Don't forget to free the mmapped memory
	    if (munmap(map, textsize) == -1)
	    {
	        close(fd);
	        perror("Error un-mmapping the file");
	        exit(EXIT_FAILURE);
	    }

	    // Un-mmaping doesn't close the file, so we still need to do that.
	    close(fd);

	    return 0;
}
