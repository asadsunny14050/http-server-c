#include <stdio.h>
#include <stdlib.h>

#include "../include/queue-ds.h"

int enqueue(Queue *queue, int client_fd, int client_id) {

  Node *new_tail = malloc(sizeof(Node));
  if (new_tail == NULL) {

    return -1;
  }
  new_tail->client_fd = client_fd;
  new_tail->client_id = client_id;
  new_tail->next = NULL;

  if (queue->tail == NULL) {
    queue->head = new_tail;

  } else {

    Node *current_tail = queue->tail;
    current_tail->next = new_tail;
  }

  queue->tail = new_tail;
  return 0;
}

Node *dequeue(Queue *queue) {
  if (queue->head == NULL) {
    return NULL;
  }

  Node *current_head = queue->head;
  Node *new_head = current_head->next;
  if (new_head == NULL) {
    queue->tail = new_head;
  }
  queue->head = new_head;

  return current_head;
}

void seed_queue(Queue *queue) {
  for (int i = 1; i < 3; i++) {
    enqueue(queue, i, i * 2);
  }
}

void print_queue(Node *head) {
  Node *tmp = head;
  if (tmp == NULL) {
    printf("Queue's empty, sire!\n");
  }
  while (tmp != NULL) {
    if (tmp == head) {

      printf("(%d, %d) ", tmp->client_fd, tmp->client_id);
    } else {

      printf("<- (%d, %d) ", tmp->client_fd, tmp->client_id);
    }
    tmp = tmp->next;
  }
  printf("\n");
}
