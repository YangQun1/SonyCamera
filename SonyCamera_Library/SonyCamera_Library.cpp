// SonyCamera_Library.cpp : ���� DLL Ӧ�ó���ĵ���������
//

#include "stdafx.h"

#include <iostream>
// #include <XCCamAPI.h>


#include "SonyCamera_Class.h"
#include "SonyCamera_Library.h"

using namespace std;

static Sony_Camera_Handle g_CameraHandle;

/*---------------------------- C/C++�ӿ�-��ʼ ------------------------------------- */
#ifdef _C_CPP_INTERFACE_

#include <opencv2\core\core.hpp>
#include <opencv2\highgui\highgui.hpp>

cv::Mat mat;

bool OpenCamera()
{
	bool ret;
	g_CameraHandle = new Sony_Camera();
	ret = g_CameraHandle->_openCam();
	if (!ret){
		cout << "Sony Camera Open Failed" << endl;
		return false;
	}
	

	int height, width, bitPerPixel;
	g_CameraHandle->_getImgInfo(&height, &width, &bitPerPixel);

	if (bitPerPixel == 8){
		mat.create(height, width, CV_8UC1);
	}
	else if (bitPerPixel == 24){
		mat.create(height, width, CV_8UC3);
	}
	else if (bitPerPixel == 16){
		mat.create(height, width, CV_16UC1);
	}
	else{
		mat.create(height, width, CV_16UC3);
	}

	if (mat.isContinuous()){
		cout << "Sony Camera Opened" << endl;
		return true;
	}

	mat.release();
	cout << "ERROR: Mat Is Not Continuous" << endl;
	return false;
}

bool CloseCamera()
{
	g_CameraHandle->_closeCam(); 
	delete g_CameraHandle;
	
	if (!mat.empty()){
		mat.release();
	}

	return true;
}


bool StartImageAcquisition()
{
	g_CameraHandle->_startAcquisition();
	return true;
}

// ��ȡͼ��ӿ�
// ���ڴ���е�ͼ�����ݿ������ӿ�ͼ���ڴ���
// ����������Ҫ��������ֹͼ������̸߳ı��ڴ���е�����
cv::Mat& GetImage()
{
	// ���󻥳���
	WaitForSingleObject(g_CameraHandle->hMutex, INFINITE);

	if (g_CameraHandle->dataType == Mono8 || \
		g_CameraHandle->dataType == BayerRG8){
		UCHAR *imgBuf = NULL;
		int height, width, channels;

		g_CameraHandle->_getImgBuf(&imgBuf, &height, &width, &channels);

		memcpy(mat.data, imgBuf, height*width*channels);
	}
	else if (g_CameraHandle->dataType == Mono12Packed || \
		g_CameraHandle->dataType == BayerRG12Packed){
		USHORT *imgBuf = NULL;
		int height, width, channels;

		g_CameraHandle->_getImgBuf(&imgBuf, &height, &width, &channels);

		memcpy(mat.data, imgBuf, height*width*channels*sizeof(USHORT));
	}
	else{
		//TODO
	}

	// �ͷŻ�����
	ReleaseMutex(g_CameraHandle->hMutex);

	return mat;
}
#endif
/*---------------------------- C/C++�ӿ�-���� ------------------------------------- */

/*--------------------------- Python�ӿ�-��ʼ ------------------------------------- */
#ifdef _PYTHON_INTERFACE_

#include <Python.h>
#include <numpy\ndarrayobject.h>

UCHAR*  g_Buffer_BYTE = NULL;
USHORT* g_Buffer_SHORT = NULL;

static PyObject * OpenCamera_Py(PyObject *self, PyObject *args)
{
	// �����
	bool ret;
	g_CameraHandle = new Sony_Camera();
	ret = g_CameraHandle->_openCam();
	if (!ret){
		cout << "Sony Camera Open Failed" << endl;
		Py_RETURN_NONE;
	}
	cout << "Sony Camera Opened" << endl;

	// Ϊ������ͼ�������ڴ�
	int height, width, bitPerPixel;
	g_CameraHandle->_getImgInfo(&height, &width, &bitPerPixel);

	if (bitPerPixel == 8 || bitPerPixel == 24){
		g_Buffer_BYTE = new UCHAR[height*width*bitPerPixel / 8];
	}
	else if (bitPerPixel == 16 || bitPerPixel == 48){
		g_Buffer_SHORT = new USHORT[height*width*bitPerPixel / 16];
	}


	Py_RETURN_NONE;
}

