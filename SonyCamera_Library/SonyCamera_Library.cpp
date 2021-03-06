// SonyCamera_Library.cpp : 定义 DLL 应用程序的导出函数。
//

#include "stdafx.h"

#include <iostream>
// #include <XCCamAPI.h>


#include "SonyCamera_Class.h"
#include "SonyCamera_Library.h"

using namespace std;

static Sony_Camera_Handle g_CameraHandle;

/*---------------------------- C/C++接口-开始 ------------------------------------- */
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
		cout << "Fail to Open Sony Camera" << endl;
		return false;
	}

	cout << "Sony Camera is Opened" << endl;
	return true;
}

bool CloseCamera()
{
	bool ret; 
	ret = g_CameraHandle->_closeCam(); 
	if (!ret){
		cout << "Fail to Close Sony Camera" << endl;
	}
	else{
		cout << "Sony Camera is Closed" << endl;
	}

	delete g_CameraHandle;
	
	return ret;
}


bool StartImageAcquisition()
{
	bool ret;
	ret = g_CameraHandle->_startAcquisition();
	if (!ret){
		cout << "ERROR: Fail to Start Image Acquisition" << endl;
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

	if (!mat.isContinuous()){
		cout << "ERROR: Mat Is Not Continuous" << endl;
		mat.release();
		return false;
	}

	cout << "Image Acquisition is Started " << endl;
	return true;
}

bool StopImageAcquisition()
{
	bool ret;
	ret = g_CameraHandle->_stopAcquisition();
	if (!ret){
		cout << "ERROR: Fail to Stop Image Acquisition" << endl;
	}
	else{
		cout << "Image Acquisition is Stopped " << endl;
	}

	if (!mat.empty()){
		mat.release();
	}
	
	return ret;
}

bool TriggerShooting()
{
	return g_CameraHandle->_triggerShooting();
}

cv::Mat GetImage(signed long timeOut)
{
	bool ret;
	ret = g_CameraHandle->_getImgBuf(mat.data, timeOut);

	if (!ret){
		cv::Mat emptyMat;
		return emptyMat;
	}

	return mat;
}

#ifdef _C_CPP_ADDITIONAL_
#include "SonyCamera_Class.h"

Sony_Camera_Handle GetCameraHandle()
{
	return g_CameraHandle;
}

#endif /* _C_CPP_ADDITIONAL_ */

#endif /* _C_CPP_INTERFACE_ */
/*---------------------------- C/C++接口-结束 ------------------------------------- */

/*--------------------------- Python接口-开始 ------------------------------------- */
#ifdef _PYTHON_INTERFACE_

#include <Python.h>
#include <numpy\ndarrayobject.h>

UCHAR*  g_Buffer = NULL;
int height, width, channels, bitPerPixel;

static PyObject * OpenCamera_Py(PyObject *self, PyObject *args)
{
	// 打开相机
	bool ret;
	g_CameraHandle = new Sony_Camera();
	ret = g_CameraHandle->_openCam();
	if (!ret){
		cout << "ERROR: Fail to Open Sony Camera" << endl;
		Py_RETURN_FALSE;
	}

	cout << "Sony Camera is Opened" << endl;
	Py_RETURN_TRUE;
}

static PyObject * CloseCamera_Py(PyObject *self, PyObject *args)
{
	// 关闭相机
	bool ret;
	ret = g_CameraHandle->_closeCam();
	if (!ret){
		cout << "Fail to Close Sony Camera" << endl;
	}
	else{
		cout << "Sony Camera is Closed" << endl;
	}

	delete g_CameraHandle;
	if (ret)
		Py_RETURN_TRUE;

	Py_RETURN_FALSE;
}

static PyObject * StartImageAcquisition_Py(PyObject *self, PyObject *args)
{
	bool ret;
	ret = g_CameraHandle->_startAcquisition();
	if (!ret){
		cout << "ERROR: Fail to Start Image Acquisition" << endl;
		Py_RETURN_FALSE;
	}

	// 为待处理图像申请内存
	g_CameraHandle->_getImgInfo(&height, &width, &bitPerPixel);

	if (bitPerPixel == 8 || bitPerPixel == 24){
		g_Buffer = new UCHAR[height*width*bitPerPixel / 8];
		channels = bitPerPixel / 8;
	}
	else if (bitPerPixel == 16 || bitPerPixel == 48){
		g_Buffer = new UCHAR[height*width*bitPerPixel / 8];
		channels = bitPerPixel / 16;
	}

	cout << "Image Acquisition is Started " << endl;
	Py_RETURN_TRUE;
}

static PyObject * StopImageAcquisition_Py(PyObject *self, PyObject *args)
{
	bool ret;
	ret = g_CameraHandle->_stopAcquisition();
	if (!ret){
		cout << "ERROR: Fail to Stop Image Acquisition" << endl;
	}
	else{
		cout << "Image Acquisition is Stopped " << endl;
	}

	// 释放待处理图像内存
	if (g_Buffer != NULL){
		delete g_Buffer;
		g_Buffer = NULL;
	}

	if (ret)
		Py_RETURN_TRUE;

	Py_RETURN_FALSE;
}

static PyObject * TriggerShooting_Py(PyObject *self, PyObject *args)
{
	if (g_CameraHandle->_triggerShooting()){
		Py_RETURN_TRUE;
	}

	Py_RETURN_FALSE;
}

static PyObject * GetImage_Py(PyObject *self, PyObject *args)
{
	PyObject *PyArray = Py_None;
	bool ret;
	signed long timeOut;

	// 解析输入参数
	PyArg_ParseTuple(args, "l", &timeOut);

	ret = g_CameraHandle->_getImgBuf(g_Buffer, timeOut);
	if (!ret){
		// 获取图像失败
		Py_RETURN_NONE;
	}

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

	if (bitPerPixel == 8 || bitPerPixel == 24){
		PyArray = PyArray_SimpleNewFromData(dims, Dims, NPY_UBYTE, g_Buffer);
	}
	else if (bitPerPixel == 16 || bitPerPixel == 48){
		PyArray = PyArray_SimpleNewFromData(dims, Dims, NPY_USHORT, g_Buffer);
	}
	else{
		delete Dims;
		Py_RETURN_NONE;
	}

	delete Dims;
	return PyArray;
}

static PyMethodDef SonyCameraMethods[] = {
	{ "OpenCamera", OpenCamera_Py, METH_NOARGS, "Function to open sony camera" },
	{ "CloseCamera", CloseCamera_Py, METH_NOARGS, "Function to close sony camera" },
	{ "StartImageAcquisition", StartImageAcquisition_Py, METH_NOARGS, "Function to start image acquisition" },
	{ "StopImageAcquisition", StopImageAcquisition_Py, METH_NOARGS, "Function to stop image acquisition" },
	{ "TriggerShooting", TriggerShooting_Py, METH_NOARGS, "Trigger zhe camera to take a photo, only to be used when the camera is in software trigger mode" },
	{ "GetImage", GetImage_Py, METH_VARARGS, "Function to get an image from camera(not directly from camera,but from image pool actually)" },
	{ NULL, NULL, 0, NULL }
};


//-------- python2 ------------------------
//PyMODINIT_FUNC initSonyCamera(void) {
//	(void)Py_InitModule("SonyCamera", SonyCameraMethods);
//	import_array();
//}
//------------------------------------------

//-------- python3 ------------------------
static struct PyModuleDef SonyCameraModule =
{
	PyModuleDef_HEAD_INIT,
	"SonyCamera", /* name of module */
	"",          /* module documentation, may be NULL */
	-1,          /* size of per-interpreter state of the module, or -1 if the module keeps state in global variables. */
	SonyCameraMethods
};

// Note that the name of the PyMODINIT_FUNC function must be of the form PyInit_<name> where <name> is the name of your module.
PyMODINIT_FUNC PyInit_SonyCamera(void)
{
	return PyModule_Create(&SonyCameraModule);
}
//------------------------------------------





#endif
/*--------------------------- Python接口-结束 ------------------------------------- */

