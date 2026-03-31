#pragma once
#include "Windows/WindowsHWrapper.h"
#include <iostream>
#include <unordered_map>
#include "LockFreeMemoryPoolLive.h"
#include "LFStack.h"
#include "GenericPlatform/GenericPlatformAtomics.h"


#define SUBPOOL_MAX 100
#define BUCKET_SIZE 1000
#define BIT_MASK    0x00007FFFFFFFFFFF

template<typename U>
class CMPoolTLS 
{
protected:

	class CSubPool;

	struct Node
	{
		U         s_data;
		Node*     s_pNext;
		int64     s_poolid;
	};

	struct Bucket
	{
		Node* s_Top;
		int32  s_Cnt;                                //��Ŷ�� ��� �ִ� ��� ����
	};

	struct SubPool
	{
		DWORD     s_ThreadId;
		CSubPool* s_ptr;
	};

public:
	LFStack<Bucket*>*    g_pBucketPool;                // ���� ��Ŷ Ǯ ������ �������� ����
	CMemoryPool<Bucket>* g_pCaseBucketPool;            // ��Ŷ ������ �������� Ǯ

protected:
	bool         m_bPlacementNew;
	int64        m_iOriginID;
	int32        m_iBKTCapacity;
	int64        m_iUseCnt;

	int32        m_iTlsIndex;

	int32        m_SubPoolIndex;
	SubPool      m_SubPoolArray[SUBPOOL_MAX];                //�� ����Ǯ ������ ���� �ڷᱸ��

public:
	CMPoolTLS(int iBucketNum = 0, bool bPlacementNew = false) : g_pBucketPool(nullptr), g_pCaseBucketPool(nullptr), m_SubPoolIndex(-1), m_bPlacementNew(bPlacementNew), 
		m_iBKTCapacity(0), m_iOriginID(-1), m_iUseCnt(0)
	{
		Bucket* pBucket = nullptr;

		m_iTlsIndex = TlsAlloc();

		if (m_iTlsIndex == TLS_OUT_OF_INDEXES)
		{
			wprintf(L"CMPoolTLS()_TlsAlloc Error : %d \n", (int)GetLastError());
			return;
		}

		//��� ���� �ʱ�ȭ
		m_iBKTCapacity = 0;
		m_iUseCnt = 0;


		//�޸� Ǯ ID ����. (����� �ּҰ��� ������)
		m_iOriginID = (int64)&m_iUseCnt;

		//���� ��Ŷ ������ ���� �ʱ�ȭ
		g_pBucketPool = new LFStack<Bucket*>;
		g_pBucketPool->Clear();

		g_pCaseBucketPool = new CMemoryPool<Bucket>;

		for (int i = 0; i < iBucketNum; i++)
		{
			pBucket = BucketAlloc();
			g_pBucketPool->Push(pBucket);
		}

	}
	~CMPoolTLS()
	{
		// ���� Ǯ�� ��� �ݳ�
		for (int i = 0; i < m_SubPoolIndex; i++)
		{
			if (m_SubPoolArray[i].s_ptr->m_iAlloc->s_Top != nullptr)
			{
				// �Ҵ� ��Ŷ�� �ִ� ��� �ݳ�
				while (1)
				{
					Node* next = m_SubPoolArray[i].s_ptr->m_iAlloc->s_Top->s_pNext;
					
					if(m_bPlacementNew == false)
						m_SubPoolArray[i].s_ptr->m_iAlloc->s_Top->s_data.~U();

					free(m_SubPoolArray[i].s_ptr->m_iAlloc->s_Top);
					m_SubPoolArray[i].s_ptr->m_iAlloc->s_Top = next;

					if (m_SubPoolArray[i].s_ptr->m_iAlloc->s_Top == nullptr)
						break;
				}

			}

			if (m_SubPoolArray[i].s_ptr->m_iRet->s_Top != nullptr)
			{
				// ��ȯ ��Ŷ�� �ִ� ��� �ݳ�
				while (1)
				{
					Node* next = m_SubPoolArray[i].s_ptr->m_iRet->s_Top->s_pNext;

					if (m_bPlacementNew == false)
						m_SubPoolArray[i].s_ptr->m_iRet->s_Top->s_data.~U();

					free(m_SubPoolArray[i].s_ptr->m_iRet->s_Top);
					m_SubPoolArray[i].s_ptr->m_iRet->s_Top = next;

					if (m_SubPoolArray[i].s_ptr->m_iRet->s_Top == nullptr)
						break;
				}

			}

			// �Ҵ� ��Ŷ ������ �ݳ�
			g_pCaseBucketPool->Free(m_SubPoolArray[i].s_ptr->m_iAlloc);

			// ��ȯ ��Ŷ ������ �ݳ�
			g_pCaseBucketPool->Free(m_SubPoolArray[i].s_ptr->m_iRet);
		}

		// ���� Ǯ�� ��� �ݳ�
		while (1)
		{
			// ���� Ǯ���� ��Ŷ �ϳ� �̱�
			Bucket* pBucket = nullptr;
			if (!g_pBucketPool->Pop(pBucket))
				break;

			// ��Ŷ �ϳ����� ��� �̾Ƽ� �ݳ�
			while (1)
			{
				Node* next = pBucket->s_Top->s_pNext;

				if (m_bPlacementNew == false)
					pBucket->s_Top->s_data.~U();

				free(pBucket->s_Top);
				pBucket->s_Top = next;

				if (pBucket->s_Top == nullptr)
					break;
			}

			// ��Ŷ ������ ��ȯ
			g_pCaseBucketPool->Free(pBucket);
		}


		delete g_pBucketPool;
		delete g_pCaseBucketPool;
		g_pBucketPool = nullptr;
		g_pCaseBucketPool = nullptr;
	}

