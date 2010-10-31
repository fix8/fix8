//-------------------------------------------------------------------------------------------------
#if 0

Fix8 is released under the New BSD License.

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
#ifndef _FIX8_SESSION_HPP_
#define _FIX8_SESSION_HPP_

//-------------------------------------------------------------------------------------------------
namespace FIX8 {

//-------------------------------------------------------------------------------------------------
class SessionID // quickfix style sessionid
{
	static RegExp _sid;

	typedef Field<f8String, Common_BeginString> begin_string;
	typedef Field<f8String, Common_SenderCompID> sender_compid;
	typedef Field<f8String, Common_TargetCompID> target_compid;
	begin_string _beginString;
	sender_compid _senderCompID;
	target_compid _targetCompID;

	f8String _id;

public:
	SessionID(const f8String& beginString, const f8String& senderCompID, const f8String& targetCompID)
		: _beginString(beginString), _senderCompID(senderCompID), _targetCompID(targetCompID) { make_id(); }
	SessionID(const begin_string& beginString, const sender_compid& senderCompID, const target_compid& targetCompID)
		: _beginString(beginString), _senderCompID(senderCompID), _targetCompID(targetCompID) { make_id(); }
	SessionID(const f8String& from) { from_string(from); }

	virtual ~SessionID() {}

	const f8String& make_id();
	void from_string(const f8String& from);

	const begin_string& get_beginString() const { return _beginString; }
	const sender_compid& get_senderCompID() const { return _senderCompID; }
	const target_compid& get_targetCompID() const { return _targetCompID; }
	const f8String& get_id() const { return _id; }

	friend std::ostream& operator<<(std::ostream& os, const SessionID& what) { return os << what._id; }
};

//-------------------------------------------------------------------------------------------------
struct SessionState
{
	enum States
	{
		st_activated,
		st_wait_for_logon, st_logon_sent, st_logon_received, st_logoff_sent, st_logoff_received,
		st_sequence_reset_sent, st_sequence_reset_received,
	};

	ebitset<States> _ss;
};

//-------------------------------------------------------------------------------------------------
class Session
{
	virtual bool Logon(Message *msg) { return false; }
	virtual bool Logout(Message *msg) { return false; }
	virtual bool Heartbeat(Message *msg) { return false; }
	virtual bool ResendRequest(Message *msg) { return false; }
	virtual bool SequenceReset(Message *msg) { return false; }
	virtual bool TestRequest(Message *msg) { return false; }
	virtual bool Reject(Message *msg) { return false; }
	virtual bool Application(Message *msg) { return false; }

	typedef StaticTable<const f8String, bool (Session::*)(Message *)> Handlers;
	Handlers _handlers;

protected:
	typedef Field<SeqNum, Common_MsgSeqNum> msg_seq_num;
	typedef Field<f8String, Common_SenderCompID> sender_comp_id;
	typedef Field<f8String, Common_TargetCompID> target_comp_id;
	typedef Field<UTCTimestamp, Common_SendingTime> sending_time;

public:
	Session();
	virtual ~Session() {}

	void start();

	friend class StaticTable<const f8String, bool (Session::*)(Message *)>;
};

//-------------------------------------------------------------------------------------------------

} // FIX8

#endif // _FIX8_SESSION_HPP_
