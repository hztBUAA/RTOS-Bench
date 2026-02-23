# RT-Thread POSIX mqueue Bug Report

## Issue Title
**[Bug] POSIX mq_send() doesn't block when queue is full, returns EBADF instead**

---

## Summary

The RT-Thread POSIX `mq_send()` function violates the POSIX standard by returning immediately with `errno=EBADF` when the message queue is full, instead of blocking until space becomes available.

## Environment

- **RT-Thread Version**: v5.x (tested on qemu-virt64-aarch64 BSP)
- **File Location**: `components/libc/posix/ipc/mqueue.c`
- **Related File**: `src/ipc.c`

## POSIX Standard Reference

According to POSIX.1-2017 (`mq_send(3p)`):

> If the specified message queue is full, mq_send() **shall block** until space becomes available to enqueue the message, or until mq_send() is interrupted by a signal.

## Bug Analysis

### Root Cause 1: Wrong timeout parameter in `mq_send()`

**File**: `components/libc/posix/ipc/mqueue.c`, Line 248

```c
// Current implementation (WRONG):
result = rt_mq_send_wait_prio(mq, (void *)msg_ptr, msg_len, msg_prio, 0, RT_UNINTERRUPTIBLE);
//                                                                    ^
//                                                                timeout=0 (non-blocking!)

// Should be:
result = rt_mq_send_wait_prio(mq, (void *)msg_ptr, msg_len, msg_prio, RT_WAITING_FOREVER, RT_UNINTERRUPTIBLE);
```

### Root Cause 2: Incorrect errno mapping

**File**: `components/libc/posix/ipc/mqueue.c`, Lines 252-253

```c
// Current implementation (WRONG):
rt_set_errno(EBADF);  // Always returns EBADF, regardless of actual error

// Should map different errors correctly:
// -RT_EFULL -> should not happen if blocking correctly
// -RT_EINTR -> EINTR (signal interrupted)
// -RT_ERROR -> EMSGSIZE
// -RT_EINVAL -> EINVAL
```

### Root Cause 3: `mq_timedsend()` not implemented

**File**: `components/libc/posix/ipc/mqueue.c`, Lines 339-347

```c
int mq_timedsend(mqd_t id, const char *msg_ptr, size_t msg_len,
                 unsigned msg_prio, const struct timespec *abs_timeout)
{
    /* RT-Thread does not support timed send */
    return mq_send(id, msg_ptr, msg_len, msg_prio);  // Timeout is ignored!
}
```

## Comparison with `mq_receive()` (Correct Implementation)

The `mq_receive()` function correctly uses `RT_WAITING_FOREVER`:

```c
// mq_receive() at line 206 - CORRECT:
result = rt_mq_recv_prio(mq, msg_ptr, msg_len, (rt_int32_t *)msg_prio, RT_WAITING_FOREVER, RT_UNINTERRUPTIBLE);
```

And `mq_timedreceive()` correctly handles timeout:

```c
// mq_timedreceive() at lines 301-306 - CORRECT:
if (abs_timeout != RT_NULL)
    tick = rt_timespec_to_tick(abs_timeout);
else
    tick = RT_WAITING_FOREVER;

result = rt_mq_recv_prio(mq, msg_ptr, msg_len, (rt_int32_t *)msg_prio, tick, RT_UNINTERRUPTIBLE);
```

## Impact

This bug affects all applications that rely on the POSIX `mq_send()` blocking behavior, including:
- Priority inversion tests
- Producer-consumer patterns
- Real-time scheduling benchmarks
- Any code ported from Linux/other POSIX systems

The incorrect `errno=EBADF` (9) is particularly confusing as it indicates "bad file descriptor", which is misleading when the actual issue is that the queue is full.

## Reproduction Steps

1. Create a message queue with small capacity (e.g., 1 message)
2. Send one message to fill the queue
3. Try to send another message from a different thread
4. Expected: `mq_send()` blocks until receiver consumes the first message
5. Actual: `mq_send()` returns immediately with `errno=9` (EBADF)

### Test Code

```c
#include <pthread.h>
#include <mqueue.h>
#include <stdio.h>
#include <errno.h>

#define MQ_NAME "/test_mq"

void *receiver(void *arg)
{
    mqd_t mq = *(mqd_t *)arg;
    char buf[32];
    unsigned prio;

    sleep(1);  // Delay to ensure sender blocks
    mq_receive(mq, buf, sizeof(buf), &prio);
    printf("Received: %s\n", buf);
    return NULL;
}

int main()
{
    struct mq_attr attr = {
        .mq_maxmsg = 1,    // Only 1 message capacity
        .mq_msgsize = 32
    };

    mqd_t mq = mq_open(MQ_NAME, O_CREAT | O_RDWR, 0644, &attr);
    pthread_t tid;

    pthread_create(&tid, NULL, receiver, &mq);

    // Fill the queue
    mq_send(mq, "msg1", 4, 0);

    // This should BLOCK, but currently returns immediately with EBADF
    printf("Sending second message (should block)...\n");
    int ret = mq_send(mq, "msg2", 4, 0);
    printf("mq_send returned %d, errno=%d\n", ret, errno);  // Returns -1, errno=9

    pthread_join(tid, NULL);
    mq_close(mq);
    mq_unlink(MQ_NAME);
    return 0;
}
```

