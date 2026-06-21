/**
 * @file linked_list.h
 * @brief Intrusive doubly-linked list.
 *
 * SPDX-License-Identifier: MIT
 * Copyright (c) 2026 Daniel Polo
 *
 * Intrusive: the list links live inside the user's own struct via an embedded d4np_list_node_t,
 * so the list itself never allocates and a node can be removed in O(1) without a search. The
 * list keeps a sentinel node, making it circular and branch-free for insert/remove. Recover the
 * owning struct from a node with D4NP_LIST_ENTRY, and iterate with D4NP_LIST_FOR_EACH (spec \#8).
 *
 * @ingroup d4np_ds
 */
#ifndef D4NP_DS_LINKED_LIST_H
#define D4NP_DS_LINKED_LIST_H

#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @addtogroup d4np_ds
 * @{
 */

/** @brief A link embedded in the user's struct to enroll it in a list. */
typedef struct d4np_list_node {
    struct d4np_list_node *prev; /**< previous node, or the sentinel */
    struct d4np_list_node *next; /**< next node, or the sentinel     */
} d4np_list_node_t;

/** @brief A circular intrusive list anchored by a sentinel node. */
typedef struct d4np_linked_list {
    d4np_list_node_t sentinel; /**< sentinel.next = first, sentinel.prev = last */
    size_t len;                /**< number of live nodes                         */
} d4np_linked_list_t;

/**
 * @brief Recover a pointer to the enclosing struct from a member node pointer.
 * @param node_ptr Pointer to the embedded node.
 * @param type     Type of the enclosing struct.
 * @param member   Name of the node member within @p type.
 */
#define D4NP_LIST_ENTRY(node_ptr, type, member) ((type *)((char *)(node_ptr) - offsetof(type, member)))

/**
 * @brief Forward iteration: @p node walks each live node.
 * @param node Loop variable of type ::d4np_list_node_t* updated each iteration.
 * @param list List to iterate.
 * @note Do not free or remove @p node mid-loop.
 */
#define D4NP_LIST_FOR_EACH(node, list)                                                                                 \
    for ((node) = (list)->sentinel.next; (node) != &(list)->sentinel; (node) = (node)->next)

/**
 * @brief Initialize an empty list.
 * @param list List to initialize.
 * @note No allocation; the caller owns node storage.
 */
void d4np_linked_list_init(d4np_linked_list_t *list);

/**
 * @brief Insert a node at the front.
 * @param list List to insert into.
 * @param node Node to insert. O(1).
 */
void d4np_linked_list_push_front(d4np_linked_list_t *list, d4np_list_node_t *node);
/**
 * @brief Insert a node at the back.
 * @param list List to insert into.
 * @param node Node to insert. O(1).
 */
void d4np_linked_list_push_back(d4np_linked_list_t *list, d4np_list_node_t *node);

/**
 * @brief Remove and return the first node.
 * @param list List to pop from.
 * @return The removed node, or NULL if empty. O(1).
 */
d4np_list_node_t *d4np_linked_list_pop_front(d4np_linked_list_t *list);
/**
 * @brief Remove and return the last node.
 * @param list List to pop from.
 * @return The removed node, or NULL if empty. O(1).
 */
d4np_list_node_t *d4np_linked_list_pop_back(d4np_linked_list_t *list);

/**
 * @brief Unlink a node known to be in the list.
 * @param list List containing the node.
 * @param node Node to unlink. O(1).
 */
void d4np_linked_list_remove(d4np_linked_list_t *list, d4np_list_node_t *node);

/**
 * @brief First node.
 * @param list List to query.
 * @return The first node, or NULL if empty.
 */
d4np_list_node_t *d4np_linked_list_front(const d4np_linked_list_t *list);
/**
 * @brief Last node.
 * @param list List to query.
 * @return The last node, or NULL if empty.
 */
d4np_list_node_t *d4np_linked_list_back(const d4np_linked_list_t *list);

/** @brief Whether the list holds no nodes. */
bool d4np_linked_list_is_empty(const d4np_linked_list_t *list);
/** @brief Number of live nodes. */
size_t d4np_linked_list_len(const d4np_linked_list_t *list);

/** @} */

#ifdef __cplusplus
}
#endif

#endif /* D4NP_DS_LINKED_LIST_H */
