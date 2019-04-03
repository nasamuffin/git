/*
 * "git psuh"
 *
 * Part of the "My First Contribution" codelab.
 */

#include <stdio.h>
#include "builtin.h"

/* This string is marked translatable with N_ as it is const. When it is
 * used, it must be wrapped in a translation macro as well.
 */
const char *psuh = N_("Here is a Pony Saying 'Um, Hello':	\n"
		      " ___________				\n"
		      "< um, hello >				\n"
		      " -----------				\n"
		      "        \\   n,,n			\n"
		      "         \\  (oo)\\\\_____		\n"
		      "            |__|\\\\      )\\\\		\n"
		      "                ||-----|| \\\\		\n"
		      "                ||     ||		\n");

int cmd_psuh(int argc, const char **argv, const char *prefix)
{
	printf("%s\n", _(psuh));
	return 0;
}
