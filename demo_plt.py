import cv2
import matplotlib.pyplot as plt
import apriltag
img = cv2.imread('src/cv2cg/data/calib_pattern_Tag36h11.png')
ids, corners, centers, Hs = apriltag.find(img)
if img.ndim==3:
    img = img[:,:,::-1] #BGR => RGB
print(ids)
plt.imshow(img)
plt.show()