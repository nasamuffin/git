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

int cmd_walken(int argc, const char **argv, const char *prefix)
{
	struct option options[] = {
		OPT_END()
	};

	argc = parse_options(argc, argv, prefix, options, walken_usage, 0);

	printf(_("cmd_walken incoming...\n"));
	return 0;
}
