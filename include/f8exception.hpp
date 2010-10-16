//-------------------------------------------------------------------------------------------------
#if 0

fix8 is released under the New BSD License.

Copyright (c) 2010, David L. Dight <fix@fix8.org>
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are
permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of
	 	conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list
	 	of conditions and the following disclaimer in the documentation and/or other
		materials provided with the distribution.
    * Neither the name of the author nor the names of its contributors may be used to
	 	endorse or promote products derived from this software without specific prior
		written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS
OR  IMPLIED  WARRANTIES,  INCLUDING,  BUT  NOT  LIMITED  TO ,  THE  IMPLIED  WARRANTIES  OF
MERCHANTABILITY AND  FITNESS FOR A PARTICULAR  PURPOSE ARE  DISCLAIMED. IN  NO EVENT  SHALL
THE  COPYRIGHT  OWNER OR  CONTRIBUTORS BE  LIABLE  FOR  ANY DIRECT,  INDIRECT,  INCIDENTAL,
SPECIAL,  EXEMPLARY, OR CONSEQUENTIAL  DAMAGES (INCLUDING,  BUT NOT LIMITED TO, PROCUREMENT
OF SUBSTITUTE  GOODS OR SERVICES; LOSS OF USE, DATA,  OR PROFITS; OR BUSINESS INTERRUPTION)
HOWEVER CAUSED  AND ON ANY THEORY OF LIABILITY, WHETHER  IN CONTRACT, STRICT  LIABILITY, OR
TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE
EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

---------------------------------------------------------------------------------------------------
$Id: f8types.hpp 514 2010-09-13 12:23:27Z davidd $
$Date: 2010-09-13 22:23:27 +1000 (Mon, 13 Sep 2010) $
$URL: svn://catfarm.electro.mine.nu/usr/local/repos/fix8/include/f8types.hpp $

#endif
//-------------------------------------------------------------------------------------------------
#ifndef _F8_EXCEPTION_HPP_
#define _F8_EXCEPTION_HPP_

#include <exception>

//-------------------------------------------------------------------------------------------------
namespace FIX8 {

//-------------------------------------------------------------------------------------------------
class f8Exception : public std::exception
{
	std::string _reason;

protected:
	template<typename T>
	void format(const std::string& msg, T what)
	{
		std::ostringstream ostr;
		ostr << msg << ": " << what;
		_reason = ostr.str();
	}

public:
	f8Exception() {}
	virtual ~f8Exception() throw() {}
	virtual const char *what() const throw() { return _reason.c_str(); }
};

//-------------------------------------------------------------------------------------------------
class InvalidMetadata : public f8Exception
{
public:
	InvalidMetadata(const std::string& detail) { format("Invalid Metadata", detail); }
	~InvalidMetadata() throw() {}
};

//-------------------------------------------------------------------------------------------------
class DuplicateField : public f8Exception
{
public:
	DuplicateField(const unsigned field) { format("Duplicate field", field); }
	~DuplicateField() throw() {}
};

//-------------------------------------------------------------------------------------------------
class InvalidField : public f8Exception
{
public:
	InvalidField(const unsigned field) { format("Invalid Field", field); }
	~InvalidField() throw() {}
};

//-------------------------------------------------------------------------------------------------
class InvalidRepeatingGroup : public f8Exception
{
public:
	InvalidRepeatingGroup(const unsigned field) { format("Invalid Repeating Group", field); }
	~InvalidRepeatingGroup() throw() {}
};

//-------------------------------------------------------------------------------------------------
class MissingRepeatingGroupField : public f8Exception
{
public:
	MissingRepeatingGroupField(const unsigned field) { format("First Field of a Repeating Group is Mandatory", field); }
	~MissingRepeatingGroupField() throw() {}
};

//-------------------------------------------------------------------------------------------------
class MissingMandatoryField : public f8Exception
{
public:
	MissingMandatoryField(const unsigned field) { format("Mandatory Field is missing", field); }
	~MissingMandatoryField() throw() {}
};

//-------------------------------------------------------------------------------------------------

} // FIX8

#endif // _F8_EXCEPTION_HPP_
