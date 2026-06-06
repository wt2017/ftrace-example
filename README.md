# ftrace-example

Kernel module that registers an ftrace callback on the `__x64_sys_clock_gettime` syscall.

```
sudo insmod ftrace_clock_gettime.ko
sudo rmmod ftrace_clock_gettime
dmesg | tail
```

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
