#include "cache.h"

#include "hook.h"
#include "config.h"
#include "run-command.h"

void free_hook(struct hook *ptr)
{
	if (ptr) {
		strbuf_release(&ptr->command);
		free(ptr);
	}
}

static struct hook* find_hook_by_command(struct list_head *head, const char *command)
{
	struct list_head *pos = NULL, *tmp = NULL;
	struct hook *found = NULL;

	list_for_each_safe(pos, tmp, head) {
		struct hook *it = list_entry(pos, struct hook, list);
		if (!strcmp(it->command.buf, command)) {
		    list_del(pos);
		    found = it;
		}
	}
	return found;
}

static void append_or_move_hook(struct list_head *head, const char *command)
{
	struct hook *to_add = find_hook_by_command(head, command);

	if (!to_add) {
		/* adding a new hook, not moving an old one */
		to_add = xmalloc(sizeof(struct hook));
		strbuf_init(&to_add->command, 0);
		strbuf_addstr(&to_add->command, command);
		to_add->from_hookdir = 0;
	}

	/* re-set the scope so we show where an override was specified */
	to_add->origin = current_config_scope();

	list_add_tail(&to_add->list, head);
}

static void remove_hook(struct list_head *to_remove)
{
	struct hook *hook_to_remove = list_entry(to_remove, struct hook, list);
	list_del(to_remove);
	free_hook(hook_to_remove);
}

void clear_hook_list(struct list_head *head)
{
	struct list_head *pos, *tmp;
	list_for_each_safe(pos, tmp, head)
		remove_hook(pos);
}

struct hook_config_cb
{
	struct strbuf *hookname;
	struct list_head *list;
};

static int hook_config_lookup(const char *key, const char *value, void *cb_data)
{
	struct hook_config_cb *data = cb_data;
	const char *hook_key = data->hookname->buf;
	struct list_head *head = data->list;

	if (!strcmp(key, hook_key)) {
		const char *command = value;
		struct strbuf hookcmd_name = STRBUF_INIT;
		int skip = 0;

		/*
		 * Check if we're removing that hook instead. Hookcmds are
		 * removed by name, and inlined hooks are removed by command
		 * content.
		 */
		strbuf_addf(&hookcmd_name, "hookcmd.%s.skip", command);
		git_config_get_bool(hookcmd_name.buf, &skip);

		/* Check if a hookcmd with that name exists. */
		strbuf_reset(&hookcmd_name);
		strbuf_addf(&hookcmd_name, "hookcmd.%s.command", command);
		git_config_get_value(hookcmd_name.buf, &command);

		if (!command) {
			strbuf_release(&hookcmd_name);
			BUG("git_config_get_value overwrote a string it shouldn't have");
		}

		/*
		 * TODO: implement an option-getting callback, e.g.
		 *   get configs by pattern hookcmd.$value.*
		 *   for each key+value, do_callback(key, value, cb_data)
		 */

		if (skip) {
			struct hook *to_remove = find_hook_by_command(head, command);
			if (to_remove)
				remove_hook(&(to_remove->list));
		} else {
			append_or_move_hook(head, command);
		}

		strbuf_release(&hookcmd_name);
	}

	return 0;
}

enum hookdir_opt configured_hookdir_opt(void)
{
	const char *key;
	if (git_config_get_value("hook.runhookdir", &key))
		return hookdir_yes; /* by default, just run it. */

	if (!strcmp(key, "no"))
		return hookdir_no;

	if (!strcmp(key, "yes"))
		return hookdir_yes;

	if (!strcmp(key, "warn"))
		return hookdir_warn;

	if (!strcmp(key, "interactive"))
		return hookdir_interactive;

	return hookdir_unknown;
}

struct list_head* hook_list(const struct strbuf* hookname)
{
	struct strbuf hook_key = STRBUF_INIT;
	struct list_head *hook_head = xmalloc(sizeof(struct list_head));
	struct hook_config_cb cb_data = { &hook_key, hook_head };
	const char *legacy_hook_path = NULL;

	INIT_LIST_HEAD(hook_head);

	if (!hookname)
		return NULL;

	strbuf_addf(&hook_key, "hook.%s.command", hookname->buf);

	git_config(hook_config_lookup, (void*)&cb_data);

	if (have_git_dir())
		legacy_hook_path = find_hook(hookname->buf);

	/* Unconditionally add legacy hook, but annotate it. */
	if (legacy_hook_path) {
		struct hook *legacy_hook;

		append_or_move_hook(hook_head, absolute_path(legacy_hook_path));
		legacy_hook = list_entry(hook_head->prev, struct hook, list);
		legacy_hook->from_hookdir = 1;
	}

	strbuf_release(&hook_key);
	return hook_head;
}
