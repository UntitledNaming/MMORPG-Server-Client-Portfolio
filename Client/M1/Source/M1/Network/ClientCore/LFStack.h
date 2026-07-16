#pragma once
#define MAX_LEN         300
#define USER_MEMORY_MAX 0x00007FFFFFFFFFFF
#define BIT_MASK        0x00007FFFFFFFFFFF

#include "LockFreeMemoryPoolLive.h"
#include "GenericPlatform/GenericPlatformAtomics.h"


using namespace std;


template <typename T>
class LFStack
{
private:
	struct Node
	{
		T        data;
		Node*    pNextNode;
	};

public:
	LFStack(int size = 0)
	{
		SYSTEM_INFO info;
		GetSystemInfo(&info);

		if (!((int64)info.lpMaximumApplicationAddress & USER_MEMORY_MAX))
		{
			wprintf(L"UserMemory Address for Tag bit is not 17Bit\n");
			__debugbreak();
		}

		//�޸� Ǯ �Ҵ�
		m_pMemoryPool = new CMemoryPool<Node>(size);

		m_pTopNode = nullptr;
		m_topCnt = 0;
		m_size = 0;

	};
	~LFStack()
	{
		delete m_pMemoryPool;
		m_pMemoryPool = nullptr;
	};


	void Clear()
	{
		T temp;
		while (Pop(temp))
		{

		}

		m_pTopNode = nullptr;
		m_size = 0;
	}

	void Push(T InputData)
	{
		//�޸� �α� �غ�
		DWORD    curID = GetCurrentThreadId();
		Node*    newNode = m_pMemoryPool->Alloc();
		Node*    t;
		Node*    real;
		int64   retCnt = FPlatformAtomics::InterlockedIncrement(&m_topCnt);

		newNode->data = InputData;

		do {
			//CAS �����ϸ� newNode�� ���� tag����
			newNode = (Node*)(((int64)newNode << 17) >> 17);


			t = m_pTopNode; 
			real = (Node*)((int64)t & BIT_MASK);
			newNode->pNextNode = real;
			newNode = (Node*)((int64)newNode | (retCnt << 47));

		} while (FPlatformAtomics::InterlockedCompareExchange((volatile int64*)&m_pTopNode, (int64)newNode, (int64)t) != (int64)t);


		FPlatformAtomics::InterlockedIncrement((volatile int64*) & m_size);
	}

	//Data�� OutParameter��.
	bool Pop(T& Data)
	{
		//�޸� �α� �غ�
		DWORD curID = GetCurrentThreadId();

		Node* t;
		Node* real;
		Node* newTopNode;

		int64 retCnt = FPlatformAtomics::InterlockedIncrement(&m_topCnt);


		do {
			t = m_pTopNode; //���� ž ��� ����

			real = (Node*)((int64)t & BIT_MASK);
			if (real == nullptr)
			{
				return false;
			}


			newTopNode = real->pNextNode;
			newTopNode = (Node*)((int64)newTopNode | (retCnt << 47));


		} while (FPlatformAtomics::InterlockedCompareExchange((volatile int64*)&m_pTopNode, (int64)newTopNode, (int64)t) != (int64)t);


		//ž ��� ����
		Data = real->data;
		if (!(m_pMemoryPool->Free(real)))
			__debugbreak();


		FPlatformAtomics::InterlockedDecrement((volatile int64*) & m_size);

		return true;
	}
	bool IsEmpty()
	{
		if (m_size == 0)
			return true;

		return false;
	}

	inline int64 GetUseCnt() { return m_size; }

private:
	Node*                               m_pTopNode;
	int64                               m_size;
	int64                               m_topCnt;
	CMemoryPool<Node>*                  m_pMemoryPool;
};



