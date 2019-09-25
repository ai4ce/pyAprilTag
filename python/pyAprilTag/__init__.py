import os
from pyAprilTag._apriltag import *

calib_pattern_path = os.path.join(os.path.dirname(os.path.abspath(__file__)), "data", "calib_pattern_Tag36h11.png")
calib_example_dir = os.path.join(os.path.dirname(os.path.abspath(__file__)), "data", "calib")

#this module contains three functions:

# 0. select tag families
# set_tagfamilies("4") #select Tag36h11

# 1. tag detection
# ids, corners, centers, Hs = pyAprilTag.find(img)

# 2. calibration
# ret = pyAprilTag.calib(rig_filename, url='camera://0', log_dir="", nDistCoeffs=2, useEachValidPhoto=True)