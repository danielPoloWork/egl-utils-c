/*
 * d4np-c — intrusive doubly-linked list.
 *
 * SPDX-License-Identifier: MIT
 * Copyright (c) 2026 Daniel Polo
 *
 * Intrusive: the list links live inside the user's own struct via an embedded d4np_list_node_t,
 * so the list itself never allocates and a node can be removed in O(1) without a search. The
 * list keeps a sentinel node, making it circular and branch-free for insert/remove. Recover the
 * owning struct from a node with D4NP_LIST_ENTRY, and iterate with D4NP_LIST_FOR_EACH (spec #8).
 */
#ifndef D4NP_DS_LINKED_LIST_H
#define D4NP_DS_LINKED_LIST_H

#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct d4np_list_node {
    struct d4np_list_node *prev;
    struct d4np_list_node *next;
} d4np_list_node_t;

typedef struct d4np_linked_list {
    d4np_list_node_t sentinel; /* sentinel.next = first, sentinel.prev = last */
    size_t len;
} d4np_linked_list_t;

/* Recover a pointer to the enclosing struct from a member node pointer. */
#define D4NP_LIST_ENTRY(node_ptr, type, member) ((type *)((char *)(node_ptr) - offsetof(type, member)))

/* Forward iteration: `node` walks each live node (do not free/remove `node` mid-loop). */
#define D4NP_LIST_FOR_EACH(node, list)                                                                                 \
    for ((node) = (list)->sentinel.next; (node) != &(list)->sentinel; (node) = (node)->next)

/* Initialize an empty list. No allocation; the caller owns node storage. */
void d4np_linked_list_init(d4np_linked_list_t *list);

/* Insert `node` at the front / back. O(1). */
void d4np_linked_list_push_front(d4np_linked_list_t *list, d4np_list_node_t *node);
void d4np_linked_list_push_back(d4np_linked_list_t *list, d4np_list_node_t *node);

/* Remove and return the first / last node, or NULL if empty. O(1). */
d4np_list_node_t *d4np_linked_list_pop_front(d4np_linked_list_t *list);
d4np_list_node_t *d4np_linked_list_pop_back(d4np_linked_list_t *list);

/* Unlink a node known to be in `list`. O(1). */
void d4np_linked_list_remove(d4np_linked_list_t *list, d4np_list_node_t *node);

/* First / last node, or NULL if empty. */
d4np_list_node_t *d4np_linked_list_front(const d4np_linked_list_t *list);
d4np_list_node_t *d4np_linked_list_back(const d4np_linked_list_t *list);

bool d4np_linked_list_is_empty(const d4np_linked_list_t *list);
size_t d4np_linked_list_len(const d4np_linked_list_t *list);

#ifdef __cplusplus
}
#endif

#endif /* D4NP_DS_LINKED_LIST_H */
