#include "CMessage.h"
#include "Windows/WindowsHWrapper.h"
#include <iostream>
#include <time.h>
#include "MemoryPoolTLS.h"
#include "LockFreeMemoryPoolLive.h"

#pragma warning(disable:4996)

CMPoolTLS<CMessage>* CMessage::m_pMessagePool = nullptr;
int32 CMessage::m_iNetHeaderSize = 0;
int32 CMessage::m_iLanHeaderSize = 0;
bool CMessage::m_netHderFlag = false;


//����ȭ ���� �����ڿ����� refCnt = 0���� �ϰ� Alloc �ϰ� Clear�� �� refCnt = 1�� ����.

CMessage::CMessage()
{
	m_iAllocPtr = (char*)malloc(eBuffer_Default);
	m_iReadPos = m_iAllocPtr;
	m_iWritePos = m_iAllocPtr;
	m_iRefCnt = 0;
	m_iBufferSize = eBuffer_Default;
	m_iDataSize = 0;
	m_iError = false;
	m_iResizeCount = 0;
	m_EncodingFlag = 0;
}

CMessage::CMessage(int size)
{
	if (size > eBuffer_Max)
		return;
	
	m_iAllocPtr = (char*)malloc(size);
	m_iReadPos = m_iAllocPtr;
	m_iWritePos = m_iAllocPtr;
	m_iRefCnt = 0;
	m_iBufferSize = size;
	m_iDataSize = 0;
	m_iError = false;
	m_iResizeCount = 0;
	m_EncodingFlag = 0;
}

CMessage::~CMessage()
{
	free(m_iAllocPtr);
	m_iAllocPtr = nullptr;
	m_iReadPos = nullptr;
	m_iWritePos = nullptr;
}

CMessage* CMessage::Alloc()
{
	return m_pMessagePool->Alloc();
}

bool CMessage::Free(CMessage* pMessage)
{
	int32 ret;
	ret = pMessage->SubRef();

	if (ret == 0)
	{
		m_pMessagePool->Free(pMessage);
		return true;
	}

	return false;
}

void CMessage::Init(int32 lanHeaderSize, int32 netHeaderSize)
{
	if (m_pMessagePool == nullptr)
	{
		m_pMessagePool = new CMPoolTLS<CMessage>;
	}

	m_iNetHeaderSize = netHeaderSize;
	m_iLanHeaderSize = lanHeaderSize;

	if (netHeaderSize != 0 || lanHeaderSize != 0)
	{
		m_netHderFlag = true;
	}
}

void CMessage::PoolDestroy()
{
	delete m_pMessagePool;
	m_pMessagePool = nullptr;
}

void CMessage::Clear(int32 type)
{
	m_iReadPos = m_iAllocPtr;
	m_iWritePos = m_iAllocPtr;
	m_iDataSize = 0;
	m_iRefCnt = 1;
	m_iError = false;
	m_EncodingFlag = 0;

	if (m_netHderFlag == true)
	{
		SetNetHeader(type);
	}
}

int32 CMessage::GetRefCount()
{
	return m_iRefCnt;
}

int32 CMessage::GetEncodingFlag()
{
	return m_EncodingFlag;
}

int32 CMessage::GetBufferSize()
{
	return m_iBufferSize;
}

int32 CMessage::GetDataSize()
{
	return m_iDataSize;
}

int32 CMessage::GetRealDataSize(int32 type)
{
	if (type == 0)
	{
		return m_iDataSize + m_iNetHeaderSize;
	}
	else if (type == 1)
	{
		return m_iDataSize + m_iLanHeaderSize;
	}

	return -1;
}

char* CMessage::GetReadPos()
{
	return m_iReadPos;
}

char* CMessage::GetWritePos()
{
	return m_iWritePos;
}

char* CMessage::GetAllocPos()
{
	return m_iAllocPtr;
}

bool CMessage::GetLastError()
{
	return m_iError;
}

void CMessage::SetEncodingFlag(int32 value)
{
	m_EncodingFlag = value;
}

void CMessage::SetNetHeader(int32 type)
{
	if (type == 0)
	{
		m_iReadPos = m_iAllocPtr + m_iNetHeaderSize;
		m_iWritePos = m_iAllocPtr + m_iNetHeaderSize;
	}
	else if (type == 1)
	{
		m_iReadPos = m_iAllocPtr + m_iLanHeaderSize;
		m_iWritePos = m_iAllocPtr + m_iLanHeaderSize;
	}
}

