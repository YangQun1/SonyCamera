#include "SonyCamera_Library.h"

#include <opencv2\core\core.hpp>
#include <opencv2\highgui\highgui.hpp>

#include <Windows.h>
#include <iostream>
#include <sstream>

int main()
{
	cv::Mat img;


	LARGE_INTEGER li;
	LONGLONG start, end, freq;

	QueryPerformanceFrequency(&li);
	freq = li.QuadPart;


	OpenCamera();
	StartImageAcquisition();

	char key = 'w';
	cv::namedWindow("Image", 0);
	cvResizeWindow("Image", 1224, 1024);
	int j = 0;
	while (1){
		QueryPerformanceCounter(&li);
		start = li.QuadPart;

		img = GetImage();

		QueryPerformanceCounter(&li);
		end = li.QuadPart;
		int useTime = (int)((end - start) * 1000 / freq);
		std::cout << "time: " << useTime << "ms" << std::endl;

		std::ostringstream filename;
		filename << "E:\\VSProject\\SonyCamera\\image\\" << j << ".png";
		cv::imwrite(filename.str(), img);
		cv::imshow("Image", img);
		key = cv::waitKey(20);
		//int i = 50000000;
		//while (i--);
		j++;
		std::cout << j <<std::endl;
		if (key == 'q')
			break;
	}

	cv::destroyAllWindows();
	CloseCamera();

	return 0;
}