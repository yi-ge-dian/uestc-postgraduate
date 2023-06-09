//
// Created by 董文龙 on 2022/11/11.
//

#pragma once

#include <cstddef>
#include <mutex>

/******************************** page **************************************************************/
/** page size **/
const int kPageSize =  4096;

/** page shift **/
const size_t  kPageShift = 12;

/******************************** span ***************************************************************/
/** span head size **/
const size_t  kSpanHeadSize = 64;

/** span max size **/
const size_t   kSpanMaxSize = 129;

/** span max list **/
const size_t  kSpanMaxListSize = 8;

/******************************** span list **********************************************************/
/** thread cache span list num **/
const size_t  kTCSpanListNum =  1024 + 15 + 1;

/** central cache span list num **/
const size_t  kCCSpanListNum  = (kPageSize * (129 - 1) - kSpanHeadSize) / 64 + 15 + 1;

/** page cache span list num **/
const size_t  kPCSpanListNum = 129;

/***************************** threshold about apply for three caches ********************************/
/** apply for thread cache max size **/
const size_t  kThreadCacheMaxSize  = 64 * 1024;
/** apply for central cache max size **/
const size_t kCentralCacheMaxSize =  kPageSize * (kSpanMaxSize - 1) - kSpanHeadSize;

/** memory block **/
class MemBlock {
 public:
  MemBlock* next_;
  MemBlock* prev_;
};


/** chunk list **/
template <class Chunk>
class ChunkList {
 private:
  int chunk_num_;
  Chunk head_;
  /**
   * insert the target chunk at the cur
   * @param target
   * @param cur
   */
  void insert(Chunk* target, Chunk* cur) {
    Chunk* temp = cur->next_;

    cur->next_->prev_ = target;
    cur->next_ = target;
    target->next_ = temp;
    target->prev_ = cur;

    chunk_num_++;
  }

  /**
   * insert multi chunks at the cur
   * @param start
   * @param end
   * @param num
   * @param cur
   */
  void multiInsert(Chunk* start, Chunk* end, int num, Chunk* cur) {
    Chunk* temp = cur->next_;

    cur->next_->prev_ = end;
    cur->next_ = start;
    end->next_ = temp;
    start->prev_ = cur;
    chunk_num_ += num;
  }

 public:
  ChunkList() {
    chunk_num_ = 0;
    head_.next_ = &head_;
    head_.prev_ = &head_;
  }

  /**
   * remove target chunk and return it
   * @param target
   * @return
   */
  Chunk* Remove(Chunk* target) {
    target->prev_->next_ = target->next_;
    target->next_->prev_ = target->prev_;
    chunk_num_--;
    return target;
  }

  /**
   * push the chunk at the head
   * @param chunk
   */
  void PushFront(Chunk* chunk) { insert(chunk, &head_); }

  /**
   * push the chunk at the tail
   * @param chunk
   */
  void PushBack(Chunk* chunk) { insert(chunk, head_.prev_); }

  /**
   * push multi chunks at the front
   * @param start
   * @param end
   * @param num
   */
  void MultiPushFront(Chunk* start, Chunk* end, int num) { multiInsert(start, end, num, &head_); }

  /**
   * push multi chunks at the tail
   * @param start
   * @param end
   * @param num
   */
  void MultiPushBack(Chunk* start, Chunk* end, int num) { mulitInsert(start, end, num, head_.prev_); }

  /**
   * remove the front chunk
   * @return
   */
  Chunk* RemoveFrontChunk() { return Remove(head_.next_); }

  /**
   * get the front chunk
   * @return
   */
  Chunk* GetFrontChunk() { return head_.next_; }

  /**
   * remove the back chunk
   * @return
   */
  Chunk* RemoveBackChunk() { return Remove(head_.prev_); }

  /**
   * get head
   * @return
   */
  Chunk* GetHeadPointer() { return &head_; }

  void Reset() {
    head_.next_ = &head_;
    head_.prev_ = &head_;
    chunk_num_ = 0;
  }
  int Size() { return chunk_num_; }
  bool IsEmpty() { return chunk_num_ == 0; }
};


void *SystemMalloc(int size);

bool SysFreePage(void *p, int k);

void *SysMallocPage(int k);

int Size2Slot(int size);

long long CeilMemSize(long long memSize);


template <int DEPTH, int N>
class RadixTree {
 private:
  int radix_depth_;
  int slot_num_;
  void** root;

  /**
   * get the slot
   * @param crr_root
   * @param slot
   * @return
   */
  inline void** getSlot(void** crr_root, int slot) { return (void**)crr_root[slot]; }

