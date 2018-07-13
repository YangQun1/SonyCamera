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
    while 1:
        start = time.clock()
        im = SonyCamera.GetImage()
        end = time.clock()
        print end-start
        print im.shape
        cv2.imshow('Image', im)
        key = cv2.waitKey(20)
        j = j + 1
        print j
        if key == ord('q'):
            break

    cv2.destroyAllWindows()
    SonyCamera.CloseCamera()

