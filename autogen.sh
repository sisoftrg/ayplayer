#!/bin/sh

autopoint --force &&
aclocal &&
autoheader &&
automake --copy --add-missing --force-missing &&
autoconf --force
