import cv2
# import numpy as np
import matplotlib.pyplot as plt
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

user_exit = False
def press(evt):
    global user_exit
    if evt.key=='escape':
        user_exit = True
    print(evt.key)

fig = plt.figure()
fig.canvas.mpl_connect('key_press_event', press)

while cap.isOpened() and not user_exit:
    # Capture frame-by-frame
    ret, frame = cap.read()
    if not ret:
        break

    if frame.shape[0]>640:
        frame = cv2.resize(frame, (640, 480))

    gframe = cv2.cvtColor(frame, cv2.COLOR_BGR2GRAY)
    ids, corners, centers, Hs = pyAprilTag.find(gframe)

    # Display the resulting frame
    if frame.ndim==3:
        frame = frame[:,:,::-1] #BGR => RGB

    plt.cla()
    plt.imshow(frame)

    for i in range(len(ids)):
        plt.plot(corners[i,[0,1,2,3,0],0], corners[i,[0,1,2,3,0],1], 'r-')
        plt.annotate('{:d}'.format(ids[i]), xy=centers[i], color='red', fontsize=12)

    plt.pause(0.01)
    plt.draw()
    cnt += 1

# When everything done, release the video capture object
cap.release()

# Closes all the frames
cv2.destroyAllWindows()