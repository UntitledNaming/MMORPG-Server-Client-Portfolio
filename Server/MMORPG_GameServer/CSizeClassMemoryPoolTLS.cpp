#include "MemoryPoolTLS.h"
#include "CSizeClassMemoryPoolTLS.h"

CMPoolTLS<BlockSize32>* CSizeClassMemoryPoolTLS::m_blockSize32Pool = nullptr;
CMPoolTLS<BlockSize64>* CSizeClassMemoryPoolTLS::m_blockSize64Pool = nullptr;
CMPoolTLS<BlockSize128>* CSizeClassMemoryPoolTLS::m_blockSize128Pool = nullptr;
CMPoolTLS<BlockSize256>* CSizeClassMemoryPoolTLS::m_blockSize256Pool = nullptr;
CMPoolTLS<BlockSize512>* CSizeClassMemoryPoolTLS::m_blockSize512Pool = nullptr;

void CSizeClassMemoryPoolTLS::PoolInit()
{
	m_blockSize32Pool = new CMPoolTLS<BlockSize32>;
	m_blockSize64Pool = new CMPoolTLS<BlockSize64>;
	m_blockSize128Pool = new CMPoolTLS<BlockSize128>;
	m_blockSize256Pool = new CMPoolTLS<BlockSize256>;
	m_blockSize512Pool = new CMPoolTLS<BlockSize512>;
}

void CSizeClassMemoryPoolTLS::PoolDestroy()
{
	delete m_blockSize32Pool;
	delete m_blockSize64Pool;
	delete m_blockSize128Pool;
	delete m_blockSize256Pool;
	delete m_blockSize512Pool;
}

void* CSizeClassMemoryPoolTLS::Alloc(int size)
{
	if (size > MAX_MEMORYPOOL_BLOCK_SIZE)
		return nullptr;

	int quo = size / 32;


	return nullptr;
}

void CSizeClassMemoryPoolTLS::Free(void* ptr, int size)
{

}
