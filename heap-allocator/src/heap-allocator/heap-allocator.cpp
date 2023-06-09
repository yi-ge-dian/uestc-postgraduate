//
// Created by 董文龙 on 2022/11/9.
//

#include "heap-allocator.h"

#include "central-cache.h"
#include "page-cache.h"
#include "thread-cache.h"
#include "util.h"

static __thread ThreadCache* tls_thread_cache = nullptr;

/**
 * 通过系统创建线程缓存
 */
void newThreadCacheBySystem() {
  int page_num = (sizeof(ThreadCache) + 64 + kPageSize - 1) / kPageSize;
  // std::cout << page_num << std::endl;

  void* mem = SysMallocPage(page_num);
  ((int*)mem)[0] = page_num;
  tls_thread_cache = (ThreadCache*)((char*)mem + 64);
  new (tls_thread_cache) ThreadCache;
}

/**
 * My malloc
 * @param size
 * @return
 */
void* HeapAllocator::MyMalloc(long long size) {
  // 如果没有 thread cache ，创建一个
  if (tls_thread_cache == nullptr) {
    newThreadCacheBySystem();
  }

  // 内存按字节大小对齐后取整
  long long align_size = CeilMemSize(size);
  void* res;

  // 根据不同的大小进行不同的处理
  if (size < kThreadCacheMaxSize) {
    // 向 thread cache 申请
    res = tls_thread_cache->DistributeMem(align_size);
  } else if (size < kCentralCacheMaxSize) {
    // 向 central cache 申请
    res = (void*)CentralCache::GetCentralCacheInstance().DistributeMemBlock(align_size);
  } else {
    // 向 系统申请
    int page_num = (align_size + 64 + kPageSize - 1) / kPageSize;
    res = SysMallocPage(page_num);
    ((int*)res)[0] = page_num;
    res = (char*)res + 64;
  }
  return res;
}

/**
 * My free
 * @param p
 */
void HeapAllocator::MyFree(void* p) {
  Span* span = PageCache::PageId2Span(((long long)p) >> kPageShift);
  // 如果没有对应的 span ，系统来释放
  if (span == nullptr) {
    int page_num = *((int*)((char*)p - 64));
    SysFreePage((char*)p - 64, page_num);
  } else {
    int size = span->mem_block_size_;
    if (size < kThreadCacheMaxSize){
      // thread cache 回收
      tls_thread_cache->TakeBackMem(p, span);
    } else if (size < kCentralCacheMaxSize) {
      // central cache 回收
      CentralCache::GetCentralCacheInstance().TakeBackMemBlock((MemBlock*)p, span);
    }
  }
}
