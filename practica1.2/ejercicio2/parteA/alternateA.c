#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <err.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>
#define N_HILOS 2

pthread_t hilos[N_HILOS];
sem_t semaforos[N_HILOS];

long tam;
int *vector;

void init()
{
    sem_init(&semaforos[0],0,0); //Semaforo pares
	sem_init(&semaforos[1],0,1); //Semaforo impares
	
}


void* alternar_impares(){
	
	int n=1;
	for (int i=0;i<tam / N_HILOS;i++,n+=2){
		sem_wait(&semaforos[1]);
		printf("impar elemento--%d\n", n);
		vector[n - 1] = n;
		sem_post(&semaforos[0]);
	}
}

void* alternar_pares(){
	int n=2;
	for (int i=0;i<tam / N_HILOS;i++,n+=2){
		sem_wait(&semaforos[0]);
		printf("par elemento--%d\n", n);
		vector[n - 1] = n;
		sem_post(&semaforos[1]);
	}
}

void* mostrar_vector(){
	int j;
	j = 0;
	printf("%d", vector[j]);

	for(j=1; j<tam; j++){
		printf(",%d", vector[j]);
	}
	printf("\n");
}


int main(int argc, char* argv[])
{
	int  opt;
	while((opt = getopt(argc, argv, "n:")) != -1) {
		switch(opt) {
		case 'n':
			tam = (int)strtol(optarg,NULL,0);
			break;
		default:
			exit(EXIT_FAILURE);
		}
	}
	printf("%d\n", tam);
	vector = (int *)malloc(tam * sizeof(int));


	init();

	unsigned long i;

    pthread_create(&hilos[0], NULL, alternar_impares, NULL);
	pthread_create(&hilos[1], NULL, alternar_pares, NULL);

    
    for(i=0; i<N_HILOS; i++){
        pthread_join(hilos[i],NULL);
	}

	for(i=0; i<N_HILOS; i++){
		sem_destroy(&semaforos[i]);
	}

	mostrar_vector();

	return 0;
}


