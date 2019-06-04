/*
 * "git walken"
 *
 * Part of the "My First Revision Walk" tutorial.
 */

#include <stdio.h>
#include "builtin.h"
#include "revision.h"
#include "commit.h"
#include "config.h"
#include "parse-options.h"
#include "pretty.h"
#include "line-log.h"
#include "list-objects.h"
#include "grep.h"

static const char * const walken_usage[] = {
	N_("git walken"),
	NULL,
};

static int commit_count;
static int tag_count;
static int blob_count;
static int tree_count;

/*
 * Within init_walken_defaults() we can call into other useful defaults to set
 * in the global scope or on the_repository. It's okay to borrow from other
 * functions which are doing something relatively similar to yours.
 */
static void init_walken_defaults(void)
{
	/* Needed by our grep filter. */
	init_grep_defaults(the_repository);
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

	/* Add a grep pattern to the author line in the header. */
	append_header_grep_pattern(&rev->grep_filter, GREP_HEADER_AUTHOR, "gmail");
	compile_grep_patterns(&rev->grep_filter);

	/* Let's force oneline format. */
	get_commit_format("oneline", rev);
	rev->verbose_header = 1;
	
	/* add the HEAD to pending so we can start */
	add_head_to_pending(rev);

	/* Reverse the order */
	rev->reverse = 1;
	
	/* Let's play with the sort order. */
	rev->topo_order = 1;
	rev->sort_order = REV_SORT_BY_COMMIT_DATE;
	/* rev->sort_order = REV_SORT_BY_AUTHOR_DATE; */
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
	grep_config(var, value, cb);
	return git_default_config(var, value, cb);
}

static void walken_show_commit(struct commit *cmt, void *buf)
{
	commit_count++;
}

static void walken_show_object(struct object *obj, const char *str, void *buf)
{
	switch (obj->type) {
	case OBJ_TREE:
		tree_count++;
		break;
	case OBJ_BLOB:
		blob_count++;
		break;
	case OBJ_TAG:
		tag_count++;
		break;
	case OBJ_COMMIT:
		printf(_("Unexpectedly encountered a commit in "
			 "walken_show_object!\n"));
		commit_count++;
		break;
	default:
		printf(_("Unexpected object type %s!\n"),
		       type_name(obj->type));
		break;
	}
}

/*
 * walken_object_walk() is invoked by cmd_walken() after initialization. It does
 * a walk of all object types.
 */
static int walken_object_walk(struct rev_info *rev)
{
	struct list_objects_filter_options filter_options = {};
	struct oidset omitted;
	oidset_init(&omitted, 0);

	printf("walken_object_walk beginning...\n");

	rev->tree_objects = 1;
	rev->blob_objects = 1;
	rev->tag_objects = 1;
	rev->tree_blobs_in_commit_order = 1;
	rev->exclude_promisor_objects = 1;

	if (prepare_revision_walk(rev))
		die(_("revision walk setup failed"));

	commit_count = 0;
	tag_count = 0;
	blob_count = 0;
	tree_count = 0;

	traverse_commit_list(rev, walken_show_commit, walken_show_object, NULL);

	printf(_("Object walk completed. Found %d commits, %d blobs, %d tags, "
	       "and %d trees.\n"), commit_count, blob_count, tag_count,
	       tree_count);

	return 0;
}

/*
 * walken_commit_walk() is invoked by cmd_walken() after initialization. It
 * does the commit walk only.
 */
static int walken_commit_walk(struct rev_info *rev)
{
	struct commit *commit;
	struct strbuf prettybuf;

	strbuf_init(&prettybuf, 0);


	/* prepare_revision_walk() gets the final steps ready for a revision
	 * walk. We check the return value for errors. */
	if (prepare_revision_walk(rev)) {
		die(_("revision walk setup failed"));
	}

	/* Now we can start the real commit walk. get_revision grabs the next
	 * revision based on the contents of rev.
	 */
	rev->diffopt.close_file = 0;
	while ((commit = get_revision(rev)) != NULL) {
		if (commit == NULL)
			continue;
		strbuf_reset(&prettybuf);
		pp_commit_easy(CMIT_FMT_ONELINE, commit, &prettybuf);
		printf(_("%s\n"), prettybuf.buf);

	}
	return 0;
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

	/* We can set our traversal flags here. */
	rev.always_show_header = 1;

	/* Before we do the walk, we need to set a starting point. It's not
	 * coming from opt. */

	if (1) {
		add_head_to_pending(&rev);
		walken_object_walk(&rev);
	} else {
		final_rev_info_setup(argc, argv, prefix, &rev);
		walken_commit_walk(&rev);
	}

	printf(_("cmd_walken incoming...\n"));
	return 0;
}