int32 CMessage::GetData(char* chpDest, int32 iSize)
{
	if (m_iDataSize < iSize)
	{
		m_iError = true;
		return 0;
	}

	memcpy_s(chpDest, iSize, m_iReadPos, iSize);

	m_iReadPos += iSize;
	m_iDataSize -= iSize;

	return iSize;
}

int32 CMessage::MoveWritePos(int32 iSize)
{
	if (iSize > (m_iBufferSize - m_iDataSize))
		return 0;


	m_iWritePos += iSize;
	m_iDataSize += iSize;

	return iSize;
}

int32 CMessage::MoveReadPos(int32 iSize)
{
	if (m_iDataSize < iSize)
		return 0;

	m_iReadPos += iSize;
	m_iDataSize -= iSize;

	return iSize;
}

int32 CMessage::PutData(char* chpSrc, int32 iSrcSize)
{
	if (iSrcSize > (m_iBufferSize - m_iDataSize))
		return 0;

	memcpy_s(m_iWritePos, iSrcSize, chpSrc, iSrcSize);

	m_iWritePos += iSrcSize;
	m_iDataSize += iSrcSize;

	return iSrcSize;
}

bool CMessage::Resize()
{
	if (m_iBufferSize * 2 >= eBuffer_Max)
		return false;

	// �ӽ� ���� ����
	char* pTemp = (char*)malloc(m_iBufferSize * 2); //2�� ū ����ȭ ���� ����
	memcpy_s(pTemp, m_iBufferSize, m_iAllocPtr, m_iBufferSize); //���� ����ȭ ���� ����
	
	__int64 tempWpos = m_iWritePos - m_iAllocPtr; //offset ���ϱ�
	__int64 tempRpos = m_iReadPos - m_iAllocPtr;

	//���� ���� ����
	free(m_iAllocPtr);

	//�ӽ� ���� �����͸� m_iBuffer ������ ���� �� ���� ũ�� �缳��
	m_iAllocPtr = pTemp;
	m_iBufferSize = m_iBufferSize * 2;
	m_iWritePos = m_iAllocPtr+ tempWpos;
	m_iReadPos = m_iAllocPtr + tempRpos;

	m_iResizeCount++;

	return true;
}

int32 CMessage::ResizeCount()
{
	return m_iResizeCount;
}

int32 CMessage::AddRef()
{
	return FPlatformAtomics::InterlockedIncrement(&m_iRefCnt);
}

int32 CMessage::SubRef()
{
	return FPlatformAtomics::InterlockedDecrement(&m_iRefCnt);
}


#pragma region ������ �����ε�
CMessage& CMessage::operator=(CMessage& clSrcMessage)
{
	m_iDataSize = clSrcMessage.GetDataSize();

	PutData(clSrcMessage.m_iReadPos, m_iDataSize);

	return *this;
}

CMessage& CMessage::operator<<(uint8 byValue)
{
	*(uint8*)m_iWritePos = byValue;
	m_iWritePos += sizeof(uint8);
	m_iDataSize += sizeof(uint8);


	return *this;
}

CMessage& CMessage::operator<<(bool bValue)
{
	*(bool*)m_iWritePos = bValue;
	m_iWritePos += sizeof(bool);
	m_iDataSize += sizeof(bool);


	return *this;
}

CMessage& CMessage::operator<<(char chValue)
{
	*(char*)m_iWritePos = chValue;
	m_iWritePos += sizeof(char);
	m_iDataSize += sizeof(char);

	return *this;
}

CMessage& CMessage::operator<<(short shValue)
{
	*(short*)m_iWritePos = shValue;
	m_iWritePos += sizeof(short);
	m_iDataSize += sizeof(short);

	return *this;
}

CMessage& CMessage::operator<<(uint16 wValue)
{
	*(uint16*)m_iWritePos = wValue;
	m_iWritePos += sizeof(uint16);
	m_iDataSize += sizeof(uint16);

	return *this;
}

CMessage& CMessage::operator<<(int iValue)
{
	*(int*)m_iWritePos = iValue;
	m_iWritePos += sizeof(int);
	m_iDataSize += sizeof(int);

	return *this;
}

CMessage& CMessage::operator<<(uint32 lValue)
{
	*(uint32*)m_iWritePos = lValue;
	m_iWritePos += sizeof(uint32);
	m_iDataSize += sizeof(uint32);

	return *this;
}

