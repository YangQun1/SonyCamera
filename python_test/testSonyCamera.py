import SonyCamera

import cv2
import time

if __name__ == "__main__":
    SonyCamera.OpenCamera()
    SonyCamera.StartImageAcquisition()

    key = ord('w')
    cv2.namedWindow("Image", 0)
    cv2.resizeWindow("Image", 1224, 1024)
    j = 0
    now = last = time.clock()
    while 1:
        im = SonyCamera.GetImage(250)
        if im is None:
            continue
        now = time.clock()
        print im.shape
        cv2.imshow('Image', im)
        key = cv2.waitKey(20)
        print now-last
        last = now
        if key == ord('q'):
            break

    cv2.destroyAllWindows()
    SonyCamera.StopImageAcquisition()
    SonyCamera.CloseCamera()

