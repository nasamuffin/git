/*
 * "git walken"
 *
 * Part of the "My First Revision Walk" tutorial.
 */

#include <stdio.h>
#include "builtin.h"
#include "parse-options.h"

static const char * const walken_usage[] = {
	N_("git walken"),
	NULL,
};

/*
 * Within init_walken_defaults() we can call into other useful defaults to set
 * in the global scope or on the_repository. It's okay to borrow from other
 * functions which are doing something relatively similar to yours.
 */
static void init_walken_defaults(void)
{
	/* We don't actually need the same components `git log` does; leave this
	 * empty for now.
	 */
}

int cmd_walken(int argc, const char **argv, const char *prefix)
{
	struct option options[] = {
		OPT_END()
	};

	argc = parse_options(argc, argv, prefix, options, walken_usage, 0);

	init_walken_defaults();
	printf(_("cmd_walken incoming...\n"));
	return 0;
}
