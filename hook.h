#ifndef HOOK_H
#define HOOK_H
#include "strvec.h"
#include "run-command.h"

struct run_hooks_opt
{
	/* Environment vars to be set for each hook */
	struct strvec env;

	/* Args to be passed to each hook */
	struct strvec args;

	/* Emit an error if the hook is missing */
	unsigned int error_if_missing:1;

	/**
	 * An optional initial working directory for the hook,
	 * translates to "struct child_process"'s "dir" member.
	 */
	const char *dir;

	/**
	 * A pointer which if provided will be set to 1 or 0 depending
	 * on if a hook was started, regardless of whether or not that
	 * was successful. I.e. if the underlying start_command() was
	 * successful this will be set to 1.
	 *
	 * Used for avoiding TOCTOU races in code that would otherwise
	 * call hook_exist() after a "maybe hook run" to see if a hook
	 * was invoked.
	 */
	int *invoked_hook;

	/**
	 * Path to file which should be piped to stdin for each hook.
	 */
	const char *path_to_stdin;

	/**
	 * Callback and state pointer to ask for more content to pipe to stdin.
	 * Will be called repeatedly, for each hook. See
	 * hook.c:pipe_from_stdin() for an example. Keep per-hook state in
	 * hook.feed_pipe_cb_data (per process). Keep initialization context in
	 * feed_pipe_ctx (shared by all processes).
	 *
	 * See 'pipe_from_string_list()' for info about how to specify a
	 * string_list as the stdin input instead of writing your own handler.
	 */
	feed_pipe_fn feed_pipe;
	void *feed_pipe_ctx;
};

#define RUN_HOOKS_OPT_INIT { \
	.env = STRVEC_INIT, \
	.args = STRVEC_INIT, \
}

struct hook_state {
	unsigned int active;

	/**
	 * Use this to keep state for your feed_pipe_fn if you are using
	 * run_hooks_opt.feed_pipe. Otherwise, do not touch it.
	 */
	void *feed_pipe_cb_data;
};

struct hook_cb_data {
	/* rc reflects the cumulative failure state */
	int rc;
	const char *hook_name;
	const char *hook_path;
	struct hook *run_me;
	struct run_hooks_opt *options;
	struct hook_state *hook_state;
};

/*
 * Returns the path to the hook file, or NULL if the hook is missing
 * or disabled. Note that this points to static storage that will be
 * overwritten by further calls to find_hook and run_hook_*.
 */
const char *find_hook(const char *name);

/**
 * A boolean version of find_hook()
 */
int hook_exists(const char *hookname);

/**
 * Takes a `hook_name`, resolves it to a path with find_hook(), and
 * runs the hook for you with the options specified in "struct
 * run_hooks opt". Will free memory associated with the "struct run_hooks_opt".
 *
 * Returns the status code of the run hook, or a negative value on
 * error().
 */
int run_hooks_opt(const char *hook_name, struct run_hooks_opt *options);

/**
 * A wrapper for run_hooks_opt() which provides a dummy "struct
 * run_hooks_opt" initialized with "RUN_HOOKS_OPT_INIT".
 */
int run_hooks(const char *hook_name);

/**
 * Like run_hooks(), a wrapper for run_hooks_opt().
 *
 * In addition to the wrapping behavior provided by run_hooks(), this
 * wrapper takes a list of strings terminated by a NULL
 * argument. These things will be used as positional arguments to the
 * hook. This function behaves like the old run_hook_le() API.
 */
int run_hooks_l(const char *hook_name, ...);

/**
 * To specify a 'struct string_list', set 'run_hooks_opt.feed_pipe_ctx' to the
 * string_list and set 'run_hooks_opt.feed_pipe' to pipe_from_string_list().
 * This will pipe each string in the list to stdin, separated by newlines.  (Do
 * not inject your own newlines.)
 */
int pipe_from_string_list(struct strbuf *pipe, void *pp_cb, void *pp_task_cb);
#endif
