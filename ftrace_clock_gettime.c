// SPDX-License-Identifier: GPL-2.0
/*
 * ftrace_clock_gettime.c — Kernel module that registers an ftrace callback
 * on the __x64_sys_clock_gettime syscall.
 *
 * The callback inspects the first argument (clock_id) and increments a
 * counter only for CLOCK_REALTIME calls. The count is exposed via debugfs
 * at /sys/kernel/debug/ftrace_clock_gettime/realtime-get-counter.
 */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/ftrace.h>
#include <linux/debugfs.h>
#include <linux/time.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("count realtime clock by ftrace");
MODULE_DESCRIPTION("ftrace callback counting CLOCK_REALTIME calls");

/* ------------------------------------------------------------------ */
/*  REALTIME call counter                                             */
/* ------------------------------------------------------------------ */

static u64 realtime_count;
static struct dentry *debugfs_root;

/* ------------------------------------------------------------------ */
/*  ftrace callback                                                   */
/* ------------------------------------------------------------------ */

static void notrace
clock_gettime_handler(unsigned long ip, unsigned long parent_ip,
		      struct ftrace_ops *ops, struct ftrace_regs *fregs)
{
	unsigned long clock_id;

	clock_id = ftrace_regs_get_argument(fregs, 0);
	if (clock_id == CLOCK_REALTIME)
		WRITE_ONCE(realtime_count, READ_ONCE(realtime_count) + 1);
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

	debugfs_root = debugfs_create_dir("ftrace_clock_gettime", NULL);
	if (IS_ERR_OR_NULL(debugfs_root)) {
		pr_err("ftrace: debugfs_create_dir failed\n");
		ftrace_set_filter(&clock_gettime_ops, NULL, 0, 1);
		unregister_ftrace_function(&clock_gettime_ops);
		return -ENOMEM;
	}

	debugfs_create_u64("realtime-get-counter", 0444, debugfs_root,
			   &realtime_count);

	return 0;
}

static void __exit ftrace_clock_gettime_exit(void)
{
	pr_info("ftrace: __x64_sys_clock_gettime hook removed\n");

	debugfs_remove_recursive(debugfs_root);
	ftrace_set_filter(&clock_gettime_ops, NULL, 0, 1);
	unregister_ftrace_function(&clock_gettime_ops);
}

module_init(ftrace_clock_gettime_init);
module_exit(ftrace_clock_gettime_exit);
