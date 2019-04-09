/*
 * "git psuh"
 *
 * Part of the "My First Contribution" codelab.
 */

#include <stdio.h>
#include "builtin.h"
#include "config.h"

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
	const char *cfg_name;

	printf(Q_("Your args (there is %d):\n",
		  "Your args (there are %d):\n",
	 	  argc),
	       argc);
	for (int i = 0; i < argc; i++)
		printf("%s\n", argv[i]);
	printf(_("Your current working directory:\n<top-level>%s%s\n"),
		       prefix ? "/" : "", prefix ? prefix : "");

	git_config(git_default_config, NULL);
	if (git_config_get_string_const("user.name", &cfg_name) > 0)
		printf(_("No name found in config\n"));
	else
		printf(_("Your name: %s\n"), cfg_name);

	printf("%s\n", _(psuh));
	return 0;
}
