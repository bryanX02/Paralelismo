import numpy as np
import pandas as pd
import time
from sklearn.preprocessing import StandardScaler

# --- Custom KMeans implementation ---
def compute_distances(data, centroids, labels):
    n_samples, n_features = data.shape
    k = centroids.shape[0]

    for i in range(n_samples):
        min_dist = np.inf
        for j in range(k):
            dist = 0.0
            for f in range(n_features):
                diff = data[i, f] - centroids[j, f]
                dist += diff * diff
            if dist < min_dist:
                min_dist = dist
                labels[i] = j

def update_centroids(data, labels, k):
    n_samples, n_features = data.shape
    new_centroids = np.zeros((k, n_features))
    counts = np.zeros(k)

    for i in range(n_samples):
        c = labels[i]
        for f in range(n_features):
            new_centroids[c, f] += data[i, f]
        counts[c] += 1

    for j in range(k):
        if counts[j] > 0:
            for f in range(n_features):
                new_centroids[j, f] /= counts[j]

    return new_centroids

def kmeans(data, k=3, max_iters=100):
    n_samples, n_features = data.shape
    centroids = data[np.random.choice(n_samples, k, replace=False)]
    labels = np.zeros(n_samples, dtype=np.int32)

    for _ in range(max_iters):
        compute_distances(data, centroids, labels)
        centroids = update_centroids(data, labels, k)

    return centroids, labels

# --- Load and preprocess data ---
df = pd.read_csv("housing_clean.csv")
data = df.select_dtypes(include=[np.number]).dropna().values
scaler = StandardScaler()
data = scaler.fit_transform(data)
k = 5
# Set a fixed random seed for reproducibility
RANDOM_SEED = 42
np.random.seed(RANDOM_SEED)


# --- Run custom KMeans ---
start_custom = time.time()
custom_centroids, custom_labels = kmeans(data, k)
end_custom = time.time()


# --- Results ---
print("Custom KMeans execution time:", round(end_custom - start_custom, 4), "seconds")
print("\nFirst 10 labels (Custom): ", custom_labels[:10])

