#!/bin/bash

test_description='config-managed multihooks, including git-hook command'

. ./test-lib.sh

ROOT=
if test_have_prereq MINGW
then
	# In Git for Windows, Unix-like paths work only in shell scripts;
	# `git.exe`, however, will prefix them with the pseudo root directory
	# (of the Unix shell). Let's accommodate for that.
	ROOT="$(cd / && pwd)"
fi

setup_hooks () {
	test_config hook.pre-commit.command "/path/ghi" --add
	test_config_global hook.pre-commit.command "/path/def" --add
}

setup_hookcmd () {
	test_config hook.pre-commit.command "abc" --add
	test_config_global hookcmd.abc.command "/path/abc" --add
}

setup_hookdir () {
	mkdir .git/hooks
	write_script .git/hooks/pre-commit <<-EOF
	echo \"Legacy Hook\"
	EOF
	test_when_finished rm -rf .git/hooks
}

test_expect_success 'git hook rejects commands without a mode' '
	test_must_fail git hook pre-commit
'


test_expect_success 'git hook rejects commands without a hookname' '
	test_must_fail git hook list
'

test_expect_success 'git hook runs outside of a repo' '
	setup_hooks &&

	cat >expected <<-EOF &&
	global: $ROOT/path/def
	EOF

	nongit git config --list --global &&

	nongit git hook list pre-commit >actual &&
	test_cmp expected actual
'

test_expect_success 'git hook list orders by config order' '
	setup_hooks &&

	cat >expected <<-EOF &&
	global: $ROOT/path/def
	local: $ROOT/path/ghi
	EOF

	git hook list pre-commit >actual &&
	test_cmp expected actual
'

test_expect_success 'git hook list dereferences a hookcmd' '
	setup_hooks &&
	setup_hookcmd &&

	cat >expected <<-EOF &&
	global: $ROOT/path/def
	local: $ROOT/path/ghi
	local: $ROOT/path/abc
	EOF

	git hook list pre-commit >actual &&
	test_cmp expected actual
'

test_expect_success 'git hook list reorders on duplicate commands' '
	setup_hooks &&

	test_config hook.pre-commit.command "/path/def" --add &&

	cat >expected <<-EOF &&
	local: $ROOT/path/ghi
	local: $ROOT/path/def
	EOF

	git hook list pre-commit >actual &&
	test_cmp expected actual
'

test_expect_success 'git hook list shows hooks from the hookdir' '
	setup_hookdir &&

	cat >expected <<-EOF &&
	hookdir: $(pwd)/.git/hooks/pre-commit
	EOF

	git hook list pre-commit >actual &&
	test_cmp expected actual
'

test_expect_success 'hook.runHookDir = no is respected by list' '
	setup_hookdir &&

	test_config hook.runHookDir "no" &&

	cat >expected <<-EOF &&
	hookdir: $(pwd)/.git/hooks/pre-commit (will not run)
	EOF

	git hook list pre-commit >actual &&
	# the hookdir annotation is translated
	test_i18ncmp expected actual
'

test_expect_success 'hook.runHookDir = warn is respected by list' '
	setup_hookdir &&

	test_config hook.runHookDir "warn" &&

	cat >expected <<-EOF &&
	hookdir: $(pwd)/.git/hooks/pre-commit (will warn)
	EOF

	git hook list pre-commit >actual &&
	# the hookdir annotation is translated
	test_i18ncmp expected actual
'

test_expect_success 'git hook list removes skipped hookcmd' '
	setup_hookcmd &&
	test_config hookcmd.abc.skip "true" --add &&

	cat >expected <<-EOF &&
	no commands configured for hook '\''pre-commit'\''
	EOF

	git hook list pre-commit >actual &&
	test_i18ncmp expected actual
'

test_expect_success 'git hook list removes skipped inlined hook' '
	setup_hooks &&
	test_config hookcmd."$ROOT/path/ghi".skip "true" --add &&

	cat >expected <<-EOF &&
	global: $ROOT/path/def
	EOF

	git hook list pre-commit >actual &&
	test_cmp expected actual
'

test_expect_success 'hook.runHookDir = interactive is respected by list' '
	setup_hookdir &&

	test_config hook.runHookDir "interactive" &&

	cat >expected <<-EOF &&
	hookdir: $(pwd)/.git/hooks/pre-commit (will prompt)
	EOF

	git hook list pre-commit >actual &&
	# the hookdir annotation is translated
	test_i18ncmp expected actual
'

test_done
