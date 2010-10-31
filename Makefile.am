#############################################################################################
# Fix8 is released under the New BSD License.
#
# Copyright (c) 2010, David L. Dight <fix@fix8.org>
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
# $Id: Makefile.am 496 2010-01-04 10:01:54Z davidd $
# $LastChangedDate: 2010-01-04 21:01:54 +1100 (Mon, 04 Jan 2010) $
# $Rev: 496 $
# $URL: svn://catfarm.electro.mine.nu/usr/local/repos/mongod/trunk/Makefile.am $
#
#############################################################################################

SUBDIRS = runtime compiler test

#############################################################################################
ACLOCAL_AMFLAGS = -I m4
INCLUDE = include

$(distdir).tar.gz:
	make dist

pkg: $(distdir).tar.gz $(PACKAGE).spec
	$(PACKAGER)

src-release:
	make dist
	md5sum $(distdir).tar.gz > $(distdir).md5sum

check-release:
	md5sum -c $(distdir).md5sum

