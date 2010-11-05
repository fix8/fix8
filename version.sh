#!/bin/bash
#############################################################################################
# Orbweb is released under the New BSD License.
#
# Copyright (c) 2007-2010, David L. Dight <www@orbweb.org>
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without modification, are
# permitted provided that the following conditions are met:
#
#   * Redistributions of source code must retain the above copyright notice, this list of
#	 	conditions and the following disclaimer.
#   * Redistributions in binary form must reproduce the above copyright notice, this list
#	 	of conditions and the following disclaimer in the documentation and/or other
#		materials provided with the distribution.
#   * Neither the name of the author nor the names of its contributors may be used to
#	 	endorse or promote products derived from this software without specific prior
#		written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS
# OR  IMPLIED  WARRANTIES,  INCLUDING,  BUT  NOT  LIMITED  TO ,  THE  IMPLIED  WARRANTIES  OF
# MERCHANTABILITY AND  FITNESS FOR A PARTICULAR  PURPOSE ARE  DISCLAIMED. IN  NO EVENT  SHALL
# THE  COPYRIGHT  OWNER OR  CONTRIBUTORS BE  LIABLE  FOR  ANY DIRECT,  INDIRECT,  INCIDENTAL,
# SPECIAL,  EXEMPLARY, OR CONSEQUENTIAL  DAMAGES (INCLUDING,  BUT NOT LIMITED TO, PROCUREMENT
# OF SUBSTITUTE  GOODS OR SERVICES; LOSS OF USE, DATA,  OR PROFITS; OR BUSINESS INTERRUPTION)
# HOWEVER CAUSED  AND ON ANY THEORY OF LIABILITY, WHETHER  IN CONTRACT, STRICT  LIABILITY, OR
# TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE
# EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
#
# $Id$
# $LastChangedDate$
# $URL$
#############################################################################################
#
# This is the master version file, called by configure
#
# If building from a source tarball, the embedded revision will be used
# otherwise svn is queried.
#
#############################################################################################
MAJOR_VERSION_NUM=0
MINOR_VERSION_NUM=1

# we would expect svnversion to return a string like 123:126
BUILD_VERSION_NUM=0
if test "`type -t svnversion`" = file; then
	BUILD_VERSION_NUM=`svnversion | sed 's/[^0-9]/ /g' | awk '{printf "%d",$2}'`
fi
if test $BUILD_VERSION_NUM = 0; then
	BUILD_VERSION_NUM=`echo "$Revision$" | sed 's/[^0-9]/ /g' | awk '{printf "%d",$1}'`
fi
echo $MAJOR_VERSION_NUM.$MINOR_VERSION_NUM.$BUILD_VERSION_NUM
