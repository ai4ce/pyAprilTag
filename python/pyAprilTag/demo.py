import cv2
import pyAprilTag
img = cv2.imread(pyAprilTag.calib_pattern_path, 0)
ids, corners, centers, Hs = pyAprilTag.find(img)
print(ids)
