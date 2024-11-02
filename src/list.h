#pragma once

namespace CacheAllocator
{
  class SlabMetadata;

  struct ListNode
  {
    void *value() const;
    ListNode *next() const;
    void setValue(void* value);
    void setNext(ListNode* value);
  private:
    void* m_value = nullptr;
    ListNode* m_next = nullptr;
  };

  struct List
  {
    void setup(SlabMetadata* systemSlab);
    bool empty() const;
    ListNode *begin() const;
    void pushBack(void* value);
    void clear();
    void remove(void* value);
  private:
    SlabMetadata* m_slab = nullptr;
    ListNode* m_first = nullptr;
  };
}

