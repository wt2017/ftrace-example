# ftrace-example

Kernel module that registers an ftrace callback on the `__x64_sys_clock_gettime` syscall.

```
git clone ...
cmake -B build
cmake --build build
sudo insmod build/ftrace_clock_gettime.ko
sudo rmmod ftrace_clock_gettime
dmesg | tail
```

## Files

| File | Purpose |
|---|---|
| `ftrace_clock_gettime.c` | Module source – init, exit, and the empty ftrace callback |
| `Kbuild` | kbuild Makefile fragment (`obj-m := ftrace_clock_gettime.o`) |
| `CMakeLists.txt` | cmake build – drives the kernel build system from `build/` |
