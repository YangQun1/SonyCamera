import SonyCamera

import cv2
import time
import copy

if __name__ == "__main__":
    SonyCamera.OpenCamera()
    SonyCamera.StartImageAcquisition()

    key = ord('w')
    path = 'E:\\VSProject\\SonyCamera\\image\\'
    ext = '.png'

    cv2.namedWindow("Image", 0)
    cv2.resizeWindow("Image", 1224, 1024)
    
    j = 0
    imList = []
    now = last = time.clock()
    while 1:
        im = SonyCamera.GetImage(250)
        if im is None:
            continue
        
        #imList.append(copy.deepcopy(im))
        j = j+1
        print j
        cv2.imshow('Image', im)
        key = cv2.waitKey(5)

        now = time.clock()
        print now-last
        last = now

        if key == ord('q'):
            break
    
    #j = 0
    #for im in imList:
    #    filename = path+str(j)+ext
    #    cv2.imwrite(filename, im)
    #    j = j +1

    cv2.destroyAllWindows()
    SonyCamera.StopImageAcquisition()
    SonyCamera.CloseCamera()

