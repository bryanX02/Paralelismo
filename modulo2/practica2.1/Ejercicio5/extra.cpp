#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include "extra.h"

void train_test_split(CSVData data, CSVData *train, CSVData *test, float test_size, int random_state) {
    // Extract values of data
    int n_samples = data.n_samples;
    int n_features = data.n_features;

    int test_count = (int)(n_samples * test_size);
    int train_count = n_samples - test_count;

    train->n_features = test->n_features = n_features;
    train->n_samples = train_count;
    test->n_samples = test_count;

    // Allocate and copy labels
    train->labels = (char **)malloc((n_features + 1) * sizeof(char *));
    test->labels = (char **)malloc((n_features + 1) * sizeof(char *));
    for (int i = 0; i < n_features +1; i++) {
        train->labels[i] = strdup(data.labels[i]);
        test->labels[i] = strdup(data.labels[i]);
    }

    train->features = (float *)malloc(n_features * train_count * sizeof(float));
    test->features = (float *)malloc(n_features * test_count * sizeof(float));

    train->y = (int *)malloc(train_count * sizeof(int));
    test->y = (int *)malloc(test_count * sizeof(int));

    // Initialize random seed
    srand(random_state);

    int *indices = (int *)malloc(n_samples * sizeof(int));
    for (int i = 0; i < n_samples; i++) {
        indices[i] = i;
    }

    // Shuffle indices
    for (int i = n_samples - 1; i > 0; i--) {
        int j = rand() % (i + 1);
        int temp = indices[i];
        indices[i] = indices[j];
        indices[j] = temp;
    }

    // Split data into training and testing sets
    for (int i = 0; i < train_count; i++) {
        int idx = indices[i];
        for (int j = 0; j < n_features; j++) {
            train->features[i * n_features + j] = data.features[idx * n_features + j];
        }
        train->y[i] = data.y[idx];
    }

    for (int i = 0; i < test_count; i++) {
        int idx = indices[train_count + i];
        for (int j = 0; j < n_features; j++) {
            test->features[i * n_features + j] = data.features[idx * n_features + j];
        }
        test->y[i] = data.y[idx];
    }
    free(indices);
}