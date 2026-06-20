/*
 * d4np-c — fixed-size thread pool (implementation).
 *
 * SPDX-License-Identifier: MIT
 * Copyright (c) 2026 Daniel Polo
 *
 * Shutdown protocol: destroy sets shutting_down and posts one permit per worker. A worker wakes
 * on every permit; it runs a task whenever the queue is non-empty and only exits when it wakes
 * to an empty queue. Since there is exactly one permit per submitted task plus one per worker,
 * all submitted tasks run before any worker observes the empty queue and exits — a graceful drain.
 */
#include "d4np/concurrency/thread_pool.h"

#include <stdalign.h>

struct d4np_task_node {
    struct d4np_task_node *next;
    d4np_task_fn fn;
    void *arg;
};

static void pool_worker(d4np_thread_pool_t *pool);

/* ---- platform threads -------------------------------------------------- */
#if defined(_WIN32)
#include <windows.h>
typedef HANDLE pool_thread_t;
static DWORD WINAPI pool_worker_main(LPVOID arg)
{
    pool_worker((d4np_thread_pool_t *)arg);
    return 0;
}
static bool pool_spawn(pool_thread_t *t, d4np_thread_pool_t *pool)
{
    HANDLE h = CreateThread(NULL, 0, pool_worker_main, pool, 0, NULL);
    if (h == NULL) {
        return false;
    }
    *t = h;
    return true;
}
static void pool_join(pool_thread_t t)
{
    WaitForSingleObject(t, INFINITE);
    CloseHandle(t);
}
#else
#include <pthread.h>
typedef pthread_t pool_thread_t;
static void *pool_worker_main(void *arg)
{
    pool_worker((d4np_thread_pool_t *)arg);
    return NULL;
}
static bool pool_spawn(pool_thread_t *t, d4np_thread_pool_t *pool)
{
    return pthread_create(t, NULL, pool_worker_main, pool) == 0;
}
static void pool_join(pool_thread_t t)
{
    (void)pthread_join(t, NULL);
}
#endif

/* ---- worker ------------------------------------------------------------ */

static void pool_worker(d4np_thread_pool_t *pool)
{
    for (;;) {
        d4np_semaphore_wait(&pool->items);

        d4np_mutex_lock(&pool->lock);
        d4np_task_node *node = pool->q_head;
        if (node == NULL) {
            /* Woken with no work: this is a shutdown signal. */
            d4np_mutex_unlock(&pool->lock);
            break;
        }
        pool->q_head = node->next;
        if (pool->q_head == NULL) {
            pool->q_tail = NULL;
        }
        d4np_mutex_unlock(&pool->lock);

        node->fn(node->arg);
        d4np_free(pool->allocator, node, sizeof(*node));
    }
}

/* ---- public API -------------------------------------------------------- */

d4np_status_t d4np_thread_pool_init(d4np_thread_pool_t *pool, const d4np_allocator_t *allocator,
                                    size_t thread_count)
{
    if (pool == NULL || thread_count == 0) {
        return D4NP_ERR_INVALID_ARGUMENT;
    }
    if (thread_count > ((size_t)-1) / sizeof(pool_thread_t)) {
        return D4NP_ERR_OVERFLOW;
    }
    allocator = (allocator != NULL) ? allocator : d4np_allocator_default();

    pool->allocator = allocator;
    pool->thread_count = thread_count;
    pool->q_head = NULL;
    pool->q_tail = NULL;
    pool->shutting_down = false;
    pool->threads = NULL;

    if (d4np_mutex_init(&pool->lock) != D4NP_OK) {
        return D4NP_ERR_INTERNAL;
    }
    if (d4np_semaphore_init(&pool->items, 0) != D4NP_OK) {
        d4np_mutex_destroy(&pool->lock);
        return D4NP_ERR_INTERNAL;
    }
    pool->threads = d4np_alloc(allocator, thread_count * sizeof(pool_thread_t), alignof(pool_thread_t));
    if (pool->threads == NULL) {
        d4np_semaphore_destroy(&pool->items);
        d4np_mutex_destroy(&pool->lock);
        return D4NP_ERR_OUT_OF_MEMORY;
    }

    pool_thread_t *handles = (pool_thread_t *)pool->threads;
    for (size_t i = 0; i < thread_count; ++i) {
        if (!pool_spawn(&handles[i], pool)) {
            /* Roll back: wake and join the workers already started, then release. */
            pool->thread_count = i;
            d4np_mutex_lock(&pool->lock);
            pool->shutting_down = true;
            d4np_mutex_unlock(&pool->lock);
            for (size_t k = 0; k < i; ++k) {
                d4np_semaphore_post(&pool->items);
            }
            for (size_t k = 0; k < i; ++k) {
                pool_join(handles[k]);
            }
            d4np_free(allocator, pool->threads, thread_count * sizeof(pool_thread_t));
            pool->threads = NULL;
            d4np_semaphore_destroy(&pool->items);
            d4np_mutex_destroy(&pool->lock);
            return D4NP_ERR_INTERNAL;
        }
    }
    return D4NP_OK;
}

d4np_status_t d4np_thread_pool_submit(d4np_thread_pool_t *pool, d4np_task_fn fn, void *arg)
{
    if (pool == NULL || fn == NULL) {
        return D4NP_ERR_INVALID_ARGUMENT;
    }
    d4np_task_node *node =
        (d4np_task_node *)d4np_alloc(pool->allocator, sizeof(*node), alignof(d4np_task_node));
    if (node == NULL) {
        return D4NP_ERR_OUT_OF_MEMORY;
    }
    node->fn = fn;
    node->arg = arg;
    node->next = NULL;

    d4np_mutex_lock(&pool->lock);
    if (pool->shutting_down) {
        d4np_mutex_unlock(&pool->lock);
        d4np_free(pool->allocator, node, sizeof(*node));
        return D4NP_ERR_UNSUPPORTED;
    }
    if (pool->q_tail != NULL) {
        pool->q_tail->next = node;
    } else {
        pool->q_head = node;
    }
    pool->q_tail = node;
    d4np_mutex_unlock(&pool->lock);

    d4np_semaphore_post(&pool->items);
    return D4NP_OK;
}

void d4np_thread_pool_destroy(d4np_thread_pool_t *pool)
{
    if (pool == NULL || pool->threads == NULL) {
        return;
    }
    d4np_mutex_lock(&pool->lock);
    pool->shutting_down = true;
    d4np_mutex_unlock(&pool->lock);

    for (size_t i = 0; i < pool->thread_count; ++i) {
        d4np_semaphore_post(&pool->items);
    }
    pool_thread_t *handles = (pool_thread_t *)pool->threads;
    for (size_t i = 0; i < pool->thread_count; ++i) {
        pool_join(handles[i]);
    }

    /* Workers have stopped; drain any node that somehow remains (defensive). */
    d4np_task_node *node = pool->q_head;
    while (node != NULL) {
        d4np_task_node *next = node->next;
        d4np_free(pool->allocator, node, sizeof(*node));
        node = next;
    }
    pool->q_head = NULL;
    pool->q_tail = NULL;

    d4np_free(pool->allocator, pool->threads, pool->thread_count * sizeof(pool_thread_t));
    pool->threads = NULL;
    d4np_semaphore_destroy(&pool->items);
    d4np_mutex_destroy(&pool->lock);
}

size_t d4np_thread_pool_thread_count(const d4np_thread_pool_t *pool)
{
    return (pool != NULL) ? pool->thread_count : 0;
}
