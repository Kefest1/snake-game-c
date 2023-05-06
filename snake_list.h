//
// Created by root on 4/29/23.
//

#ifndef SNAKE_SNAKE_LIST_H
#define SNAKE_SNAKE_LIST_H

#include "snake.h"

struct node_t {
    component_t component;
    struct node_t *next;
};

struct linked_list_t {
    int size;
    struct node_t *head;
    struct node_t *tail;
};

struct linked_list_t *listCreate(void);
int list_push(struct linked_list_t *list, int x, int y);
struct node_t *list_get_node_by_index(struct linked_list_t *list, int x);
#endif //SNAKE_SNAKE_LIST_H
