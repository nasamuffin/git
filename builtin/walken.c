/*
 * "git walken"
 *
 * Part of the "My First Revision Walk" tutorial.
 */

#include "builtin.h"
#include "parse-options.h"


/*
 * Within init_walken_defaults() we can call into other useful defaults to set
 * in the global scope or on the_repository. It's okay to borrow from other
 * functions which are doing something relatively similar to yours.
 */
static void init_walken_defaults(void)
{
	/*
	 * We don't actually need the same components `git log` does; leave this
	 * empty for now.
	 */
}

int cmd_walken(int argc, const char **argv, const char *prefix)
{
	/*
	 * All builtins are expected to provide a usage to provide a consistent user
	 * experience.
	 */
	const char * const walken_usage[] = {
		N_("git walken"),
		NULL,
	};

	struct option options[] = {
		OPT_END()
	};

	/*
	 * parse_options() handles showing usage if incorrect options are
	 * provided, or if '-h' is passed.
	 */
	argc = parse_options(argc, argv, prefix, options, walken_usage, 0);

	init_walken_defaults();

	/*
	 * This line is "human-readable" and we are writing a plumbing command,
	 * so we localize it and use the trace library to print only when
	 * the GIT_TRACE environment variable is set.
	 */
	trace_printf(_("cmd_walken incoming...\n"));
	return 0;
}
