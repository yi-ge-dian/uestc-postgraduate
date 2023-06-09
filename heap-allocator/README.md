# heap-allocator

## 总体要求

实现一个堆分配器，即实现内存分配和释放的功能

## 基本要求

- 实现的内存分配函数名为 MyMalloc，内存释放函数名为 MyFree，这两个函数原型同 malloc 和 free
- 使用 mmap 函数从内核分配整个堆空间，而 MyMalloc 和 MyFree 管理该空间
    - 即用这个堆空间模拟内存，每次分配和释放在这个空间内进行
    - 如空间不够，再通过 mmap 重新分配堆空间，重复上述工作
- 要考虑内存碎片问题
    - 例如可以考虑采用内存池技术来管理
    - 或者空闲链表
- 要考虑并发性问题，即多个线程同时分配和释放内存的场景
    - 例如可以采用互斥技术
    - 互斥的粒度不宜过粗，如不宜针对每个 malloc 和 free 直接加锁，而应当将锁的粒度放到对指定内存池元素的分配和回收上

## 示例代码 malloc.c

示例代码只是一个最简单的内存分配器，帮助大家弄清基本原理，这个例子并没有考虑碎片和并发性问题

```c
#include "stdlib.h"
#include "sys/mman.h"

#define ALIGN_UP_N_BYTES(size, n) ((((size) + (n) - 1) / (n)) * (n))
#define ALIGN_UP_8_BYTES(size) ALIGN_UP_N_BYTES(size, 8)
#define ALIGN_UP_4KB(size) ALIGN_UP_N_BYTES(size, 4096)

#define THRESHOLD_FOR_MMAP (256 * 1024)
#define SEGMENT_SIZE (2 * 1024 * 1024)

#define MEM_BLOCK_MINIMUM 16

struct SLMemSegment
{
	struct SLMemSegment *pPrevMemSegment;
	struct SLMemSegment *pNextMemSegment;
};

struct SLMemBlock
{
	unsigned long CurBlkInUseBit : 1;
	unsigned long PrevBlkInUseBit : 1;
	unsigned long FromMmapBit : 1;
	unsigned long ulBlockSize : 61;
};

static struct SLMemSegment *g_pSegmentList = NULL;

void *MallocBymmap(size_t size);
void *MallocBySegmentList(size_t size);
int GetAndInsertSegment(void);
void *GetFreeMemBlockFromSegment(struct SLMemSegment *pSegment, size_t size);
int IsTheLastBlockInSegment(struct SLMemBlock *pBlock);
void FreeSegment(struct SLMemSegment *pSegment);

void *malloc(size_t size)
{
	size = ALIGN_UP_8_BYTES(size);
	
	if(size < MEM_BLOCK_MINIMUM)
		size = MEM_BLOCK_MINIMUM;
	else
	{
		size += sizeof(struct SLMemBlock);
	
		if(size >= THRESHOLD_FOR_MMAP)
			return MallocBymmap(size);
	}

	return MallocBySegmentList(size);
}

void *MallocBymmap(size_t size)
{
	size = ALIGN_UP_4KB(size);
	
	void *addr = mmap(NULL, size, PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
	if(addr == MAP_FAILED)
		return NULL;

	struct SLMemBlock *p = (struct SLMemBlock *)addr;

	p->CurBlkInUseBit = 1;
	p->PrevBlkInUseBit = 1;
	p->FromMmapBit = 1;
	p->ulBlockSize = size;

	return (char *)addr + sizeof(struct SLMemBlock);
}

void *MallocBySegmentList(size_t size)
{
	for(struct SLMemSegment *pSegment = g_pSegmentList; pSegment != NULL; pSegment = pSegment->pNextMemSegment)
	{
		void* addr = GetFreeMemBlockFromSegment(pSegment, size);
		if(addr != NULL)
			return addr;
	}

	if(GetAndInsertSegment() == -1)
		return NULL;

	return GetFreeMemBlockFromSegment(g_pSegmentList, size);
}

int GetAndInsertSegment(void)
{
	void *addr = mmap(NULL, SEGMENT_SIZE, PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
	if(addr == MAP_FAILED)
		return -1;

	struct SLMemSegment *pSegment = (struct SLMemSegment *)addr;
	
	pSegment->pNextMemSegment = g_pSegmentList;
	pSegment->pPrevMemSegment = NULL;

	if(g_pSegmentList != NULL)
		g_pSegmentList->pPrevMemSegment = pSegment;	
	
	g_pSegmentList = pSegment;

	struct SLMemBlock *pFirstBlock = (struct SLMemBlock *)((char *)addr + sizeof(struct SLMemSegment));

	pFirstBlock->CurBlkInUseBit = 0;
	pFirstBlock->PrevBlkInUseBit = 1;
	pFirstBlock->FromMmapBit = 0;
	pFirstBlock->ulBlockSize = SEGMENT_SIZE - sizeof(struct SLMemSegment);

	*((unsigned long *)((char *)pFirstBlock + pFirstBlock->ulBlockSize) - 1) = pFirstBlock->ulBlockSize;

	return 0;
}

void *GetFreeMemBlockFromSegment(struct SLMemSegment *pSegment, size_t size)
{
	struct SLMemBlock *pBlock = (struct SLMemBlock *)((char *)pSegment + sizeof(struct SLMemSegment));
	struct SLMemBlock *pEndBlock = (struct SLMemBlock *)((char *)pSegment + SEGMENT_SIZE);
	
	for(; pBlock < pEndBlock; pBlock = (struct SLMemBlock *)((char *)pBlock + pBlock->ulBlockSize))
	{
		if(pBlock->CurBlkInUseBit == 1)
			continue;

		if(pBlock->ulBlockSize < size)
			continue;

		if(pBlock->ulBlockSize - size < MEM_BLOCK_MINIMUM)
		{
			struct SLMemBlock *pNextBlock = (struct SLMemBlock *)((char *)pBlock + pBlock->ulBlockSize);

			if(pNextBlock < pEndBlock)
				pNextBlock->PrevBlkInUseBit = 1;
		}
		else
		{
			struct SLMemBlock *pNextBlock = (struct SLMemBlock *)((char *)pBlock + size);

			pNextBlock->CurBlkInUseBit = 0;
			pNextBlock->PrevBlkInUseBit = 1;
			pNextBlock->FromMmapBit = 0;
			pNextBlock->ulBlockSize = pBlock->ulBlockSize - size;
			
			*((unsigned long *)((char *)pNextBlock + pNextBlock->ulBlockSize) - 1) = pNextBlock->ulBlockSize;

			pBlock->ulBlockSize = size;
		}

		pBlock->CurBlkInUseBit = 1;
		return (char *)pBlock + sizeof(struct SLMemBlock);
	}

	return NULL;
}

void free(void *ptr)
{
	if(ptr == NULL)
		return;
	
	struct SLMemBlock *pBlock = (struct SLMemBlock *)((char *)ptr - sizeof(struct SLMemBlock));
	if(pBlock->FromMmapBit == 1)
	{
		munmap((void*)pBlock, pBlock->ulBlockSize);
		return;
	}

	size_t len = pBlock->ulBlockSize;

	int bLast = IsTheLastBlockInSegment(pBlock);

	if(!bLast)
	{
		struct SLMemBlock *pNextBlock = (struct SLMemBlock *)((char *)pBlock + len);
		if(pNextBlock->CurBlkInUseBit == 0)
			len += pNextBlock->ulBlockSize;
	}

	if(pBlock->PrevBlkInUseBit == 0)
	{
		unsigned long prev_size = *((unsigned long *)pBlock - 1);
		len += prev_size;

		pBlock = (struct SLMemBlock *)((char *)pBlock - prev_size);
	}

	pBlock->ulBlockSize = len;
	pBlock->CurBlkInUseBit = 0;
	pBlock->PrevBlkInUseBit = 1;
	pBlock->FromMmapBit = 0;

	bLast = IsTheLastBlockInSegment(pBlock);

	struct SLMemBlock *pNextBlock = (struct SLMemBlock *)((char *)pBlock + pBlock->ulBlockSize);
	*((unsigned long *)pNextBlock - 1) = pBlock->ulBlockSize;

	if(!bLast)
	{
		pNextBlock->PrevBlkInUseBit = 0;
	}
	else
	{
		if(pBlock->ulBlockSize == SEGMENT_SIZE - sizeof(struct SLMemSegment))
			FreeSegment((struct SLMemSegment *)((char *)pBlock - sizeof(struct SLMemSegment)));
	}
}

int IsTheLastBlockInSegment(struct SLMemBlock *pBlock)
{
	char *pEnd = (char *)pBlock + pBlock->ulBlockSize;
	
	for(struct SLMemSegment *pSegment = g_pSegmentList; pSegment != NULL; pSegment = pSegment->pNextMemSegment)
	{
		char *pSegmentEnd = (char *)pSegment + SEGMENT_SIZE;

		if(((char *)pBlock > (char *)pSegment) && (pEnd < pSegmentEnd))
			return 0;
		
		if(pEnd == pSegmentEnd)
			return 1;
	}

	return 0;
}

void FreeSegment(struct SLMemSegment *pSegment)
{
	if(g_pSegmentList == pSegment)
	{
		g_pSegmentList = pSegment->pNextMemSegment;

		if(g_pSegmentList != NULL)
			g_pSegmentList->pPrevMemSegment = NULL;
	}
	else
	{
		pSegment->pPrevMemSegment->pNextMemSegment = pSegment->pNextMemSegment;

		if(pSegment->pNextMemSegment != NULL)
			pSegment->pNextMemSegment->pPrevMemSegment = pSegment->pPrevMemSegment;
	}

	munmap((void*)pSegment, SEGMENT_SIZE);
}
```
