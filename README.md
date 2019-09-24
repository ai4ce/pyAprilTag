pyAprilTag
==========

Overview
--------

This is the python3 wrapper for AprilTag implemented in cv2cg.

Currently it is tested under Windows10(VC2017, x64), OpenCV3.4.7(official prebuilt version), and python3.6 in Anaconda3.

Version
-------

0.0.4

Installation
------------

### Install from Pip

For Linux and Windows users, simply run the following command:   
```
pip install pyAprilTag
python -m pyAprilTag.demo
# expected output:
#[ 0  1  2  3  4  5  6  7  8  9 10 11 12 13 14 15 16 17 18 19 20 21 22 23
# 24 25 26 27 28 29 30 31 32 33 34 35 36 37 38 39]
```

### Install by Compiling from Source

For Mac users, you need to compile from source. **First install the C++ version of OpenCV 3.4.x.**
Then: 

0. clone repository recursively:   
    ```
    git clone --recursive https://github.com/ai4ce/pyAprilTag.git
    ```

1. prepare your build environment:   
    ```
    #this example is for Windows built
    "C:\Users\cfeng\Anaconda3\Scripts\activate.bat"
    "C:\Program Files (x86)\Microsoft Visual Studio\2017\Community\VC\Auxiliary\Build\vcvars64.bat"
    ```

2. **Check the setup.py file, modify the OpenCV include and libs path according to your system.**   

3. compile the project:   
    ```
    pip install -e .
    ```

4. Enjoy!   
    ```
    python -m pyAprilTag.demo
    ```


Contact
-------

Chen Feng <cfeng at nyu dot edu>

**Feel free to email any bugs or suggestions to help us improve the code. Thank you!**