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

static int list(int argc, const char **argv, const char *prefix)
{
	struct list_head *head, *pos;
	const char *hookname = NULL;

	struct option list_options[] = {
		OPT_END(),
	};

	argc = parse_options(argc, argv, prefix, list_options,
			     builtin_hook_usage, 0);

	if (argc < 1) {
		usage_msg_opt(_("You must specify a hook event name to list."),
			      builtin_hook_usage, list_options);
	}

	hookname = argv[0];

	head = hook_list(hookname);

	if (list_empty(head)) {
		printf(_("no commands configured for hook '%s'\n"),
		       hookname);
		return 0;
	}

	list_for_each(pos, head) {
		struct hook *item = list_entry(pos, struct hook, list);
		item = list_entry(pos, struct hook, list);
		if (item) {
			/*
			 * TRANSLATORS: "<config scope>: <path>". Both fields
			 * should be left untranslated; config scope matches the
			 * output of 'git config --show-scope'. Marked for
			 * translation to provide better RTL support later.
			 */
			printf(_("%s: %s\n"),
			       (item->from_hookdir
				? "hookdir"
				: config_scope_name(item->origin)),
			       item->command.buf);
		}
	}

	clear_hook_list(head);

	return 0;
}

int cmd_hook(int argc, const char **argv, const char *prefix)
{
	struct option builtin_hook_options[] = {
		OPT_END(),
	};
	if (argc < 2)
		usage_with_options(builtin_hook_usage, builtin_hook_options);

	git_config(git_default_config, NULL);

	if (!strcmp(argv[1], "list"))
		return list(argc - 1, argv + 1, prefix);

	usage_with_options(builtin_hook_usage, builtin_hook_options);
}