	Bucket* BucketAlloc()
	{

		Bucket* pBucket = g_pCaseBucketPool->Alloc();
		pBucket->s_Top = nullptr;
		pBucket->s_Cnt = BUCKET_SIZE;

		//��Ŷ�� ��� �Ҵ�
		for (int i = 0; i < BUCKET_SIZE; i++)
		{
			//��� ���� �� �ʱ�ȭ
			Node* newNode = (Node*)malloc(sizeof(Node));
			newNode->s_poolid = m_iOriginID;
			newNode->s_pNext = nullptr;

			//��ü ������ ȣ��
			if(m_bPlacementNew == false)
				new(&newNode->s_data) U;

			newNode->s_pNext = pBucket->s_Top;
			pBucket->s_Top = newNode;

		}

		FPlatformAtomics::InterlockedIncrement(&m_iBKTCapacity);

		return pBucket;
	}

	//////////////////////////////////////////////////////////////////////////
    // ���� Ȯ�� �� ��� ������ ��´�. (�޸�Ǯ ������ ��ü ����)
    //
    // Parameters: ����.
    // Return: (int) �޸� Ǯ ���� ��ü ����
    //////////////////////////////////////////////////////////////////////////
	inline int32 GetCapacityCnt()
	{
		return m_iBKTCapacity;
	}

	//////////////////////////////////////////////////////////////////////////
	//
	//  �� ���� Ǯ�� UseCnt �ջ�.
	//
	//////////////////////////////////////////////////////////////////////////
	inline int64 GetUseCnt()
	{
		return m_iUseCnt;
	}


	//////////////////////////////////////////////////////////////////////////
	// ��� �ϳ��� �Ҵ�޴´�.  
	//
	// Parameters: ����.
	// Return: (DATA *) ����Ÿ ��� ������.
	//////////////////////////////////////////////////////////////////////////
	U* Alloc()
	{
		CSubPool* ret;
		int32      retindex;
		ret = (CSubPool*)TlsGetValue(m_iTlsIndex);
		if (ret == nullptr)
		{
			ret = new CSubPool(this);
			retindex = FPlatformAtomics::InterlockedIncrement(&m_SubPoolIndex);
			if (retindex >= SUBPOOL_MAX)
			{
				delete ret;
				return nullptr;
			}

			TlsSetValue(m_iTlsIndex, ret);
			m_SubPoolArray[retindex].s_ptr = ret;
			m_SubPoolArray[retindex].s_ThreadId = GetCurrentThreadId();
		}


		FPlatformAtomics::InterlockedIncrement((int64*) & m_iUseCnt);
		return ret->Alloc();
	}


	//////////////////////////////////////////////////////////////////////////
	// ������̴� ����� �����Ѵ�.
	//
	// Parameters: (DATA *) ��� ������.
	// Return: (BOOL) TRUE, FALSE.
	//////////////////////////////////////////////////////////////////////////
	bool Free(U* pData)
	{
		CSubPool* ret;
		int32      retindex;

		ret = (CSubPool*)TlsGetValue(m_iTlsIndex);
		if (ret == nullptr)
		{
			//�ٸ� �����忡�� Alloc�Ѱ� �� �����忡�� ���� Free ȣ�� �� �� ����.
			ret = new CSubPool(this);

			retindex = FPlatformAtomics::InterlockedIncrement(&m_SubPoolIndex);
			if (retindex >= SUBPOOL_MAX)
			{
				delete ret;
				return false;
			}
			TlsSetValue(m_iTlsIndex, ret);
			m_SubPoolArray[retindex].s_ptr = ret;
			m_SubPoolArray[retindex].s_ThreadId = GetCurrentThreadId();

		}

		FPlatformAtomics::InterlockedDecrement((volatile int64*) & m_iUseCnt);

		return (ret->Free(pData));
	}
	
