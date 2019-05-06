/*
 * "git walken"
 *
 * Part of the "My First Revision Walk" tutorial.
 */

#include "builtin.h"
#include "config.h"
#include "parse-options.h"


/*
 * Within init_walken_defaults() we can call into other useful defaults to set
 * in the global scope or on the_repository. It's okay to borrow from other
 * functions which are doing something relatively similar to yours.
 */
static void init_walken_defaults(void)
{
	/*
	 * We don't use any other components or have settings to initialize, so
	 * leave this empty.
	 */
}

/*
 * This method will be called back by git_config(). It is used to gather values
 * from the configuration files available to Git.
 *
 * Each time git_config() finds a configuration file entry, it calls this
 * callback. Then, this function should compare it to entries which concern us,
 * and make settings changes as necessary.
 *
 * If we are called with a config setting we care about, we should use one of
 * the helpers which exist in config.h to pull out the value for ourselves, i.e.
 * git_config_string(...) or git_config_bool(...).
 *
 * If we don't match anything, we should pass it along to another stakeholder
 * who may otherwise care - in log's case, grep, gpg, and diff-ui. For our case,
 * we'll ignore everybody else.
 */
static int git_walken_config(const char *var, const char *value, void *cb)
{
	/*
	 * For now, we don't have any custom configuration, so fall back on the
	 * default config.
	 */
	return git_default_config(var, value, cb);
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

	git_config(git_walken_config, NULL);

	/*
	 * This line is "human-readable" and we are writing a plumbing command,
	 * so we localize it and use the trace library to print only when
	 * the GIT_TRACE environment variable is set.
	 */
	trace_printf(_("cmd_walken incoming...\n"));
	return 0;
}
