#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>

// Definimos la estructura para pasar los argumentos a los hilos
typedef struct {
	int numeroHilo;
	char prioridad;
} HiloArgs;


// Función que ejecutará cada hilo
void *thread_usuario(void *arg) {

    // Copiamos los argumentos en variables locales
    HiloArgs *args = (HiloArgs *)arg;
    int numeroHilo = args->numeroHilo;
    char prioridad = args->prioridad;
    
    // Liberamos la memoria dinámica reservada
    free(arg);
    
    // Obtenemos el identificador del hilo
    pthread_t idHilo = pthread_self();
    
    // Imprimimos la información del hilo
    printf("Hilo ID: %lu, Número de hilo: %d, Prioridad: %c\n", idHilo, numeroHilo, prioridad);
    
    pthread_exit(NULL);
}

int main(int argc, char* argv[]) {
    int cantidadHilos = 5; // Número de hilos a crear
    pthread_t hilos[cantidadHilos];
    
    // Creamos los hilos
    for (int i = 0; i < cantidadHilos; i++) {

        // Ahora reservamos memoria para los argumentos de cada hilo
        HiloArgs *args = (HiloArgs *)malloc(sizeof(HiloArgs));
        if (args == NULL) {
            perror("Error al asignar memoria");
            exit(EXIT_FAILURE);
        }
        
        // Inicializamos los valores de la estructura
        args->numeroHilo = i;
        args->prioridad = (i % 2 == 0) ? 'P' : 'N'; // Prioridad par: P, Prioridad impar: N
        
        // Creamos el hilo
        if (pthread_create(&hilos[i], NULL, thread_usuario, args) != 0) {
            perror("Error al crear el hilo");
            free(args);
            exit(EXIT_FAILURE);
        }
    }
    
    // Esperamos a que los hilos terminen
    for (int i = 0; i < cantidadHilos; i++) {
        pthread_join(hilos[i], NULL);
    }
    
    return 0;
}