  /**
   * new slot
   * @param crr_root
   * @param slot
   * @return
   */
  void** newSlot(void** crr_root, int slot) {
    crr_root[slot] = SysMallocPage((sizeof(void*) * slot_num_ + kPageSize - 1) / kPageSize);
    return (void**)crr_root[slot];
  }

  /**
   * free node
   * @param p
   * @param d
   */
  void freeNode(void** p, int d) {
    if (p != nullptr && d < DEPTH) {
      for (int i = 0; i < slot_num_; i++) {
        if (p[i] != nullptr) {
          freeNode((void**)p[i], d + 1);
        }
      }
      SysFreePage(p, (sizeof(void*) * slot_num_ + kPageSize - 1) / kPageSize);
    }
  }

 public:
  RadixTree() : radix_depth_(DEPTH), slot_num_(1 << N) {
    root = (void**)SystemMalloc(sizeof(void*) * slot_num_);
    //log_heap_top = sbrk(0);
    for (int i = 0; i < slot_num_; i++) {
      root[i] = nullptr;
    }
  }

  /**
   * key: page_id
   * @param key
   * @return
   */
  void* GetValue(long long key) {
    void** node = root;
    for (int i = 0; i < DEPTH - 1; i++) {
      int shift = N * (DEPTH - 1 - i);
      int node_key = 0xfff & (key >> shift);
      node = getSlot(node, node_key);
    }
    int shift = 0;
    int node_key = 0xfff & (key >> shift);
    return node[node_key];
  }

  /**
   * key: pageId
   * value:span pointer
   * @param key
   * @param value
   */
  void Insert(long long key, void* value) {
    void** node = root;
    void** temp;
    for (int i = 0; i < DEPTH - 1; i++) {
      int shift = N * (DEPTH - 1 - i);
      int node_key = 0xfff & (key >> shift);
      temp = getSlot(node, node_key);
      if (temp == nullptr) {
        node = newSlot(node, node_key);
      } else {
        node = temp;
      }
    }
    int shift = 0;
    int node_key = 0xfff & (key >> shift);
    node[node_key] = value;
  }

  ~RadixTree() {
    freeNode(root, 0);
    root = nullptr;
  }
};


class Span {
 public:
  long long page_id_;   // 第一个page_id
  long long page_num_;  // page的数量

  ChunkList<MemBlock> mem_block_list_;  // mem block list
  int mem_block_size_;                  // mem_block 的大小
  int mem_block_num_;                   // span 分裂后的数目，包含已经分裂的和未分裂的

  Span* next_ = nullptr;
  Span* prev_ = nullptr;

  Span() : page_id_(-1), page_num_(-1), mem_block_size_(-1), mem_block_num_(0) {}
  Span(int page_num) : page_num_(page_num), mem_block_size_(-1), mem_block_num_(0) {
    page_id_ = ((long long)this) >> kPageShift;
  }

  bool IsFree() { return mem_block_num_ == mem_block_list_.Size(); }

  bool IsSplit() { return mem_block_size_ != -1; }

  /**
   * split the larger span into smaller mem block and push into mem block list
   * @param mem_block_size
   * @return
   */
  bool Split(int mem_block_size) {
    // 已经被分割过了
    if (IsSplit()) {
      return false;
    }
    mem_block_size_ = mem_block_size;
    // 对齐
    void* user_space = (char*)this + kSpanHeadSize;
    int num = (int)(page_num_ * kPageSize - kSpanHeadSize) / mem_block_size_;
    this->mem_block_num_ = num;
    void* end = (char*)user_space + (num - 1) * mem_block_size_;
    // 插入到链表中
    for (int i = 0; i < num; i++) {
      auto* p = (MemBlock*)((char*)user_space + i * mem_block_size_);
      p->next_ = (MemBlock*)((char*)user_space + (i + 1) * mem_block_size_);
      p->prev_ = (MemBlock*)((char*)user_space + (i - 1) * mem_block_size_);
    }
    mem_block_list_.MultiPushFront((MemBlock*)user_space, (MemBlock*)end, num);
    return true;
  }

  /**
   * merge the small mem block
   * @return
   */
  bool Merge() {
    if (IsFree()) {
      mem_block_list_.Reset();
      mem_block_num_ = 0;
      mem_block_size_ = -1;
      return true;
    } else {
      return false;
    }
  }
};

class SpanList : public ChunkList<Span> {
 private:
  std::mutex latch_;

 public:
  void Lock() { latch_.lock(); }
  void Unlock() { latch_.unlock(); };
};

