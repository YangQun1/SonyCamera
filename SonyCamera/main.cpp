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

		//std::ostringstream filename;
		//filename << "E:\\VSProject\\SonyCamera\\image\\" << j << ".png";
		//cv::imwrite(filename.str(), img);

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
	CloseCamera();

	system("pause");
	return 0;
}