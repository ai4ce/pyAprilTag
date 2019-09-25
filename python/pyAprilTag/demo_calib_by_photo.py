import os
import sys
import pyAprilTag

CUR_DIR = os.getcwd()
LOG_DIR = os.path.join(CUR_DIR,'calib_log')
sys.path.insert(0, CUR_DIR) #otherwise importlib cannot find the path
if not os.path.exists(LOG_DIR):
    os.makedirs(LOG_DIR)
    pyAprilTag.calib(pyAprilTag.calib_pattern_path,
                   'photo://{}'.format(os.path.join(pyAprilTag.calib_example_dir, '*.png')),
                   log_dir=LOG_DIR, nDistCoeffs=4)

import importlib
logs = sorted([f for f in os.listdir(LOG_DIR) if f.endswith('.py')])
if len(logs) == 0:
    print('no calibration log available!')
    exit(-1)

last_log = os.path.relpath(os.path.join(LOG_DIR, logs[-1])).replace(os.path.sep,'.')[:-3]
calib = importlib.import_module(last_log)
print('last log: '+last_log)
print('camera intrinsic matrix:')
print(calib.K)
print('camera distortion parameters:')
print(calib.distCoeffs)