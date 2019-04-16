/*
 * "git walken"
 *
 * Part of the "My First Revision Walk" tutorial.
 */

#include "builtin.h"

int cmd_walken(int argc, const char **argv, const char *prefix)
{
	/*
	 * This line is "human-readable" and we are writing a plumbing command,
	 * so we localize it and use the trace library to print only when
	 * the GIT_TRACE environment variable is set.
	 */
	trace_printf(_("cmd_walken incoming...\n"));
	return 0;
}
