//
// Created by 董文龙 on 2022/11/11.
//

#include "util.h"
#include <sys/mman.h>
#include <unistd.h>
/**
 * malloc k pages from system using mmap
 * void *mmap(void *addr, size_t length, int prot, int flags, int fd, off_t offset);
 *      addr: 映射区首地址，传 NULL
 *      length: 映射区的大小,会自动调为 4k 的整数倍。不能为 0 一般文件多大，length 就指定多大
 *      prot: 映射区保护模式
 *              PROT_EXEC   映射区域可被执行
 *              PROT_READ   映射区域可被读取
 *              PROT_WRITE  映射区域可被写入
 *              PROT_NONE   映射区域不能存取
 *      flags: 影响映射区域的各种特性。在调用 mmap() 时必须要指定 MAP_SHARED 或 MAP_PRIVATE
 *              MAP_FIXED       如果参数 start 所指的地址无法成功建立映射时，则放弃映射，不对地址做修正。通常不鼓励用此
 *              MAP_SHARED      对映射区域的写入数据会复制回文件内，而且允许其他映射该文件的进程共享
 *              MAP_PRIVATE     对映射区域的写入操作会产生一个映射文件的复制，即私人的“写入时复制”（copy on
 write）对此区域作的任何修改都不会写回原来的文件内容
 *              MAP_ANONYMOUS   建立匿名映射。此时会忽略参数 fd，不涉及文件，而且映射区域无法和其他进程共享
                MAP_DENYWRITE   只允许对映射区域的写入操作，其他对文件直接写入的操作将会被拒绝
                MAP_LOCKED      将映射区域锁定住，这表示该区域不会被置换（swap）
 *      fd: 要映射到内存中的文件描述符。如果使用匿名内存映射时，即 flags 中设置了 MAP_ANONYMOUS ，fd 设为 -1
 *      off_t:文件映射的偏移量，通常设置为 0，代表从文件最前方开始对应，offset 必须是分页大小的整数倍。
 * @param page_num
 * @return
 */
void* SysMallocPage(int k) {
  void* ptr = mmap(
      nullptr, (long long)k * kPageSize, PROT_READ | PROT_WRITE, MAP_ANON | MAP_PRIVATE, -1, 0);
  //  if (ptr == MAP_FAILED) {
  //    std::cout << "MMAP_FAILED" << std::endl;
  //  } else {
  //    std::cout << "MMAP_SUCCESS" << std::endl;
  //  }
  return ptr;
}



/**
 * free k pages to system using munmap
 * @param p
 * @param k
 * @return
 */
bool SysFreePage(void* p, int k) { return munmap(p, (long long)k * kPageSize) != -1; }


/**
 * malloc size from heap
 * @param size
 * @return
 */
void* SystemMalloc(int size) { return sbrk(size); }


/**
 * ceil memory size, less 16 B = 16; more 16B less 64B, 4B* xxx align; more 64B align
 * @param mem_size
 * @return
 */
long long CeilMemSize(long long mem_size) {
  // at least 16B , two chunk lists needs 128 bit
  if (mem_size < 16) {
    mem_size = 16;
  } else if (mem_size < 64) {
    // 16,20,24,...,64 align
    mem_size = ((mem_size + 4 - 1) / 4) * 4;
  } else {
    // max 64 B align
    if (mem_size % 64) {
      mem_size = (mem_size / 64 + 1) * 64;
    }
  }
  return mem_size;
}

/**
 * given a size, return its slot
 * @param size
 * @return
 */
int Size2Slot(int size) {
  int slot = 0;
  if (size < 64) {
    slot = size / 4;
  } else {
    slot = size / 64 + 60 / 4;
  }
  return slot;
}


