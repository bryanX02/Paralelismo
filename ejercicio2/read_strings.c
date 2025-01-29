#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <err.h>
#include<fcntl.h>

/** Loads a string from a file.
 *
 * fd: integer file descriptor
 *
 * The loadstr() function must allocate memory from the heap to store
 * the contents of the string read from the FILE.
 * Once the string has been properly built in memory, the function returns
 * the starting address of the string (pointer returned by malloc())
 *
 * Returns: !=NULL if success, NULL if error
 */
char *loadstr(int fd)
{
	
	int tam;
	int i = 0;
	char buffer;
	tam = read(fd,&buffer,1);
	if(tam==0){
		return 0;
	}

	while(buffer!='\0'){
		i++;
		tam = read(fd,&buffer,1);
		printf("%c\n", buffer);
		printf("tam tam -- %d\n", tam);
	}
	
	
	printf("%d\n", i);
	char *cadena = malloc(i + 1);

	off_t of = lseek(fd,-i - 1,SEEK_CUR);

	printf("of -- %ld\n", of);
	
	tam = read(fd, cadena, i + 1);

	printf("tam -- %d\n", tam);

	cadena[i] = '\0';
	printf("cadena -- %s\n", cadena);

	return NULL;	
}

int main(int argc, char *argv[])
{
	/* To be completed */
	int fd;
	fd = open(argv[1],O_RDONLY);

	char *cadena = loadstr(fd);

	/*
	cadena = loadstr(fd);

	cadena = loadstr(fd);
	cadena = loadstr(fd);
	cadena = loadstr(fd);
	cadena = loadstr(fd);
	*/
	return 0;
}
