//
// Created by 董文龙 on 2022/11/9.
//

#pragma once

class HeapAllocator {
 public:
  /** my malloc  **/
  static void* MyMalloc(long long size);
  /** my free **/
  static void MyFree(void* p);
};