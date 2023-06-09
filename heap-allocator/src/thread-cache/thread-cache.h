//
// Created by 董文龙 on 2022/11/9.
//
#pragma once

#include "util.h"

class ThreadCache {
 private:
  SpanList tc_span_lists_[kTCSpanListNum];

  void fetchSpanFromPageCache(int size, int min_num);
  void releaseSpanToPageCache(Span* span);

 public:
  void* DistributeMem(int size);
  void TakeBackMem(void* mem, Span* span);
};