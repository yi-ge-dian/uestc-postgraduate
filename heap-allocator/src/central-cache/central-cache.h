//
// Created by 董文龙 on 2022/11/9.
//
#pragma once
#include "util.h"

class CentralCache {
 private:
  SpanList cc_span_lists_[kCCSpanListNum];

 public:
  /** 实例 **/
  static CentralCache& GetCentralCacheInstance() {
    static CentralCache cc;
    return cc;
  }

  MemBlock* DistributeMemBlock(int size);
  void TakeBackMemBlock(MemBlock* p, Span* span);
};