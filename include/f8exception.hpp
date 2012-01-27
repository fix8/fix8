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
    * Products derived from this software may not be called "Fix8", nor can "Fix8" appear
	   in their name without written permission from fix8.org

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS
OR  IMPLIED  WARRANTIES,  INCLUDING,  BUT  NOT  LIMITED  TO ,  THE  IMPLIED  WARRANTIES  OF
MERCHANTABILITY AND  FITNESS FOR A PARTICULAR  PURPOSE ARE  DISCLAIMED. IN  NO EVENT  SHALL
THE  COPYRIGHT  OWNER OR  CONTRIBUTORS BE  LIABLE  FOR  ANY DIRECT,  INDIRECT,  INCIDENTAL,
SPECIAL,  EXEMPLARY, OR CONSEQUENTIAL  DAMAGES (INCLUDING,  BUT NOT LIMITED TO, PROCUREMENT
OF SUBSTITUTE  GOODS OR SERVICES; LOSS OF USE, DATA,  OR PROFITS; OR BUSINESS INTERRUPTION)
HOWEVER CAUSED  AND ON ANY THEORY OF LIABILITY, WHETHER  IN CONTRACT, STRICT  LIABILITY, OR
TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE
EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#endif
//-------------------------------------------------------------------------------------------------
#ifndef _F8_EXCEPTION_HPP_
#define _F8_EXCEPTION_HPP_

#include <exception>

//-------------------------------------------------------------------------------------------------
namespace FIX8 {

//-------------------------------------------------------------------------------------------------
/// Base exception class.
class f8Exception : public std::exception
{
	std::string _reason;
	bool _force_logoff;

public:
	/*! Ctor.
	    \param force_logoff if true, logoff when thrown */
	f8Exception(bool force_logoff=false) : _force_logoff(force_logoff) {}

	/*! Ctor.
	    \param msg message associated with this exception
	    \param force_logoff if true, logoff when thrown */
	f8Exception(const std::string& msg, bool force_logoff=false)
		: _reason(msg), _force_logoff(force_logoff) {}

	/*! Ctor.
	    \tparam T type of value to format
	    \param msg message associated with this exception
	    \param val value to display with this exception
	    \param force_logoff if true, logoff when thrown */
	template<typename T>
	f8Exception(const std::string& msg, const T& val, bool force_logoff=false)
		: _reason(msg), _force_logoff(force_logoff) { format(msg, val); }

	/// Dtor.
	virtual ~f8Exception() throw() {}

	/*! Get message associated with this exception.
	    \return text message */
	virtual const char *what() const throw() { return _reason.c_str(); }

