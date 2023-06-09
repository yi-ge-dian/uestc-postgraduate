//
// Created by 董文龙 on 2022/11/9.
//

#include <iostream>
#include <vector>
#include <thread>
#include "heap-allocator.h"

void myMallocAndMyFreeTest(){
  int kTimes = 1024;
  void** mem = new void*[kTimes];
  // test my malloc
  auto start = clock();
  for (int i = 0; i < kTimes; i++) {
    mem[i] = HeapAllocator::MyMalloc(16);
  }
  auto end = clock();
  std::cout << "MyMalloc Time:" << end - start << "ns"<< std::endl;

  // test my free
  start = clock();
  for (int i = 0; i < kTimes; i++) {
    HeapAllocator::MyFree(mem[i]);
  }
  end = clock();
  std::cout << "MyFree Time:" << end - start <<"ns" << std::endl;
}


void systemMallocAndSystemFreeTest(){
  int kTimes = 1024;
  void** mem = new void*[kTimes];
  // test malloc
  auto start = clock();
  for (int i = 0; i < kTimes; i++) {
    mem[i] = malloc(16);
  }
  auto end = clock();
  std::cout << "Malloc Time:" << end - start << "ns"<<std::endl;

  // test free
  start = clock();
  for (int i = 0; i < kTimes; i++) {
    free(mem[i]);
  }
  end = clock();
  std::cout << "Free Time:" << end - start <<"ns" <<  std::endl;
}



// test right
void testRight(){
  std::cout << "***********************test right***************************" << std::endl;
  // my malloc
  void* mem_61B = HeapAllocator::MyMalloc(61); // 64
  void* mem_100KB = HeapAllocator::MyMalloc(100 * 1024);
  void* mem_2GB = HeapAllocator::MyMalloc(2LL * 1024 * 1024 * 1024);
  // use it
  ((char*)mem_61B)[60] = 'a';
  ((char*)mem_61B)[61] = 'b';
  ((char*)mem_61B)[62] = 'c';
  ((char*)mem_61B)[63] = 'd';
  ((char*)mem_100KB)[2000 - 1] = 'e';
  ((char*)mem_2GB)[2 * 1024 * 1024 * 1024LL - 1] = 'f';
  // print
  std::cout << ((char *)mem_61B)[60] << std::endl;
  std::cout << ((char *)mem_61B)[61] << std::endl;
  std::cout << ((char *)mem_61B)[62] << std::endl;
  std::cout << ((char *)mem_61B)[63] << std::endl;
  std::cout << ((char*)mem_100KB)[2000 - 1] << std::endl;
  std::cout << ((char*)mem_2GB)[2 * 1024 * 1024 * 1024LL - 1] << std::endl;
  // my free
  HeapAllocator::MyFree(mem_61B);
  HeapAllocator::MyFree(mem_100KB);
  HeapAllocator::MyFree(mem_2GB);
  // should error
  std::cout << ((char *)mem_61B)[60] << std::endl;
  std::cout << ((char *)mem_61B)[61] << std::endl;
  std::cout << ((char *)mem_61B)[62] << std::endl;
  std::cout << ((char *)mem_61B)[63] << std::endl;
  std::cout << ((char*)mem_100KB)[2000 - 1] << std::endl;
  std::cout << ((char*)mem_2GB)[2 * 1024 * 1024 * 1024LL - 1] << std::endl;
}


// test single
void testSingle() {
  std::cout << "***********************test single***************************" << std::endl;
  myMallocAndMyFreeTest();
  systemMallocAndSystemFreeTest();
}

// test concurrent
void testConcurrent(){
  std::cout << "***********************test Concurrent***************************" << std::endl;
  auto start = clock();
  std::thread thread01(myMallocAndMyFreeTest);
  std::thread thread02(myMallocAndMyFreeTest);
  thread01.join();
  thread02.join();
  auto end =clock();
  std::cout << "concurrent MyMalloc and MyFree Time:" << end - start <<"ns" <<  std::endl;

  start= clock();
  std::thread thread03(systemMallocAndSystemFreeTest);
  std::thread thread04(systemMallocAndSystemFreeTest);
  thread03.join();
  thread04.join();
  end =clock();
  std::cout << "concurrent Malloc and Free Time:" << end - start <<"ns" <<  std::endl;
}


int main() {
  //testRight();
  //testSingle();
  testConcurrent();
  return 0;
}