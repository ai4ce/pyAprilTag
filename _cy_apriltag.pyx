'''
_cy_apriltag.pyx in pyAprilTag

author  : cfeng
created : 9/17/18 11:40AM
'''

cimport cython

from libcpp.vector cimport vector
from libcpp.string cimport string
import numpy as np
cimport numpy as np

#ref: https://stackoverflow.com/a/45928289
np.import_array()

from libcpp cimport bool
from cpython.ref cimport PyObject

# References PyObject to OpenCV object conversion code borrowed from OpenCV's own conversion file, cv2.cpp
cdef extern from 'src/pyopencv_converter.cpp':
    cdef PyObject* pyopencv_from(const Mat& m)
    cdef bool pyopencv_to(PyObject* o, Mat& m)

cdef extern from 'opencv2/core/core.hpp' namespace 'cv':
    cdef cppclass Mat:
        Mat() except +
        int rows
        int cols

cdef extern from "src/pyAprilTagDetector.cpp" namespace "py_apriltag":
    size_t detect_apriltags(
        Mat& frame,
        vector[int]& ids,
        vector[double]& corners,
        vector[double]& centers,
        vector[double]& Hs,
        const int hammingThresh
    )

cdef extern from "src/pyAprilTagDetector.cpp" namespace "py_apriltag":
    void set_tagfamilies(string tagid)

cdef extern from "src/pyAprilTagCalibrator.cpp" namespace "py_apriltag":
    int calib_by_apriltags(
        string rig_filename,
        string url,
        string log_dir,
        int nDistCoeffs,
        bool useEachValidPhoto
    )


@cython.boundscheck(False)
@cython.wraparound(False)
def _cy_set_tagfamilies(str tagid):
    cdef string tagid_ = tagid.encode()
    set_tagfamilies(tagid_)


@cython.boundscheck(False)
@cython.wraparound(False)
def _cy_calib_by_apriltags(
    str rig_filename,
    str url,
    str log_dir,
    int nDistCoeffs,
    bool useEachValidPhoto
    ):
    cdef string rig_filename_ = rig_filename.encode()
    cdef string url_ = url.encode()
    cdef string log_dir_ = log_dir.encode()
    return calib_by_apriltags(rig_filename_, url_, log_dir_, nDistCoeffs, useEachValidPhoto)


@cython.boundscheck(False)
@cython.wraparound(False)
def _cy_find_apriltags(
    np.ndarray[np.uint8_t, ndim=2, mode="c"]    frame,
    int                                         hammingTresh
    ):
    '''
    Input:
        frame <HxW>: input image
        hammingTresh <int>: hamming threshold
    Output:
        ids <Nx3>: list of detected ids within hammingThresh
        corners <Nx(4*2)>:  list of tag corners
        centers <Nx(2)>:  list of tag centers
        Hs <Nx(3*3)>:  list of homography
    '''
    cdef vector[int] ids_
    cdef vector[double] corners_
    cdef vector[double] centers_
    cdef vector[double] Hs_

    cdef Mat frame_mat = Mat()
    pyopencv_to(<PyObject*> frame, frame_mat)

    N = detect_apriltags(
        frame_mat,
        ids_, corners_, centers_, Hs_,
        hammingTresh
    )

    ids = np.array(ids_)
    corners = np.array(corners_).reshape(N,4,2)
    centers = np.array(centers_).reshape(N,2)
    Hs = np.array(Hs_).reshape(N,3,3)
    return ids, corners, centers, Hs


@cython.boundscheck(False)
@cython.wraparound(False)
def _cy_find_apriltags_and_vis(
    np.ndarray[np.uint8_t, ndim=3, mode="c"]    frame,
    int                                         hammingTresh
    ):
    '''
    Input:
        frame <HxWx3>: input color image
        hammingTresh <int>: hamming threshold
    Output:
        ids <Nx3>: list of detected ids within hammingThresh
        corners <Nx(4*2)>:  list of tag corners
        centers <Nx(2)>:  list of tag centers
        Hs <Nx(3*3)>:  list of homography
    '''
    cdef vector[int] ids_
    cdef vector[double] corners_
    cdef vector[double] centers_
    cdef vector[double] Hs_

    cdef Mat frame_mat = Mat()
    pyopencv_to(<PyObject*> frame, frame_mat)

    N = detect_apriltags(
        frame_mat,
        ids_, corners_, centers_, Hs_,
        hammingTresh
    )

    ids = np.array(ids_)
    corners = np.array(corners_).reshape(N,4,2)
    centers = np.array(centers_).reshape(N,2)
    Hs = np.array(Hs_).reshape(N,3,3)
    return ids, corners, centers, Hs

def set(tagid):
    tagid = str(tagid)
    _cy_set_tagfamilies(tagid)

def find(img, thresh=0):
    img = np.require(img, dtype=np.uint8, requirements=['C'])
    thresh = int(thresh)
    if img.ndim!=3:
        return _cy_find_apriltags(img, thresh)
    else:
        return _cy_find_apriltags_and_vis(img, thresh)

def calib(rig_filename, url='camera://0', log_dir="", nDistCoeffs=2, useEachValidPhoto=True):
    return _cy_calib_by_apriltags(rig_filename, url, log_dir, nDistCoeffs, useEachValidPhoto)