/*

A bump allocator serves allocations consecutively from one fixed buffer and never reclaims
individual objects. This is useful in local contest code that creates many small objects whose
lifetimes all extend to the end of the program. Replacing global `operator new` preserves ordinary
allocation call sites while avoiding the time and per-allocation metadata of a general-purpose
allocator.

- `bump_alloc(size, align)` allocates `size` bytes with the requested alignment.
- `operator new(size)` allocates an aligned block of `size` bytes from the static buffer.
- `operator delete(ptr)` is a no-op; memory is reclaimed only when the program exits.
- `BumpAllocator<T>` provides the same monotonic allocation strategy to STL containers.

When only one object type is allocated, a typed pool such as `Node nodes[MAX_NODES]` with an index
into the next unused node is simpler and less invasive. The global override is useful when existing
code allocates multiple types, while `BumpAllocator<T>` can be attached to selected containers. The
two interfaces below share one buffer; a solution may keep only the interface it uses. This
contest-oriented snippet does not replace other allocation forms such as `new[]`, nothrow `new`, or
over-aligned allocation.

Time Complexity:
- O(1) per allocation.

Space Complexity:
- O(`BUFFER_SIZE`) static memory.

*/

#include <cassert>
#include <cstddef>
#include <vector>

static constexpr std::size_t BUFFER_SIZE = 64 << 20;  // 64 MiB; adjust to the memory limit.
alignas(std::max_align_t) static unsigned char buffer[BUFFER_SIZE];
static std::size_t buffer_pos = BUFFER_SIZE;

void *bump_alloc(std::size_t size, std::size_t align) {
  if (size == 0) {
    size = 1;
  }
  assert(size <= buffer_pos);
  buffer_pos = (buffer_pos - size) & ~(align - 1);
  return buffer + buffer_pos;
}

// clang-format off
void *operator new(std::size_t size) { return bump_alloc(size, alignof(std::max_align_t)); }
void operator delete(void *) noexcept {}
void operator delete(void *, std::size_t) noexcept {}
// clang-format on

template<typename T>
struct BumpAllocator {
  using value_type = T;

  BumpAllocator() = default;

  template<typename U>
  BumpAllocator(const BumpAllocator<U> &) {}

  T *allocate(std::size_t n) {
    static_assert(alignof(T) <= alignof(std::max_align_t), "over-aligned types are not supported");
    assert(n <= BUFFER_SIZE / sizeof(T));
    return reinterpret_cast<T *>(bump_alloc(n * sizeof(T), alignof(T)));
  }

  // clang-format off
  void deallocate(T *, std::size_t) {}
  template<typename U> bool operator==(const BumpAllocator<U> &) const { return true; }
  template<typename U> bool operator!=(const BumpAllocator<U> &) const { return false; }
  // clang-format on
};

/*** Example Usage ***/

using namespace std;

struct Node {
  int value;
  Node *next;
};

int main() {
  Node *tail = new Node{2, nullptr};
  Node *head = new Node{1, tail};
  assert(head->value == 1 && head->next->value == 2);

  vector<int, BumpAllocator<int>> values{1, 2, 3, 4};
  assert((values == vector<int, BumpAllocator<int>>{1, 2, 3, 4}));
  return 0;
}
