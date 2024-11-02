#include "list.h"
#include "cache.h"

namespace CacheAllocator
{
  void List::setup(SlabMetadata *systemSlab)
  {
    m_slab = systemSlab;
  }

  bool List::empty() const
  {
    return (m_first == nullptr);
  }

  ListNode *List::begin() const
  {
    return m_first;
  }

  void List::pushBack(void* value)
  {
    //выделяем память
    void* ptr = m_slab->alloc();

    // инициализируем ноду
    ListNode* newNode = static_cast<ListNode*>(ptr);
    newNode->setValue(value);
    newNode->setNext(m_first);

    // теперь это нода - первая
    m_first = newNode;
  }

  void List::clear()
  {
    // пока не пусто
    while (m_first != nullptr)
    {
      void* ptr = m_first;

      // переходим к следующей ноде
      m_first = m_first->next();

      // и удаляем
      m_slab->free(ptr);
    }
  }

  void List::remove(void *value)
  {
    ListNode* prev = nullptr;
    ListNode* node = m_first;

    // Идем по нодам до конца
    while (node != nullptr)
    {
      // если нашли значение
      if (node->value() == value)
      {
        // перекидываем ссылку на next элемент
        if (prev == nullptr)
        {
          m_first = node->next();
        }
        else
        {
          prev->setNext(node->next());
        }

        // и удаляем
        m_slab->free(static_cast<void*>(node));
        return;
      }

      // Если не нашли, то переходим к следующему
      prev = node;
      node = node->next();
    }
  }

  void *ListNode::value() const
  {
    return m_value;
  }

  ListNode *ListNode::next() const
  {
    return m_next;
  }

  void ListNode::setValue(void *value)
  {
    m_value = value;
  }

  void ListNode::setNext(ListNode *value)
  {
    m_next = value;
  }
}
