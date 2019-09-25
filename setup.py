import os
import sys
import glob
import platform
import numpy as np
from setuptools import setup, Extension
from Cython.Build import cythonize
from Cython.Distutils import build_ext

"""
Run setup with the following command:
```
python setup.py build_ext --inplace
```
"""

# Avoid a gcc warning below:
# cc1plus: warning: command line option ‘-Wstrict-prototypes’ is valid
# for C/ObjC but not for C++
class BuildExt(build_ext):
    def build_extensions(self):
        if '-Wstrict-prototypes' in self.compiler.compiler_so:
            self.compiler.compiler_so.remove('-Wstrict-prototypes')
        super().build_extensions()

# Determine current directory of this setup file to find our module
CUR_DIR = os.path.dirname(__file__)
self_incs = [os.path.join(CUR_DIR, 'src'),
             os.path.join(CUR_DIR, 'src/cv2cg/include'),
             os.path.join(CUR_DIR, 'src/cv2cg/3rdparty/lch/include')]

required_opencv_modules = ['opencv_'+it for it in
                           'calib3d core features2d highgui imgcodecs imgproc videoio world'.split(' ')]

def get_conda_opencv_info(sys_name):
    try:
        import cv2
    except ImportError as e:
        print(str(e))
        print('Please install opencv for python in your current Anaconda environment via the following:\n'
              'conda install -c conda-forge opencv=3.4.7')
        exit(-1)

    assert(cv2.getVersionMajor() == 3) #currently we do not support opencv4

    if sys_name == 'Linux':
        opencv_lib_dir = os.path.abspath(os.path.join(os.path.dirname(cv2.__file__), *(['..'] * 2)))
        available_opencv_libs = glob.glob(os.path.join(opencv_lib_dir, 'libopencv*.so'))
    elif sys_name == 'Darwin':
        opencv_lib_dir = os.path.abspath(os.path.join(os.path.dirname(cv2.__file__), *(['..'] * 2)))
        available_opencv_libs = glob.glob(os.path.join(opencv_lib_dir, 'libopencv*.dylib'))
    else: #sys_name=='Windows'
        opencv_lib_dir = os.path.abspath(os.path.join(os.path.dirname(cv2.__file__), *['..'] * 2, 'Library', 'lib'))
        available_opencv_libs = glob.glob(os.path.join(opencv_lib_dir, 'opencv*.lib'))

    opencv_inc_dir = os.path.abspath(os.path.join(opencv_lib_dir, '..', 'include'))
    assert(os.path.exists(opencv_lib_dir))
    assert(os.path.exists(opencv_inc_dir))
    assert(os.path.exists(os.path.join(opencv_inc_dir, 'opencv2')))

    assert(any(available_opencv_libs))
    available_opencv_libs = [os.path.splitext(os.path.basename(it))[0].lstrip('lib')
                             for it in available_opencv_libs]

    if sys_name == 'Windows':
        opencv_libs = [lib for lib in available_opencv_libs
                       if any([lib.startswith(it) for it in required_opencv_modules])]
    else:
        opencv_libs = [lib for lib in required_opencv_modules if lib in available_opencv_libs]
    return opencv_lib_dir, opencv_inc_dir, opencv_libs

sys_name = platform.system()
opencv_lib_dir, opencv_inc_dir, opencv_libs = get_conda_opencv_info(sys_name)
print(opencv_lib_dir)
print(opencv_inc_dir)
print(opencv_libs)

extensions = [
Extension('pyAprilTag._apriltag',
          sources=[os.path.join(CUR_DIR, '_cy_apriltag.pyx')],
          language='c++',
          include_dirs=[np.get_include(), opencv_inc_dir] + self_incs,
          libraries=opencv_libs,
          library_dirs=[opencv_lib_dir,],
          extra_compile_args=['/O2'] if sys_name=='Windows' else ['-O3', '-w'],
          # extra_link_args=None if sys_name=='Windows' else ['-Wl,-R$ORIGIN/.']
)]

setup(
    name="pyAprilTag",
    version="0.0.6",
    author="Chen Feng",
    author_email="cfeng@nyu.edu",
    description="python wrapper for AprilTag implemented in library cv2cg",
    url="https://github.com/ai4ce/pyAprilTag",
    packages=['pyAprilTag'],
    package_dir={'': 'python'},
    package_data={'pyAprilTag': ['data/*.png', 'data/**/*']},
    include_package_data=True,
    license="BSD",
    cmdclass={'build_ext': build_ext if sys_name=='Windows' else BuildExt},
    ext_modules=cythonize(extensions, compiler_directives={'language_level' : sys.version_info[0]}),
    install_requires=[
        'numpy',
        'matplotlib'
    ]
)