import matplotlib.pyplot as plt
import numpy as np
import cv2

def extract_keypoints(img_gray):
    # Initialize SIFT detector
    sift = cv2.SIFT_create()  # Use SIFT_create() for feature extraction
    
    # Detect keypoints and compute descriptors
    keypoints, descriptors = sift.detectAndCompute(img_gray, None)
    return keypoints, descriptors

def match_keypoints(descriptors1, descriptors2, ratio=0.8):
    # Initialize Brute Force Matcher
    bf = cv2.BFMatcher()
    
    # Perform knn matching to find two nearest matches for each descriptor
    matches = bf.knnMatch(descriptors1, descriptors2, k=2)
    
    # Apply ratio test to find good matches
    good_matches = [m for m, n in matches if m.distance < ratio * n.distance]
    return good_matches

# Load images
img1 = cv2.imread('yosemite1.jpg')
img1_RGB = cv2.cvtColor(img1, cv2.COLOR_BGR2RGB)
img1_gray = cv2.cvtColor(img1, cv2.COLOR_BGR2GRAY)

img2 = cv2.imread('yosemite2.jpg')
img2_RGB = cv2.cvtColor(img2, cv2.COLOR_BGR2RGB)
img2_gray = cv2.cvtColor(img2, cv2.COLOR_BGR2GRAY)

# Extract keypoints and descriptors
keypoints1, descriptors1 = extract_keypoints(img1_gray)
keypoints2, descriptors2 = extract_keypoints(img2_gray)

# Match keypoints
good_matches = match_keypoints(descriptors1, descriptors2)

# Draw matches
result = cv2.drawMatches(img1_RGB, keypoints1, img2_RGB, keypoints2, good_matches, None, flags=cv2.DrawMatchesFlags_NOT_DRAW_SINGLE_POINTS)

# Display the final result
plt.figure(figsize=(15, 7))
plt.imshow(result)
plt.axis('off')
plt.show()