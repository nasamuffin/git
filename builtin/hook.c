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
	struct hook *item;
	struct strbuf hookname = STRBUF_INIT;

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

	list_for_each(pos, head) {
		item = list_entry(pos, struct hook, list);
		if (item)
			printf("%s: %s\n",
			       config_scope_name(item->origin),
			       item->command.buf);
	}

	clear_hook_list(head);
	strbuf_release(&hookname);

	return 0;
}

int cmd_hook(int argc, const char **argv, const char *prefix)
{
	struct option builtin_hook_options[] = {
		OPT_END(),
	};
	if (argc < 2)
		usage_with_options(builtin_hook_usage, builtin_hook_options);

	if (!strcmp(argv[1], "list"))
		return list(argc - 1, argv + 1, prefix);

	usage_with_options(builtin_hook_usage, builtin_hook_options);
}
