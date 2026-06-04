/*

Moves nodes or ranges between singly linked lists in O(1) pointer changes once the neighboring nodes
are known. Splicing is the main operation that makes linked lists useful: it moves existing nodes
without copying values or invalidating pointers to the moved nodes.

The functions below use a dummy head node for each list. This makes insertions and removals at the
beginning of a list match all other positions. In the C++ standard library, `std::list::splice()`
provides analogous operations for doubly linked lists, while `std::forward_list::splice_after()`
provides analogous operations for singly linked lists.

- `splice_after(pos, before)` moves the node after `before` so it appears after `pos`.
- `splice_range_after(pos, before_first, last)` moves the half-open range `(before_first, last)` so
  it appears after `pos`.

Time Complexity:
- O(1) per call to `splice_after(pos, before)`.
- O(k) per call to `splice_range_after(pos, before_first, last)`, where $k$ is the number of moved
  nodes. The actual pointer splicing is O(1), but this compact version walks to the end of the moved
  range.

Space Complexity:
- O(1) auxiliary.

*/

#include <cstddef>

struct ListNode {
  int value;
  ListNode *next;

  explicit ListNode(int value = 0) : value(value), next(nullptr) {}
};

void splice_after(ListNode *pos, ListNode *before) {
  ListNode *node = before->next;
  if (node == nullptr || pos == before || pos == node) {
    return;
  }
  before->next = node->next;
  node->next = pos->next;
  pos->next = node;
}

void splice_range_after(ListNode *pos, ListNode *before_first, ListNode *last) {
  ListNode *first = before_first->next;
  if (first == last) {
    return;
  }
  ListNode *tail = first;
  while (tail->next != last) {
    tail = tail->next;
  }
  before_first->next = last;
  tail->next = pos->next;
  pos->next = first;
}

/*** Example Usage ***/

#include <cassert>
#include <vector>
using namespace std;

vector<int> values(ListNode *dummy) {
  vector<int> res;
  for (ListNode *p = dummy->next; p != nullptr; p = p->next) {
    res.push_back(p->value);
  }
  return res;
}

int main() {
  ListNode dummy(0), a(1), b(2), c(3), d(4), e(5);
  dummy.next = &a;
  a.next = &b;
  b.next = &c;
  c.next = &d;
  d.next = &e;

  splice_after(&dummy, &c);  // Move 4 to the front: 4, 1, 2, 3, 5.
  vector<int> v = values(&dummy);
  vector<int> expected1{4, 1, 2, 3, 5};
  for (int i = 0; i < static_cast<int>(expected1.size()); i++) {
    assert(v[i] == expected1[i]);
  }

  splice_range_after(&d, &a, &e);  // Move 2, 3 after 4: 4, 2, 3, 1, 5.
  v = values(&dummy);
  vector<int> expected2{4, 2, 3, 1, 5};
  for (int i = 0; i < static_cast<int>(expected2.size()); i++) {
    assert(v[i] == expected2[i]);
  }
  return 0;
}
