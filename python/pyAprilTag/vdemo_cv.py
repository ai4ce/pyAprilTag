'''
demo_cv in pyAprilTag

author  : cfeng
created : 9/19/18 12:25AM
'''

import cv2
import pyAprilTag

# Create a VideoCapture object and read from input file
# If the input is the camera, pass 0 instead of the video file name
camid = 0
cap = cv2.VideoCapture(camid)
if not cap.isOpened():
    cap = cv2.VideoCapture(camid+cv2.CAP_DSHOW)
cap.set(cv2.CAP_PROP_FRAME_WIDTH, 640)
cap.set(cv2.CAP_PROP_FRAME_HEIGHT, 480)
cap.set(cv2.CAP_PROP_FPS, 60)
print('{:d}x{:d} @ {:d}Hz'.format(
    int(cap.get(cv2.CAP_PROP_FRAME_WIDTH)),
    int(cap.get(cv2.CAP_PROP_FRAME_HEIGHT)),
    int(cap.get(cv2.CAP_PROP_FPS))))

# Check if camera opened successfully
if not cap.isOpened():
    print("Error opening video stream or file")
    exit(0)

# Read until video is completed
cnt = 0
cv2.namedWindow('Frame', cv2.WINDOW_NORMAL)
while cap.isOpened():
    # Capture frame-by-frame
    ret, frame = cap.read()
    if not ret:
        break

    if frame.shape[0]>640:
        frame = cv2.resize(frame, (640, 480))

    # gframe = cv2.cvtColor(frame, cv2.COLOR_BGR2GRAY)
    ids, corners, centers, Hs = pyAprilTag.find(frame)

    # Display the resulting frame
    cv2.imshow('Frame',frame)
    # Press ESC on keyboard to exit
    if cv2.waitKey(10) == 27:
        break
    cnt += 1

# When everything done, release the video capture object
cap.release()

# Closes all the frames
cv2.destroyAllWindows()