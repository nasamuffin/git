#!/bin/sh

test_description='git-psuh prints a nice pony'

. ./test-lib.sh

test_expect_success 'runs correctly with no args and good output' '
	git psuh > actual &&
	test_i18ngrep Pony actual
'

test_done
