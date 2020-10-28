#include "config.h"
#include "list.h"
#include "strbuf.h"

struct hook
{
	struct list_head list;
	/*
	 * Config file which holds the hook.*.command definition.
	 * (This has nothing to do with the hookcmd.<name>.* configs.)
	 */
	enum config_scope origin;
	/* The literal command to run. */
	struct strbuf command;
	int from_hookdir;
};

/*
 * Provides a linked list of 'struct hook' detailing commands which should run
 * in response to the 'hookname' event, in execution order.
 */
struct list_head* hook_list(const struct strbuf *hookname);

enum hookdir_opt
{
	hookdir_no,
	hookdir_warn,
	hookdir_interactive,
	hookdir_yes,
	hookdir_unknown,
};

/*
 * Provides the hookdir_opt specified in the config without consulting any
 * command line arguments.
 */
enum hookdir_opt configured_hookdir_opt(void);

/* Free memory associated with a 'struct hook' */
void free_hook(struct hook *ptr);
/* Empties the list at 'head', calling 'free_hook()' on each entry */
void clear_hook_list(struct list_head *head);
