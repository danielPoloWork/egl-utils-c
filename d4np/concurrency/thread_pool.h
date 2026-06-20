/*
 * d4np-c — fixed-size thread pool.
 *
 * SPDX-License-Identifier: MIT
 * Copyright (c) 2026 Daniel Polo
 *
 * A pool of native worker threads draining a shared FIFO task queue. Submitting a task is
 * O(1) and wait-free of the workers; workers block on a counting semaphore until work arrives,
 * so idle threads burn no CPU. The queue is guarded by a d4np_mutex_t and signalled by a
 * d4np_semaphore_t (this module dogfoods #11/#14) (spec #12).
 *
 * Destroy performs a graceful shutdown: every task already submitted runs to completion, then
 * the workers stop and join. Submitting after shutdown has begun is rejected.
 *
 * Tasks may run on any worker, in parallel; a task that touches shared state must synchronize
 * itself. The pool guarantees each submitted task runs exactly once.
 */
#ifndef D4NP_CONCURRENCY_THREAD_POOL_H
#define D4NP_CONCURRENCY_THREAD_POOL_H

#include <stdbool.h>
#include <stddef.h>

#include "d4np/concurrency/mutex.h"
#include "d4np/concurrency/semaphore.h"
#include "d4np/core/d4np_allocator.h"
#include "d4np/core/d4np_status.h"

#ifdef __cplusplus
extern "C" {
#endif

/* A unit of work: a function and its opaque argument. */
typedef void (*d4np_task_fn)(void *arg);

/* Opaque task-queue node; defined in the implementation. */
typedef struct d4np_task_node d4np_task_node;

typedef struct d4np_thread_pool {
    const d4np_allocator_t *allocator;
    void *threads;          /* opaque array of native thread handles */
    size_t thread_count;
    d4np_mutex_t lock;      /* guards the task queue + shutting_down  */
    d4np_semaphore_t items; /* counts queued tasks / shutdown wakeups */
    d4np_task_node *q_head;
    d4np_task_node *q_tail;
    bool shutting_down;
} d4np_thread_pool_t;

/*
 * Start a pool of `thread_count` (> 0) worker threads, drawing memory from `allocator`
 * (NULL -> default). Returns D4NP_OK, D4NP_ERR_INVALID_ARGUMENT, D4NP_ERR_OUT_OF_MEMORY, or
 * D4NP_ERR_INTERNAL (thread/primitive creation failed).
 */
d4np_status_t d4np_thread_pool_init(d4np_thread_pool_t *pool, const d4np_allocator_t *allocator,
                                    size_t thread_count);

/*
 * Submit a task. Returns D4NP_OK, D4NP_ERR_INVALID_ARGUMENT, D4NP_ERR_OUT_OF_MEMORY, or
 * D4NP_ERR_UNSUPPORTED if the pool is shutting down.
 */
d4np_status_t d4np_thread_pool_submit(d4np_thread_pool_t *pool, d4np_task_fn fn, void *arg);

/* Graceful shutdown: run all queued tasks, stop and join the workers, release resources. */
void d4np_thread_pool_destroy(d4np_thread_pool_t *pool);

size_t d4np_thread_pool_thread_count(const d4np_thread_pool_t *pool);

#ifdef __cplusplus
}
#endif

#endif /* D4NP_CONCURRENCY_THREAD_POOL_H */
