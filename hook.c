#include "cache.h"
#include "hook.h"
#include "run-command.h"
#include "config.h"

const char *find_hook(const char *name)
{
	static struct strbuf path = STRBUF_INIT;

	strbuf_reset(&path);
	strbuf_git_path(&path, "hooks/%s", name);
	if (access(path.buf, X_OK) < 0) {
		int err = errno;

#ifdef STRIP_EXTENSION
		strbuf_addstr(&path, STRIP_EXTENSION);
		if (access(path.buf, X_OK) >= 0)
			return path.buf;
		if (errno == EACCES)
			err = errno;
#endif

		if (err == EACCES && advice_enabled(ADVICE_IGNORED_HOOK)) {
			static struct string_list advise_given = STRING_LIST_INIT_DUP;

			if (!string_list_lookup(&advise_given, name)) {
				string_list_insert(&advise_given, name);
				advise(_("The '%s' hook was ignored because "
					 "it's not set as executable.\n"
					 "You can disable this warning with "
					 "`git config advice.ignoredHook false`."),
				       path.buf);
			}
		}
		return NULL;
	}
	return path.buf;
}

int hook_exists(const char *name)
{
	return !!find_hook(name);
}

int pipe_from_string_list(struct strbuf *pipe, void *pp_cb, void *pp_task_cb)
{
	int *item_idx;
	struct hook_state *ctx = pp_task_cb;
	struct hook_cb_data *hook_cb = pp_cb;
	struct string_list *to_pipe = hook_cb->options->feed_pipe_ctx;

	/* Bootstrap the state manager if necessary. */
	if (!ctx->feed_pipe_cb_data) {
		ctx->feed_pipe_cb_data = xmalloc(sizeof(unsigned int));
		*(int*)ctx->feed_pipe_cb_data = 0;
	}

	item_idx = ctx->feed_pipe_cb_data;

	if (*item_idx < to_pipe->nr) {
		strbuf_addf(pipe, "%s\n", to_pipe->items[*item_idx].string);
		(*item_idx)++;
		return 0;
	}
	return 1;
}

static int pick_next_hook(struct child_process *cp,
			  struct strbuf *out,
			  void *pp_cb,
			  void **pp_task_cb)
{
	struct hook_cb_data *hook_cb = pp_cb;
	struct hook_state *hook_state = hook_cb->hook_state;
	const char *hook_path = hook_cb->hook_path;

	if (!hook_state->active)
		return 0;

	strvec_pushv(&cp->env_array, hook_cb->options->env.v);
	/* reopen the file for stdin; run_command closes it. */
	if (hook_cb->options->path_to_stdin) {
		cp->no_stdin = 0;
		cp->in = xopen(hook_cb->options->path_to_stdin, O_RDONLY);
	} else if (hook_cb->options->feed_pipe) {
		/* ask for start_command() to make a pipe for us */
		cp->in = -1;
		cp->no_stdin = 0;
	} else {
		cp->no_stdin = 1;
	}
	cp->stdout_to_stderr = 1;
	cp->trace2_hook_name = hook_cb->hook_name;
	cp->dir = hook_cb->options->dir;

	strvec_push(&cp->args, hook_path);
	strvec_pushv(&cp->args, hook_cb->options->args.v);

	/* Provide context for errors if necessary */
	*pp_task_cb = hook_state;

	/*
	 * This pick_next_hook() will be called again, we're only
	 * running one hook, so indicate that no more work will be
	 * done.
	 */
	hook_state->active = 0;

	return 1;
}

static int notify_start_failure(struct strbuf *out,
				void *pp_cb,
				void *pp_task_cp)
{
	struct hook_cb_data *hook_cb = pp_cb;
	const char *hook_path = pp_task_cp;

	hook_cb->rc |= 1;

	strbuf_addf(out, _("Couldn't start hook '%s'\n"),
		    hook_path);

	return 1;
}

static int notify_hook_finished(int result,
				struct strbuf *out,
				void *pp_cb,
				void *pp_task_cb)
{
	struct hook_cb_data *hook_cb = pp_cb;
	struct run_hooks_opt *opt = hook_cb->options;

	hook_cb->rc |= result;

	if (opt->invoked_hook)
		*opt->invoked_hook = 1;

	return 0;
}

static void run_hooks_opt_clear(struct run_hooks_opt *options)
{
	strvec_clear(&options->env);
	strvec_clear(&options->args);
}

int run_hooks_opt(const char *hook_name, struct run_hooks_opt *options)
{
	struct strbuf abs_path = STRBUF_INIT;
	struct hook_state state = {
		.active = 1,
	};
	struct hook_cb_data cb_data = {
		.rc = 0,
		.hook_name = hook_name,
		.options = options,
	};
	const char *const hook_path = find_hook(hook_name);
	int jobs = 1;
	int ret = 0;

	if (!options)
		BUG("a struct run_hooks_opt must be provided to run_hooks");

	if (options->invoked_hook)
		*options->invoked_hook = 0;

	if (!hook_path && !options->error_if_missing)
		goto cleanup;

	if (!hook_path) {
		ret = error("cannot find a hook named %s", hook_name);
		goto cleanup;
	}

	cb_data.hook_path = hook_path;
	if (options->dir) {
		strbuf_add_absolute_path(&abs_path, hook_path);
		cb_data.hook_path = abs_path.buf;
	}
	cb_data.hook_state = &state;

	run_processes_parallel_tr2(jobs,
				   pick_next_hook,
				   notify_start_failure,
				   options->feed_pipe,
				   options->consume_sideband,
				   notify_hook_finished,
				   &cb_data,
				   "hook",
				   hook_name);
	ret = cb_data.rc;
cleanup:
	strbuf_release(&abs_path);
	run_hooks_opt_clear(options);
	free(state.feed_pipe_cb_data);
	return ret;
}

int run_hooks(const char *hook_name)
{
	struct run_hooks_opt opt = RUN_HOOKS_OPT_INIT;

	return run_hooks_opt(hook_name, &opt);
}

int run_hooks_l(const char *hook_name, ...)
{
	struct run_hooks_opt opt = RUN_HOOKS_OPT_INIT;
	va_list ap;
	const char *arg;

	va_start(ap, hook_name);
	while ((arg = va_arg(ap, const char *)))
		strvec_push(&opt.args, arg);
	va_end(ap);

	return run_hooks_opt(hook_name, &opt);
}
