import numpy as np
from sklearn.svm import LinearSVC
from sklearn.model_selection import train_test_split
from sklearn.metrics import accuracy_score
from sklearn.cluster import MiniBatchKMeans
from sklearn.preprocessing import StandardScaler
from sklearn.pipeline import make_pipeline

def main():
    # Load the data from sift_features.npz
    data = np.load("sift.npz", allow_pickle=True)
    train_feats = data['X_train']
    test_feats = data['X_test']
    train_labels = data['y_train']
    test_labels = data['y_test']

    # Preprocessing: Scale features
    scaler = StandardScaler()
    train_feats_scaled = scaler.fit_transform(train_feats)
    test_feats_scaled = scaler.transform(test_feats)

    # Train the SVM model
    svm_classifier = LinearSVC(random_state=42, dual=False, max_iter=10000)
    svm_classifier.fit(train_feats_scaled, train_labels)

    # Evaluate the SVM model
    predictions = svm_classifier.predict(test_feats_scaled)
    accuracy = accuracy_score(test_labels, predictions)
    correct_predictions = np.sum(predictions == test_labels)

    print(f'Number of correct matches found from test data: {correct_predictions:d}')
    print(f'Accuracy of the classifiers: {accuracy:.2f}')

if __name__ == '__main__':
    main()