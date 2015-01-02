//-------------------------------------------------------------------------------------------------
/*

Fix8 is released under the GNU LESSER GENERAL PUBLIC LICENSE Version 3.

Fix8 Open Source FIX Engine.
Copyright (C) 2010-15 David L. Dight <fix@fix8.org>

Fix8 is free software: you can  redistribute it and / or modify  it under the  terms of the
GNU Lesser General  Public License as  published  by the Free  Software Foundation,  either
version 3 of the License, or (at your option) any later version.

Fix8 is distributed in the hope  that it will be useful, but WITHOUT ANY WARRANTY;  without
even the  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

You should  have received a copy of the GNU Lesser General Public  License along with Fix8.
If not, see <http://www.gnu.org/licenses/>.

THE EXTENT  PERMITTED  BY  APPLICABLE  LAW.  EXCEPT WHEN  OTHERWISE  STATED IN  WRITING THE
COPYRIGHT HOLDERS AND/OR OTHER PARTIES  PROVIDE THE PROGRAM "AS IS" WITHOUT WARRANTY OF ANY
KIND,  EITHER EXPRESSED   OR   IMPLIED,  INCLUDING,  BUT   NOT  LIMITED   TO,  THE  IMPLIED
WARRANTIES  OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.  THE ENTIRE RISK AS TO
THE QUALITY AND PERFORMANCE OF THE PROGRAM IS WITH YOU. SHOULD THE PROGRAM PROVE DEFECTIVE,
YOU ASSUME THE COST OF ALL NECESSARY SERVICING, REPAIR OR CORRECTION.

IN NO EVENT UNLESS REQUIRED  BY APPLICABLE LAW  OR AGREED TO IN  WRITING WILL ANY COPYRIGHT
HOLDER, OR  ANY OTHER PARTY  WHO MAY MODIFY  AND/OR REDISTRIBUTE  THE PROGRAM AS  PERMITTED
ABOVE,  BE  LIABLE  TO  YOU  FOR  DAMAGES,  INCLUDING  ANY  GENERAL, SPECIAL, INCIDENTAL OR
CONSEQUENTIAL DAMAGES ARISING OUT OF THE USE OR INABILITY TO USE THE PROGRAM (INCLUDING BUT
NOT LIMITED TO LOSS OF DATA OR DATA BEING RENDERED INACCURATE OR LOSSES SUSTAINED BY YOU OR
THIRD PARTIES OR A FAILURE OF THE PROGRAM TO OPERATE WITH ANY OTHER PROGRAMS), EVEN IF SUCH
HOLDER OR OTHER PARTY HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES.

*/
//-------------------------------------------------------------------------------------------------
#ifndef FIX8_EXCEPTION_HPP_
#define FIX8_EXCEPTION_HPP_

//-------------------------------------------------------------------------------------------------
#include <string>
#include <sstream>
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
	    \param msg message associated with this exception
	    \param force_logoff if true, logoff when thrown */
	f8Exception(const char *msg, bool force_logoff=false)
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
	const char *what() const throw() { return _reason.c_str(); }

	/*! Get the force logoff setting.
	    \return true if force logoff is set */
	bool force_logoff() const { return _force_logoff; }

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

	/*! Format a message to associate with this exception.
	    \tparam T type of value to format
	    \tparam S type of 2nd value to format
	    \param msg message associated with this exception
	    \param what to display with this exception
	    \param msg2 message associated with this exception
	    \param what2 to display with this exception */
	template<typename T, typename S>
	void format(const std::string& msg, const T what, const std::string& msg2, const S what2)
	{
		std::ostringstream ostr;
		ostr << msg << ": " << what << msg2 << ": " << what2;
		_reason = ostr.str();
	}
};

//-------------------------------------------------------------------------------------------------
/// Indicates a static metadata lookup failed. With the exception of user defined fields there should never be an instance where a metatdata lookup fails.
template<typename T>
struct InvalidMetadata : f8Exception
{
	const T _tagid;
	InvalidMetadata(T field) : _tagid(field) { format("Invalid Metadata", field); }
};

//-------------------------------------------------------------------------------------------------
/// A field was decoded in a message that has already been decoded.
struct DuplicateField : f8Exception
{
	const int _tagid;
	DuplicateField(unsigned field) : _tagid(field) { format("Duplicate Field", field); }
};

//-------------------------------------------------------------------------------------------------
/// For field types with a specified domain (realm), a value was decoded that was not in the domain set/range.
/*! \tparam T the value type */
template<typename T>
struct InvalidDomainValue : f8Exception
{
	InvalidDomainValue(T what) { format("Invalid Domain Value", what); }
};

//-------------------------------------------------------------------------------------------------
/// An invalid field was requested or added
struct InvalidField : f8Exception
{
	const int _tagid;
	InvalidField(unsigned field) : _tagid(field) { format("Invalid Field Added", field); }
};

