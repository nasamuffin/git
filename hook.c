#include "cache.h"

#include "hook.h"
#include "config.h"

void free_hook(struct hook *ptr)
{
	if (ptr) {
		strbuf_release(&ptr->command);
		free(ptr);
	}
}

static void append_or_move_hook(struct list_head *head, const char *command)
{
	struct list_head *pos = NULL, *tmp = NULL;
	struct hook *to_add = NULL;

	/*
	 * remove the prior entry with this command; we'll replace it at the
	 * end.
	 */
	list_for_each_safe(pos, tmp, head) {
		struct hook *it = list_entry(pos, struct hook, list);
		if (!strcmp(it->command.buf, command)) {
		    list_del(pos);
		    /* we'll simply move the hook to the end */
		    to_add = it;
		}
	}

	if (!to_add) {
		/* adding a new hook, not moving an old one */
		to_add = xmalloc(sizeof(struct hook));
		strbuf_init(&to_add->command, 0);
		strbuf_addstr(&to_add->command, command);
	}

	/* re-set the scope so we show where an override was specified */
	to_add->origin = current_config_scope();

	list_add_tail(&to_add->list, pos);
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

		/* Check if a hookcmd with that name exists. */
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

		append_or_move_hook(head, command);

		strbuf_release(&hookcmd_name);
	}

	return 0;
}

struct list_head* hook_list(const struct strbuf* hookname)
{
	struct strbuf hook_key = STRBUF_INIT;
	struct list_head *hook_head = xmalloc(sizeof(struct list_head));
	struct hook_config_cb cb_data = { &hook_key, hook_head };

	INIT_LIST_HEAD(hook_head);

	if (!hookname)
		return NULL;

	strbuf_addf(&hook_key, "hook.%s.command", hookname->buf);

	git_config(hook_config_lookup, (void*)&cb_data);

	strbuf_release(&hook_key);
	return hook_head;
}