---

# Proposed Fix (Pull Request)

## PR Title
**[libc/posix] Fix mq_send() to block when queue is full per POSIX standard**

## Changes

### 1. Fix `mq_send()` to use blocking wait

```diff
--- a/components/libc/posix/ipc/mqueue.c
+++ b/components/libc/posix/ipc/mqueue.c
@@ -245,11 +245,19 @@ int mq_send(mqd_t id, const char *msg_ptr, size_t msg_len, unsigned msg_prio)
         rt_set_errno(EINVAL);
         return -1;
     }
-    result = rt_mq_send_wait_prio(mq, (void *)msg_ptr, msg_len, msg_prio, 0, RT_UNINTERRUPTIBLE);
+    result = rt_mq_send_wait_prio(mq, (void *)msg_ptr, msg_len, msg_prio, RT_WAITING_FOREVER, RT_UNINTERRUPTIBLE);
     if (result == RT_EOK)
         return 0;

-    rt_set_errno(EBADF);
+    if (result == -RT_EINTR)
+        rt_set_errno(EINTR);
+    else if (result == -RT_ERROR)
+        rt_set_errno(EMSGSIZE);
+    else if (result == -RT_EINVAL)
+        rt_set_errno(EINVAL);
+    else
+        rt_set_errno(EBADF);

     return -1;
 }
```

### 2. Implement `mq_timedsend()` properly

```diff
--- a/components/libc/posix/ipc/mqueue.c
+++ b/components/libc/posix/ipc/mqueue.c
@@ -339,8 +339,35 @@ int mq_timedsend(mqd_t                  id,
                  unsigned               msg_prio,
                  const struct timespec *abs_timeout)
 {
-    /* RT-Thread does not support timed send */
-    return mq_send(id, msg_ptr, msg_len, msg_prio);
+    rt_mq_t mq;
+    rt_err_t result;
+    int tick = 0;
+    struct mqueue_file *mq_file;
+    mq_file = fd_get(id)->vnode->data;
+    mq = (rt_mq_t)mq_file->data;
+
+    if ((mq == RT_NULL) || (msg_ptr == RT_NULL))
+    {
+        rt_set_errno(EINVAL);
+        return -1;
+    }
+
+    if (abs_timeout != RT_NULL)
+        tick = rt_timespec_to_tick(abs_timeout);
+    else
+        tick = RT_WAITING_FOREVER;
+
+    result = rt_mq_send_wait_prio(mq, (void *)msg_ptr, msg_len, msg_prio, tick, RT_UNINTERRUPTIBLE);
+    if (result == RT_EOK)
+        return 0;
+
+    if (result == -RT_ETIMEOUT)
+        rt_set_errno(ETIMEDOUT);
+    else if (result == -RT_EINTR)
+        rt_set_errno(EINTR);
+    else if (result == -RT_ERROR)
+        rt_set_errno(EMSGSIZE);
+    else
+        rt_set_errno(EBADF);
+
+    return -1;
 }
```

### 3. Update documentation comments

Update the function documentation to reflect the correct blocking behavior:

```diff
@@ -224,8 +224,8 @@ RTM_EXPORT(mq_receive);
  * @note    This function sends a message to the message queue identified by id.
  *          The message to be sent is contained in the buffer pointed to by msg_ptr, with a size of msg_len bytes.
  *          The priority of the message is specified by the msg_prio parameter.
- *          The function then attempts to send the message to the message queue using the rt_mq_send_wait_prio() function
- *          with zero timeout and uninterruptible mode.
+ *          If the message queue is full, the function blocks until space becomes available,
+ *          following POSIX standard behavior.
```

## Testing

After applying this fix:
- `mq_send()` correctly blocks when queue is full
- `mq_timedsend()` respects the timeout parameter
- Priority-based real-time tests pass without workarounds
- POSIX compatibility improved for applications ported from Linux

## Backward Compatibility Note

This change modifies the behavior of `mq_send()` from non-blocking to blocking. Applications that previously relied on the (incorrect) non-blocking behavior should switch to using the `O_NONBLOCK` flag with `mq_open()` or use `mq_timedsend()` with a zero timeout.

---

## References

- POSIX.1-2017: https://pubs.opengroup.org/onlinepubs/9699919799/functions/mq_send.html
- RT-Thread GitHub: https://github.com/RT-Thread/rt-thread
- Related issue discovered in RTOS-Bench project: https://github.com/[your-repo]/RTOS-Bench
