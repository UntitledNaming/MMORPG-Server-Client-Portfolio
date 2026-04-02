#pragma once
#include "LockFreeMemoryPoolLive.h"
#include "GenericPlatform/GenericPlatformAtomics.h"

#define LOG_BUFFER_SIZE 5000
#define MAX_LEN         300
#define USER_MEMORY_MAX 0x00007FFFFFFFFFFF
#define BITMASK         0x00007FFFFFFFFFFF
#define TAGMASK         0xFFFF800000000000


template<typename T>
class LFQueue
{
private:
	struct Node
	{
		T     _data;
		Node* _next;
	};

private:
	Node*                  m_pHead;
	Node*                  m_pTail;
	int32                  m_size;
	int64                  m_HeadCnt;
	int64                  m_TailCnt;

	static CMemoryPool<Node>*     m_pMemoryPool;


public:
	LFQueue(int size = 0) 
	{
		//�ּ� bit üũ
		SYSTEM_INFO info;
		GetSystemInfo(&info);

		if (!((int64)info.lpMaximumApplicationAddress & USER_MEMORY_MAX))
		{
			wprintf(L"UserMemory Address for Tag bit is not 17Bit\n");
			__debugbreak();
		}

		//��� ���� �ʱ�ȭ
		m_size = size;
		m_HeadCnt = 0;
		m_TailCnt = 0;

		if (m_pMemoryPool == nullptr)
			m_pMemoryPool = new CMemoryPool<Node>(size);


		//���� ��� 1�� ����
		Node* dmyNode = m_pMemoryPool->Alloc();
		dmyNode->_next = nullptr;

		m_pHead = (Node*)((int64)dmyNode | (FPlatformAtomics::InterlockedIncrement((volatile int64*)&m_HeadCnt) << 47));
		m_pTail = (Node*)((int64)dmyNode | (FPlatformAtomics::InterlockedIncrement((volatile int64*)&m_TailCnt) << 47));

	}
	~LFQueue()
	{
		delete m_pMemoryPool;
		m_pMemoryPool = nullptr;
	}

	void Clear()
	{
		m_size = 0;
		m_HeadCnt = 0;
		m_TailCnt = 0;
	}

	void Enqueue(T InputParam)
	{
		Node*    newNode;
		Node*    localTail;
		Node*    localTailNext;
		Node*    localRealTail;
		int64   retCnt;


		//�ű� ��� ����
		newNode = m_pMemoryPool->Alloc();
		newNode->_data = InputParam;
		newNode->_next = (Node*)0xFFFFFFFFFFFFFFFF;
		
		retCnt = FPlatformAtomics::InterlockedIncrement(&m_TailCnt);

		//���� �۾�
		while (1)
		{
			localTail = m_pTail;
			localRealTail = (Node*)((int64)localTail & BITMASK);
			localTailNext = localRealTail->_next;

			if (localTailNext == nullptr)
				break;

			if (localTailNext == (Node*)0xFFFFFFFFFFFFFFFF)
				continue;

			localTailNext = (Node*)((int64)localTailNext | (retCnt << 47));

			//next�� nullptr�� �ƴ϶�� tail�� �ٲ���.
			if (FPlatformAtomics::InterlockedCompareExchange((int64*)&m_pTail, (int64)localTailNext, (int64)localTail) == (int64)localTail)
			{
				retCnt = FPlatformAtomics::InterlockedIncrement(&m_TailCnt);
			}

		}

		//CAS �۾�
		while (1)
		{
			localTail = m_pTail;
			localRealTail = (Node*)((int64)localTail & BITMASK);


			//_tail->next ���������� ���� �õ�
			if (FPlatformAtomics::InterlockedCompareExchange((int64*)&localRealTail->_next, (int64)newNode, (int64)nullptr) == (int64)nullptr)
			{
				newNode->_next = nullptr;
				newNode = (Node*)((int64)newNode | (retCnt << 47));

				//�����ϸ� tail�� ���������� ����
				FPlatformAtomics::InterlockedCompareExchange((int64*)&m_pTail, (int64)newNode, (int64)localTail);
				break;
			}
		}


		//size ������ ��� ó�� ������
		FPlatformAtomics::InterlockedIncrement(&m_size);
	}


	bool Dequeue(T& OutputParam)
	{
		Node*    localHead = nullptr;
		Node*    localHeadNext = nullptr;
		Node*    realHead = nullptr;
		Node*    realHeadNext = nullptr;
		Node*    localTail;
		Node*    localRealTail;
		Node*    localTailNext;
		int64   retCntHead;
		int64   retCntTail;

		retCntTail = FPlatformAtomics::InterlockedIncrement(&m_TailCnt);

		//���� �۾�
		while (1)
		{
			localTail = m_pTail;
			localRealTail = (Node*)((int64)localTail & BITMASK);
			localTailNext = localRealTail->_next;

			if ((localTailNext == nullptr))
				break;

			if (localTailNext == (Node*)0xFFFFFFFFFFFFFFFF)
				continue;

			localTailNext = (Node*)((UINT64)localTailNext | (retCntTail << 47));

			//next�� nullptr�� �ƴ϶�� tail�� �ٲ���.
			if (FPlatformAtomics::InterlockedCompareExchange((int64*)&m_pTail, (int64)localTailNext, (int64)localTail) == (int64)localTail)
			{
				retCntTail = FPlatformAtomics::InterlockedIncrement(&m_TailCnt);

			}

		}

		retCntHead = FPlatformAtomics::InterlockedIncrement(&m_HeadCnt);

		while (1)
		{
			localHead = m_pHead;
			realHead = (Node*)((int64)localHead & BITMASK);
			realHeadNext = realHead->_next;
			if (realHeadNext == nullptr)
				return false;


			localHeadNext = (Node*)((int64)realHeadNext | (retCntHead << 47));

			if (FPlatformAtomics::InterlockedCompareExchange((volatile __int64*)&m_pHead, (__int64)localHeadNext, (__int64)localHead) != (UINT64)localHead)
				continue;

			break;
		}


		//������ ��ȯ
		localHeadNext = (Node*)((int64)localHeadNext & BITMASK);

		OutputParam = localHeadNext->_data;

		//��� ����
		if (!m_pMemoryPool->Free(realHead))
			__debugbreak();

		FPlatformAtomics::InterlockedDecrement(&m_size);

		return true;
	}

	inline int GetUseSize()
	{
		return m_size;
	}

};

template <typename T>
CMemoryPool<typename LFQueue<T>::Node>* LFQueue<T>::m_pMemoryPool = nullptr;