/*
 * d4np-c — intrusive doubly-linked list (implementation).
 *
 * SPDX-License-Identifier: MIT
 * Copyright (c) 2026 Daniel Polo
 */
#include "d4np/ds/linked_list.h"

/* Splice `node` between `prev` and `next` (which must be adjacent). */
static void link_between(d4np_list_node_t *node, d4np_list_node_t *prev, d4np_list_node_t *next)
{
    node->prev = prev;
    node->next = next;
    prev->next = node;
    next->prev = node;
}

static void unlink(d4np_list_node_t *node)
{
    node->prev->next = node->next;
    node->next->prev = node->prev;
    node->prev = NULL;
    node->next = NULL;
}

void d4np_linked_list_init(d4np_linked_list_t *list)
{
    if (list == NULL) {
        return;
    }
    list->sentinel.next = &list->sentinel;
    list->sentinel.prev = &list->sentinel;
    list->len = 0;
}

void d4np_linked_list_push_front(d4np_linked_list_t *list, d4np_list_node_t *node)
{
    if (list == NULL || node == NULL) {
        return;
    }
    link_between(node, &list->sentinel, list->sentinel.next);
    list->len++;
}

void d4np_linked_list_push_back(d4np_linked_list_t *list, d4np_list_node_t *node)
{
    if (list == NULL || node == NULL) {
        return;
    }
    link_between(node, list->sentinel.prev, &list->sentinel);
    list->len++;
}

d4np_list_node_t *d4np_linked_list_pop_front(d4np_linked_list_t *list)
{
    if (list == NULL || list->len == 0) {
        return NULL;
    }
    d4np_list_node_t *node = list->sentinel.next;
    unlink(node);
    list->len--;
    return node;
}

d4np_list_node_t *d4np_linked_list_pop_back(d4np_linked_list_t *list)
{
    if (list == NULL || list->len == 0) {
        return NULL;
    }
    d4np_list_node_t *node = list->sentinel.prev;
    unlink(node);
    list->len--;
    return node;
}

void d4np_linked_list_remove(d4np_linked_list_t *list, d4np_list_node_t *node)
{
    if (list == NULL || node == NULL || list->len == 0) {
        return;
    }
    unlink(node);
    list->len--;
}

d4np_list_node_t *d4np_linked_list_front(const d4np_linked_list_t *list)
{
    if (list == NULL || list->len == 0) {
        return NULL;
    }
    return list->sentinel.next;
}

d4np_list_node_t *d4np_linked_list_back(const d4np_linked_list_t *list)
{
    if (list == NULL || list->len == 0) {
        return NULL;
    }
    return list->sentinel.prev;
}

bool d4np_linked_list_is_empty(const d4np_linked_list_t *list)
{
    return (list == NULL) || (list->len == 0);
}

size_t d4np_linked_list_len(const d4np_linked_list_t *list)
{
    return (list != NULL) ? list->len : 0;
}
