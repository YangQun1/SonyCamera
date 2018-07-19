#pragma once


#include "stdafx.h"
#include <XCCamAPI.h>

#include <queue>


using namespace std;


#define Max_Buffer 5


/*
该数据结构实现了一个有序的内存池。
该数据结构的内部维持了一个Max_Buffer*singleBufferSize大小的内存池，并使用一个队列维持内存池的顺序。

当writer使用内存池存储数据的时候，首先要向内存池申请一块内存，如果内存池中有空闲的内存，则直接
返回空闲内存段的首地址，并将该段标记为占用，否则，返回队列中的队头元素（最先被占用的内存，时间最久远），
队头元素指向的原始的内存数据将被覆盖丢失
writer申请到内存空间后，使用自己的数据填充该空间，并将其插入到队列的队尾。

reader总是从队列的队首读取数据，并将该段内存放回到内存池，标记为未占用

使用队列维持有序结构是为了使最先被写入的数据能最先被读出

*/
template <class T>
class Sequence_Pool
{
public:
	Sequence_Pool(int singleBufferSize);
	Sequence_Pool(HCAMERA hCamera);
	~Sequence_Pool();

	T*		ReqBuffer();	// 向缓冲区申请一块buffer，用于存储数据
	void	PushBack(T*);	// 向队尾插入一个元素，即将填写好的数据插入队尾，同时队列大小加1
	T*		PopFront();		// 从队头取出一个元素，用于读取已经填写好的数据，同时队列大小减1
	void	RetBuffer(T*);	// 将一块内存归还给缓冲区
	bool	IsEmpty();		// 检查队列是否为空
	int		QueueSize();	// 获取队列的大小

private:
	queue<T*>	bufferQueue;				// 用于管理buffer
	T*			bufferPool[Max_Buffer];		// 内存池
	bool		bufferOccup[Max_Buffer];	// 内存池中的内存是否被占用标志

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
	// 队列为空则返回NULL
	if (IsEmpty()){
		return NULL;
	}

	// 取出队头元素
	T* temp = bufferQueue.front();
	bufferQueue.pop();

	return temp;
}

template <class T>
void Sequence_Pool<T>::RetBuffer(T* element)
{
	// 将该元素放回内存池中
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

	// 返回内存池中某个未被占用的buffer
	for (int i = 0; i < Max_Buffer; i++){
		if (bufferOccup[i] == false){
			bufferOccup[i] = true;
			temp = bufferPool[i];
			break;
		}
	}

	if(temp == NULL){
		temp = bufferQueue.front();	// 返回队首元素
		bufferQueue.pop();			// 移除队首元素
	}

	return temp;
}

//template <class T>
//T* Sequence_Pool<T>::ReqBuffer()
//{
//	T *temp = NULL;
//
//	// 返回内存池中某个未被占用的buffer
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
	// 注意：
	// 在队列没有到最大长度的时候，Push进去的元素从内存池获取
	// 在队列达到最大长度的时候，Push进去的元素是从队首获取的
	// 因此，队列长度永远不可能超过最大长度，不需要做长度检查
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