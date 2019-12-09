#include "cache.h"

#include "hook.h"
#include "config.h"

static LIST_HEAD(hook_head);

void free_hook(struct hook *ptr)
{
	if (ptr) {
		strbuf_release(&ptr->command);
		free(ptr);
	}
}

static void emplace_hook(struct list_head *pos, const char *command)
{
	struct hook *to_add = malloc(sizeof(struct hook));
	to_add->origin = current_config_scope();
	strbuf_init(&to_add->command, 0);
	strbuf_addstr(&to_add->command, command);

	list_add_tail(&to_add->list, pos);
}

static void remove_hook(struct list_head *to_remove)
{
	struct hook *hook_to_remove = list_entry(to_remove, struct hook, list);
	list_del(to_remove);
	free_hook(hook_to_remove);
}

void clear_hook_list(void)
{
	struct list_head *pos, *tmp;
	list_for_each_safe(pos, tmp, &hook_head)
		remove_hook(pos);
}

struct list_head* hook_list(const struct strbuf* hookname)
{
	struct strbuf hook_key = STRBUF_INIT;
	const struct string_list *commands = NULL;
	struct string_list_item *it = NULL;
	struct list_head *pos = NULL, *tmp = NULL;
	struct strbuf hookcmd_name = STRBUF_INIT;
	struct hook *hook = NULL;

	if (!hookname)
		return NULL;

	strbuf_addf(&hook_key, "hook.%s.command", hookname->buf);

	commands = git_config_get_value_multi(hook_key.buf);

	if (!commands)
		return NULL;

	for_each_string_list_item(it, commands) {
		const char *command = it->string;

		strbuf_reset(&hookcmd_name);
		strbuf_addf(&hookcmd_name, "hookcmd.%s.command", command);

		/* If no hookcmd with that name exists, &command is untouched */
		git_config_get_value(hookcmd_name.buf, &command);

		if (!command)
			return NULL;

		/*
		 * TODO: implement an option-getting callback, e.g.
		 *   get configs by pattern hookcmd.$value.*
		 *   for each key+value, do_callback(key, value, cb_data)
		 */

		list_for_each_safe(pos, tmp, &hook_head) {
			hook = list_entry(pos, struct hook, list);
			/*
			 * The list of hooks to run can be reordered by being redeclared
			 * in the config. Options about hook ordering should be checked
			 * here.
			 */
			if (0 == strcmp(hook->command.buf, command))
				remove_hook(pos);
		}
		emplace_hook(pos, command);

	}

	return &hook_head;
}
