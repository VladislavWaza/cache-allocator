#include "memory.h"

#include <cstdlib>

namespace CacheAllocator
{
  void *allocSlab(int order)
  {
    #ifdef _WIN32
      return _aligned_malloc(slabSize(order), slabSize(order));
    #elif __linux__
      return aligned_alloc(slabSize(order), slabSize(order));
    #endif
  }

  void freeSlab(void *slab)
  {
    #ifdef _WIN32
      _aligned_free(slab);
    #elif __linux__
      free(slab);
    #endif
  }
}
