//
// Created by 董文龙 on 2022/11/9.
//

#include "page-cache.h"
RadixTree<3, kPageShift> PageCache::radix_tree_;

/**
 * 分配一个 span, 大小为页的数目，并从链表中摘除
 * @param num_of_page
 * @return
 */
Span* PageCache::DistributeSpan(int page_num) {
  if (page_num > kPCSpanListNum) {
    return nullptr;
  }
  Span* res = nullptr;
  pc_span_lists_[page_num].Lock();
  // 申请多少页的内存
  if (pc_span_lists_[page_num].IsEmpty()) {
    pc_span_lists_[page_num].Unlock();
    res = mallocSpan(page_num);
  } else {
    res = pc_span_lists_[page_num].RemoveFrontChunk();
    pc_span_lists_[page_num].Unlock();
  }
  return res;
}

/**
 * 向系统申请 span
 * @param num_of_page
 * @return
 */
Span* PageCache::mallocSpan(int page_num) {
  latch_.lock();
  if (page_num > kPCSpanListNum) {
    return nullptr;
  }
  Span* new_span = (Span*)SysMallocPage(page_num);
  new (new_span) Span(page_num);

  // 在基数树中注册，page_id 和 span 的映射
  for (int i = 0; i < page_num; i++) {
    radix_tree_.Insert((((long long)new_span) >> kPageShift) + i, new_span);
  }
  latch_.unlock();
  return new_span;
}

/**
 * 收回空闲的 span ，太多了就还给系统
 * @param span
 */
void PageCache::TakeBackSpan(Span* span) {
  auto page_num = span->page_num_;
  pc_span_lists_[span->page_num_].Lock();
  // 某一个 page_num 类型的 span，只留下 8 个
  if (pc_span_lists_[span->page_num_].Size() < kSpanMaxListSize) {
    pc_span_lists_[span->page_num_].PushFront(span);
  } else {
    deleteSpan(span);
  }
  pc_span_lists_[page_num].Unlock();
}

/**
 * 删除span，还给系统
 * @param p
 * @return
 */
bool PageCache::deleteSpan(Span* p) {
  latch_.lock();
  SysFreePage(p, p->page_num_);
  // todo 前缀树中删除
  latch_.unlock();
  return true;
}