static PyObject * CloseCamera_Py(PyObject *self, PyObject *args)
{
	// �ر����
	g_CameraHandle->_closeCam();
	delete g_CameraHandle;
	cout << "Sony Camera Closed" << endl;

	// �ͷŴ�����ͼ���ڴ�
	if (g_Buffer_BYTE != NULL){
		delete g_Buffer_BYTE;
		g_Buffer_BYTE = NULL;
	}
	if (g_Buffer_SHORT != NULL){
		delete g_Buffer_SHORT;
		g_Buffer_SHORT = NULL;
	}

	Py_RETURN_NONE;
}

static PyObject * StartImageAcquisition_Py(PyObject *self, PyObject *args)
{
	g_CameraHandle->_startAcquisition();

	Py_RETURN_NONE;
}

static PyObject * GetImage_Py(PyObject *self, PyObject *args)
{
	// ���󻥳���
	WaitForSingleObject(g_CameraHandle->hMutex, INFINITE);

	PyObject *PyArray = Py_None;

	if (g_CameraHandle->dataType == Mono8 || \
		g_CameraHandle->dataType == BayerRG8){
		UCHAR *imgBuf = NULL;
		int height, width, channels;

		g_CameraHandle->_getImgBuf(&imgBuf, &height, &width, &channels);

		memcpy(g_Buffer_BYTE, imgBuf, height*width*channels);

		npy_intp *Dims = NULL;
		int dims;
		if (channels == 1){
			dims = 2;
			Dims = new npy_intp[2];
			Dims[0] = height;
			Dims[1] = width;
		}
		else if (channels == 3){
			dims = 3;
			Dims = new npy_intp[3];
			Dims[0] = height;
			Dims[1] = width;
			Dims[2] = channels;
		}
		else{
			Py_RETURN_NONE;
		}

		PyArray = PyArray_SimpleNewFromData(dims, Dims, NPY_UBYTE, g_Buffer_BYTE);

		delete Dims;
	}
	else if (g_CameraHandle->dataType == Mono12Packed || \
		g_CameraHandle->dataType == BayerRG12Packed){
		USHORT *imgBuf = NULL;
		int height, width, channels;

		g_CameraHandle->_getImgBuf(&imgBuf, &height, &width, &channels);

		memcpy(g_Buffer_SHORT, imgBuf, height*width*channels*sizeof(USHORT));

		npy_intp *Dims = NULL;
		int dims;
		if (channels == 1){
			dims = 2;
			Dims = new npy_intp[2];
			Dims[0] = height;
			Dims[1] = width;
		}
		else if (channels == 3){
			dims = 3;
			Dims = new npy_intp[3];
			Dims[0] = height;
			Dims[1] = width;
			Dims[2] = channels;
		}
		else{
			Py_RETURN_NONE;
		}

		PyArray = PyArray_SimpleNewFromData(dims, Dims, NPY_USHORT, g_Buffer_SHORT);

		delete Dims;
	}
	else{
		//TODO
	}

	// �ͷŻ�����
	ReleaseMutex(g_CameraHandle->hMutex);

	return PyArray;
}

static PyMethodDef SonyCameraMethods[] = {
	{ "OpenCamera", OpenCamera_Py, METH_NOARGS, "Function to open sony camera" },
	{ "CloseCamera", CloseCamera_Py, METH_NOARGS, "Function to close sony camera" },
	{ "StartImageAcquisition", StartImageAcquisition_Py, METH_NOARGS, "Function to start image acquisition" },
	{ "GetImage", GetImage_Py, METH_NOARGS, "Function to get an image from camera(not directly from camera,but from image pool actually)" },
	{ NULL, NULL, 0, NULL }
};

PyMODINIT_FUNC initSonyCamera(void) {
	(void)Py_InitModule("SonyCamera", SonyCameraMethods);
	import_array();
}


#endif
/*--------------------------- Python�ӿ�-���� ------------------------------------- */

