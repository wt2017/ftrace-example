// SPDX-License-Identifier: GPL-2.0
/*
 * ftrace_clock_gettime.c — Kernel module that registers an ftrace callback
 * on the __x64_sys_clock_gettime syscall.
 *
 * The callback body is intentionally left empty; the purpose is to
 * demonstrate the ftrace registration mechanism.
 */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/ftrace.h>
#include <linux/version.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("ftrace-example");
MODULE_DESCRIPTION("ftrace callback on clock_gettime syscall");

/* ------------------------------------------------------------------ */
/*  ftrace callback                                                   */
/* ------------------------------------------------------------------ */

static void notrace
clock_gettime_handler(unsigned long ip, unsigned long parent_ip,
		      struct ftrace_ops *ops, struct ftrace_regs *fregs)
{
	/*
	 * Intentionally empty.
	 *
	 * When FTRACE_OPS_FL_SAVE_REGS is set the syscall arguments are
	 * accessible via the ftrace_regs helpers:
	 *   ftrace_regs_get_instruction_pointer(fregs)
	 *   ftrace_regs_get_argument(fregs, n)   — arg n (0-based)
	 * Fill this in when you need to trace or modify the call.
	 */
}

/* ------------------------------------------------------------------ */
/*  ftrace_ops descriptor                                             */
/* ------------------------------------------------------------------ */

static struct ftrace_ops clock_gettime_ops __read_mostly = {
	.func		= clock_gettime_handler,
	.flags		= FTRACE_OPS_FL_SAVE_REGS
			  | FTRACE_OPS_FL_SAVE_REGS_IF_SUPPORTED,
};

/* ------------------------------------------------------------------ */
/*  Module init / exit                                                */
/* ------------------------------------------------------------------ */

static int __init ftrace_clock_gettime_init(void)
{
	int ret;

	ret = register_ftrace_function(&clock_gettime_ops);
	if (ret) {
		pr_err("ftrace: register_ftrace_function failed: %d\n", ret);
		return ret;
	}

	ret = ftrace_set_filter(&clock_gettime_ops,
				"__x64_sys_clock_gettime",
				strlen("__x64_sys_clock_gettime"), 0);
	if (ret) {
		pr_err("ftrace: ftrace_set_filter failed: %d\n", ret);
		unregister_ftrace_function(&clock_gettime_ops);
		return ret;
	}

	pr_info("ftrace: tracing __x64_sys_clock_gettime\n");
	return 0;
}

static void __exit ftrace_clock_gettime_exit(void)
{
	/*
	 * Reset the filter (disable tracing of the function) before
	 * unregistering, then tear down the ftrace hook.
	 */
	ftrace_set_filter(&clock_gettime_ops, NULL, 0, 1);
	unregister_ftrace_function(&clock_gettime_ops);

	pr_info("ftrace: __x64_sys_clock_gettime hook removed\n");
}

module_init(ftrace_clock_gettime_init);
module_exit(ftrace_clock_gettime_exit);
