/*

Implements common doubly linked list operations using raw intrusive nodes. A node is intrusive when
the `prev` and `next` pointers live inside the stored object itself, rather than inside a separate
container allocation. This is useful when a problem already gives node pointers, when values must
not be copied, or when a map needs to jump directly to a list node in O(1) time, as in an LRU cache.

The functions below use a circular sentinel node. When the list is empty, `sentinel.next` and
`sentinel.prev` both point back to `sentinel`. This removes special cases for inserting or erasing
at the front and back of the list.

- `init_list(sentinel)` initializes an empty circular list around `sentinel`.
- `insert_after(pos, node)` inserts detached `node` immediately after `pos`.
- `insert_before(pos, node)` inserts detached `node` immediately before `pos`.
- `erase(node)` removes `node` from its current list and leaves it detached.
- `push_front(sentinel, node)` inserts detached `node` at the front of the list.
- `push_back(sentinel, node)` inserts detached `node` at the back of the list.
- `move_to_front(sentinel, node)` moves an already-linked `node` to the front of the list.
- `splice(pos, node)` moves an already-linked `node` so it appears immediately before `pos`.
- `splice_range(pos, first, last)` moves the half-open range [`first`, `last`) so it appears
  immediately before `pos`.
- `empty(sentinel)` returns whether the list has no data nodes.

The data nodes passed to insertion functions must be detached, meaning their `prev` and `next`
pointers are both null. The node passed to `erase()` or `move_to_front()` must already be linked
into a list and must not be the sentinel. For `splice_range(pos, first, last)`, the `pos` node must
not lie inside the moved range.

Time Complexity:
- O(1) per call to all operations.

Space Complexity:
- O(1) auxiliary for all operations.

*/

#include <cstddef>

struct DListNode {
  int value;
  DListNode *prev, *next;

  explicit DListNode(int value = 0) : value(value), prev(nullptr), next(nullptr) {}
};

void init_list(DListNode *sentinel) {
  sentinel->prev = sentinel;
  sentinel->next = sentinel;
}

bool empty(DListNode *sentinel) {
  return sentinel->next == sentinel;
}

void insert_after(DListNode *pos, DListNode *node) {
  node->prev = pos;
  node->next = pos->next;
  pos->next->prev = node;
  pos->next = node;
}

void insert_before(DListNode *pos, DListNode *node) {
  insert_after(pos->prev, node);
}

DListNode *erase(DListNode *node) {
  node->prev->next = node->next;
  node->next->prev = node->prev;
  node->prev = nullptr;
  node->next = nullptr;
  return node;
}

void push_front(DListNode *sentinel, DListNode *node) {
  insert_after(sentinel, node);
}

void push_back(DListNode *sentinel, DListNode *node) {
  insert_before(sentinel, node);
}

void move_to_front(DListNode *sentinel, DListNode *node) {
  push_front(sentinel, erase(node));
}

void splice(DListNode *pos, DListNode *node) {
  if (pos == node || pos == node->next) {
    return;
  }
  insert_before(pos, erase(node));
}

void splice_range(DListNode *pos, DListNode *first, DListNode *last) {
  if (first == last || pos == first) {
    return;
  }
  DListNode *before_first = first->prev;
  DListNode *tail = last->prev;
  before_first->next = last;
  last->prev = before_first;

  DListNode *before_pos = pos->prev;
  before_pos->next = first;
  first->prev = before_pos;
  tail->next = pos;
  pos->prev = tail;
}

/*** Example Usage ***/

#include <cassert>
#include <vector>
using namespace std;

// Returns the data values from front to back (walks the circular list until it loops to sentinel).
vector<int> values(DListNode *sentinel) {
  vector<int> res;
  for (DListNode *p = sentinel->next; p != sentinel; p = p->next) {
    res.push_back(p->value);
  }
  return res;
}

int main() {
  // `dummy` is the sentinel; a, b, c, d start detached. push_back/push_front link at back/front.
  DListNode dummy, a(1), b(2), c(3), d(4);
  init_list(&dummy);
  assert(empty(&dummy));

  push_back(&dummy, &a);
  push_back(&dummy, &b);
  push_back(&dummy, &c);
  push_front(&dummy, &d);
  assert((values(&dummy) == vector<int>{4, 1, 2, 3}));

  move_to_front(&dummy, &b);  // list: 2 4 1 3
  assert((values(&dummy) == vector<int>{2, 4, 1, 3}));

  erase(&d);  // detach d; list: 2 1 3
  assert((values(&dummy) == vector<int>{2, 1, 3}));

  insert_before(&dummy, &d);  // before the sentinel == the back; list: 2 1 3 4
  assert((values(&dummy) == vector<int>{2, 1, 3, 4}));

  splice(&d, &a);  // move a to just before d; list: 2 3 1 4
  assert((values(&dummy) == vector<int>{2, 3, 1, 4}));

  splice_range(&dummy, &a, &dummy);  // range {a, d} already at the back: no-op
  assert((values(&dummy) == vector<int>{2, 3, 1, 4}));

  splice_range(&b, &a, &dummy);  // move {a, d} to before b; list: 1 4 2 3
  assert((values(&dummy) == vector<int>{1, 4, 2, 3}));
  return 0;
}
