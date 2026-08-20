/*

A typed bump pool replaces many small heap allocations with consecutive slots in one fixed vector.
This simple form is useful for pointer-based trees, tries, and linked structures whose nodes all
live until the algorithm finishes. Allocation advances one index, and individual nodes are never
freed.

- `make_node(value, next = nullptr)` returns a pointer to a new node initialized with `value` and
  `next`.
- `BumpArena<N>` owns an inline $N$-byte monotonic buffer. Pass `arena.resource()` to a `std::pmr`
  container such as `std::pmr::vector<T>` or `std::pmr::map<K, V>`.

The vector is created at its final size and never resized, so returned pointers remain valid. Adjust
`MAX_NODES` and the fields of `Node` for the problem at hand.

For standard containers, the C++17 polymorphic allocator interface avoids both a global
`operator new` override and the full allocator protocol. An arena must outlive every container using
it; declare a large arena statically if it would exceed the stack limit. The wrapper uses
`std::pmr::null_memory_resource()` as its upstream resource, so exhausting the buffer throws
`std::bad_alloc` instead of silently allocating from the heap.

Time Complexity:
- O(`MAX_NODES`) for construction of the pool.
- O(1) per call to `make_node()`.

Space Complexity:
- O(`MAX_NODES`) for storage of the pool.
- O(N) for storage of each `BumpArena<N>`.

*/

#include <cassert>
#include <cstddef>
#include <memory_resource>
#include <vector>

struct Node {
  int value;
  Node *next;
};

static constexpr int MAX_NODES = 100000;
static std::vector<Node> node_pool(MAX_NODES);
static int node_count = 0;

Node *make_node(int value, Node *next = nullptr) {
  assert(node_count < MAX_NODES);
  node_pool[node_count] = {value, next};
  return &node_pool[node_count++];
}

template<std::size_t N>
class BumpArena {
  alignas(std::max_align_t) std::byte buffer[N];
  std::pmr::monotonic_buffer_resource arena{buffer, N, std::pmr::null_memory_resource()};

 public:
  std::pmr::memory_resource *resource() { return &arena; }
};

/*** Example Usage ***/

#include <map>
using namespace std;

int main() {
  Node *tail = make_node(2);
  Node *head = make_node(1, tail);
  assert(head->value == 1 && head->next->value == 2);

  BumpArena<1024> arena;
  pmr::vector<int> values(arena.resource());
  values.reserve(4);
  values.insert(values.end(), {1, 2, 3, 4});
  assert(values.size() == 4 && values.back() == 4);

  pmr::map<int, int> counts(arena.resource());
  counts.emplace(7, 2);
  assert(counts[7] == 2);
  return 0;
}
