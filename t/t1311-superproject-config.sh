#!/bin/sh

test_description='Test git config --superproject in different settings'

. ./test-lib.sh

# follow t7400's example and use the trash dir repo as a submodule to add
submodurl=$(pwd -P)

# since only the configs are modified, set up the repo structure only once
test_expect_success 'setup repo structure' '
	test_commit "base" &&
	git submodule add "${submodurl}" sub/ &&
	git commit -m "add a submodule"
'

test_expect_success 'superproject config applies to super and submodule' '
	cat >.git/config.superproject <<-EOF &&
	[foo]
		bar = baz
	EOF
	test_when_finished rm .git/config.superproject &&

	git config --get foo.bar &&
	git -C sub config --get foo.bar

'

test_done

test_expect_success 'can add from super or sub' '
	git config --superproject apple.species honeycrisp &&
	git -C sub config --superproject banana.species cavendish &&
	test_when_finished rm .git/config.superproject &&

	cat >expect <<-EOF &&
	apple.species=honeycrisp
	banana.species=cavendish
	EOF

	git config --list >actual &&
	grep -Ff expect actual &&

	git -C sub config --list >actual &&
	grep -Ff expect actual
'

test_expect_success 'can --unset from super or sub' '
	git config --superproject apple.species honeycrisp &&
	git -C sub config --superproject banana.species cavendish &&
	test_when_finished rm .git/config.superproject &&

	git config --unset --superproject banana.species &&
	git -C sub config --unset --superproject apple.species
'

test_expect_success 'can --edit superproject config' '
	test_config core.editor "echo [foo]bar=baz >" &&
	git config --edit --superproject &&
	test_when_finished rm .git/config.superproject &&

	git config --get foo.bar
'

test_expect_success 'can --show-origin the superproject config' '
	git config --superproject --add foo.bar baz &&
	test_when_finished rm .git/config.superproject &&

	git config --list --show-origin >actual &&
	grep -F "config.superproject" actual
'

test_expect_success 'can --show-scope the superproject config' '
	git config --superproject --add foo.bar baz &&
	test_when_finished rm .git/config.superproject &&

	git config --list --show-scope >actual &&
	grep "superproject" actual
'

test_expect_success 'pre-existing config applies to new submodule' '
	git config --superproject --add foo.bar baz &&
	test_when_finished rm .git/config.superproject &&

	git submodule add "${submodurl}" sub2/ &&
	test_when_finished rm -fr sub2 &&
	git commit -m "add a second submodule" &&
	test_when_finished git reset HEAD^ &&

	git -C sub2 config --get foo.bar
'

# NEEDSWORK: submodule.c:get_superproject_working_tree doesn't support worktree
test_expect_failure 'worktrees can still access config.superproject' '
	git config --superproject --add foo.bar baz &&
	test_when_finished rm .git/config.superproject &&

	git worktree add wt &&
	test_when_finished git worktree remove wt &&
	(
		cd wt &&
		git config --get foo.bar
	)
'

# This test deletes the submodule! Keep it at the end of the test suite.
test_expect_success 'config.superproject works even with no submodules' '
	# get rid of the submodule
	git reset HEAD^ &&
	rm -fr sub &&

	git config --superproject --add foo.bar baz &&
	test_when_finished rm .git/config.superproject &&

	git config --get foo.bar
'

test_done
