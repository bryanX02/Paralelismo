#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <err.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>
#define N_HILOS 3

pthread_t hilos[N_HILOS];
sem_t semaforos[N_HILOS];

long tam;
int *vector;

void init()
{
    
	// sem_init(&semaforos[0],0,1); 
	// sem_init(&semaforos[1],0,0); 
	// sem_init(&semaforos[2],0,0); 

	sem_init(&semaforos[0],0,1);
	for(int i=1; i<N_HILOS; i++){
		sem_init(&semaforos[i],0,0);
	}

}


void* alternar(void* i){
	
	int n = (int)i;
	
	int sem = n;
	//printf("sem -- %d\n", sem);
	n++; 
	//El hilo 0 empezará en 1, el 0 en 2 y el 2 en 3.

	//printf("resto -- %d\n", (tam%N_HILOS));
	int resto =(tam%N_HILOS);

	//printf("diferente -- %d\n", diferente);

	int acabar = (tam / N_HILOS) + ((sem + 1) <= resto);
	//printf("Acabar sem %d -- %d--\n", sem, acabar);

	for (int i=0;i<acabar;i++,n+=N_HILOS){
		sem_wait(&semaforos[sem]);
		printf("HILO -- %d --  elemento--%d\n",sem,  n);
		vector[n - 1] = n;

		//printf("semXX -- %d\n", ((sem + 1) % N_HILOS));
		sem_post(&semaforos[(sem + 1)% N_HILOS]);
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
	//printf("%d\n", tam);
	vector = (int *)malloc(tam * sizeof(int));


	init();

	unsigned long i;

    for(i=0; i<N_HILOS; i++){
        pthread_create(&hilos[i], NULL, alternar, (void*)i);
	}

    
    for(i=0; i<N_HILOS; i++){
        pthread_join(hilos[i],NULL);
	}

	for(i=0; i<N_HILOS; i++){
		sem_destroy(&semaforos[i]);
	}

	mostrar_vector();

	return 0;
}


