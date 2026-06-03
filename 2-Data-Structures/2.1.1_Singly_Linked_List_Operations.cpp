/*

Implements common singly linked list operations using raw nodes. Linked lists are
rarely the best contest choice compared with arrays, vectors, and `std::list`, but
the pointer patterns are useful for interview-style problems and for understanding
constant-time splicing.

For production C++, prefer containers such as `std::list`, `std::forward_list`, or
`std::vector` when they fit the problem. Use the raw-node style below when the
problem statement already gives nodes, or when manual pointer manipulation is the
point of the exercise.

- `reverse_list(head)` reverses a singly linked list and returns the new head.
- `merge_sorted_lists(a, b)` merges two sorted lists using a dummy node and returns
  the merged head.
- `split_half(head, &second)` cuts a list into two halves, returning the first half
  and storing the second half in `second`.
- For linked-list cycle detection, use Floyd or Brent cycle detection from
  Chapter 1, Section 4.

Time Complexity:
- O(n) per call to `reverse_list(head)` and `split_half(head, &second)`.
- O(n + m) per call to `merge_sorted_lists(a, b)`.

Space Complexity:
- O(1) auxiliary for all operations.

*/

#include <cstddef>

struct list_node {
  int value;
  list_node *next;

  explicit list_node(int value = 0) : value(value), next(NULL) {}
};

list_node* reverse_list(list_node *head) {
  list_node *prev = NULL;
  while (head != NULL) {
    list_node *next = head->next;
    head->next = prev;
    prev = head;
    head = next;
  }
  return prev;
}

list_node* merge_sorted_lists(list_node *a, list_node *b) {
  list_node dummy;
  list_node *tail = &dummy;
  while (a != NULL && b != NULL) {
    if (b->value < a->value) {
      tail->next = b;
      b = b->next;
    } else {
      tail->next = a;
      a = a->next;
    }
    tail = tail->next;
  }
  tail->next = a != NULL ? a : b;
  return dummy.next;
}

list_node* split_half(list_node *head, list_node **second) {
  if (head == NULL || head->next == NULL) {
    *second = NULL;
    return head;
  }
  list_node *slow = head, *fast = head->next;
  while (fast != NULL && fast->next != NULL) {
    slow = slow->next;
    fast = fast->next->next;
  }
  *second = slow->next;
  slow->next = NULL;
  return head;
}

/*** Example Usage ***/

#include <cassert>
#include <vector>
using namespace std;

vector<int> values(list_node *head) {
  vector<int> res;
  for (; head != NULL; head = head->next) {
    res.push_back(head->value);
  }
  return res;
}

int main() {
  list_node a[3] = {list_node(1), list_node(3), list_node(5)};
  list_node b[3] = {list_node(2), list_node(4), list_node(6)};
  for (int i = 0; i + 1 < 3; i++) {
    a[i].next = &a[i + 1];
    b[i].next = &b[i + 1];
  }
  list_node *merged = merge_sorted_lists(&a[0], &b[0]);
  vector<int> merged_values = values(merged);
  for (int i = 0; i < 6; i++) {
    assert(merged_values[i] == i + 1);
  }

  list_node *second = NULL;
  list_node *first = split_half(merged, &second);
  assert(values(first).size() == 3);
  assert(values(second).size() == 3);
  assert(values(reverse_list(first))[0] == 3);
  return 0;
}
