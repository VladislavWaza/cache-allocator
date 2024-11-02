#pragma once

#include "list.h"

#include <cstdio>

namespace CacheAllocator
{
  /**
   * Эта структура представляет данные, которые лежат в конце слеба
   * и позволяет работать с данным слебом
   **/
  struct SlabMetadata
  {
    void setup(std::size_t objectSize, int slabObjects);
    bool isEmpty() const;
    bool isFull() const;
    void *alloc();
    void free(void *ptr);

  private:
    void* m_freeBlockPtr; /*Указатель на первый свободный элемент*/
    std::size_t m_busyBlocks = 0; /*Число занятых блоков*/
  };

  /**
   * Эта структура представляет аллокатор
   **/
  struct Cache
  {
    /**
     * Функция инициализации должна быть вызвана перед тем, как
     * использовать это кеширующий аллокатор для аллокации.
     * Параметры:
     *  - objectSize - размер объектов, которые должен
     *    аллоцировать этот кеширующий аллокатор
     * Возврат:
     *  - true, если удовлетворил требованиям и инициализация произошла
     *
     * Ограничения:
     *  - размер объекта + размер служебных данных не должны превышать максимальный размер слеба
     *  - размер объекта не должен быть меньше размера указателя
     **/
    bool setup(std::size_t objectSize);

    /**
     * Функция аллокации памяти из кеширующего аллокатора.
     * Возвращает указатель на участок памяти размера
     * как минимум objectSize байт (см setup).
     **/
    void *alloc();

    /**
     * Функция освобождения должна быть вызвана когда работа с
     * аллокатором будет закончена. Она освободит
     * всю память занятую аллокатором.
     **/
    void release();

    /**
     * Функция освобождения памяти назад в кеширующий аллокатор.
     **/
    void free(void* ptr);

    /**
     * Функция освободит все SLAB, которые не содержат
     * занятых объектов.
     **/
    void shrink();

    SlabMetadata *getMetadata(void* slab) const;

  private:
    void* m_systemSlab; /*начало SLAB-а для хранения очередей*/
    SlabMetadata* m_systemSlabMetadata; /* SlabMetadata для управления SLAB-ом для хранения очередей*/

    List m_emptySlabs; /* список пустых SLAB-ов*/
    List m_partialSlabs; /* список частично занятых SLAB-ов */
    List m_fullSlabs; /* список заполненных SLAB-ов */

    std::size_t m_objectSize; /* размер аллоцируемого объекта */
    int m_slabOrder; /* используемый размер SLAB-а */
    int m_slabObjects; /* количество объектов в одном SLAB-е */
  };
}
