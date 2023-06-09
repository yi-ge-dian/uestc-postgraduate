//
// Created by 董文龙 on 2022/11/9.
//

#include "thread-cache.h"

#include <algorithm>

#include "central-cache.h"
#include "page-cache.h"


/**
 * 从 page cache 获得 一个 span ，span 的大小为 size * min_num
 * @param align_size
 * @param min_num
 */
void ThreadCache::fetchSpanFromPageCache(int size, int min_num) {
  int slot = Size2Slot(size);
  Span* span = PageCache::GetPageCacheInstance().DistributeSpan((int)(size * min_num + kPageShift - 1) / kPageSize);
  span->Split(size);
  tc_span_lists_[slot].PushBack(span);
}

 /**
 * 空闲的 span 还给 page cache
 * @param span
 */
void ThreadCache::releaseSpanToPageCache(Span* span) {
  int slot = Size2Slot(span->mem_block_size_);
  SpanList& list = tc_span_lists_[slot];
  list.Remove(span);
  span->Merge();
  PageCache::GetPageCacheInstance().TakeBackSpan(span);
}

/**
 * 分配部分内存
 * @param size
 * @return
 */
void* ThreadCache::DistributeMem(int size) {
  int slot = Size2Slot(size);
  SpanList& span_list = tc_span_lists_[slot];
  // 如果内存不足，直接向 page cache 申请内存
  if (span_list.IsEmpty()) {
    int min_num = std::max(128 * kPageSize / size, 8);
    fetchSpanFromPageCache(size, min_num);
  }
  Span* span = span_list.GetFrontChunk();
  void* mem = span->mem_block_list_.RemoveFrontChunk();
  // 如果 span 已经使用完了，从链表移除
  if (span->mem_block_list_.Size() == 0) {
    span_list.Remove(span);
  }
  return mem;
}

/**
 * 回收 mem
 * @param mem
 * @param span
 * @return
 */
void ThreadCache::TakeBackMem(void* mem, Span* span) {
  span->mem_block_list_.PushFront((MemBlock*)mem);
  // 在插入之前 span 是空的
  if (span->mem_block_list_.Size() == 1) {
    int slot = Size2Slot(span->mem_block_size_);
    tc_span_lists_[slot].PushFront(span);
  } else if (span->IsFree()) {
    // 触发回收
    releaseSpanToPageCache(span);
  }
}
