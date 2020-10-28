#include "cache.h"

#include "builtin.h"
#include "config.h"
#include "hook.h"
#include "parse-options.h"
#include "strbuf.h"

static const char * const builtin_hook_usage[] = {
	N_("git hook list <hookname>"),
	NULL
};

static enum hookdir_opt should_run_hookdir;

static int list(int argc, const char **argv, const char *prefix)
{
	struct list_head *head, *pos;
	struct hook *item;
	struct strbuf hookname = STRBUF_INIT;
	struct strbuf hookdir_annotation = STRBUF_INIT;

	struct option list_options[] = {
		OPT_END(),
	};

	argc = parse_options(argc, argv, prefix, list_options,
			     builtin_hook_usage, 0);

	if (argc < 1) {
		usage_msg_opt(_("You must specify a hook event name to list."),
			      builtin_hook_usage, list_options);
	}

	strbuf_addstr(&hookname, argv[0]);

	head = hook_list(&hookname);

	if (list_empty(head)) {
		printf(_("no commands configured for hook '%s'\n"),
		       hookname.buf);
		strbuf_release(&hookname);
		return 0;
	}

	switch (should_run_hookdir) {
		case hookdir_no:
			strbuf_addstr(&hookdir_annotation, _(" (will not run)"));
			break;
		case hookdir_interactive:
			strbuf_addstr(&hookdir_annotation, _(" (will prompt)"));
			break;
		case hookdir_warn:
		case hookdir_unknown:
			strbuf_addstr(&hookdir_annotation, _(" (will warn)"));
			break;
		case hookdir_yes:
		/*
		 * The default behavior should agree with
		 * hook.c:configured_hookdir_opt().
		 */
		default:
			break;
	}

	list_for_each(pos, head) {
		item = list_entry(pos, struct hook, list);
		if (item) {
			/* Don't translate 'hookdir' - it matches the config */
			printf("%s: %s%s\n",
			       (item->from_hookdir
				? "hookdir"
				: config_scope_name(item->origin)),
			       item->command.buf,
			       (item->from_hookdir
				? hookdir_annotation.buf
				: ""));
		}
	}

	clear_hook_list(head);
	strbuf_release(&hookname);

	return 0;
}

int cmd_hook(int argc, const char **argv, const char *prefix)
{
	const char *run_hookdir = NULL;

	struct option builtin_hook_options[] = {
		OPT_STRING(0, "run-hookdir", &run_hookdir, N_("option"),
			   N_("what to do with hooks found in the hookdir")),
		OPT_END(),
	};

	argc = parse_options(argc, argv, prefix, builtin_hook_options,
			     builtin_hook_usage, 0);

	/* after the parse, we should have "<command> <hookname> <args...>" */
	if (argc < 1)
		usage_with_options(builtin_hook_usage, builtin_hook_options);


	/* argument > config */
	if (run_hookdir)
		if (!strcmp(run_hookdir, "no"))
			should_run_hookdir = hookdir_no;
		else if (!strcmp(run_hookdir, "yes"))
			should_run_hookdir = hookdir_yes;
		else if (!strcmp(run_hookdir, "warn"))
			should_run_hookdir = hookdir_warn;
		else if (!strcmp(run_hookdir, "interactive"))
			should_run_hookdir = hookdir_interactive;
		else
			die(_("'%s' is not a valid option for --run-hookdir "
			      "(yes, warn, interactive, no)"), run_hookdir);
	else
		should_run_hookdir = configured_hookdir_opt();

	if (!strcmp(argv[0], "list"))
		return list(argc, argv, prefix);

	usage_with_options(builtin_hook_usage, builtin_hook_options);
}
