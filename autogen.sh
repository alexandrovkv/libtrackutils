#! /bin/sh
#
# Execute build stage.
#
# $Id: autogen.sh,v 1.1 2014/12/09 17:18:22 ic Exp $
#
# $Log: autogen.sh,v $
# Revision 1.1  2014/12/09 17:18:22  ic
# Initial revision.
#
#

touch NEWS README AUTHORS ChangeLog
libtoolize --copy --force
aclocal
autoconf
automake --add-missing

#./configure --enable-shared --disable-static "$@"

