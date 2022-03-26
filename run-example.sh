#!/bin/sh
set -e

example="$1"
shift 1
args="$@"
example_roc="examples/$example.roc"
if [ -f "$example_roc" ]; then
	cmd="cd roc && ./target/release/roc ../$example_roc $args"
	nix-shell \
		--command "$cmd" \
		./roc/shell.nix
else
	echo "$example is not an example!"
fi