	/*! Get the force logoff setting.
	    \return true if force logoff is set */
	const bool force_logoff() const { return _force_logoff; }

protected:
	/*! Format a message to associate with this exception.
	    \tparam T type of value to format
	    \param msg message associated with this exception
	    \param what to display with this exception */
	template<typename T>
	void format(const std::string& msg, const T what)
	{
		std::ostringstream ostr;
		ostr << msg << ": " << what;
		_reason = ostr.str();
	}
};

//-------------------------------------------------------------------------------------------------
/// Indicates a static metadata lookup failed. With the exception of user defined fields there should never be an instance where a metatdata lookup fails.
struct InvalidMetadata : f8Exception
{
	InvalidMetadata(const std::string& detail) { format("Invalid Metadata", detail); }
	InvalidMetadata(const unsigned field) { format("Invalid Metadata", field); }
};

//-------------------------------------------------------------------------------------------------
/// A field was decoded in a message that has already been decoded.
struct DuplicateField : f8Exception
{
	DuplicateField(const unsigned field) { format("Duplicate Field", field); }
};

//-------------------------------------------------------------------------------------------------
/// For field types with a specified domain (realm), a value was decoded that was not in the domain set/range.
/*! \tparam T the value type */
template<typename T>
struct InvalidDomainValue : f8Exception
{
	InvalidDomainValue(const T what) { format("Invalid Domain Value", what); }
};

//-------------------------------------------------------------------------------------------------
/// An invalid field was requested or decoded.
struct InvalidField : f8Exception
{
	InvalidField(const unsigned field) { format("Invalid Field", field); }
};

//-------------------------------------------------------------------------------------------------
/// A message was received with an out of sequence sequence number.
struct InvalidMsgSequence : f8Exception
{
	InvalidMsgSequence(const unsigned field) : f8Exception(true) { format("Invalid Message Sequence", field); }
};

//-------------------------------------------------------------------------------------------------
/// A message was decoded with an invalid message body length.
struct InvalidBodyLength : f8Exception
{
	InvalidBodyLength(const unsigned field) { format("Invalid BodyLength", field); }
};

//-------------------------------------------------------------------------------------------------
/// An invalid message was requested or decoded.
struct InvalidMessage : f8Exception
{
	InvalidMessage(const std::string& str) { format("Invalid FIX Message", str); }
};

//-------------------------------------------------------------------------------------------------
/// A message was read that was in an illegal format.
struct IllegalMessage : f8Exception
{
	IllegalMessage(const std::string& str) { format("Illegal FIX Message", str); }
};

//-------------------------------------------------------------------------------------------------
/// A message was decoded that had a Fix version not configured for this session.
struct InvalidVersion : f8Exception
{
	InvalidVersion(const std::string& str) : f8Exception(true) { format("Invalid FIX Version", str); }
};

//-------------------------------------------------------------------------------------------------
/// A message was received with an invalid sending time.
struct BadSendingTime : f8Exception
{
	BadSendingTime(const std::string& str) : f8Exception(true) { format("Bad Sending Time", str); }
};

//-------------------------------------------------------------------------------------------------
/// A message was received with an invalid sender/target compid id.
struct BadCompidId : f8Exception
{
	BadCompidId(const std::string& str) : f8Exception(true) { format("Invalid CompId", str); }
};

//-------------------------------------------------------------------------------------------------
/// An invalid repeating group was found while attempting to encode a message.
struct InvalidRepeatingGroup : f8Exception
{
	InvalidRepeatingGroup(const unsigned field) { format("Invalid Repeating Group", field); }
};

//-------------------------------------------------------------------------------------------------
/// An invalid repeating group field was found while decoding a message (first field is mandatory).
struct MissingRepeatingGroupField : f8Exception
{
	MissingRepeatingGroupField(const unsigned field) { format("First Field in a Repeating Group is Mandatory", field); }
};

//-------------------------------------------------------------------------------------------------
/// An outbound message was missing a header or trailer.
struct MissingMessageComponent : f8Exception
{
	MissingMessageComponent(const char *text) { format("Missing Message Component", text); }
};

//-------------------------------------------------------------------------------------------------
/// An outbound message was missing a mandatory field.
struct MissingMandatoryField : f8Exception
{
	MissingMandatoryField(const unsigned field) { format("Missing Mandatory Field", field); }
	MissingMandatoryField(const std::string& reason) { format("Missing Mandatory Field", reason); }
};

//-------------------------------------------------------------------------------------------------
/// An inbound message had an invalid (incorrect) checksum.
struct BadCheckSum : f8Exception
{
	BadCheckSum(const unsigned int chkval) { format("Checksum failure", chkval); }
};

//-------------------------------------------------------------------------------------------------
/// A pthread attribute error occured.
struct ThreadException : f8Exception
{
	ThreadException(const std::string& reason) { format("Thread error", reason); }
};

//-------------------------------------------------------------------------------------------------
/// A connected peer has reset the connection (disconnected).
struct PeerResetConnection : f8Exception
{
	PeerResetConnection(const std::string& reason) { format("Peer reset connection", reason); }
};

//-------------------------------------------------------------------------------------------------
/// An attempt was made to free an invalid block of memory.
struct InvalidMemoryPtr : f8Exception
{
	InvalidMemoryPtr(const void *ptr) { format("Invalid Memory pool ptr", ptr); }
};

//-------------------------------------------------------------------------------------------------
/// The memory pool free list was unexpectedly full.
struct FreelistFull : f8Exception
{
	FreelistFull() { format("Memory poll freelist", "freelist is full"); }
};

//-------------------------------------------------------------------------------------------------

} // FIX8

#endif // _F8_EXCEPTION_HPP_
