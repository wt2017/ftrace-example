# count realtime clock by ftrace

Kernel module that registers an ftrace callback on the `__x64_sys_clock_gettime` syscall.

```
sudo insmod ftrace_clock_gettime.ko
sudo rmmod ftrace_clock_gettime
dmesg | tail
```

## Zero-coded observation via ftrace

For **pure observation** (no parameter inspection or modification), the same data can be obtained without any kernel module at all — ftrace's built-in `function` tracer is enough:

```bash
# enable function tracing
echo function | sudo tee /sys/kernel/debug/tracing/current_tracer

# filter to just this syscall
echo __x64_sys_clock_gettime | sudo tee /sys/kernel/debug/tracing/set_ftrace_filter

# watch the live call stream (Ctrl+C to stop)
cat /sys/kernel/debug/tracing/trace_pipe
```

Clean up afterward:

```bash
# stop tracing
echo nop | sudo tee /sys/kernel/debug/tracing/current_tracer

# clear the filter
echo | sudo tee /sys/kernel/debug/tracing/set_ftrace_filter
```

The module becomes necessary only when you need to **inspect arguments, modify return values, or add custom logic** inside the callback.

## Ftrace callback anatomy

```c
clock_gettime_handler(unsigned long ip, unsigned long parent_ip,
                      struct ftrace_ops *ops, struct ftrace_regs *fregs)
```

| Parameter | Meaning |
|---|---|
| `ip` | Address of the traced function (`__x64_sys_clock_gettime`) — "what function is running" |
| `parent_ip` | Return address of the caller — "who called it" |
| `ops` | Your `struct ftrace_ops` descriptor (identifies which registration fired) |
| `fregs` | **CPU register snapshot** at the moment ftrace intercepted the function. Not the syscall's argument struct — it contains *all* registers saved by ftrace, from which syscall arguments are extracted. |

### Syscall arguments via fregs

On x86_64, syscall arguments are passed in registers following the kernel's calling convention:

```
clock_gettime(clock_id, tp)
     ↑            ↑
    rdi          rsi
```

`ftrace_regs_get_argument(fregs, n)` extracts the n-th argument (0‑based):

```c
ftrace_regs_get_argument(fregs, 0)   →  clock_id  (the first syscall arg, from rdi)
ftrace_regs_get_argument(fregs, 1)   →  tp        (the second syscall arg, from rsi)
```

The maximum index `FTRACE_REGS_MAX_ARGS` is 6 on x86_64, matching the register‑only calling convention:

```
arg 0: rdi,  arg 1: rsi,  arg 2: rdx
arg 3: r10,  arg 4: r8,   arg 5: r9
```

### Beyond 6 arguments

Functions with more than 6 arguments push the 7th+ onto **the stack**. Read them via stack pointer offset:

```c
/*                                                                   
          high addr                                                   
 +-------------------+                                                 
 | arg 7             |  ← rsp + 16                                    
 +-------------------+                                                 
 | arg 6             |  ← rsp + 8                                     
 +-------------------+                                                 
 | return address    |  ← rsp ← ftrace_regs_get_stack_pointer(fregs)  
 +-------------------+                                                 
          low addr                                                    
*/

unsigned long sp = ftrace_regs_get_stack_pointer(fregs);
unsigned long arg6 = *((unsigned long *)sp + 1);   // rsp + 8
unsigned long arg7 = *((unsigned long *)sp + 2);   // rsp + 16
```

In practice, **kernel syscalls never exceed 6 arguments** (the `SYSCALL_DEFINE` macro caps at `SYSCALL_DEFINE6`), so the stack path is rarely needed for syscall tracing. It exists for completeness when hooking arbitrary kernel functions.

## Build

Two build systems are provided. Choose whichever you prefer.

### A) Makefile (traditional kernel pattern)

```bash
make        # produces ftrace_clock_gettime.ko in the current directory
make clean
```

### B) CMake (out-of-tree, keeps build/ separate)

```bash
cmake -B build
cmake --build build
cmake --build build --target clean
```

## Files

| File | Purpose |
|---|---|
| `ftrace_clock_gettime.c` | Module source – init, exit, and the empty ftrace callback |
| `Kbuild` | kbuild fragment (`obj-m := ftrace_clock_gettime.o`) |
| `Makefile` | build solution A: traditional kernel module Makefile |
| `CMakeLists.txt` | build solution B: cmake out-of-tree build |
