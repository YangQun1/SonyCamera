#pragma once


#include "stdafx.h"
#include <XCCamAPI.h>

#include <queue>


using namespace std;


#define Max_Buffer 5


/*
�����ݽṹʵ����һ��������ڴ�ء�
�����ݽṹ���ڲ�ά����һ��Max_Buffer*singleBufferSize��С���ڴ�أ���ʹ��һ������ά���ڴ�ص�˳��

��writerʹ���ڴ�ش洢���ݵ�ʱ������Ҫ���ڴ������һ���ڴ棬����ڴ�����п��е��ڴ棬��ֱ��
���ؿ����ڴ�ε��׵�ַ�������öα��Ϊռ�ã����򣬷��ض����еĶ�ͷԪ�أ����ȱ�ռ�õ��ڴ棬ʱ�����Զ����
��ͷԪ��ָ���ԭʼ���ڴ����ݽ������Ƕ�ʧ
writer���뵽�ڴ�ռ��ʹ���Լ����������ÿռ䣬��������뵽���еĶ�β��

reader���ǴӶ��еĶ��׶�ȡ���ݣ������ö��ڴ�Żص��ڴ�أ����Ϊδռ��

ʹ�ö���ά������ṹ��Ϊ��ʹ���ȱ�д������������ȱ�����

*/
template <class T>
class Sequence_Pool
{
public:
	Sequence_Pool(int singleBufferSize);
	Sequence_Pool(HCAMERA hCamera);
	~Sequence_Pool();

	T*		ReqBuffer();	// �򻺳�������һ��buffer�����ڴ洢����
	void	PushBack(T*);	// ���β����һ��Ԫ�أ�������д�õ����ݲ����β��ͬʱ���д�С��1
	T*		PopFront();		// �Ӷ�ͷȡ��һ��Ԫ�أ����ڶ�ȡ�Ѿ���д�õ����ݣ�ͬʱ���д�С��1
	void	RetBuffer(T*);	// ��һ���ڴ�黹��������
	bool	IsEmpty();		// �������Ƿ�Ϊ��
	int		QueueSize();	// ��ȡ���еĴ�С

private:
	queue<T*>	bufferQueue;				// ���ڹ���buffer
	T*			bufferPool[Max_Buffer];		// �ڴ��
	bool		bufferOccup[Max_Buffer];	// �ڴ���е��ڴ��Ƿ�ռ�ñ�־

	HANDLE		m_hCamera;
};

typedef Sequence_Pool<UCHAR>  Image_Pool_8Bit;
typedef Sequence_Pool<USHORT> Image_Pool_16Bit;


template <class T>
Sequence_Pool<T>::Sequence_Pool(int singleBufferSize)
{
	for (int i = 0; i < Max_Buffer; i++){
		bufferPool[i] = new T[singleBufferSize];
		bufferOccup[i] = false;
	}
}

template<>
inline Sequence_Pool<XCCAM_IMAGE>::Sequence_Pool(HCAMERA hCamera)
{
	for (int i = 0; i < Max_Buffer; i++){
		XCCAM_ImageAlloc(hCamera, &bufferPool[i]);
		bufferOccup[i] = false;
	}

	m_hCamera = hCamera;
}

template <class T>
Sequence_Pool<T>::~Sequence_Pool()
{
	for (int i = 0; i < Max_Buffer; i++){
		delete[] bufferPool[i];
		bufferOccup[i] = false;
	}
}

template<>
inline Sequence_Pool<XCCAM_IMAGE>::~Sequence_Pool()
{
	for (int i = 0; i < Max_Buffer; i++){
		XCCAM_ImageFree(m_hCamera, bufferPool[i]);
		bufferOccup[i] = false;
	}
}

template <class T>
T* Sequence_Pool<T>::PopFront()
{
	// ����Ϊ���򷵻�NULL
	if (IsEmpty()){
		return NULL;
	}

	// ȡ����ͷԪ��
	T* temp = bufferQueue.front();
	bufferQueue.pop();

	return temp;
}

template <class T>
void Sequence_Pool<T>::RetBuffer(T* element)
{
	// ����Ԫ�طŻ��ڴ����
	for (int i = 0; i < Max_Buffer; i++){
		if (bufferPool[i] == element){
			bufferOccup[i] = false;
			break;
		}
	}

	return;
}

template <class T>
T* Sequence_Pool<T>::ReqBuffer()
{
	T *temp = NULL;

	// �����ڴ����ĳ��δ��ռ�õ�buffer
	for (int i = 0; i < Max_Buffer; i++){
		if (bufferOccup[i] == false){
			bufferOccup[i] = true;
			temp = bufferPool[i];
			break;
		}
	}

	if(temp == NULL){
		temp = bufferQueue.front();	// ���ض���Ԫ��
		bufferQueue.pop();			// �Ƴ�����Ԫ��
	}

	return temp;
}

//template <class T>
//T* Sequence_Pool<T>::ReqBuffer()
//{
//	T *temp = NULL;
//
//	// �����ڴ����ĳ��δ��ռ�õ�buffer
//	for (int i = 0; i < Max_Buffer; i++){
//		if (bufferOccup[i] == false){
//			bufferOccup[i] = true;
//			temp = bufferPool[i];
//			break;
//		}
//	}
//
//	return temp;
//}

template <class T>
void Sequence_Pool<T>::PushBack(T* element)
{
	// ע�⣺
	// �ڶ���û�е���󳤶ȵ�ʱ��Push��ȥ��Ԫ�ش��ڴ�ػ�ȡ
	// �ڶ��дﵽ��󳤶ȵ�ʱ��Push��ȥ��Ԫ���ǴӶ��׻�ȡ��
	// ��ˣ����г�����Զ�����ܳ�����󳤶ȣ�����Ҫ�����ȼ��
	bufferQueue.push(element);

	return;
}

template <class T>
bool Sequence_Pool<T>::IsEmpty()
{
	return bufferQueue.empty();
}

template <class T>
int Sequence_Pool<T>::QueueSize()
{
	return bufferQueue.size();
}