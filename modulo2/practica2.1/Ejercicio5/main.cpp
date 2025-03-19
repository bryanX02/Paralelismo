#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <omp.h>
#include "csv_dataset.h"
#include "extra.h"

// ALUMNOS:
// - Daniel Coleto
// - Bryan Quilumba

// Hemos modificado el código original para optimizar sus tiempos de ejecución mediante vectorización.
// Cambios que hemos aplicado:
// - Array of Structs -> Struct of Arrays
// - Omp simd en las funciones de cálculo de distancias (warning)
// - Omp simd en la función de clasificación (warning)
// - Omp simd en la función de evaluación del modelo
// - Hemos pasado de usar una matriz de features a un array unidimension de features

// Con esto hemos logrado una mejora del 10% en el tiempo de ejecución (2,95s -> 2,67s).
// Creemos que se podría mejorar más si las sumas se hicieran vectorizadas, y lograsemos resolver los warnings por conflictos.


// Compute Euclidean distance
float euclideanDistance(float *features, int sample_idx, float *queryX, int n_features, int feature_row_len) {
    float sum = 0.0f;

    #pragma omp simd aligned(features, queryX: 32) reduction(+:sum)
    for (int i = 0; i < n_features; i++) {
        float diff = features[sample_idx*feature_row_len + i] - queryX[i];
        sum += diff * diff;
    }
    return sqrtf(sum);
}

int classify(CSVData train_data, float *queryX, int k, float *distances, int *labels) {

    #pragma omp simd
    for (int i = 0; i < train_data.n_samples; i++) {
        distances[i] = euclideanDistance(train_data.features, i, queryX, train_data.n_features, train_data.n_features);
        labels[i] = train_data.y[i];
    }

    // Find k nearest neighbors using a selection approach
    #pragma ivdep
    for (int i = 0; i < k; i++) {
        int min_index = i;
        for (int j = i + 1; j < train_data.n_samples; j++) {
            if (distances[j] < distances[min_index]) {
                min_index = j;
            }
        }
        // Swap distances and labels
        float temp_dist = distances[i];
        distances[i] = distances[min_index];
        distances[min_index] = temp_dist;
        
        int temp_label = labels[i];
        labels[i] = labels[min_index];
        labels[min_index] = temp_label;
    }

    // Count label occurrences
    int label_count[10] = {0};
    #pragma ivdep
    for (int i = 0; i < k; i++) {
        label_count[labels[i]]++;
    }
    
    // Determine most frequent label
    int max_label = 0;
    #pragma ivdep
    for (int i = 1; i < 10; i++) {
        if (label_count[i] > label_count[max_label]) {
            max_label = i;
        }
    }

    return max_label;
}

// Evaluate model with precision and recall
void evaluate_model(float *precision, float *recall, CSVData test_data, int* y_pred) {
    int tp = 0, fp = 0, fn = 0;
    int *test_y = test_data.y;

    #pragma omp simd aligned(y_pred, test_y: 32) reduction(+:tp, fp, fn)
    for (int i = 0; i < test_data.n_samples; ++i) {
        if (y_pred[i] == 1 && test_data.y[i] == 1) tp++;
        if (y_pred[i] == 1 && test_data.y[i] == 0) fp++;
        if (y_pred[i] == 0 && test_data.y[i] == 1) fn++;
    }

    *precision = (float)(tp) / (float)(tp + fp);
    *recall = (float)(tp) / (float)(tp + fn);
}

int main() {
    setbuf(stdout, NULL);

    // Read dataset
    CSVData data = read_csv("Datasets/KNN_Large_Dataset.csv");

    // Split dataset into training and testing
    CSVData train_data, test_data;
    train_test_split(data, &train_data, &test_data, 0.2f, 7);

    float *distances = (float *)malloc(train_data.n_samples * sizeof(float));
    int *labels = (int *)malloc(train_data.n_samples * sizeof(int));
    int *y_pred = (int *)malloc(test_data.n_samples * sizeof(int));

    clock_t start = clock();

    // Classify each test sample
    int k = 4;
    float *queryX = (float *)malloc(test_data.n_features * sizeof(float));
    for (int i = 0; i < test_data.n_samples; i++) {
        for (int j = 0; j < test_data.n_features; j++) {
            queryX[j] = test_data.features[i * test_data.n_features + j]; // Corrected access
        }
        y_pred[i] = classify(train_data, queryX, k, distances, labels);
    }
    free(queryX);

    clock_t end = clock();
    double duration = (double)(end - start) / CLOCKS_PER_SEC;

    // Print results
    float precision, recall;
    evaluate_model(&precision, &recall, test_data, y_pred);
    printf("Precision: %.2f Recall: %.2f\n", precision, recall);
    printf("Testing (%d) completed in %f seconds.\n", test_data.n_samples, duration);

    // Free memory
    free_csv_data(data);
    free_csv_data(train_data);
    free_csv_data(test_data);

    free(distances);
    free(labels);
    free(y_pred);

    return 0;
}