//-------------------------------------------------------------------------------------------------
/// An invalid field was decoded.
struct UnknownField : f8Exception
{
	const int _tagid;
	UnknownField(unsigned field) : _tagid(field) { format("Unknown Field Decoded", field); }
};

//-------------------------------------------------------------------------------------------------
/// A message was received with an out of sequence sequence number.
struct InvalidMsgSequence : f8Exception
{
	const unsigned _rseqnum, _eseqnum;
	InvalidMsgSequence(unsigned rseqnum, unsigned eseqnum)
		: f8Exception(true), _rseqnum(rseqnum), _eseqnum(eseqnum)
		{ format("Invalid Sequence number, received", rseqnum, " expected", eseqnum); }
};

//-------------------------------------------------------------------------------------------------
/// A message was received with an out of sequence sequence number.
struct MsgSequenceTooLow : f8Exception
{
	const unsigned _rseqnum, _eseqnum;
	MsgSequenceTooLow(unsigned rseqnum, unsigned eseqnum)
		: f8Exception(true), _rseqnum(rseqnum), _eseqnum(eseqnum)
		{ format("Message Sequence too low, received", rseqnum, " expected", eseqnum); }
};

//-------------------------------------------------------------------------------------------------
/// A message was decoded with an invalid message body length.
struct InvalidBodyLength : f8Exception
{
	const unsigned _length;
	InvalidBodyLength(unsigned length) : _length(length) { format("Invalid BodyLength", length); }
};

//-------------------------------------------------------------------------------------------------
/// An invalid message was requested or decoded.
struct InvalidMessage : f8Exception
{
	InvalidMessage(const std::string& str) { format("Invalid FIX Message", str); }
	InvalidMessage(const std::string& str, const char *str1) { format("Invalid FIX Message", str, " at", str1); }
};

//-------------------------------------------------------------------------------------------------
/// A message was read that was in an illegal format.
struct IllegalMessage : f8Exception
{
	IllegalMessage(const std::string& str) { format("Illegal FIX Message", str); }
	IllegalMessage(const std::string& str, const char *str1) { format("Illegal FIX Message", str, " at", str1); }
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
	const int _tagid;
	InvalidRepeatingGroup(unsigned field) : _tagid(field) { format("Invalid Repeating Group", field); }
	InvalidRepeatingGroup(unsigned field, const char *str)
	 : _tagid(field)	{ format("Invalid Repeating Group", field, " in", str); }
};

//-------------------------------------------------------------------------------------------------
/// An invalid repeating group field was found while decoding a message (first field is mandatory).
struct MissingRepeatingGroupField : f8Exception
{
	int _tagid;
	MissingRepeatingGroupField(unsigned field) : _tagid(field) { format("First Field in a Repeating Group is Mandatory", field); }
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
	const int _tagid;
	MissingMandatoryField(unsigned field) : _tagid(field) { format("Missing Mandatory Field", field); }
	MissingMandatoryField(const std::string& reason) : _tagid() { format("Missing Mandatory Field", reason); }
};

//-------------------------------------------------------------------------------------------------
/// An inbound message had an invalid (incorrect) checksum.
struct BadCheckSum : f8Exception
{
	const unsigned int _chkval;
	BadCheckSum(unsigned int chkval) : _chkval(chkval) { format("Checksum failure", chkval); }
};

//-------------------------------------------------------------------------------------------------
/// A pthread attribute error occured.
struct f8_threadException : f8Exception
{
	f8_threadException(const std::string& reason) { format("Thread error", reason); }
};

//-------------------------------------------------------------------------------------------------
/// A connected peer has reset the connection (disconnected).
struct PeerResetConnection : f8Exception
{
	PeerResetConnection(const std::string& reason) { format("Peer reset connection", reason); }
};

//-------------------------------------------------------------------------------------------------
/// An invalid configuration parameter was passed.
struct InvalidConfiguration : f8Exception
{
	InvalidConfiguration(const std::string& str) { format("Invalid configuration setting in", str.empty() ? "unknown" : str); }
};

//-------------------------------------------------------------------------------------------------
/// An bad or missing configuration parameter.
struct ConfigurationError : f8Exception
{
	ConfigurationError(const std::string& str) { format("Configuration error", str); }
	ConfigurationError(const std::string& str, const std::string& str1) { format(str, str1); }
};

//-------------------------------------------------------------------------------------------------
/// Could not open a logfile
struct LogfileException : f8Exception
{
	LogfileException(const std::string& str) { format("Error opening logfile", str); }
};

//-------------------------------------------------------------------------------------------------
/// An invalid configuration parameter was passed.
struct XMLError : f8Exception
{
	XMLError(const std::string& str) { format("XML parsing error", str); }
};

//-------------------------------------------------------------------------------------------------

} // FIX8

#endif // _F8_EXCEPTION_HPP_
