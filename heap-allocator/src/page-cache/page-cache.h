//
// Created by 董文龙 on 2022/11/9.
//

#pragma once
#include "util.h"

class PageCache {
 private:
  SpanList pc_span_lists_[kPCSpanListNum];
  static RadixTree<3, kPageShift> radix_tree_;
  std::mutex latch_;

  Span* mallocSpan(int num_of_page);
  bool deleteSpan(Span* p);

 public:
  /** 实例 **/
  static PageCache& GetPageCacheInstance() {
    static PageCache pc;
    return pc;
  }


  /** page id 映射到 span  **/
  static Span* PageId2Span(long long key) { return (Span*)radix_tree_.GetValue(key); }

  Span* DistributeSpan(int page_num);
  void TakeBackSpan(Span* span);
};