/*
 * "git psuh"
 *
 * Part of the "My First Contribution" codelab.
 */

#include "builtin.h"
#include "commit.h"
#include "config.h"
#include "pretty.h"
#include "wt-status.h"
#include "parse-options.h"

static const char * const psuh_usage[] = {
	N_("git psuh [arg]..."),
	NULL,
};

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
	struct wt_status status;
	struct commit *c = NULL;
	struct strbuf commitline = STRBUF_INIT;

	struct option options[] = {
		OPT_END()
	};

	argc = parse_options(argc, argv, prefix, options, psuh_usage, 0);

	printf(Q_("Your args (there is %d):\n",
		  "Your args (there are %d):\n",
	 	  argc),
	       argc);
	for (int i = 0; i < argc; i++)
		printf("%s\n", argv[i]);
	printf(_("Your current working directory:\n<top-level>%s%s\n"),
		       prefix ? "/" : "", prefix ? prefix : "");

	wt_status_prepare(the_repository, &status);
	git_config(git_default_config, NULL);
	if (git_config_get_string_const("user.name", &cfg_name) > 0)
		printf(_("No name found in config\n"));
	else
		printf(_("Your name: %s\n"), cfg_name);

	printf(_("Your current branch: %s\n"), status.branch);

	c = lookup_commit_reference_by_name("origin/master");
	if (c != NULL) {
		pp_commit_easy(CMIT_FMT_ONELINE, c, &commitline);
		printf(_("Current commit: %s\n"), commitline.buf);
	}

	printf("%s\n", _(psuh));
	return 0;
}
