#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "csv_dataset.h"

// Function to read a CSV file and return CSVData structure
CSVData read_csv(const char *filename) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        perror("Failed to open file");
        exit(EXIT_FAILURE);
    }

    char buffer[1024];
    int n_samples = 0;
    int n_features = 0;

    // Read the header line to get labels and feature count
    if (fgets(buffer, sizeof(buffer), file)) {
        char *token = strtok(buffer, ",\n"); // Remove \n at the end
        while (token) {
            n_features++;
            token = strtok(NULL, ",\n");
        }
        n_features--; // Exclude target column
    } else {
        fclose(file);
        fprintf(stderr, "File is empty\n");
        exit(EXIT_FAILURE);
    }

    // Count lines to determine number of data rows
    while (fgets(buffer, sizeof(buffer), file)) {
        n_samples++;
    }
    rewind(file); // Reset file pointer

    // Allocate memory for dataset
    CSVData data;
    data.n_features = n_features;
    data.n_samples = n_samples;

    data.labels = (char **)malloc((n_features + 1) * sizeof(char *));

    data.features = (float *)malloc(n_features * n_samples * sizeof(float)); // Allocate as 1D array
    data.y = (int *)malloc(n_samples * sizeof(int));

    // Read the header again and store labels
    if (fgets(buffer, sizeof(buffer), file)) {
        int i = 0;
        char *token = strtok(buffer, ",\n");
        while (token && i < n_features + 1) {
            data.labels[i] = strdup(token); // Allocate and copy token
            i++;
            token = strtok(NULL, ",\n");
        }
    }

    // Read the data
    int row = 0;
    while (fgets(buffer, sizeof(buffer), file) && row < n_samples) {
        char *token = strtok(buffer, ",\n");
        for (int col = 0; col < n_features; col++) {
            data.features[row * n_features + col] = atof(token); // Access as 1D array
            token = strtok(NULL, ",\n");
        }
        data.y[row] = atoi(token); // Última columna es `y`
        row++;
    }

    fclose(file);
    return data;
}

// Function to free CSVData memory
void free_csv_data(CSVData data) {
    if (data.features) {
        free(data.features);
    }
    if (data.y) {
        free(data.y);
    }
    if (data.labels){
        for(int i = 0; i < data.n_features +1; i++){
            free(data.labels[i]);
        }
        free(data.labels);
    }
}