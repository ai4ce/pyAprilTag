import os
import numpy as np
from distutils.core import setup, Extension
from Cython.Build import cythonize
from Cython.Distutils import build_ext

"""
Run setup with the following command:
```
python setup.py build_ext --inplace
```
"""

# Determine current directory of this setup file to find our module
CUR_DIR = os.path.dirname(__file__)
opencv_libs_str = 'C:/lib/opencv-3-4-2-vc14vc15/build/x64/vc15/lib/opencv_world342.lib'
opencv_incs_str = 'C:/lib/opencv-3-4-2-vc14vc15/build/include'
# Parse into usable format for Extension call
opencv_libs = [str(lib) for lib in opencv_libs_str.strip().split()]
opencv_incs = [str(inc) for inc in opencv_incs_str.strip().split()]

self_incs = [os.path.join(CUR_DIR, 'src'),
             os.path.join(CUR_DIR, 'src/cv2cg/include'),
             os.path.join(CUR_DIR, 'src/cv2cg/3rdparty/lch/include')]

extensions = [
    Extension('apriltag._apriltag',
              sources=[os.path.join(CUR_DIR, '_cy_apriltag.pyx')],
              language='c++',
              include_dirs=[np.get_include()] + opencv_incs + self_incs,
              extra_compile_args=['/O2'],#['-O3', '-w'],
              extra_link_args=opencv_libs)
]

setup(
    name="apriltag._apriltag",
    version = "0.0.1",
    author = "Chen Feng",
    author_email= "cfeng@nyu.edu",
    description="python wrapper for AprilTag implemented in library cv2cg",
    # platforms=
    license="BSD",
    cmdclass={'build_ext': build_ext},
    ext_modules=cythonize(extensions)
)