import os
import numpy as np
from skimage import io, color, feature
from skimage.transform import ProjectiveTransform, SimilarityTransform, warp, AffineTransform
import sys
import matplotlib.pyplot as plt
from matplotlib.patches import ConnectionPatch

# Function to extract filenames from command line arguments
def extract_filenames():
    arguments = sys.argv[1:]
    num_arguments = len(arguments)
    
    if num_arguments != 2:
        sys.exit('Error: Expected exactly 2 arguments. Usage: ./stitch_images.py <filename1> <filename2>')
    
    filename1 = arguments[0]
    filename2 = arguments[1]

    if not (os.path.isfile(filename1) and os.path.isfile(filename2)):
        sys.exit('Error: One or both of the specified files do not exist.')
        
    return filename1, filename2

# Function to extract keypoints and descriptors from an image
def feature_extraction(img):
    detector = feature.SIFT()
    detector.detect_and_extract(img)
    keypoints = detector.keypoints
    descriptors = detector.descriptors.astype(np.float32)
    return keypoints, descriptors

# Function to compute affine transformation matrix given corresponding points
def compute_affine_transform(src_points, dst_points):
    A = np.zeros((2 * len(src_points), 6))
    b = np.zeros((2 * len(src_points), 1))
    
    for i, (src, dst) in enumerate(zip(src_points, dst_points)):
        A[2*i] = [src[0], src[1], 1, 0, 0, 0]
        A[2*i+1] = [0, 0, 0, src[0], src[1], 1]
        b[2*i] = dst[0]
        b[2*i+1] = dst[1]

    params, _, _, _ = np.linalg.lstsq(A, b, rcond=None)

    transform_matrix = np.vstack([params.reshape(2, 3), [0, 0, 1]])
    
    return AffineTransform(matrix=transform_matrix)

# Function to compute projective (homography) transformation matrix given corresponding points
def compute_projective_transform(src_points, dst_points):
    num_points = src_points.shape[0]
    A = np.zeros((2 * num_points, 9))

    for i in range(num_points):
        x, y = src_points[i]
        u, v = dst_points[i]
        A[2 * i] = [-x, -y, -1, 0, 0, 0, x * u, y * u, u]
        A[2 * i + 1] = [0, 0, 0, -x, -y, -1, x * v, y * v, v]
    
    U, S, V = np.linalg.svd(A)
    H = V[-1, :].reshape((3, 3))

    return ProjectiveTransform(matrix=H)

# Function to plot matched keypoints between two images without lines
def plot_matched_keypoints_without_lines(img1, img2, keypoints1, keypoints2, inliers, matches):
    src_best = keypoints2[matches[inliers, 1]][:, ::-1]
    dst_best = keypoints1[matches[inliers, 0]][:, ::-1]

    fig, ax = plt.subplots(1, 2, figsize=(10, 5))
    ax[0].imshow(img1)
    ax[1].imshow(img2)

    for i in range(src_best.shape[0]):
        ax[1].plot(src_best[i, 0], src_best[i, 1], 'ro', fillstyle='none')  # Plotting keypoints without filling
        ax[0].plot(dst_best[i, 0], dst_best[i, 1], 'ro', fillstyle='none')  # Plotting keypoints without filling

    plt.show()

# Function to plot matched keypoints between two images with lines
def plot_matched_keypoints_with_lines(img1, img2, keypoints1, keypoints2, inliers, matches):
    src_best = keypoints2[matches[inliers, 1]][:, ::-1]
    dst_best = keypoints1[matches[inliers, 0]][:, ::-1]

    fig, ax = plt.subplots(1, 2, figsize=(10, 5))
    ax[0].imshow(img1)
    ax[1].imshow(img2)

    for i in range(src_best.shape[0]):
        coordB = [dst_best[i, 0], dst_best[i, 1]]
        coordA = [src_best[i, 0], src_best[i, 1]]
        con = ConnectionPatch(xyA=coordA, xyB=coordB, coordsA="data", coordsB="data", axesA=ax[1], axesB=ax[0], color="red")
        ax[1].add_artist(con)
        ax[0].plot(dst_best[i, 0], dst_best[i, 1], 'ro')
        ax[1].plot(src_best[i, 0], src_best[i, 1], 'ro')

    plt.show()

