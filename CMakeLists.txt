cmake_minimum_required(VERSION 3.13)
project(ftrace-example C)

# ─────────────────────────────────────────────────────────────
# Out-of-tree kernel module build via cmake
#
# Build:   cmake -B build
#          cmake --build build
# Clean:   cmake --build build --target clean
# Load:    sudo insmod build/ftrace_clock_gettime.ko
# Unload:  sudo rmmod ftrace_clock_gettime
# Log:     sudo dmesg | tail
# ─────────────────────────────────────────────────────────────

# -----------------------------------------------------------------
# 1. Determine the kernel build directory
# -----------------------------------------------------------------
execute_process(COMMAND uname -r
  OUTPUT_VARIABLE KERNEL_RELEASE
  OUTPUT_STRIP_TRAILING_WHITESPACE)

set(KERNEL_BUILD_DIR "/lib/modules/${KERNEL_RELEASE}/build"
  CACHE PATH "Path to the kernel build tree")

if(NOT EXISTS "${KERNEL_BUILD_DIR}")
  message(FATAL_ERROR
    "Kernel build tree not found at ${KERNEL_BUILD_DIR}.\n"
    "Install it with: sudo apt install linux-headers-${KERNEL_RELEASE}")
endif()

message(STATUS "Kernel release  : ${KERNEL_RELEASE}")
message(STATUS "Kernel build dir: ${KERNEL_BUILD_DIR}")

# -----------------------------------------------------------------
# 2. Copy source files into build/ at configure time.
#    kbuild writes .cmd / .o / .ko alongside the source it sees;
#    by giving it copies in build/ we keep the source tree clean.
# -----------------------------------------------------------------
configure_file("ftrace_clock_gettime.c" "ftrace_clock_gettime.c" COPYONLY)
configure_file("Kbuild"                "Kbuild"                COPYONLY)

# -----------------------------------------------------------------
# 3. Custom target that calls kbuild to compile the module
# -----------------------------------------------------------------
add_custom_target(ftrace_clock_gettime ALL
  COMMAND ${CMAKE_MAKE_PROGRAM}
    -C "${KERNEL_BUILD_DIR}"
    M="${CMAKE_BINARY_DIR}"
    modules
  COMMENT "Building kernel module ftrace_clock_gettime.ko"
  BYPRODUCTS
    "${CMAKE_BINARY_DIR}/ftrace_clock_gettime.ko"
    "${CMAKE_BINARY_DIR}/ftrace_clock_gettime.o"
  WORKING_DIRECTORY "${CMAKE_BINARY_DIR}")

# -----------------------------------------------------------------
# 4. Forward clean to kbuild, then purge kbuild dotfiles
# -----------------------------------------------------------------
add_custom_target(clean_kmod
  COMMAND ${CMAKE_MAKE_PROGRAM}
    -C "${KERNEL_BUILD_DIR}"
    M="${CMAKE_BINARY_DIR}"
    clean
  COMMAND find "${CMAKE_BINARY_DIR}" -maxdepth 1
    -name ".*.cmd" -o -name ".module-common*"
    -delete
  COMMENT "Cleaning kernel module artifacts"
  WORKING_DIRECTORY "${CMAKE_BINARY_DIR}")
set_target_properties(clean_kmod PROPERTIES FOLDER "custom")

# -----------------------------------------------------------------
# 5. Top-level aliases
# -----------------------------------------------------------------
add_custom_target(modules   DEPENDS ftrace_clock_gettime)
add_custom_target(clean_all DEPENDS clean_kmod)
