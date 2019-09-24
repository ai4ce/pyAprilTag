import cv2
import matplotlib.pyplot as plt
import pyAprilTag
img = cv2.imread(pyAprilTag.calib_pattern_path)
ids, corners, centers, Hs = pyAprilTag.find(img)
if img.ndim==3:
    img = img[:,:,::-1] #BGR => RGB
print(ids)
plt.imshow(img)
plt.show()