	class CSubPool
	{
		using Bucket = CMPoolTLS<U>::Bucket;
		using Node = CMPoolTLS<U>::Node;

	public:
		Bucket*    m_iAlloc;    //�Ҵ� ��Ŷ ������ ����
		Bucket*    m_iRet;      //��ȯ�� ��Ŷ ������ ����
		CMPoolTLS* m_parent;
		int32        m_AllocCnt;
		int32        m_FreeCnt;

	public:
		CSubPool(CMPoolTLS* parent) : m_parent(parent)
		{
			//��Ŷ �Ҵ� �ϱ�
			if (!m_parent->g_pBucketPool->Pop(m_iAlloc))
			{
				// ���� ��Ŷ ������ ���ÿ� ��Ŷ* ���Ұ� ���� ���
				// ��Ŷ�� �ٷ� �����ؼ� �����ع�����
				m_iAlloc = m_parent->BucketAlloc();
			}

			//SubPool�� MemoryPoolTLS ������ ȣ�� �Ŀ� �����Ǳ� ������ ���� ���� ���ص� ��.
			m_iRet = m_parent->g_pCaseBucketPool->Alloc();
			m_iRet->s_Cnt = 0;
			m_iRet->s_Top = nullptr;
			m_AllocCnt = 0;
			m_FreeCnt = 0;
		}
		~CSubPool()
		{

		}

		U* Alloc()
		{
			Node* temp;

			if (m_iAlloc->s_Top == nullptr)
			{

				m_parent->g_pCaseBucketPool->Free(m_iAlloc);

				//��Ŷ�� �ִ� ��� �� ���� ��� ���� ��Ŷ ������ ���ÿ��� Pop�ؼ� ��������
				if (!m_parent->g_pBucketPool->Pop(m_iAlloc))
				{
					m_iAlloc = m_parent->BucketAlloc();
				}

			}

			m_iAlloc->s_Cnt--;

			//���� ��Ŷ�� top �ӽ� ����
			temp = m_iAlloc->s_Top;

			//top ����
			m_iAlloc->s_Top = temp->s_pNext;

			m_AllocCnt++;

			//placement == true�� ������ ȣ��
			if(m_parent->m_bPlacementNew == true)
				new(&temp->s_data) U;

			return &(temp->s_data);
		}

		bool Free(U* pData)
		{
			//��ȯ���� ��ü �ּҸ� ���� ���� �޸� Ǯ ��� �ּ� ���ϱ�
			Node* newNode = (Node*)pData;

			//�ٸ� �޸�Ǯ�� ��带 ��ȯ���� �� �׳� false return�ϱ�
			if (newNode->s_poolid != m_parent->m_iOriginID)
				return false;

			//bPlacementNew üũ�ؼ� true �� �Ҹ��� ȣ�����ֱ�
			if (m_parent->m_bPlacementNew == true)
				pData->~U();
			

			//���� ��Ŷ�� �ְ� ���ѷ� �Ѿ����� ��ȯ�� ��Ŷ�� �ֱ�

			//���� ��Ŷ ��뷮 üũ
			if (m_iAlloc->s_Cnt == BUCKET_SIZE)
			{
				//��Ŷ�� ���� �� á���� ��ȯ ��Ŷ�� �ֱ�

				//��ȯ ��Ŷ�� ���� �� ���࿡ ��Ŷ �� á���� ������ ���ÿ� ��Ŷ ������ �־��ֱ�
				if (m_iRet->s_Cnt == BUCKET_SIZE)
				{
					m_parent->g_pBucketPool->Push(m_iRet);

					//���ο� ��Ŷ ������ ��������
					m_iRet = m_parent->g_pCaseBucketPool->Alloc();
					m_iRet->s_Cnt = 0;
					m_iRet->s_Top = nullptr;
				}

				//���� ��ȯ ��Ŷ�� ��� �ְ� ��
				m_iRet->s_Cnt++;
				newNode->s_pNext = m_iRet->s_Top;
				m_iRet->s_Top = newNode;
				m_FreeCnt++;

				return true;
			}



			//���� ��Ŷ �� á���� head, cnt ����
			newNode->s_pNext = m_iAlloc->s_Top;

			m_iAlloc->s_Top = newNode;
			m_iAlloc->s_Cnt++;

			m_FreeCnt++;
			return true;
		}

	};

};


