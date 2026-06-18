/*

Simple bump allocators for local contest code. These are useful when creating many small objects
and individual `new`/`delete` overhead is measurable. They allocate from one fixed buffer and do
not reclaim individual objects.

- The global `operator new` override sends all dynamic allocations through a static buffer. This is
  the shortest version, but it affects the whole program and should be used only when you are sure
  the fixed buffer is large enough.
- `BumpAllocator<T>` is an STL allocator for containers such as `std::vector<T, BumpAllocator<T>>`.
  It is less invasive than overriding global `new`, but still has no real deallocation.

These snippets are not portable production allocators: they are fixed-size, monotonic, and intended
for local performance-sensitive contest solutions. The global override below covers ordinary
single-object `new`; it does not attempt to be a complete replacement for all allocation forms such
as `new[]` or over-aligned allocation.

Time Complexity:
- O(1) per allocation.

Space Complexity:
- O(`BUFFER_SIZE`) static memory.

*/

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <new>
#include <type_traits>
#include <vector>

static constexpr size_t BUFFER_SIZE = 64 << 20;
alignas(std::max_align_t) static unsigned char bump_buffer[BUFFER_SIZE];
static size_t bump_pos = BUFFER_SIZE;

void *operator new(size_t size) {
  size = (size + alignof(std::max_align_t) - 1) & ~(alignof(std::max_align_t) - 1);
  assert(size <= bump_pos);
  bump_pos -= size;
  return bump_buffer + bump_pos;
}

void operator delete(void *) noexcept {
}

void operator delete(void *, size_t) noexcept {
}

template<typename T>
struct BumpAllocator {
  using value_type = T;

  BumpAllocator() = default;

  template<typename U>
  BumpAllocator(const BumpAllocator<U> &) {}

  T *allocate(size_t n) {
    size_t size = n * sizeof(T);
    size_t align = alignof(T);
    assert(size <= bump_pos);
    bump_pos = (bump_pos - size) & ~(align - 1);
    return reinterpret_cast<T *>(bump_buffer + bump_pos);
  }

  void deallocate(T *, size_t) {}

  template<typename U>
  bool operator==(const BumpAllocator<U> &) const {
    return true;
  }

  template<typename U>
  bool operator!=(const BumpAllocator<U> &) const {
    return false;
  }
};

/*** Example Usage ***/

int main() {
  int *x = new int(42);
  assert(*x == 42);
  delete x;  // No-op.

  std::vector<int, BumpAllocator<int>> v;
  for (int i = 0; i < 1000; i++) {
    v.push_back(i);
  }
  assert(v[123] == 123);
  return 0;
}
