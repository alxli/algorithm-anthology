/*

Implements common singly linked list operations using raw nodes. Linked lists are rarely the best
contest choice compared with arrays, vectors, and `std::list`, but the pointer patterns are useful
for interview-style problems and for understanding constant-time splicing.

For production C++, prefer containers such as `std::list`, `std::forward_list`, or `std::vector`
when they fit the problem. Use the raw-node style below when the problem statement already gives
nodes, or when manual pointer manipulation is the point of the exercise.

- `reverse_list(head)` reverses a singly linked list and returns the new head.
- `merge_sorted_lists(a, b)` merges two sorted lists using a dummy node and returns the merged head.
- `split_half(head, &second)` cuts a list into two halves, returning the first half and storing the
  second half in `second`.

For linked-list cycle detection, use Floyd or Brent cycle detection from section 1.5.

Time Complexity:
- O(n) per call to `reverse_list(head)` and `split_half(head, &second)`.
- O(n + m) per call to `merge_sorted_lists(a, b)`.

Space Complexity:
- O(1) auxiliary for all operations.

*/

#include <cstddef>

struct ListNode {
  int value;
  ListNode *next;

  explicit ListNode(int value = 0) : value(value), next(nullptr) {}
};

ListNode *reverse_list(ListNode *head) {
  ListNode *prev = nullptr;
  while (head != nullptr) {
    ListNode *next = head->next;
    head->next = prev;
    prev = head;
    head = next;
  }
  return prev;
}

ListNode *merge_sorted_lists(ListNode *a, ListNode *b) {
  ListNode dummy;
  ListNode *tail = &dummy;
  while (a != nullptr && b != nullptr) {
    if (b->value < a->value) {
      tail->next = b;
      b = b->next;
    } else {
      tail->next = a;
      a = a->next;
    }
    tail = tail->next;
  }
  tail->next = a != nullptr ? a : b;
  return dummy.next;
}

ListNode *split_half(ListNode *head, ListNode **second) {
  if (head == nullptr || head->next == nullptr) {
    *second = nullptr;
    return head;
  }
  ListNode *slow = head, *fast = head->next;
  while (fast != nullptr && fast->next != nullptr) {
    slow = slow->next;
    fast = fast->next->next;
  }
  *second = slow->next;
  slow->next = nullptr;
  return head;
}

/*** Example Usage ***/

#include <cassert>
#include <vector>
using namespace std;

vector<int> values(ListNode *head) {
  vector<int> res;
  for (; head != nullptr; head = head->next) {
    res.push_back(head->value);
  }
  return res;
}

int main() {
  vector<ListNode> a{ListNode(1), ListNode(3), ListNode(5)};
  vector<ListNode> b{ListNode(2), ListNode(4), ListNode(6)};
  for (int i = 0; i + 1 < static_cast<int>(a.size()); i++) {
    a[i].next = &a[i + 1];
    b[i].next = &b[i + 1];
  }
  ListNode *merged = merge_sorted_lists(&a[0], &b[0]);
  vector<int> merged_values = values(merged);
  for (int i = 0; i < 6; i++) {
    assert(merged_values[i] == i + 1);
  }

  ListNode *second = nullptr;
  ListNode *first = split_half(merged, &second);
  assert(values(first).size() == 3);
  assert(values(second).size() == 3);
  assert(values(reverse_list(first))[0] == 3);
  return 0;
}
