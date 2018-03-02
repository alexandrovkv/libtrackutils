#! /bin/sh
#
# Execute build stage.
#

touch NEWS README AUTHORS ChangeLog
libtoolize --copy --force
aclocal
autoconf
automake --add-missing

#./configure --enable-shared --disable-static "$@"
