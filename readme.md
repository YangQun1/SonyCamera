# Sony XCG-CG相机模块的的C++/Python接口

## Introduction
本工程为`Sony`的`XCG-CG`系列相机模块实现了简单易用的接口程序，并将其封装为动态链接库（`DLL`）。用户可以很方便的在`C++`语言或者`Python`语言中调用该接口程序对`XCG-CG`系列相机进行操作。

接口程序目前支持以下几种类型的操作：

- 打开相机
- 开始采集图像
- 获取一幅图像（被动方式）
- 主动触发相机开始拍摄
- 停止采集图像
- 关闭相机

## Dependencies
在使用本接口程序之前，首先需要安装相机的官方`SDK`。官方`SDK`可以在`Sony`的官方网站上下载，我使用的是2018年最新版本的`SDK`。在下载之前，你可能需要先注册一个`Sony`的账号。下载地址：https://www.image-sensing-solutions.eu/XCG-CG510.html

如果你使用`C++`接口，还需要安装`OpenCV`。因为该接口返回的图像是`OpenCV`的`Mat`类型。我使用的是`OpenCV 2.4.13`
如果你使用`Python`接口，还需要安装`Python`和`numpy`。因为该接口返回的图像是`ndarray`类型。我使用的是`Python 2.7`

## Develop Tool
本程序是在Visual Studio2013 Update5平台上开发的。

## Mechanism
本接口程序的图像采集过程使用了`多线程`、`内存池`和`多缓冲队列`等机制。

图像采集是一个IO密集型的任务，采用多线程实现可以允许程序在等待图像采集的过程中切换到其他的计算任务，更大程度的发挥CPU的计算性能。
当你使用本接口程序开始图像采集时，程序会创建一个图像采集线程专门用于采集图像。图像采集线程采集到的图像被存放在内存池中。用户的图像处理代码应该在主线程中实现，主线程读取内存池中的图像，然后进行需要的处理。另外我们使用一个缓冲队列来维持内存池中内存段的读写顺序。更具体的实现细节可以到代码中寻找。

## Interface
用户可以在自己的程序中调用本接口提供的下列接口函数来实现对相机的相应操作

### C++接口：

- bool OpenCamera()
- bool CloseCamera()
- bool StartImageAcquisition()
- bool StopImageAcquisition()
- bool TriggerShooting()
- cv::Mat GetImage(signed long timeOut = 250)

### Python接口:

- OpenCamera()
- CloseCamera()
- StartImageAcquisition()
- StopImageAcquisition()
- TriggerShooting()
- GetImage(timeOut)

## Example
下面两个示例展示了如何在`C++`程序和`Python`程序中使用本接口

### C++示例
使用步骤如下：

1. 首先，使用Visual Studio构建本工程，生成`.dll`和`.lib`文件
2. 将生成的`.dll`和`.lib`文件连同`SonyCamera_Library.h`文件拷贝到你的工程目录下面，或者拷贝到特定目录，然后配置环境变量（这些操作过程可以轻易的在网上找到）
3. 在代码中包含`SonyCamera_Library.h`，并在工程属性中设置相应的目录
4. OK！

下面是代码示例：

本段在`SonyCamera`目录下的`main.cpp`文件中

``` c++
#include "SonyCamera_Library.h"

#include <opencv2\core\core.hpp>
#include <opencv2\highgui\highgui.hpp>

#include <Windows.h>
#include <iostream>
#include <sstream>

int main()
{
	cv::Mat img;
	cv::Mat img_covert;

	double alpha = (double)255 / 0xFFFF;

	LARGE_INTEGER li;
	LONGLONG last, now, freq;

	QueryPerformanceFrequency(&li);
	freq = li.QuadPart;

	QueryPerformanceCounter(&li);
	now = last = li.QuadPart;


	OpenCamera();
	StartImageAcquisition();

	char key = 'w';
	cv::namedWindow("Image", 0);
	cvResizeWindow("Image", 1224, 1024);
	int j = 0;
	while (1){
		img = GetImage();
		if (img.empty())
			continue;

		cv::imshow("Image", img);
		key = cv::waitKey(5);

		QueryPerformanceCounter(&li);
		now = li.QuadPart;
		int useTime = (int)((now - last) * 1000 / freq);
		std::cout << "get time: " << useTime << "ms" << std::endl;
		last = now;

		j++;
		std::cout << j <<std::endl;
		if (key == 'q')
			break;


	}

	cv::destroyAllWindows();
	StopImageAcquisition();
	CloseCamera();

	system("pause");
	return 0;
}
```

### Python示例
使用步骤如下：

1. 首先，使用Visual Studio构建本工程，生成`.dll`和`.lib`文件
2. 将生成的`.dll`文件拷贝到你的`python`工程目录下面，并将其名字修改为`SonyCamera.pyd`
3. 在代码中带入`SonyCamera`模块
4. OK！

下面是代码示例：

本代码在`python_test`目录下的`testSonyCamera.py`文件中

``` python
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
 

    cv2.destroyAllWindows()
    SonyCamera.StopImageAcquisition()
    SonyCamera.CloseCamera()
```
# GUI
本工程还实现了一个简单的GUI程序用于对相机进行测试和参数配置

# Notes
- 如果你想在其他版本的`Python`程序中使用本接口，只需要将本接口的工程属性中与`Python`相关的选项重新配置即可，包括`Python.h`、`numpy\ndarrayobject.h`文件的路径和对应版本的`pythonxx.lib`文件的路径。另外，不同版本的Python的C-API的函数名或参数可能不一样，需要根据实际情况调整。
- 本程序目前实现的功能均在XCG-CG510相机上测试通过

# Update
- 增加了12Bit灰度图像的采集功能
- 完善GUI，实现相机常用配置参数的读取和保存功能
- 改进缓存机制，修复帧间数据串扰的Bug