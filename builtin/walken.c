/*
 * "git walken"
 *
 * Part of the "My First Revision Walk" tutorial.
 */

#include "builtin.h"
#include "revision.h"
#include "commit.h"
#include "config.h"
#include "parse-options.h"
#include "pretty.h"
#include "line-log.h"
#include "list-objects.h"
#include "list-objects-filter-options.h"
#include "grep.h"


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
 * Perform configuration for commit walk here. Within this function we set a
 * starting point, and can customize our walk in various ways.
 */
static void final_rev_info_setup(int argc, const char **argv, const char *prefix,
		struct rev_info *rev)
{
	/*
	 * Optional:
	 * setup_revision_opt is used to pass options to the setup_revisions()
	 * call. It's got some special items for submodules and other types of
	 * optimizations, but for now, we'll just point it to HEAD. First we
	 * should make sure to reset it. This is useful for more complicated
	 * stuff but a decent shortcut for the first pass is
	 * add_head_to_pending().
	 */

	/*
	 * struct setup_revision_opt opt;

	 * memset(&opt, 0, sizeof(opt));
	 * opt.def = "HEAD";
	 * opt.revarg_opt = REVARG_COMMITTISH;
	 * setup_revisions(argc, argv, rev, &opt);
	 */

	/* add the HEAD to pending so we can start */
	add_head_to_pending(rev);

	/* Apply a 'grep' pattern to the 'author' header. */
	append_header_grep_pattern(&rev->grep_filter, GREP_HEADER_AUTHOR, "gmail");
	compile_grep_patterns(&rev->grep_filter);

	/* Let's force oneline format. */
	get_commit_format("oneline", rev);
	rev->verbose_header = 1;

	/* Reverse the order */
	rev->reverse = 1;
	
	/* Let's play with the sort order. */
	rev->topo_order = 1;

	/* Toggle between these and observe the difference. */
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
		/*
		 * BUG() is used to warn developers when they've made a change
		 * which breaks some relied-upon behavior of Git. In this case,
		 * we're telling developers that we don't expect commits to be
		 * routed as objects during an object walk. BUG() messages
		 * should not be localized.
		 */
		BUG("unexpected commit object in walken_show_object\n");
	default:
		/*
		 * This statement will only be hit if a new object type is added
		 * to Git; we BUG() to tell developers that the new object type
		 * needs to be handled and counted here.
		 */
		BUG("unexpected object type %s in walken_show_object\n"),
				type_name(obj->type);
	}
}

/*
 * walken_object_walk() is invoked by cmd_walken() after initialization. It does
 * a walk of all object types.
 */
static void walken_object_walk(struct rev_info *rev)
{
	struct list_objects_filter_options filter_options = {};
	struct oidset omitted;
	struct oidset_iter oit;
	struct object_id *oid = NULL;
	int omitted_count = 0;
	oidset_init(&omitted, 0);

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

	if (0) {
		/* Unfiltered: */
		trace_printf(_("Unfiltered object walk.\n"));
		traverse_commit_list(rev, walken_show_commit,
				walken_show_object, NULL);
	} else {
		trace_printf(_("Filtered object walk with filterspec "
				"'tree:1'.\n"));
		/*
		 * We can parse a tree depth of 1 to demonstrate the kind of
		 * filtering that could occur during various operations (see
		 * `git help rev-list` and read the entry on `--filter`).
		 */
		parse_list_objects_filter(&filter_options, "tree:1");

		traverse_commit_list_filtered(&filter_options, rev,
			walken_show_commit, walken_show_object, NULL, NULL);
	}

	/* Count the omitted objects. */
	oidset_iter_init(&omitted, &oit);

	while ((oid = oidset_iter_next(&oit)))
		omitted_count++;

	/*
	 * This print statement is designed to be script-parseable. Script
	 * authors will rely on the output not to change, so we will not
	 * localize this string. It will go to stdout directly.
	 */
	printf("commits %d\n blobs %d\n tags %d\n trees %d omitted %d\n",
	       commit_count, blob_count, tag_count, tree_count, omitted_count);
}

/*
 * walken_commit_walk() is invoked by cmd_walken() after initialization. It
 * performs the actual commit walk.
 */
static void walken_commit_walk(struct rev_info *rev)
{
	struct commit *commit;
	struct strbuf prettybuf = STRBUF_INIT;

	/*
	 * prepare_revision_walk() gets the final steps ready for a revision
	 * walk. We check the return value for errors.
	 */
	if (prepare_revision_walk(rev)) {
		die(_("revision walk setup failed"));
	}

	/*
	 * Now we can start the real commit walk. get_revision() grabs the next
	 * revision based on the contents of rev.
	 */
	while ((commit = get_revision(rev))) {
		strbuf_reset(&prettybuf);
		pp_commit_easy(CMIT_FMT_ONELINE, commit, &prettybuf);
		/*
		 * We expect this part of the output to be machine-parseable -
		 * one commit message per line - so we send it to stdout.
		 */
		puts(prettybuf.buf);
	}

	strbuf_release(&prettybuf);
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

	struct rev_info rev;

	/*
	 * parse_options() handles showing usage if incorrect options are
	 * provided, or if '-h' is passed.
	 */
	argc = parse_options(argc, argv, prefix, options, walken_usage, 0);

	init_walken_defaults();

	git_config(git_walken_config, NULL);

	/*
	 * Time to set up the walk. repo_init_revisions sets up rev_info with
	 * the defaults, but then you need to make some configuration settings
	 * to make it do what's special about your walk.
	 */
	repo_init_revisions(the_repository, &rev, prefix);

	/* We can set our traversal flags here. */
	rev.always_show_header = 1;


	if (1) {
		add_head_to_pending(&rev);
		walken_object_walk(&rev);
	} else {
		/*
		 * Before we do the walk, we need to set a starting point by giving it
		 * something to go in `pending` - that happens in here
		 */
		final_rev_info_setup(argc, argv, prefix, &rev);
		walken_commit_walk(&rev);
	}

	/*
	 * This line is "human-readable" and we are writing a plumbing command,
	 * so we localize it and use the trace library to print only when
	 * the GIT_TRACE environment variable is set.
	 */
	trace_printf(_("cmd_walken incoming...\n"));
	return 0;
}
