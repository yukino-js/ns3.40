#!/usr/bin/env bash

set -euo pipefail

REPO_ROOT="$(git rev-parse --show-toplevel)"
CLANG_FORMAT_CONFIG="$REPO_ROOT/.clang-format"

if [[ ! -f "$CLANG_FORMAT_CONFIG" ]]; then
	echo "error: clang-format config not found: $CLANG_FORMAT_CONFIG" >&2
	exit 1
fi

git ls-files -z \
	'*.h' '*.c' \
	'*.hh' '*.cc' \
	'*.hpp' '*.cpp' \
	'*.hxx' '*.cxx' \
	'*.h++' '*.c++' |
	xargs -0 -r clang-format -style=file:"$CLANG_FORMAT_CONFIG" -i

uv run ruff format ./

git ls-files -z '*.sh' |
	xargs -0 -r shfmt -l -w
