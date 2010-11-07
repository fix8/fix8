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
$Id$
$Date$
$URL$

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
	f8Exception() {}

	template<typename T>
	void format(const std::string& msg, T what)
	{
		std::ostringstream ostr;
		ostr << msg << ": " << what;
		_reason = ostr.str();
	}

public:
	virtual ~f8Exception() throw() {}
	virtual const char *what() const throw() { return _reason.c_str(); }
};

//-------------------------------------------------------------------------------------------------
class InvalidMetadata : public f8Exception
{
public:
	InvalidMetadata(const std::string& detail) { format("Invalid Metadata", detail); }
};

//-------------------------------------------------------------------------------------------------
class DuplicateField : public f8Exception
{
public:
	DuplicateField(const unsigned field) { format("Duplicate Field", field); }
};

//-------------------------------------------------------------------------------------------------
class InvalidField : public f8Exception
{
public:
	InvalidField(const unsigned field) { format("Invalid Field", field); }
};

//-------------------------------------------------------------------------------------------------
class InvalidBodyLength : public f8Exception
{
public:
	InvalidBodyLength(const unsigned field) { format("Invalid BodyLength", field); }
};

//-------------------------------------------------------------------------------------------------
class IllegalMessage : public f8Exception
{
public:
	IllegalMessage(const std::string& str) { format("Illegal FIX Message", str); }
};

//-------------------------------------------------------------------------------------------------
class InvalidVersion : public f8Exception
{
public:
	InvalidVersion(const std::string& str) { format("Invalid FIX Version", str); }
};

//-------------------------------------------------------------------------------------------------
class InvalidRepeatingGroup : public f8Exception
{
public:
	InvalidRepeatingGroup(const unsigned field) { format("Invalid Repeating Group", field); }
};

//-------------------------------------------------------------------------------------------------
class MissingRepeatingGroupField : public f8Exception
{
public:
	MissingRepeatingGroupField(const unsigned field) { format("First Field in a Repeating Group is Mandatory", field); }
};

//-------------------------------------------------------------------------------------------------
class MissingMessageComponent : public f8Exception
{
public:
	MissingMessageComponent(const char *text) { format("Missing Message Component", text); }
};

//-------------------------------------------------------------------------------------------------
class MissingMandatoryField : public f8Exception
{
public:
	MissingMandatoryField(const unsigned field) { format("Missing Mandatory Field", field); }
};

//-------------------------------------------------------------------------------------------------
class BadCheckSum : public f8Exception
{
public:
	BadCheckSum(const std::string& msgtype) { format("Checksum failure", msgtype); }
};

//-------------------------------------------------------------------------------------------------
class ThreadException : public f8Exception
{
public:
	ThreadException(const std::string& reason) { format("Thread exception", reason); }
};

//-------------------------------------------------------------------------------------------------

} // FIX8

#endif // _F8_EXCEPTION_HPP_
