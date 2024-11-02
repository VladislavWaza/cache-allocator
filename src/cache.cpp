#include "cache.h"
#include "memory.h"

#include <stdint.h>

namespace CacheAllocator
{
  void SlabMetadata::setup(std::size_t objectSize, int slabObjects)
  {
    //Начало текущего слеба
    void* slab = reinterpret_cast<uint8_t*>(this) - slabObjects * objectSize;

    void* lastBlock = nullptr;
    void* curBlock = slab;

    //Строим списочную зависимость внутри каждого блока
    while (curBlock != static_cast<void*>(this))
    {
      *static_cast<void**>(curBlock) = lastBlock;
      lastBlock = curBlock;
      curBlock = static_cast<uint8_t*>(curBlock) + objectSize;
    }

    m_freeBlockPtr = lastBlock;
    m_busyBlocks = 0;
  }

  bool SlabMetadata::isEmpty() const
  {
    return (m_busyBlocks == 0);
  }

  bool SlabMetadata::isFull() const
  {
    return (m_freeBlockPtr == nullptr);
  }

  void *SlabMetadata::alloc()
  {
    if (isFull())
      return nullptr;

    ++m_busyBlocks;

    // Обновляем данные о свободном блоке (адрес свободного блока лежит внутри блока, который отдается)
    void* ret = m_freeBlockPtr;
    m_freeBlockPtr = *static_cast<void**>(m_freeBlockPtr);
    return ret;
  }

  void SlabMetadata::free(void *ptr)
  {
    if (ptr == nullptr || isEmpty())
      return;

    --m_busyBlocks;

    // Обновляем данные о свободном блоке
    *static_cast<void**>(ptr) = m_freeBlockPtr;
    m_freeBlockPtr = ptr;
  }


  bool Cache::setup(std::size_t objectSize)
  {
    // размер блока + размер служебных данных не должны преывшать максимальный размер слеба
    if (objectSize + sizeof(SlabMetadata) > MAX_SLAB_SIZE)
      return false;

    // размер блока не должен быть меньше размера указателя
    if (objectSize < sizeof(void*))
      return false;

    // Подбираем степень двойки (порядок) для выяснения размера слеба
    for (int i = 0; i <= MAX_SLAB_ORDER; ++i)
    {
      if (objectSize + sizeof(SlabMetadata) <= slabSize(i))
      {
        // Сохраняем параметры слеба
        m_slabOrder = i;
        m_objectSize = objectSize;
        m_slabObjects = (slabSize(m_slabOrder) - sizeof(SlabMetadata)) / m_objectSize;

        // Параметры системного слеба (для очередей)
        int systemSlabOrder = MAX_SLAB_ORDER;
        std::size_t systemObjectSize = sizeof(ListNode);
        int systemSlabObjects = (slabSize(systemSlabOrder) - sizeof(SlabMetadata)) / systemObjectSize;

        // Аллокация системного слеба
        m_systemSlab = allocSlab(MAX_SLAB_ORDER);

        // Инициализация системного слеба
        m_systemSlabMetadata = reinterpret_cast<SlabMetadata*>(
                    static_cast<uint8_t*>(m_systemSlab) + systemSlabObjects * systemObjectSize);
        m_systemSlabMetadata->setup(systemObjectSize, systemSlabObjects);

        // Передача в списики адреса системного слеба для управления им
        m_emptySlabs.setup(m_systemSlabMetadata);
        m_partialSlabs.setup(m_systemSlabMetadata);
        m_fullSlabs.setup(m_systemSlabMetadata);

        return true;
      }
    }

    return false;
  }

  void *Cache::alloc()
  {
    // Сначала проверяем наличие частично заполненных слебов
    if (!m_partialSlabs.empty())
    {
      void* slab = m_partialSlabs.begin()->value();
      SlabMetadata* metadata = getMetadata(slab);
      void* result = metadata->alloc();

      if (result == nullptr)
        return result;

      // Слеб увеличил заполненность: если слеб стал занятым его надо перенести в другую очередь
      if (metadata->isFull())
      {
        m_fullSlabs.pushBack(slab);
        m_partialSlabs.remove(slab);
      }
      return result;
    }

    // Если нет частично заполненных слебов, то проверяем наличие пустых
    if (m_emptySlabs.empty())
    {
      // Если нет, то создаем
      void* slab = allocSlab(m_slabOrder);
      // Добавляем в очередь
      m_emptySlabs.pushBack(slab);
      // и инициализируем
      getMetadata(slab)->setup(m_objectSize, m_slabObjects);
    }

    void* slab = m_emptySlabs.begin()->value();
    SlabMetadata* metadata = getMetadata(slab);

    void* result = metadata->alloc();
    if (result == nullptr)
      return result;

    // Слеб увеличил заполненность: если слеб стал занятым его надо перенести в другую очередь
    if (metadata->isFull())
    {
      m_fullSlabs.pushBack(slab);
    }
    else // Слеб увеличил заполненность: но не стал полностью занятым, его надо перенести в другую очередь
    {
      m_partialSlabs.pushBack(slab);
    }
    m_emptySlabs.remove(slab);

    return result;
  }

  void Cache::release()
  {
    // Освобождение пустых слебов
    for (ListNode* node = m_emptySlabs.begin(); node != nullptr; node = node->next())
    {
      freeSlab(node->value());
    }
    m_emptySlabs.clear();

    // Освобождение частично занятых слебов
    for (ListNode* node = m_partialSlabs.begin(); node != nullptr; node = node->next())
    {
      freeSlab(node->value());
    }
    m_partialSlabs.clear();

    // Освобождение полностью занятых слебов
    for (ListNode* node = m_fullSlabs.begin(); node != nullptr; node = node->next())
    {
      freeSlab(node->value());
    }
    m_fullSlabs.clear();

    // Освобождение системного слеба
    freeSlab(m_systemSlab);
  }

  void Cache::free(void *ptr)
  {
    if (ptr == nullptr)
      return;

    // Вычисляем начало слеба
    void* slab = reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(ptr) & ~(slabSize(m_slabOrder) - 1));

    SlabMetadata* metadata = getMetadata(slab);
    if (metadata->isEmpty())
      return;

    // Если данный слеб - полный
    if (metadata->isFull())
    {
      metadata->free(ptr);

      // Заполненность слеба уменьшилась, переносим в другую очередь
      if (metadata->isEmpty())
      {
        m_emptySlabs.pushBack(slab);
      }
      else
      {
        m_partialSlabs.pushBack(slab);
      }

      m_fullSlabs.remove(slab);
      return;
    }

    // Если слеб - частично занятый
    metadata->free(ptr);
    if (metadata->isEmpty())
    {
      m_emptySlabs.pushBack(slab);
      m_partialSlabs.remove(slab);
    }
  }

  void Cache::shrink()
  {
    // Освобождаем пустые слебы
    for (ListNode* node = m_emptySlabs.begin(); node != nullptr; node = node->next())
    {
      freeSlab(node->value());
    }
    m_emptySlabs.clear();
  }

  SlabMetadata *Cache::getMetadata(void *slab) const
  {
    return reinterpret_cast<SlabMetadata*>(static_cast<uint8_t*>(slab) + m_slabObjects * m_objectSize);
  }
}
