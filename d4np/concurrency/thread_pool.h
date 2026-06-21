/**
 * @file thread_pool.h
 * @brief Fixed-size thread pool.
 *
 * SPDX-License-Identifier: MIT
 * Copyright (c) 2026 Daniel Polo
 *
 * A pool of native worker threads draining a shared FIFO task queue. Submitting a task is
 * O(1) and wait-free of the workers; workers block on a counting semaphore until work arrives,
 * so idle threads burn no CPU. The queue is guarded by a d4np_mutex_t and signalled by a
 * d4np_semaphore_t (this module dogfoods \#11/\#14) (spec \#12).
 *
 * Destroy performs a graceful shutdown: every task already submitted runs to completion, then
 * the workers stop and join. Submitting after shutdown has begun is rejected.
 *
 * Tasks may run on any worker, in parallel; a task that touches shared state must synchronize
 * itself. The pool guarantees each submitted task runs exactly once.
 *
 * @ingroup d4np_concurrency
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

/**
 * @addtogroup d4np_concurrency
 * @{
 */

/** @brief A unit of work: a function and its opaque argument. */
typedef void (*d4np_task_fn)(void *arg);

/** @brief Opaque task-queue node; defined in the implementation. */
typedef struct d4np_task_node d4np_task_node;

/** @brief A fixed-size pool of worker threads draining a shared FIFO task queue. */
typedef struct d4np_thread_pool {
    const d4np_allocator_t *allocator; /**< backing allocator for pool memory */
    void *threads;                     /**< opaque array of native thread handles */
    size_t thread_count;               /**< number of worker threads */
    d4np_mutex_t lock;                 /**< guards the task queue + shutting_down  */
    d4np_semaphore_t items;            /**< counts queued tasks / shutdown wakeups */
    d4np_task_node *q_head;            /**< head of the task queue */
    d4np_task_node *q_tail;            /**< tail of the task queue */
    bool shutting_down;                /**< set once graceful shutdown has begun */
} d4np_thread_pool_t;

/**
 * @brief Start a pool of worker threads.
 * @param pool         Pool to initialize.
 * @param allocator    Backing allocator, or NULL to select the default allocator.
 * @param thread_count Number of worker threads; must be greater than 0.
 * @return ::D4NP_OK on success; ::D4NP_ERR_INVALID_ARGUMENT, ::D4NP_ERR_OUT_OF_MEMORY, or
 *         ::D4NP_ERR_INTERNAL if thread/primitive creation failed.
 */
d4np_status_t d4np_thread_pool_init(d4np_thread_pool_t *pool, const d4np_allocator_t *allocator, size_t thread_count);

/**
 * @brief Submit a task to the pool.
 * @param pool Pool to submit to.
 * @param fn   Task function to run on a worker.
 * @param arg  Opaque argument passed to @p fn.
 * @return ::D4NP_OK on success; ::D4NP_ERR_INVALID_ARGUMENT, ::D4NP_ERR_OUT_OF_MEMORY, or
 *         ::D4NP_ERR_UNSUPPORTED if the pool is shutting down.
 * @note Thread-safe. Submitting after shutdown has begun is rejected. Tasks may run on any
 *       worker, in parallel; a task that touches shared state must synchronize itself.
 */
d4np_status_t d4np_thread_pool_submit(d4np_thread_pool_t *pool, d4np_task_fn fn, void *arg);

/**
 * @brief Graceful shutdown: run all queued tasks, stop and join the workers, release resources.
 * @param pool Pool to destroy.
 */
void d4np_thread_pool_destroy(d4np_thread_pool_t *pool);

/**
 * @brief Number of worker threads in the pool.
 * @param pool Pool to inspect.
 * @return The worker thread count.
 */
size_t d4np_thread_pool_thread_count(const d4np_thread_pool_t *pool);

/** @} */

#ifdef __cplusplus
}
#endif

#endif /* D4NP_CONCURRENCY_THREAD_POOL_H */
