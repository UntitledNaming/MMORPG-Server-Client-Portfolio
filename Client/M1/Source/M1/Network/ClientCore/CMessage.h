#pragma once
#include "MemoryPoolTLS.h"
#include "ContentsType.h"

class CMessage
{
public:
	enum en_Message
	{
		eBuffer_Default = 500, // ����ȭ ���� �⺻ ������
		eBuffer_Max = 10000,
	};

	CMessage();
	CMessage(int32 size);
	~CMessage();

public:

	static CMessage* Alloc();
	static bool      Free(CMessage* pMessage);
	static void      Init(int32 lanHeaderSize, int32 netHeaderSize);        // ����ȭ ���� �޸� Ǯ �����Ҵ� �ϱ�. ���ڰ����� ��Ʈ��ũ ��� ũ�� 1�� �ޱ�
	static void      PoolDestroy();
				     										                  
															                  
public:														                  
	void             Clear(int32 type = 0);                               // ����ȭ ���� ��ü �ʱ�ȭ(Lan ȯ��(1) or Net ȯ��(0) ���� ���� ����)
	void             SetNetHeader(int32 type);                            // ��Ʈ��ũ ����� ���� ���ۿ� �ֱ� ���� �Լ�. ��Ʈ��ũ ����� ���� ���� Ȯ����.
	void             SetEncodingFlag(int32 value);

	int32              GetRefCount();
	int32              GetEncodingFlag();
	int32              GetBufferSize();                                   // ��ȯ�� : ���� ������
	int32              GetDataSize();                                     // ��ȯ�� : ���� ������� ������ ũ��(��Ʈ��ũ ��� ����)
	int32              GetRealDataSize(int32 type = 0);                     // ��ȯ�� : ��Ʈ��ũ ��� ���� ũ��

	char*            GetReadPos();
	char*            GetWritePos();
	char*            GetAllocPos();

	bool             GetLastError();                                    // ������ �����ε����� ������ ������ üũ�ϴ� �Լ�
																        
	int32              MoveWritePos(int32 iSize);                           // ������ Pos �̵�
	int32              MoveReadPos(int32 iSize);
	int32              GetData(char* chpDest, int32 iSize);                 // ��ȯ�� : �ܺη� ������ ������ ũ��
	int32              PutData(char* chpSrc, int32 iSrcSize);               // ��ȯ�� : ����ȭ ���۷� ������ ������
																           
	bool             Resize();                                          // ����ȭ ���� ũ�� resize
																           
	int32              ResizeCount();                                     // resize Ƚ�� Ȯ�� �Լ�
	int32              AddRef();
	int32              SubRef();

	//������ �����ε� =
	CMessage& operator=(CMessage& clSrcMessage);

	//������ �����ε� << , ����ȭ ���ۿ� ������ �ֱ�
	CMessage& operator<<(uint8 byValue);
	CMessage& operator<<(bool bValue);
	CMessage& operator<<(char chValue);
	CMessage& operator<<(short shValue);
	CMessage& operator<<(uint16 wValue);
	CMessage& operator<<(int iValue);
	CMessage& operator<<(uint32 lValue);
	CMessage& operator<<(float fValue);
	CMessage& operator<<(__int64 iValue);
	CMessage& operator<<(uint64 iValue);
	CMessage& operator<<(double iValue);


	//������ �����ε� >> , ����ȭ ���ۿ��� ������ ����
	CMessage& operator >> (uint8& byValue);
	CMessage& operator >> (bool& bValue);
	CMessage& operator >> (char& chValue);
	CMessage& operator >> (short& shValue);
	CMessage& operator >> (uint16& wValue);
	CMessage& operator >> (int& iValue);
	CMessage& operator >> (uint32& dwValue);
	CMessage& operator >> (float& fValue);
	CMessage& operator >> (__int64& iValue);
	CMessage& operator >> (uint64& iValue);
	CMessage& operator >> (double& dValue);

public:
	static CMPoolTLS<CMessage>* m_pMessagePool;

	static int32                m_iNetHeaderSize;    // ��� ������
	static int32                m_iLanHeaderSize;    // ��� ������
											         
	static bool                 m_netHderFlag;       // ��Ʈ��ũ ��� ��� �÷���

private:
	CHAR*                       m_iAllocPtr;           // ����ȭ ���� �Ҵ� �޸� ������
	CHAR*                       m_iReadPos;
	CHAR*                       m_iWritePos;
		                        
	int32                       m_iBufferSize;       // ����ȭ ���� ũ��
	int32                       m_iDataSize;         // ���� ������� ũ��(��Ʈ��ũ ��� ����)
	int32                       m_iResizeCount;      // resize Ƚ��
	int32                       m_iRefCnt;	       
	int32                       m_EncodingFlag;      
			                      			       
	BOOL                        m_iError;            // ������ �����ε����� ���� Ž�� �÷���
};

