#pragma once

struct alignas(std::max_align_t) BlockSize32
{
	char  array[32];
};

struct alignas(std::max_align_t) BlockSize64
{
	char  array[64];
};

struct alignas(std::max_align_t) BlockSize128
{
	char  array[128];
};

struct alignas(std::max_align_t) BlockSize256
{
	char  array[256];
};

struct alignas(std::max_align_t) BlockSize512
{
	char  array[512];
};

constexpr int MAX_MEMORYPOOL_BLOCK_SIZE = 512;

template<typename T>
class CMPoolTLS;

class CSizeClassMemoryPoolTLS
{
public:
	static void  PoolInit();
	static void  PoolDestroy();
	static void* Alloc(size_t size);
	static void  Free(void* ptr, size_t size);

public:
	static CMPoolTLS<BlockSize32>*  m_blockSize32Pool  ;
	static CMPoolTLS<BlockSize64>*  m_blockSize64Pool  ;
	static CMPoolTLS<BlockSize128>* m_blockSize128Pool ;
	static CMPoolTLS<BlockSize256>* m_blockSize256Pool ;
	static CMPoolTLS<BlockSize512>* m_blockSize512Pool ;
};

