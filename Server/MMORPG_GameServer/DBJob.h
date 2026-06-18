#pragma once
#include "CSizeClassMemoryPoolTLS.h"
#include "ContentsType.h"

class DBTLS;

template<typename T>
class LFQueueMul;

struct DBJob
{
	virtual void Execute(DBTLS* DBTLS) = 0;
	virtual ~DBJob() = default;

	void* operator new(size_t size)
	{
		return CSizeClassMemoryPoolTLS::Alloc(size);
	}

	void  operator delete(void* ptr, size_t size)
	{
		CSizeClassMemoryPoolTLS::Free(ptr, size);
	}

	LFQueueMul<DBJob>* startQueue = nullptr;
	uint64             sessionID = 0;                        // 이 Job을 던진 세션의 유효성 검증용 
};


struct CharacterSelectJob : public DBJob
{
	virtual void Execute(DBTLS* DBTLS) override;
};