pyAprilTag
==========

Overview
--------

This is the python3 wrapper for AprilTag implemented in cv2cg.

Currently it is tested under Windows10(VC2017, x64)/Ubuntu 18.04, OpenCV3.4.7(official prebuilt version),
and python3.7/3.6 in Anaconda3.

Version
-------

0.0.6

Installation
------------

### Install from Pip/Conda (easiest)

0. Download file [opencv_py37_conda-forge.txt](https://github.com/ai4ce/pyAprilTag/raw/master/opencv_py37_conda-forge.txt) to your anaconda base directory (default in windows is %USERPROFILE%)
1. try to create and activate a new conda environment with:     
    ```
    conda create --name opencv_py37 python=3.7 --file opencv_py37_conda-forge.txt -c conda-forge
    conda activate opencv_py37
    ```

2. for Windows users:    
    ```
    pip install https://github.com/ai4ce/pyAprilTag/releases/download/0.0.6/pyAprilTag-0.0.6-cp37-cp37m-win_amd64.whl
    ```
    
   for Linux users:       
   ```
   pip install https://github.com/ai4ce/pyAprilTag/releases/download/0.0.6/pyAprilTag-0.0.6-cp37-cp37m-linux_x86_64.whl
   ```

3. Enjoy the demo:     
    ```
    python -m pyAprilTag.demo
    ```

### Install by Compiling from Source

Following is for Mac users who need to compile from source, or other users who want to try installing from source. 

0. clone repository **recursively**, and install pre-built opencv 3.4.7 for python from conda-forge:   
    ```
    git clone --recursive https://github.com/ai4ce/pyAprilTag.git
    conda install -c conda-forge opencv=3.4.7
    ```

1. prepare your build environment:   
    ```
    #this example is for Windows built
    "C:\Users\cfeng\Anaconda3\Scripts\activate.bat"
    "C:\Program Files (x86)\Microsoft Visual Studio\2017\Community\VC\Auxiliary\Build\vcvars64.bat"
    ```

2. compile the project and you are done:   
    ```
    pip install -e .
    ```
   
Usage
-----

```
python -m pyAprilTag.demo
# expected output:
#[ 0  1  2  3  4  5  6  7  8  9 10 11 12 13 14 15 16 17 18 19 20 21 22 23
# 24 25 26 27 28 29 30 31 32 33 34 35 36 37 38 39]

python -m pyAprilTag.demo_plt
python -m pyAprilTag.demo_calib
python -m pyAprilTag.demo_calib_by_photo #remember to remove the calib_log folder in the current working directory
python -m pyAprilTag.vdemo_cv
python -m pyAprilTag.vdemo_plt
```

Contact
-------

Chen Feng <cfeng at nyu dot edu>

**Feel free to email any bugs or suggestions to help us improve the code. Thank you!**
