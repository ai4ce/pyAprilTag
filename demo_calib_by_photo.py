import os
import cv2
import numpy as np
from numpy import array
import apriltag

CUR_DIR = os.path.dirname(__file__)
LOG_DIR = os.path.join(CUR_DIR,'calib_photo')
if not os.path.exists(LOG_DIR):
    os.makedirs(LOG_DIR)
    apriltag.calib('src/cv2cg/data/calib_pattern_Tag36h11.png',
                   'photo://calib\*.png',
                   log_dir=LOG_DIR, nDistCoeffs=4)

import importlib
logs = sorted([f for f in os.listdir(LOG_DIR) if f.endswith('.py')])
last_log = os.path.relpath(os.path.join(LOG_DIR, logs[-1])).replace('\\','.')[:-3]
calib = importlib.import_module(last_log)
print('last log: '+last_log)
print('camera intrinsic matrix:')
print(calib.K)
print('camera distortion parameters:')
print(calib.distCoeffs)