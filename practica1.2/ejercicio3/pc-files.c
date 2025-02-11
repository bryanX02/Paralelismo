#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <err.h>
#include <unistd.h>
#include <pthread.h>
#include <fcntl.h>
#include <semaphore.h>

// Definimos las variables globales
#define MAX_SBUFFER_SIZE 4
char* shared_buffer[MAX_SBUFFER_SIZE]; 
int ridx = 0;
int widx = 0;

// Esta definicion la realizamos para que visual studio code 
// no nos marque error en la variable optarg de unistd.h
extern char *optarg;

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
	int fd = open(fichero_salida, O_WRONLY | O_CREAT | O_TRUNC, 0666);
    if (fd < 0) {
        perror("Error abriendo el archivo de salida");
        exit(EXIT_FAILURE);
    }

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
			write(fd, shared_buffer[ridx], strlen(shared_buffer[ridx]));
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

	// Evaluamos los argumentos de entrada
	int  opt;
	while((opt = getopt(argc, argv, "i:o:")) != -1) {

		switch(opt) {
		
		// En caso de que se pase un argumento -i
		case 'i':
			fichero_entrada = optarg;
			hay_fE = 1;
			break;
		// En caso de que se pase un argumento -o
		case 'o':
			fichero_salida = optarg;
			break;
		default:
			break;
		}
	}

	// Inicializamos las condiciones y los mutex
	pthread_cond_init(&prod, NULL);
	pthread_cond_init(&cons, NULL);

	pthread_create(&productor, NULL, fproductor, NULL);
	pthread_create(&consumidor, NULL, fconsumidor, NULL);

	// Esperamos a que los hilos terminen
    pthread_join(productor,NULL);
	pthread_join(consumidor,NULL);

	// Destruimos los mutex
	pthread_cond_destroy(&prod);
	pthread_cond_destroy(&cons);
	
	return 0;
}
