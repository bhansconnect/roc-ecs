#!/bin/sh
set -e
SCRIPT_DIR=$(cd $(dirname "${BASH_SOURCE[0]}") && pwd)

args="$@"
cmd="cd ../roc && ./target/release/roc build ../roc-simple-ecs/simpleEcs.roc --no-link --precompiled-host $args"

(cd $SCRIPT_DIR && \
    ([ -d build ] || meson setup build --buildtype release) && \
	nix-shell --command "$cmd" ../roc/shell.nix && \
	ar rcs libroc-simple-ecs.a roc-simple-ecs.o && \
	ninja -C build)


