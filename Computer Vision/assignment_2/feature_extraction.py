import numpy as np
from skimage import io, color, feature
from tqdm import tqdm

def main():
    # Load the pre-split data
    data = np.load("cifar10.npz", allow_pickle=True) # dictionary of arrays

    train_descriptors, train_labels = convert_and_extract(data['X_train'].astype(np.uint8), data['y_train'].astype(np.uint32), 'Extracting features for training data')
    test_descriptors, test_labels = convert_and_extract(data['X_test'].astype(np.uint8), data['y_test'].astype(np.uint32), 'Extracting features for testing data')

    # hyperparams
    vocab_size = 50

    # Build a shared vocab set from the training set
    train_histograms = generate_histograms(train_descriptors, vocab_size)
    test_histograms = generate_histograms(test_descriptors, vocab_size)

    # Save the extracted features to a file
    save(train_histograms, test_histograms, train_labels, test_labels, 'sift.npz')

def convert_and_extract(images, labels, desc='Extracting features'):
    out_descriptors = []
    out_labels = []
    total_features = 0

    for idx in tqdm(range(images.shape[0]), desc=desc):
        try:
            # Convert the image to grayscale
            cifar_rgb = images[idx].reshape(3, 32, 32).transpose(1, 2, 0)
            cifar_gray = color.rgb2gray(cifar_rgb)

            # Extract features using SIFT
            sift = feature.SIFT()
            sift.detect_and_extract(cifar_gray)
            out_descriptors.append(sift.descriptors)
            total_features += sift.descriptors.shape[0]
            out_labels.append(labels[idx])  # Only stores the label if the features are successfully extracted
        except:
            pass

    print(f'Extracted {total_features} features')
    
    return out_descriptors, out_labels

def generate_histograms(all_descriptors, vocab_size=50):
    # Convert the list of SIFT features to a numpy array
    descriptors_np = np.concatenate(all_descriptors)

    # Flatten the descriptors array
    descriptors_flat = descriptors_np.flatten()

    # Randomly select vocab_size descriptors to form the vocabulary
    vocabulary = np.random.choice(descriptors_flat, size=vocab_size, replace=False)

    # Build histograms with the above vocab set
    image_histograms = []
    
    for descriptors in tqdm(all_descriptors, desc="Creating histograms"):
        # Predict the closest cluster for each feature
        clusters = predict_clusters(vocabulary, descriptors)
        # Build a histogram of the clusters
        histogram, _ = np.histogram(clusters, bins=vocab_size, range=(0, vocab_size))
        image_histograms.append(histogram)

    # Convert the list of histograms to a numpy array
    image_histograms_np = np.array(image_histograms)

    return image_histograms_np

def predict_clusters(vocabulary, descriptors):
    # Reshape the vocabulary array to match the shape of descriptors
    vocab_reshaped = vocabulary.reshape(1, -1, 1)

    # Compute distances between each descriptor and all vocabulary descriptors
    distances = np.linalg.norm(descriptors[:, None] - vocab_reshaped, axis=-1)

    # Find the nearest cluster for each descriptor
    clusters = np.argmin(distances, axis=-1)
    return clusters

def save(X_train, X_test, Y_train, Y_test, filename):
    # map the data
    data = {
        "X_train": X_train,
        "X_test": X_test,
        "y_train": Y_train,
        "y_test": Y_test
    }

    # Save the dictionary to a file
    np.savez(filename, **data)
    print(f'Data is saved in file as {filename}')

if __name__ == '__main__':
    main()