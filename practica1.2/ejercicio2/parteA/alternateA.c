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
    sem_init(&semaforos[0],0,0);
	sem_init(&semaforos[1],0,1);
	
}



void* alternar_pares(void* i){
	int n=2;
	for (int i=0;i<tam / N_HILOS;i++,n+=2){
		wait(&semaforos[0]);
		printf("par--%d\n", n-1);
		printf("par elemento--%d\n", n);
		vector[n - 1] = n;
		signal(&semaforos[1]);
	}
}

void* alternar_impares(void* i){
	
	int n=1;
	
	for (int i=0;i<tam / N_HILOS;i++,n+=2){
		wait(&semaforos[1]);
		printf("impar--%d\n", n-1);
		printf("impar elemento--%d\n", n);
		vector[n - 1] = n;
		signal(&semaforos[0]);
	}
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



	unsigned long i;

    pthread_create(&hilos[1], NULL, alternar_impares, (void*)0);
	pthread_create(&hilos[0], NULL, alternar_pares, (void*)1);

    
    for(i=0; i<N_HILOS; i++){
        pthread_join(hilos[i],NULL);
	}

	for(i=0; i<N_HILOS; i++){
		sem_destroy(&semaforos[i]);
	}

	int j;
	j = 0;
	printf("%d", vector[j]);

	for(j=1; j<tam; j++){
		printf(",%d", vector[j]);
	}
	printf("\n");
	return 0;
}

