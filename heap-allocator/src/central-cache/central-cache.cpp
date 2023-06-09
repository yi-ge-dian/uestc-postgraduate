//
// Created by 董文龙 on 2022/11/9.
//

#include "central-cache.h"

#include <cassert>
#include <cmath>
#include "page-cache.h"

/**
 * 分配一个 mem block
 * @param align_size
 * @return
 */
MemBlock* CentralCache::DistributeMemBlock(int size) {
  assert(size % 4 == 0 && size > 64 * 1024);
  int slot = Size2Slot(size);
  cc_span_lists_[slot].Lock();
  Span* head = cc_span_lists_[slot].GetHeadPointer();
  Span* p = head->next_;

  // 一直找，开始遍历
  while (p != head && p->mem_block_list_.Size() == 0) {
    p = p->next_;
  }

  // 如果没找到空闲的 span，向 page cache 申请
  if (p == head) {
    int page_num = (int)ceilf((float)(size + kSpanHeadSize) / (float)kPageSize);
    p = PageCache::GetPageCacheInstance().DistributeSpan(page_num);
    p->Split(size);
    cc_span_lists_[slot].PushFront(p);
  }
  // 返回一个 mem block
  MemBlock* res = p->mem_block_list_.RemoveFrontChunk();
  cc_span_lists_[slot].Unlock();
  return res;
}

/**
 * 回收 mem block
 * @param p
 * @param span
 */
void CentralCache::TakeBackMemBlock(MemBlock* p, Span* span) {
  int slot = Size2Slot(span->mem_block_size_);
  cc_span_lists_[slot].Lock();
  span->mem_block_list_.PushFront(p);
  if (span->IsFree()) {
    cc_span_lists_[slot].Remove(span);
    span->Merge();
    PageCache::GetPageCacheInstance().TakeBackSpan(span);
  }
  cc_span_lists_[slot].Unlock();
}
