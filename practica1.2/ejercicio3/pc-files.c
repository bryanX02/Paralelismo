#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <err.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>

#define MAX_SBUFFER_SIZE 4
char* shared_buffer[MAX_SBUFFER_SIZE]; 
int ridx = 0;
int widx = 0;


char *fichero_entrada;
char *fichero_salida = "out.txt";
int hay_fE = 0;
int nr_items=0;

pthread_mutex_t mutex;

pthread_cond_t prod, cons;

pthread_t productor, consumidor;


void* fproductor(void* i)
{	
	char buffer[1024];
	if(hay_fE){
		FILE *ficheroE = fopen(fichero_entrada,"r");
		while (fgets(buffer, sizeof(buffer), ficheroE)) {  
        	//printf("Leído: %s", buffer); 
			

			//printf("%s\n", "PROD - Mutex ANTES lock");
			pthread_mutex_lock(&mutex);
			//printf("%s\n", "PROD - Mutex lock");

			while (nr_items==MAX_SBUFFER_SIZE){
				//printf("%s\n", "PROD - Mutex ANTES WAIT");
				pthread_cond_wait(&prod,&mutex);
			}

			char *ptrabuffer = (char *)malloc(strlen(buffer));

			if(ptrabuffer == NULL){
				exit(0);
			}

			strcpy(ptrabuffer, buffer);

			shared_buffer[widx] = ptrabuffer;
			//printf("sharedbuffer -- %s\n", shared_buffer[widx]);
			widx = (widx + 1) % MAX_SBUFFER_SIZE;
			nr_items++;

			//printf("%s\n", "PROD - ANTES SIGNAL");
			pthread_cond_signal(&cons);
			//printf("%s\n", "PROD - DESPUES SIGNAL");


			//printf("%s\n", "PROD - ANTES UNlock");
			pthread_mutex_unlock(&mutex);

		}

	}else{
		while (fgets(buffer, 1024, stdin)) {  
        	//printf("Leído: %s", buffer); 
			

			//printf("%s\n", "PROD - Mutex ANTES lock");
			pthread_mutex_lock(&mutex);
			//printf("%s\n", "PROD - Mutex lock");

			while (nr_items==MAX_SBUFFER_SIZE){
				//printf("%s\n", "PROD - Mutex ANTES WAIT");
				pthread_cond_wait(&prod,&mutex);
			}

			char *ptrabuffer = (char *)malloc(strlen(buffer));

			if(ptrabuffer == NULL){
				exit(0);
			}

			strcpy(ptrabuffer, buffer);

			shared_buffer[widx] = ptrabuffer;
			//printf("sharedbuffer -- %s\n", shared_buffer[widx]);
			widx = (widx + 1) % MAX_SBUFFER_SIZE;
			nr_items++;

			//printf("%s\n", "PROD - ANTES SIGNAL");
			pthread_cond_signal(&cons);
			//printf("%s\n", "PROD - DESPUES SIGNAL");


			//printf("%s\n", "PROD - ANTES UNlock");
			pthread_mutex_unlock(&mutex);

			
    	}
	}

	
	strcpy(buffer, "NULL");
	//printf("buffer -- %s\n", buffer);
	pthread_mutex_lock(&mutex);

	while (nr_items==MAX_SBUFFER_SIZE){
		pthread_cond_wait(&prod,&mutex);
	}

	char *ptrabuffer = (char *)malloc(strlen(buffer));

	if(ptrabuffer == NULL){
		exit(0);
	}

	strcpy(ptrabuffer, buffer);

	shared_buffer[widx] = ptrabuffer;
	widx = (widx + 1) % MAX_SBUFFER_SIZE;
	nr_items++;

	pthread_cond_signal(&cons);

	pthread_mutex_unlock(&mutex);
	
}



void* fconsumidor(void* i)
{
	//printf("%s\n", "Hola");
	FILE *ficheroS = fopen(fichero_salida, "wb");
	int seguir = 1;
	//char bufferE[1024];
	while (seguir) {  
		//printf("%s\n", "Hola2");
		
		//printf("%s\n", "CONS - WAITING FOR Mutex lock");
		pthread_mutex_lock(&mutex);
		//printf("%s\n", "CONS - Mutex lock");

		while (nr_items==0){
			//printf("%s\n", "CONS - ANTES de WAIT");
			pthread_cond_wait(&cons,&mutex);
			//printf("%s\n", "CONS - DESPUES de WAIT");
		}



		//strcpy(bufferE, shared_buffer[ridx]);
		if(strcmp(shared_buffer[ridx], "NULL") == 0){
			//printf("%s\n", "SEGUIR == 0");
			seguir = 0;
		}

		if (seguir){
			fwrite(shared_buffer[ridx], sizeof(char),strlen(shared_buffer[ridx]), ficheroS);
			//printf("CONSSSSS-- Escrito: %s\n", shared_buffer[ridx]); 
		}
		
		//printf("bufferE-- %s\n", bufferE);
		free(shared_buffer[ridx]);

		ridx = (ridx + 1) % MAX_SBUFFER_SIZE;
		nr_items--;

		//printf("%s\n", "CONS - ANTES de SIGNAL");
		pthread_cond_signal(&prod);
		//printf("%s\n", "CONS - DESPUES de SIGNAL");

		
		pthread_mutex_unlock(&mutex);
		//printf("%s\n", "CONS - Mutex UNlock");


		//printf("%s\n", "fin de bucleeeeeeeeeeeeeeeeeee");
	}



}




int main(int argc, char* argv[])
{

	
	int  opt;
	while((opt = getopt(argc, argv, "i:o:")) != -1) {
		switch(opt) {
		case 'i':
			fichero_entrada = optarg;
			hay_fE = 1;
			break;
		case 'o':
			fichero_salida = optarg;
			break;
		default:
			break;
		}
	}




	pthread_cond_init(&prod, NULL);
	pthread_cond_init(&cons, NULL);


	pthread_create(&productor, NULL, fproductor, NULL);
	pthread_create(&consumidor, NULL, fconsumidor, NULL);



    pthread_join(productor,NULL);
	pthread_join(consumidor,NULL);


	pthread_cond_destroy(&prod);
	pthread_cond_destroy(&cons);
	
	return 0;
}
