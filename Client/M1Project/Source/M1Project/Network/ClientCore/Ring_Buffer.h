#pragma once

#define DEFAULT_SIZE 10000

class CRingBuffer
{
private:
	int _size; //�� ũ��
	int _usingSize; // ��� ���� ũ��
	char* _writePos; // �����ϱ� ������ ��ġ
	char* _readPos; // ������ ������ ��ġ
	char* _allocPos; // �Ҵ� ��ġ

public:
	CRingBuffer();
	CRingBuffer(int iBufferSize);
	~CRingBuffer();
	inline int GetBufferSize()
	{
		return _size;
	}


	//���� ������� �뷮 ���
	inline __int64 GetUseSize()
	{
		char* oldWritePos = _writePos;
		char* oldReadPos = _readPos;


		if (oldWritePos >= oldReadPos)
		{
			return oldWritePos - oldReadPos;
		}

		return (oldWritePos - _allocPos) + (_allocPos + _size - oldReadPos);

	}

	//���� ���ۿ� ���� �뷮 ���
	inline __int64 GetFreeSize()
	{
		char* oldWritePos = _writePos;
		char* oldReadPos = _readPos;


		if (oldWritePos >= oldReadPos)
		{

			return (oldReadPos - _allocPos) + (_allocPos + _size - oldWritePos);
		}

		return (oldReadPos - oldWritePos);
	}


	//WritePos�� �����͸� ����.
	//��ȯ���� ���� ������ ũ��, �����ۿ� ���� ũ�⺸�� iSize Ŭ �� 0��ȯ
	int Enqueue(const char* chpData, int iSize);

	//ReadPos���� �����͸� ������
	//��ȯ�� : ������ ũ��, 
	int Dequeue(char* chpData, int iSize);


	//ReadPos���� �����͸� �о��. ReadPos�� �ȿ�����
	//��ȯ�� : ������ ũ��
	int Peek(char* chpDatat, int iSize);

	//������ ��� ������ ����
	inline void Clear()
	{
		_writePos = _allocPos;
		_readPos = _allocPos;
		_usingSize = 0;
	}


	//���� �����۴� 1�������� �̾��� ���� ����. �׷��� WritePos���� Enqueue�� �ϸ鼭 WritePos�� �̵��ϴµ�
	//�Ҵ� ���� �޸� �ּұ��� �̵��ϸ� �ٽ� �� �� �޸𸮷� �̵��ؾ� ��. �� �� WritePos���� �Ҵ� ���� �޸� �������� ���̰ų�
	// 1���� �޸� �迭 �� WritePos�� ReadPos �տ� �� �� ����. �̷��� ReadPos�� WritePos ���� ����
	// DirectDequeueSize�� ����������
	//��ȯ�� : ��밡���� �뷮
	inline __int64 DirectEnqueueSize()
	{
		char* oldReadPos = _readPos;
		char* oldWritePos = _writePos;

		if (_size == GetUseSize())
			return 0;

		if (oldWritePos < oldReadPos)
			return (oldReadPos - oldWritePos);

		return (_allocPos + _size - oldWritePos);
	}
	inline __int64 DirectDequeueSize()
	{
		char* oldReadPos = _readPos;
		char* oldWritePos = _writePos;

		long long useSize = GetUseSize();

		if (useSize == 0)
			return 0;

		if (oldReadPos < oldWritePos)
		{

			return (oldWritePos - oldReadPos);
		}

		return(_allocPos + _size - oldReadPos);
	}


	//���ϴ� ���̸�ŭ ReadPos���� �ش� ���� ��ŭ ����
	//���ϴ� ���̸�ŭ WritePos���� �ش� ���� ��ŭ �̵�
	//��ȯ�� : �̵� ũ��
	inline int MoveWritePos(int iSize)
	{
		__int64 left;

		_writePos += iSize;
		if (_writePos >= _allocPos + _size)
		{
			left = _writePos - (_allocPos + _size);
			_writePos = _allocPos + left;
		}

		_usingSize += iSize;

		return iSize;
	}

	inline int MoveReadPos(int iSize)
	{
		__int64 left;

		_readPos += iSize;
		if (_readPos >= _allocPos + _size)
		{
			left = _readPos - (_allocPos + _size);
			_readPos = _allocPos + left;
		}

		_usingSize -= iSize;

		return iSize;
	}


// ������ Front������ ȹ��
//��ȯ�� : ���� ������
	inline char* GetWritePtr()
	{
		return _writePos;
	}

	//������ ReadPos ������ ����.
	//��ȯ�� : ���� ������
	inline char* GetReadPtr()
	{
		return _readPos;
	}

	inline char* GetAllocPtr() { return _allocPos; }

};
