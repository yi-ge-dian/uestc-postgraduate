#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>

typedef struct Node {
  int data;
  struct Node *next;
} Node;

Node *head = nullptr;

void add(int num) {
  Node *t = (Node *)malloc(sizeof(Node));
  t->data = num;
  t->next = head;
  head = t;
}

void printList(Node *n) {
  Node *t = n;
  while (t != nullptr) {
    std::cout << t->data << std::endl;
    t = t->next;
  }
}

int main() {
  for (int i = 0; i < 10; i++) {
    add(i);
  }
  printList(head);
  return 0;
}