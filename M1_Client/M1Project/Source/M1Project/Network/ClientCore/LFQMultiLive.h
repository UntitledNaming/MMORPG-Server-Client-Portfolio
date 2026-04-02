#pragma once
#include "LockFreeMemoryPoolLive.h"
#include "GenericPlatform/GenericPlatformAtomics.h"

#define LOG_BUFFER_SIZE 5000
#define MAX_LEN         300
#define USER_MEMORY_MAX 0x00007FFFFFFFFFFF
#define BITMASK         0x00007FFFFFFFFFFF
#define TAGMASK         0xFFFF800000000000


template<typename T>
class LFQueueMul
{
private:
	struct Node
	{
		T                               _data;
		alignas(16)Node*                _next;
		int64                          _Qid;
	};

	struct CmpNode
	{
		Node*                           s_next;
		int64                          s_Qid;
	};

private:
	Node*                               m_pHead;
	Node*                               m_pTail;
	int32                               m_size;
	int64                              m_HeadCnt;
	int64                              m_TailCnt;
	int64                              m_Qid;

public:
	static CMemoryPool<Node>*           m_pMemoryPool;
	static LONG                         m_refCnt;


public:
	LFQueueMul(int size = 0)
	{
		//�ּ� bit üũ
		SYSTEM_INFO info;
		GetSystemInfo(&info);

		if (!((UINT64)info.lpMaximumApplicationAddress & USER_MEMORY_MAX))
		{
			wprintf(L"UserMemory Address for Tag bit is not 17Bit\n");
			__debugbreak();
		}

		// static �޸� Ǯ ���� Ȯ��
		if (FPlatformAtomics::InterlockedIncrement(&m_refCnt) == 1)
			m_pMemoryPool = new CMemoryPool<Node>;
		else
		{
			while (m_pMemoryPool == nullptr)
			{

			}
		}


		//��� ���� �ʱ�ȭ
		m_size = size;
		m_HeadCnt = 0;
		m_TailCnt = 0;
		m_Qid = (uint64)&m_size;


		//���� ��� 1�� ����
		Node* dmyNode = m_pMemoryPool->Alloc();
		dmyNode->_next = nullptr;
		dmyNode->_Qid = m_Qid;


		m_pHead = (Node*)((uint64)dmyNode | (FPlatformAtomics::InterlockedIncrement64((int64*)&m_HeadCnt) << 47));
		m_pTail = (Node*)((uint64)dmyNode | (FPlatformAtomics::InterlockedIncrement64((int64*)&m_TailCnt) << 47));

	}
	~LFQueueMul()
	{
		if (InterlockedDecrement(&m_refCnt) == 0)
		{
			delete m_pMemoryPool;
			m_pMemoryPool = nullptr;
		}
	}

	void Clear()
	{
		m_size = 0;
		m_HeadCnt = 0;
		m_TailCnt = 0;
	}

	void Enqueue(T InputParam)
	{
		Node*    newNode = nullptr;
		Node*    localTail = nullptr;
		Node*    localRealTail = nullptr;
		Node*    localTailNext = nullptr;
		uint64   retCnt;

		//�� ���
		CmpNode cmp;

		//�ű� ��� ����
		newNode = m_pMemoryPool->Alloc();
		newNode->_data = InputParam;
		newNode->_next = (Node*)0xFFFFFFFFFFFFFFFF;
		newNode->_Qid = m_Qid;


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
			cmp.s_Qid = m_Qid;
			cmp.s_next = nullptr;

			//_tail->next ���������� ���� �õ�
			if (FPlatformAtomics::InterlockedCompareExchange128((__int64*)&localRealTail->_next, (__int64)m_Qid, (__int64)newNode, (__int64*)&cmp) == 1)
			{
				newNode->_next = nullptr;

				newNode = (Node*)((uint64)newNode | (retCnt << 47));

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
		Node*    localRealTail = nullptr;
		Node*    realHeadNext = nullptr;
		Node*    localTail;
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

			localTailNext = (Node*)((int64)localTailNext | (retCntTail << 47));

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

			if (FPlatformAtomics::InterlockedCompareExchange((int64*)&m_pHead, (int64)localHeadNext, (int64)localHead) != (int64)localHead)
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
CMemoryPool<typename LFQueueMul<T>::Node>* LFQueueMul<T>::m_pMemoryPool = nullptr;

template <typename T>
LONG LFQueueMul<T>::m_refCnt = 0;