# Function to perform RANSAC for robustly estimating a transformation model
def ransac(data, img1, img2, keypoints1, keypoints2, matches, compute_transform=compute_projective_transform, min_samples=4, residual_threshold=1, max_trials=300):
    input_data, target_data = data
    optimal_transform = None
    optimal_inliers = []

    for _ in range(max_trials):
        selected_indices = np.random.choice(input_data.shape[0], min_samples, replace=False)
        sample_input = input_data[selected_indices]
        sample_target = target_data[selected_indices]

        transform_model = compute_transform(sample_input, sample_target)

        inliers = []
        for i in range(input_data.shape[0]):
            predicted_target = transform_model(input_data[i])
            if np.linalg.norm(predicted_target - target_data[i]) < residual_threshold:
                inliers.append(i)

        if len(inliers) > len(optimal_inliers):
            optimal_inliers = inliers
            optimal_transform = compute_transform(input_data[optimal_inliers], target_data[optimal_inliers])
    
    return optimal_transform, optimal_inliers

# Function to warp one image onto another based on a given transformation
def warp_img(img_gray1, img1, img2, sk_M):
    rows, cols = img_gray1.shape
    corners = np.array([
        [0,     0],
        [cols,  0],
        [0,    rows],
        [cols, rows]
    ])

    corners_proj = sk_M(corners)
    all_corners = np.vstack((corners_proj[:, :2], corners[:, :2]))

    corner_min = np.min(all_corners, axis=0)
    corner_max = np.max(all_corners, axis=0)
    output_shape = (corner_max - corner_min)
    output_shape = np.ceil(output_shape[::-1]).astype(int)

    offset = SimilarityTransform(translation=-corner_min)
    dst_warped = warp(img1, offset.inverse, output_shape=output_shape)

    tf_img = warp(img2, (sk_M + offset).inverse, output_shape=output_shape)

    foreground_pixels = tf_img[tf_img > 0]
    dst_warped[tf_img > 0] = foreground_pixels

    fig = plt.figure()
    ax1 = fig.add_subplot(111)
    ax1.imshow(dst_warped)

    plt.show()

# Main function to execute the image stitching process
def main():
    filename1, filename2 = extract_filenames()

    img1 = io.imread(filename1)
    if img1.shape[2] == 4:
        img1 = color.rgba2rgb(img1)
    img_gray1 = color.rgb2gray(img1)

    img2 = io.imread(filename2)
    if img2.shape[2] == 4:
        img2 = color.rgba2rgb(img2)
    img_gray2 = color.rgb2gray(img2)

    keypoints1, descriptors1 = feature_extraction(img_gray1)
    keypoints2, descriptors2 = feature_extraction(img_gray2)

    matches = feature.match_descriptors(descriptors1, descriptors2, cross_check=True)

    dst = keypoints1[matches[:, 0]]
    src = keypoints2[matches[:, 1]]

    affine_transform, affine_inliers = ransac((src[:, ::-1], dst[:, ::-1]), img1, img2, keypoints1, keypoints2, matches, compute_transform=compute_affine_transform, min_samples=4, residual_threshold=1, max_trials=300)
    print("Affine Transformation Matrix:")
    print(affine_transform.params)

    projective_transform, projective_inliers = ransac((src[:, ::-1], dst[:, ::-1]), img1, img2, keypoints1, keypoints2, matches, compute_transform=compute_projective_transform, min_samples=4, residual_threshold=1, max_trials=300)
    print("\nProjective Transformation Matrix:")
    print(projective_transform.params)

    # Plot matched keypoints without lines
    plot_matched_keypoints_without_lines(img1, img2, keypoints1, keypoints2, affine_inliers, matches)

    # Plot matched keypoints with lines
    plot_matched_keypoints_with_lines(img1, img2, keypoints1, keypoints2, affine_inliers, matches)

    # Warp the image
    warp_img(img_gray1, img1, img2, projective_transform)

if __name__ == '__main__':
    main()