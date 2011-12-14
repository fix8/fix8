//-------------------------------------------------------------------------------------------------
#if 0

fix8 is released under the New BSD License.

Copyright (c) 2010-12, David L. Dight <fix@fix8.org>
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
	bool _force_logoff;

public:
	f8Exception(bool force_logoff=false) : _force_logoff(force_logoff) {}
	f8Exception(const std::string& msg, bool force_logoff=false)
		: _reason(msg), _force_logoff(force_logoff) {}

	template<typename T>
	f8Exception(const std::string& msg, const T& val, bool force_logoff=false)
		: _reason(msg), _force_logoff(force_logoff) { format(msg, val); }

	virtual ~f8Exception() throw() {}
	virtual const char *what() const throw() { return _reason.c_str(); }
	const bool force_logoff() const { return _force_logoff; }

protected:
	template<typename T>
	void format(const std::string& msg, const T what)
	{
		std::ostringstream ostr;
		ostr << msg << ": " << what;
		_reason = ostr.str();
	}
};

//-------------------------------------------------------------------------------------------------
struct InvalidMetadata : f8Exception
{
	InvalidMetadata(const std::string& detail) { format("Invalid Metadata", detail); }
	InvalidMetadata(const unsigned field) { format("Invalid Metadata", field); }
};

//-------------------------------------------------------------------------------------------------
struct DuplicateField : f8Exception
{
	DuplicateField(const unsigned field) { format("Duplicate Field", field); }
};

//-------------------------------------------------------------------------------------------------
struct InvalidField : f8Exception
{
	InvalidField(const unsigned field) { format("Invalid Field", field); }
};

//-------------------------------------------------------------------------------------------------
struct InvalidMsgSequence : f8Exception
{
	InvalidMsgSequence(const unsigned field) : f8Exception(true) { format("Invalid Message Sequence", field); }
};

//-------------------------------------------------------------------------------------------------
struct InvalidBodyLength : f8Exception
{
	InvalidBodyLength(const unsigned field) { format("Invalid BodyLength", field); }
};

//-------------------------------------------------------------------------------------------------
struct InvalidMessage : f8Exception
{
	InvalidMessage(const std::string& str) { format("Invalid FIX Message", str); }
};

//-------------------------------------------------------------------------------------------------
struct IllegalMessage : f8Exception
{
	IllegalMessage(const std::string& str) { format("Illegal FIX Message", str); }
};

//-------------------------------------------------------------------------------------------------
struct InvalidVersion : f8Exception
{
	InvalidVersion(const std::string& str) : f8Exception(true) { format("Invalid FIX Version", str); }
};

//-------------------------------------------------------------------------------------------------
struct BadSendingTime : f8Exception
{
	BadSendingTime(const std::string& str) : f8Exception(true) { format("Bad Sending Time", str); }
};

//-------------------------------------------------------------------------------------------------
struct BadCompidId : f8Exception
{
	BadCompidId(const std::string& str) : f8Exception(true) { format("Invalid CompId", str); }
};

//-------------------------------------------------------------------------------------------------
struct InvalidRepeatingGroup : f8Exception
{
	InvalidRepeatingGroup(const unsigned field) { format("Invalid Repeating Group", field); }
};

//-------------------------------------------------------------------------------------------------
struct MissingRepeatingGroupField : f8Exception
{
	MissingRepeatingGroupField(const unsigned field) { format("First Field in a Repeating Group is Mandatory", field); }
};

//-------------------------------------------------------------------------------------------------
struct MissingMessageComponent : f8Exception
{
	MissingMessageComponent(const char *text) { format("Missing Message Component", text); }
};

//-------------------------------------------------------------------------------------------------
struct MissingMandatoryField : f8Exception
{
	MissingMandatoryField(const unsigned field) { format("Missing Mandatory Field", field); }
	MissingMandatoryField(const std::string& reason) { format("Missing Mandatory Field", reason); }
};

//-------------------------------------------------------------------------------------------------
struct BadCheckSum : f8Exception
{
	BadCheckSum(const std::string& msgtype) { format("Checksum failure", msgtype); }
};

//-------------------------------------------------------------------------------------------------
struct ThreadException : f8Exception
{
	ThreadException(const std::string& reason) { format("Thread error", reason); }
};

//-------------------------------------------------------------------------------------------------
struct PeerResetConnection : f8Exception
{
	PeerResetConnection(const std::string& reason) { format("Peer reset connection", reason); }
};

//-------------------------------------------------------------------------------------------------
struct InvalidMemoryPtr : f8Exception
{
	InvalidMemoryPtr(const void *ptr) { format("Invalid Memory pool ptr", ptr); }
};

//-------------------------------------------------------------------------------------------------
struct FreelistFull : f8Exception
{
	FreelistFull() { format("Memory poll freelist", "freelist is full"); }
};

//-------------------------------------------------------------------------------------------------

} // FIX8

#endif // _F8_EXCEPTION_HPP_
