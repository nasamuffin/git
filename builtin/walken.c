/*
 * "git walken"
 *
 * Part of the "My First Revision Walk" tutorial.
 */

#include <stdio.h>
#include "builtin.h"
#include "revision.h"
#include "config.h"
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

/*
 * cmd_log calls a second set of init after the repo_init_revisions call. We'll
 * mirror those settings in post_repo_init_init.
 */
static void final_rev_info_setup(int argc, const char **argv, const char *prefix,
		struct rev_info *rev)
{
	struct setup_revision_opt opt;

	/* setup_revision_opt is used to pass options to the setup_revisions()
	 * call. It's got some special items for submodules and other types of
	 * optimizations, but for now, we'll just point it to HEAD and call it
	 * good. First we should make sure to reset it. TODO: This is useful for
	 * more complicated stuff revisions, but a decent shortcut for the first
	 * pass is add_head_to_pending().
	 */
	memset(&opt, 0, sizeof(opt));
	opt.def = "HEAD";
	opt.revarg_opt = REVARG_COMMITTISH;
	//setup_revisions(argc, argv, rev, &opt);

	/* Let's force oneline format. */
	get_commit_format("oneline", rev);
	rev->verbose_header = 1;
	
	/* add the HEAD to pending so we can start */
	add_head_to_pending(rev);
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
	/* For now, let's not bother with anything. */
	return git_default_config(var, value, cb);
}

int cmd_walken(int argc, const char **argv, const char *prefix)
{
	struct option options[] = {
		OPT_END()
	};

	struct rev_info rev;

	argc = parse_options(argc, argv, prefix, options, walken_usage, 0);

	init_walken_defaults();

	git_config(git_walken_config, NULL);

	/* Time to set up the walk. repo_init_revisions sets up rev_info with
	 * the defaults, but then you need to make some configuration settings
	 * to make it do what's special about your walk.
	 */
	repo_init_revisions(the_repository, &rev, prefix);

	/* Before we do the walk, we need to set a starting point. It's not
	 * coming from opt. */
	final_rev_info_setup(argc, argv, prefix, &rev);

	printf(_("cmd_walken incoming...\n"));
	return 0;
}
