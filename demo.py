import cv2
import apriltag
img = cv2.imread('src/cv2cg/data/calib_pattern_Tag36h11.png', 0)
ids, corners, centers, Hs = apriltag.find(img)
print(ids)
