/*
 * d4np-c — tests for the intrusive doubly-linked list (#8).
 *
 * SPDX-License-Identifier: MIT
 * Copyright (c) 2026 Daniel Polo
 */
#include "unity.h"

#include "d4np_c.h"

/* A user struct that embeds a list node — the intrusive pattern. */
typedef struct item {
    int value;
    d4np_list_node_t node;
} item_t;

static int value_of(const d4np_list_node_t *n)
{
    return D4NP_LIST_ENTRY(n, item_t, node)->value;
}

static void test_empty(void)
{
    d4np_linked_list_t list;
    d4np_linked_list_init(&list);
    TEST_ASSERT_TRUE(d4np_linked_list_is_empty(&list));
    TEST_ASSERT_EQUAL_size_t(0, d4np_linked_list_len(&list));
    TEST_ASSERT_NULL(d4np_linked_list_front(&list));
    TEST_ASSERT_NULL(d4np_linked_list_back(&list));
    TEST_ASSERT_NULL(d4np_linked_list_pop_front(&list));
}

static void test_push_back_order_and_iterate(void)
{
    d4np_linked_list_t list;
    d4np_linked_list_init(&list);
    item_t items[3] = {{10, {0}}, {20, {0}}, {30, {0}}};
    for (int i = 0; i < 3; ++i) {
        d4np_linked_list_push_back(&list, &items[i].node);
    }
    TEST_ASSERT_EQUAL_size_t(3, d4np_linked_list_len(&list));
    TEST_ASSERT_EQUAL_INT(10, value_of(d4np_linked_list_front(&list)));
    TEST_ASSERT_EQUAL_INT(30, value_of(d4np_linked_list_back(&list)));

    int expect[] = {10, 20, 30};
    int n = 0;
    d4np_list_node_t *node;
    D4NP_LIST_FOR_EACH(node, &list) {
        TEST_ASSERT_EQUAL_INT(expect[n], value_of(node));
        ++n;
    }
    TEST_ASSERT_EQUAL_INT(3, n);
}

static void test_push_front_order(void)
{
    d4np_linked_list_t list;
    d4np_linked_list_init(&list);
    item_t a = {1, {0}}, b = {2, {0}};
    d4np_linked_list_push_front(&list, &a.node);
    d4np_linked_list_push_front(&list, &b.node); /* b now first */
    TEST_ASSERT_EQUAL_INT(2, value_of(d4np_linked_list_front(&list)));
    TEST_ASSERT_EQUAL_INT(1, value_of(d4np_linked_list_back(&list)));
}

static void test_pop_and_remove_middle(void)
{
    d4np_linked_list_t list;
    d4np_linked_list_init(&list);
    item_t items[3] = {{10, {0}}, {20, {0}}, {30, {0}}};
    for (int i = 0; i < 3; ++i) {
        d4np_linked_list_push_back(&list, &items[i].node);
    }

    /* Remove the middle node in O(1) without searching. */
    d4np_linked_list_remove(&list, &items[1].node);
    TEST_ASSERT_EQUAL_size_t(2, d4np_linked_list_len(&list));
    TEST_ASSERT_EQUAL_INT(10, value_of(d4np_linked_list_front(&list)));
    TEST_ASSERT_EQUAL_INT(30, value_of(d4np_linked_list_back(&list)));

    d4np_list_node_t *f = d4np_linked_list_pop_front(&list);
    TEST_ASSERT_EQUAL_INT(10, value_of(f));
    d4np_list_node_t *b = d4np_linked_list_pop_back(&list);
    TEST_ASSERT_EQUAL_INT(30, value_of(b));
    TEST_ASSERT_TRUE(d4np_linked_list_is_empty(&list));
}

void suite_linked_list(void)
{
    RUN_TEST(test_empty);
    RUN_TEST(test_push_back_order_and_iterate);
    RUN_TEST(test_push_front_order);
    RUN_TEST(test_pop_and_remove_middle);
}
