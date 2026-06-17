#pragma once

struct BlockSize32
{
	char  array[32];
};

struct BlockSize64
{
	char  array[64];
};

struct BlockSize128
{
	char  array[128];
};

struct BlockSize256
{
	char  arrray[256];
};

struct BlockSize512
{
	char  array[512];
};

constexpr int MAX_MEMORYPOOL_BLOCK_SIZE = 512;

template<typename T>
class CMPoolTLS;

class CSizeClassMemoryPoolTLS
{
	static void  PoolInit();
	static void  PoolDestroy();
	static void* Alloc(int size);
	static void  Free(void* ptr, int size);

private:
	static CMPoolTLS<BlockSize32>*  m_blockSize32Pool  ;
	static CMPoolTLS<BlockSize64>*  m_blockSize64Pool  ;
	static CMPoolTLS<BlockSize128>* m_blockSize128Pool ;
	static CMPoolTLS<BlockSize256>* m_blockSize256Pool ;
	static CMPoolTLS<BlockSize512>* m_blockSize512Pool ;
};

