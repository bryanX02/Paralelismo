#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <err.h>
#include<fcntl.h>

int main(int argc, char* argv[])
{
	//printf("%s - %s - %s\n", argv[2], argv[3], argv[4]);
	int fd_in;
	//fd_in = creat(argv[1],0751);
	fd_in = open(argv[1],O_WRONLY|O_CREAT|O_TRUNC,0751);
	if (fd_in< 0){
		perror("open"); exit(1);
 	}
	int i = 2;
	while(argv[i]!=NULL){
		//printf("%s\n", argv[i]);
		int len_i = strlen(argv[i]);
		char buffer[len_i];
		strcpy(buffer, argv[i]);
		//buffer = argv[i];
		//printf("%s\n", buffer);

		if(write(fd_in,buffer,len_i + 1)!=len_i + 1){
			perror("write");
 			close(fd_in);
 			exit(1);
 			
		}

		i++;
	}
	

	return 0;
}
