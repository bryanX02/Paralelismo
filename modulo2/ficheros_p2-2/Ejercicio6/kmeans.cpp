#include <math.h>
#include <omp.h>

#include "csv_dataset.h"
#include "extra.h"

// Tiempos en ordenador personal con i7-11800H
// 0. Sin paralelizar nada: 7,44s
// 1. Paralelizando la asignación de clusters: 5,16s (30,65% mejora)
// 2. Paralelizando la actualización de centroides: 2,50 s (51,55% mejora)

void initializeCentroids(std::vector<Centroid>& centroids, std::vector<Point>& data, int k) {
    srand(42);
    for (int i = 0; i < k; i++) {
        int randIdx = rand() % data.size();
        centroids[i].coords = data[randIdx].coords;
    }
}


// Compute Euclidean distance for N-dimensional space
double euclideanDistance(const Point& a, const Centroid& b) {
    double sum = 0.0;
    for (size_t i = 0; i < a.coords.size(); i++) {
        sum += pow(a.coords[i] - b.coords[i], 2);
    }
    return sqrt(sum);
}

// Assign clusters to points
void assignClusters(std::vector<Point>& data, std::vector<Centroid>& centroids, int k) {

    // 1º mejora
    // Paralaelizamos la asignación de clusters, ya que cada punto es independiente
    // Se utiliza schedule(static) para dividir el trabajo de manera equitativa entre los hilos
    // Solo con esto, el rendimiento mejora un 30,65% en un i7-11800H
    #pragma omp parallel for schedule(static)
    for (size_t i = 0; i < data.size(); i++) {
        double minDist = std::numeric_limits<double>::max();
        int bestCluster = -1;
        for (int j = 0; j < k; j++) {
            double dist = euclideanDistance(data[i], centroids[j]);
            if (dist < minDist) {
                minDist = dist;
                bestCluster = j;
            }
        }
        data[i].cluster = bestCluster;
    }
}

// Update centroids 
void updateCentroids(std::vector<Point>& data, std::vector<Centroid>& centroids, int k, int dimensions) {
    std::vector<std::vector<double>> sumCoords(k, std::vector<double>(dimensions, 0.0));
    std::vector<int> count(k, 0);

    // 2º mejora
    // En un principio, se intento paraalelizar la actualización de centroides usando reduction, pero no funcionó
    // porque no se admiten arrglos multidimensionales. Entonces, hemos decidido usar una reducción manual
    // Para ello, se crean vectores locales para cada hilo y luego se combinan en una sección crítica  
    // Gracias a esto, el rendimiento mejora un 51,55% en un i7-11800H (redimiento acumulado del 66,4%)
    #pragma omp parallel 
    {

        // Vectores locales para evitar condiciones de carrera
        std::vector<std::vector<double>> localSumCoords(k, std::vector<double>(dimensions, 0.0));
        std::vector<int> localCount(k, 0);

        // Cada hilo calcula su propia suma y cuenta
        #pragma omp for
        for (size_t i = 0; i < data.size(); i++) {
            int cluster = data[i].cluster;
            localCount[cluster]++;
            for (int d = 0; d < dimensions; d++) {
                localSumCoords[cluster][d] += data[i].coords[d];
            }
        }

        // Reducción manual, combinando resultados de todos los hilos
        #pragma omp critical
        {
            for (int j = 0; j < k; j++) {
                count[j] += localCount[j];
                for (int d = 0; d < dimensions; d++) {
                    sumCoords[j][d] += localSumCoords[j][d];
                }
            }
        }
    }

    // Finalmente, dejamos el mismo bloque que calcula los nuevos centroides
    for (int j = 0; j < k; j++) {
        if (count[j] > 0) {
            for (int d = 0; d < dimensions; d++) {
                centroids[j].coords[d] = sumCoords[j][d] / count[j];
            }
        }
    }
}

// K-Means clustering function
void kmeans(std::vector<Point>& data, int k, int max_iters = 20000) {
    int dimensions = data[0].coords.size();
    std::vector<Centroid> centroids(k);
    initializeCentroids(centroids, data, k);

    for (int iter = 0; iter < max_iters; iter++) {
        assignClusters(data, centroids, k);

        std::vector<Centroid> newCentroids = centroids;
        updateCentroids(data, newCentroids, k, dimensions);

        centroids = newCentroids;
    }

    std::cout << "Final cluster centers:\n";
    for (int i = 0; i < k; i++) {
        std::cout << "Cluster " << i << ": (";
        for (size_t d = 0; d < centroids[i].coords.size(); d++) {
            std::cout << centroids[i].coords[d] << (d == centroids[i].coords.size() - 1 ? ")" : ", ");
        }
        std::cout << "\n";
    }
}

int main() {
    int k = 5;  // Number of clusters
    int dimensions;
//    std::vector<Point> data = loadCSV("mallClusters.csv", &dimensions, {0,1,2,5});
    std::vector<Point> data = loadCSV("housing_clean.csv", &dimensions, {0,1,9});
    
   
    if (data.empty()) {
        std::cerr << "Error loading data!" << std::endl;
        return -1;
    }

    // Start timing
    double start_time = get_time();
    kmeans(data, k);
    double end_time = get_time();
    
    // Calculate elapsed time
    double duration = end_time - start_time;
    printf("Training completed in %f seconds.\n", duration);

    return 0;
}
