#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <err.h>
#include <pthread.h>
#include <fcntl.h>
/*
#include <getopt.h>
#include <sys/types.h>
#include <ctype.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <time.h>
#include <errno.h>
*/

#define MAX_CHARS_MSG   128
char* nombre_programa=NULL;
char* nombre_propio = NULL;
char nombre_ajeno[MAX_CHARS_MSG];
char* path_fifo_1=NULL;
char* path_fifo_2=NULL;

pthread_t chatprimero, chatsegundo;

typedef enum  {
    NORMAL_MSG,	 /* Mensaje para transferir lineas de la 
						 entre 	ambos usuarios del chat */
    USERNAME_MSG,  /* Tipo de mensaje reservado para enviar 
						el nombre de usuario al otro extremo*/    
    END_MSG	 /* Tipo de mensaje que se envía por el FIFO 
				cuando un extremo finaliza la comunicación */
} message_type_t;

struct chat_message{
     char contenido[MAX_CHARS_MSG]; //Cadena de caracteres (acabada en '\0) 
     message_type_t type;	
};

void* fifo_send(void* i)
{	
     const int size=sizeof(struct chat_message);
     int wbytes=0;
     int fd_fifo=0;
     fd_fifo=open(path_fifo_1,O_WRONLY);
     if (fd_fifo < 0) {
          perror("Error al abrir FIFO de envío");
          exit(1);
     }
     printf("%s\n", "Conexión de envío establecida!!");
     struct chat_message msg;
     msg.type=USERNAME_MSG;
     strncpy(msg.contenido, nombre_propio, MAX_CHARS_MSG);
     wbytes=write(fd_fifo,&msg,size);

	if (wbytes > 0 && wbytes!=size) {
		fprintf(stderr,"Can't write the whole register\n");
		exit(1);
  	}else if (wbytes < 0){
		perror("Error when writing to the FIFO\n");
		exit(1);
  	}		
     

     printf("> ");
     //ENVIAR MENSAJES DE TIPO NORMAL
     while(fgets(msg.contenido, MAX_CHARS_MSG, stdin)>0){
          
          msg.type=NORMAL_MSG;
          wbytes=write(fd_fifo,&msg,size);
          if (wbytes > 0 && wbytes!=size) {
               fprintf(stderr,"Can't write the whole register\n");
               exit(1);
          }
          else if (wbytes < 0){
               perror("Error when writing to the FIFO\n");
               exit(1);
          }		
          printf("> ");
     }


     //AL ACABAR POR CTRL+D, ENVIAR UN MENsaje de tipo END
     msg.type=END_MSG;
     //strcpy(msg.contenido, nombre_propio);
     wbytes=write(fd_fifo,&msg,size);

	if (wbytes > 0 && wbytes!=size) {
		fprintf(stderr,"3- Can't write the whole register\n");
		exit(1);
  	}else if (wbytes < 0){
		perror("3- Error when writing to the FIFO\n");
		exit(1);
  	}		


     close(fd_fifo);
}

void* fifo_receive(void* i)
{	
     
     struct chat_message msg;
     int fd_fifo=0;
     int bytes=0;
     const int size=sizeof(struct chat_message);
     
     fd_fifo=open(path_fifo_2,O_RDONLY);
     if (fd_fifo < 0) {
          perror("Error al abrir el FIFO de entrada.");
          exit(1);
     }
     printf("%s\n", "Conexión de recepción establecida!!");

     
     while((bytes=read(fd_fifo,&msg,size))==size){
          if (msg.type == USERNAME_MSG) {
               strncpy(nombre_ajeno, msg.contenido, MAX_CHARS_MSG);
          } 
          else if (msg.type == NORMAL_MSG) {
               printf("\n%s dice:\t%s", nombre_ajeno, msg.contenido);
          } 
          else if (msg.type == END_MSG) {
               printf("Conexión finalizada por %s!!\n", nombre_ajeno);
               exit(0);
          }
     }

     //printf("\n\nBYTES %d\n\n", bytes);
     //printf("\n\SIZE %d\n\n", size);

     //CERRAR EL FIFO
     close(fd_fifo);

}


int main(int argc, char* argv[])
{
     nombre_programa = argv[0];
     nombre_propio=argv[1];
     path_fifo_1=argv[2];
     path_fifo_2=argv[3];


     pthread_create(&chatprimero, NULL, fifo_send, NULL);
	pthread_create(&chatsegundo, NULL, fifo_receive, NULL);

	// Esperamos a que los hilos terminen
     pthread_join(chatprimero,NULL);
	pthread_join(chatsegundo,NULL);

	return 0;
}


