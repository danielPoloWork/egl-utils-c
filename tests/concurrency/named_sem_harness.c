/*
 * d4np-c — multi-process verification harness for the named/IPC semaphore (#14, M8.5).
 *
 * SPDX-License-Identifier: MIT
 * Copyright (c) 2026 Daniel Polo
 *
 * This is a standalone executable, not a Unity test: it needs two real OS processes. The parent
 * re-launches this same binary as a child, then the two run a lock-step ping-pong over two named
 * semaphores. Because each process opens the semaphores independently *by name*, a clean run
 * proves cross-process (kernel-backed) signaling — not just thread synchronization.
 *
 *   parent: for ROUNDS { post(to_child); wait(to_parent); }
 *   child : for ROUNDS { wait(to_child);  post(to_parent); }
 *
 * Exit code 0 means success; anything else is a failure the CTest wrapper reports.
 */
/* fork/execv/waitpid/getpid are POSIX; the build compiles as strict -std=c11. */
#define _POSIX_C_SOURCE 200809L

#include "d4np/concurrency/named_semaphore.h"

#include <stdio.h>
#include <string.h>

enum { ROUNDS = 2000 };

static int run_child(const char *to_child, const char *to_parent)
{
    d4np_named_semaphore_t c, p;
    if (d4np_named_semaphore_open(&c, to_child, 0) != D4NP_OK) {
        return 10;
    }
    if (d4np_named_semaphore_open(&p, to_parent, 0) != D4NP_OK) {
        d4np_named_semaphore_close(&c);
        return 11;
    }
    for (int i = 0; i < ROUNDS; ++i) {
        d4np_named_semaphore_wait(&c);
        d4np_named_semaphore_post(&p);
    }
    d4np_named_semaphore_close(&p);
    d4np_named_semaphore_close(&c);
    return 0;
}

/* ----- platform process spawning ------------------------------------------------------------ */
#if defined(_WIN32)
#include <windows.h>

typedef HANDLE d4np_child_t;
static unsigned long d4np_self_pid(void)
{
    return GetCurrentProcessId();
}

static int d4np_self_path(char *buf, size_t bufsz)
{
    DWORD n = GetModuleFileNameA(NULL, buf, (DWORD)bufsz);
    return (n > 0 && n < bufsz) ? 0 : -1;
}

static int d4np_spawn_child(const char *self, const char *n1, const char *n2, d4np_child_t *out)
{
    char cmd[1024];
    if (snprintf(cmd, sizeof cmd, "\"%s\" child %s %s", self, n1, n2) >= (int)sizeof cmd) {
        return -1;
    }
    STARTUPINFOA si;
    PROCESS_INFORMATION pi;
    memset(&si, 0, sizeof si);
    si.cb = sizeof si;
    memset(&pi, 0, sizeof pi);
    if (!CreateProcessA(self, cmd, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi)) {
        return -1;
    }
    CloseHandle(pi.hThread);
    *out = pi.hProcess;
    return 0;
}

static int d4np_wait_child(d4np_child_t child)
{
    DWORD code = 1;
    WaitForSingleObject(child, INFINITE);
    GetExitCodeProcess(child, &code);
    CloseHandle(child);
    return (int)code;
}

#else
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

typedef pid_t d4np_child_t;
static unsigned long d4np_self_pid(void)
{
    return (unsigned long)getpid();
}

/* On POSIX the parent re-execs via argv[0] (CTest passes the binary's absolute path), so there
 * is no GetModuleFileName equivalent to provide here. */

static int d4np_spawn_child(const char *self, const char *n1, const char *n2, d4np_child_t *out)
{
    pid_t pid = fork();
    if (pid < 0) {
        return -1;
    }
    if (pid == 0) {
        char *args[] = {(char *)self, (char *)"child", (char *)n1, (char *)n2, NULL};
        execv(self, args);
        _exit(127); /* exec failed */
    }
    *out = pid;
    return 0;
}

static int d4np_wait_child(d4np_child_t child)
{
    int status = 0;
    if (waitpid(child, &status, 0) < 0) {
        return 1;
    }
    return WIFEXITED(status) ? WEXITSTATUS(status) : 1;
}
#endif

static int run_parent(const char *self_argv0)
{
    char self[1024];
#if defined(_WIN32)
    if (d4np_self_path(self, sizeof self) != 0) {
        fprintf(stderr, "harness: cannot determine own path\n");
        return 2;
    }
#else
    if (snprintf(self, sizeof self, "%s", self_argv0) >= (int)sizeof self) {
        return 2;
    }
#endif
    (void)self_argv0;

    unsigned long pid = d4np_self_pid();
    char to_child[28], to_parent[28];
    snprintf(to_child, sizeof to_child, "d4np_%lu_tc", pid);
    snprintf(to_parent, sizeof to_parent, "d4np_%lu_tp", pid);

    /* Start clean in case a previous crashed run left the names behind (POSIX). */
    (void)d4np_named_semaphore_unlink(to_child);
    (void)d4np_named_semaphore_unlink(to_parent);

    d4np_named_semaphore_t c, p;
    int rc = 3;
    if (d4np_named_semaphore_open(&c, to_child, 0) != D4NP_OK) {
        fprintf(stderr, "harness: parent open(to_child) failed\n");
        goto out_unlink;
    }
    if (d4np_named_semaphore_open(&p, to_parent, 0) != D4NP_OK) {
        fprintf(stderr, "harness: parent open(to_parent) failed\n");
        d4np_named_semaphore_close(&c);
        goto out_unlink;
    }

    d4np_child_t child;
    if (d4np_spawn_child(self, to_child, to_parent, &child) != 0) {
        fprintf(stderr, "harness: failed to spawn child\n");
        d4np_named_semaphore_close(&p);
        d4np_named_semaphore_close(&c);
        goto out_unlink;
    }

    for (int i = 0; i < ROUNDS; ++i) {
        d4np_named_semaphore_post(&c);
        d4np_named_semaphore_wait(&p);
    }

    int child_rc = d4np_wait_child(child);
    d4np_named_semaphore_close(&p);
    d4np_named_semaphore_close(&c);
    if (child_rc == 0) {
        printf("named-semaphore IPC OK: %d ping-pong rounds across 2 processes\n", ROUNDS);
        rc = 0;
    } else {
        fprintf(stderr, "harness: child exited with %d\n", child_rc);
        rc = 4;
    }

out_unlink:
    (void)d4np_named_semaphore_unlink(to_child);
    (void)d4np_named_semaphore_unlink(to_parent);
    return rc;
}

int main(int argc, char **argv)
{
    if (argc >= 4 && strcmp(argv[1], "child") == 0) {
        return run_child(argv[2], argv[3]);
    }
    return run_parent(argv[0]);
}