CMessage& CMessage::operator<<(float fValue)
{
	*(float*)m_iWritePos = fValue;
	m_iWritePos += sizeof(float);
	m_iDataSize += sizeof(float);

	return *this;
}

CMessage& CMessage::operator<<(__int64 iValue)
{
	*(__int64*)m_iWritePos = iValue;
	m_iWritePos += sizeof(__int64);
	m_iDataSize += sizeof(__int64);

	return *this;
}

CMessage& CMessage::operator<<(uint64 iValue)
{
	*(uint64*)m_iWritePos = iValue;
	m_iWritePos += sizeof(uint64);
	m_iDataSize += sizeof(uint64);

	return *this;
}


CMessage& CMessage::operator<<(double iValue)
{
	*(double*)m_iWritePos = iValue;
	m_iWritePos += sizeof(double);
	m_iDataSize += sizeof(double);

	return *this;
}

CMessage& CMessage::operator>>(uint8& byValue)
{
	if (m_iDataSize < sizeof(uint8))
	{
		m_iError = true;
		return *this;
	}

	byValue = *(uint8*)m_iReadPos;

	m_iReadPos += sizeof(uint8);
	m_iDataSize -= sizeof(uint8);

	return *this;
}

CMessage& CMessage::operator>>(bool& bValue)
{
	if (m_iDataSize < sizeof(bool))
	{
		m_iError = true;
		return *this;
	}

	bValue = *(bool*)m_iReadPos;

	m_iReadPos += sizeof(bool);
	m_iDataSize -= sizeof(bool);

	return *this;
}

CMessage& CMessage::operator>>(char& chValue)
{
	if (m_iDataSize < sizeof(char))
	{
		m_iError = true;
		return *this;
	}

	chValue = *(char*)m_iReadPos;

	m_iReadPos += sizeof(char);
	m_iDataSize -= sizeof(char);

	return *this;
}

CMessage& CMessage::operator>>(short& shValue)
{
	if (m_iDataSize < sizeof(short))
	{
		m_iError = true;
		return *this;
	}

	shValue = *(short*)m_iReadPos;

	m_iReadPos += sizeof(short);
	m_iDataSize -= sizeof(short);

	return *this;
}

CMessage& CMessage::operator>>(uint16& wValue)
{
	if (m_iDataSize < sizeof(uint16))
	{
		m_iError = true;
		return *this;
	}

	wValue = *(uint16*)m_iReadPos;

	m_iReadPos += sizeof(uint16);
	m_iDataSize -= sizeof(uint16);

	return *this;
}

CMessage& CMessage::operator>>(int& iValue)
{
	if (m_iDataSize < sizeof(int))
	{
		m_iError = true;
		return *this;
	}

	iValue = *(int*)m_iReadPos;

	m_iReadPos += sizeof(int);
	m_iDataSize -= sizeof(int);

	return *this;
}

CMessage& CMessage::operator>>(uint32& dwValue)
{
	if (m_iDataSize < sizeof(uint32))
	{
		m_iError = true;
		return *this;
	}

	dwValue = *(uint32*)m_iReadPos;

	m_iReadPos += sizeof(uint32);
	m_iDataSize -= sizeof(uint32);

	return *this;
}

CMessage& CMessage::operator>>(float& fValue)
{
	if (m_iDataSize < sizeof(float))
	{
		m_iError = true;
		return *this;
	}

	fValue = *(float*)m_iReadPos;

	m_iReadPos += sizeof(float);
	m_iDataSize -= sizeof(float);

	return *this;
}

CMessage& CMessage::operator>>(__int64& iValue)
{
	if (m_iDataSize < sizeof(__int64))
	{
		m_iError = true;
		return *this;
	}

	iValue = *(__int64*)m_iReadPos;

	m_iReadPos += sizeof(__int64);
	m_iDataSize -= sizeof(__int64);

	return *this;
}

CMessage& CMessage::operator>>(uint64& iValue)
{
	if (m_iDataSize < sizeof(uint64))
	{
		m_iError = true;
		return *this;
	}

	iValue = *(uint64*)m_iReadPos;

	m_iReadPos += sizeof(uint64);
	m_iDataSize -= sizeof(uint64);

	return *this;
}


CMessage& CMessage::operator>>(double& dValue)
{
	if (m_iDataSize < sizeof(double))
	{
		m_iError = true;
		return *this;
	}

	dValue = *(double*)m_iReadPos;

	m_iReadPos += sizeof(double);
	m_iDataSize -= sizeof(double);

	return *this;
}

#pragma endregion