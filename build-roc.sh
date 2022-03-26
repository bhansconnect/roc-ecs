#!/bin/sh
set -e

nix-shell \
	--command 'cd roc && cargo build --release --bin roc' \
	./roc/shell.nix
