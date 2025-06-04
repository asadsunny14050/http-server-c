#pragma once

typedef struct node {
  int client_fd;
  int client_id;
  struct node *next;
} Node;

typedef struct {
  Node *head;
  Node *tail;
} Queue;

int enqueue(Queue *queue, int client_fd, int client_id);

Node *dequeue(Queue *queue);

void print_queue(Node *head);
