#include "Ring_Buffer.h"
#include "M1Project.h"
#include "GenericPlatform/GenericPlatformAtomics.h"
#pragma comment(lib, "winmm.lib")
#include <iostream>
#include "Windows/WindowsHWrapper.h"
#include <atomic>
#include <process.h>

//���ǻ�����.
//
// recv üũ�ϴ� ���۴� �� ���� ���� Ŀ�� 



#pragma region RingBuffer


CRingBuffer::CRingBuffer()
{
	_size = DEFAULT_SIZE; //����Ʈ�� �� ���� ũ�� 100����Ʈ�� ����
	_usingSize = 0;

	_allocPos = (char*)malloc(_size);
	_readPos = _allocPos;
	_writePos = _allocPos;

}

CRingBuffer::CRingBuffer(int iBufferSize)
{
	_size = iBufferSize;
	_usingSize = 0;

	_allocPos = (char*)malloc(_size);
	_readPos = _allocPos;
	_writePos = _allocPos;

}

CRingBuffer::~CRingBuffer()
{
	free(_allocPos);
}

// �����ۿ� ������ ������ ũ�Ⱑ �����ۿ� ���� ������ ���� Ŭ ���� 
// ���� writePos�� �Ű��� �� readPos�� ��ġ�� 0�� ��ȯ�ؼ� Enqueue�۾��� ���� ����.
int CRingBuffer::Enqueue(const char* chpData, int iSize)
{
	//�����ۿ� ������ ������ ũ�Ⱑ ���� ������ ũ�⺸�� Ŭ ��
	if (iSize >= GetFreeSize())
		return 0;

	long long des;
	__int64 leftenq;


	des = DirectEnqueueSize();

	//���� Enqueue �۾�
	if (iSize <= des)
	{
		memcpy_s(_writePos, iSize, chpData, iSize);
		_writePos += iSize;

		if (_writePos == _allocPos + _size)
			_writePos = _allocPos;

	}


	//���� ũ�Ⱑ DES���� ũ�� �ѹ� �� �۾� �ؾ� ��
	else
	{
		leftenq = iSize - des;

		memcpy_s(_writePos, des, chpData, des);
		_writePos = _allocPos;

		memcpy_s(_writePos, leftenq, chpData + des, leftenq);
		_writePos += leftenq;
	}

	_usingSize += iSize;

	return iSize;
}
// ��ť�۾��ϰ� ���� readPos�� ����Ǵ� ��ġ�� writePos�� ������ ���ʿ� �׳� Dequeue �۾� ���� ����.
int CRingBuffer::Dequeue(char* chpData, int iSize)
{
	long long dds;
	__int64 left;
	int len;

	if (iSize > GetUseSize())
		return 0;

	len = iSize;


	dds = DirectDequeueSize();

	//�װ� �ƴϸ� �׳� ���� ũ�⸸ŭ�� ���� ���� ��.
	if (len <= dds)
	{

		memcpy_s(chpData, len, _readPos, len);

		_readPos = _readPos + len;

		if (_readPos == _allocPos + _size)
			_readPos = _allocPos;

	}

	//Ȯ���� ���� ũ�Ⱑ DirectDequeueSize���� ũ�� ������ �̵��ؼ� ���� �ѹ� �� ���ƾ� ��.
	else
	{
		left = len - dds;


		memcpy_s(chpData, dds, _readPos, dds);
		_readPos = _allocPos;

		memcpy_s(chpData + dds, left, _readPos, left);
		_readPos = _readPos + left;

	}


	_usingSize -= len;

	return len;
}

int CRingBuffer::Peek(char* chpData, int iSize)
{
	long long dds;
	__int64 left;

	if (iSize > GetUseSize())
		return 0;

	dds = DirectDequeueSize();

	//�װ� �ƴϸ� �׳� ���� ũ�⸸ŭ�� ���� ���� ��.
	if (iSize <= dds)
	{
		memcpy_s(chpData, iSize, _readPos, iSize);
		
	}

	//Ȯ���� ���� ũ�Ⱑ DirectDequeueSize���� ũ�� ������ �̵��ؼ� ���� �ѹ� �� ���ƾ� ��.

	else
	{
		left = iSize - dds;

		memcpy_s(chpData, dds, _readPos, dds);
		memcpy_s(chpData + dds, left, _allocPos, left);

	}

	return iSize;
}

#pragma endregion

