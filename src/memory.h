#pragma once

#include <cstdio>

namespace CacheAllocator
{
  constexpr std::size_t PGSIZE = 4096; /*Размер страницы*/
  constexpr int MAX_SLAB_ORDER = 10; /*Максимальный порядок слаба*/
  constexpr std::size_t MAX_SLAB_SIZE = PGSIZE * (1 << MAX_SLAB_ORDER); /*Максимальный размер слаба*/
  constexpr std::size_t MIN_SLAB_SIZE = PGSIZE * (1 << 0); /*Минимальный размер слаба*/

  /**
   * Полный размер слеба при заданном порядке и размере страницы
   */
  constexpr std::size_t slabSize(int order) { return PGSIZE * (1 << order); }


  /**
   * Аллоцирует участок размером 4096 * 2^order байт,
   * выровненный на границу 4096 * 2^order байт. order
   * должен быть в интервале [0; 10] (обе границы
   * включительно), т. е. вы не можете аллоцировать больше
   * 4Mb за раз.
   **/
  void *allocSlab(int order);

  /**
   * Освобождает участок ранее аллоцированный с помощью
   * функции alloc_slab.
   **/
  void freeSlab(void *slab);
}
