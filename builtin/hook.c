#include "cache.h"

#include "builtin.h"
#include "config.h"
#include "hook.h"
#include "parse-options.h"
#include "strbuf.h"

static const char * const builtin_hook_usage[] = {
	N_("git hook --list <hookname>"),
	NULL
};

enum hook_command {
	HOOK_NO_COMMAND = 0,
	HOOK_LIST,
};

static int print_hook_list(const struct strbuf *hookname)
{
	struct list_head *head, *pos;
	struct hook *item;

	head = hook_list(hookname);

	if (!head) {
		printf(_("no commands configured for hook '%s'\n"),
		       hookname->buf);
		return 0;
	}

	list_for_each(pos, head) {
		item = list_entry(pos, struct hook, list);
		if (item)
			printf("%s\n",
			       item->command.buf);
	}

	return 0;
}

int cmd_hook(int argc, const char **argv, const char *prefix)
{
	enum hook_command command = 0;
	struct strbuf hookname = STRBUF_INIT;

	struct option builtin_hook_options[] = {
		OPT_CMDMODE('l', "list", &command,
			    N_("list scripts which will be run for <hookname>"),
			    HOOK_LIST),
		OPT_END(),
	};

	argc = parse_options(argc, argv, prefix, builtin_hook_options,
			     builtin_hook_usage, 0);

	if (argc < 1) {
		usage_msg_opt("a hookname must be provided to operate on.",
			      builtin_hook_usage, builtin_hook_options);
	}

	strbuf_addstr(&hookname, argv[0]);

	switch(command) {
		case HOOK_LIST:
			return print_hook_list(&hookname);
			break;
		default:
			usage_msg_opt("no command given.", builtin_hook_usage,
				      builtin_hook_options);
	}

	clear_hook_list();
	strbuf_release(&hookname);

	return 0;
}
