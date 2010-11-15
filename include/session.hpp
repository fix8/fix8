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

#include <Poco/Net/StreamSocket.h>
#include <Poco/Util/Timer.h>
#include <Poco/Util/TimerTask.h>
#include <Poco/Util/TimerTaskAdapter.h>
#include <tbb/atomic.h>
#include <connection.hpp>

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
	SessionID(const SessionID& from) : _beginString(from._beginString), _senderCompID(from._senderCompID),
		_targetCompID(from._targetCompID), _id(from._id) {}
	SessionID() {}

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
struct States
{
	enum Tests
	{
		pr_begin_str, pr_logged_in, pr_low, pr_high, pr_comp_id, pr_target_id, pr_logon_timeout,
	};

	enum ConnectionStates
	{
		ct_activated,
		ct_logged_in
	};

	enum SessionStates
	{
		st_wait_for_logon, st_logon_sent, st_logon_received, st_logoff_sent, st_logoff_received,
		st_sequence_reset_sent, st_sequence_reset_received,
	};

	//ebitset<States> _ss;
};

//-------------------------------------------------------------------------------------------------
class Persister;
class Logger;

//-------------------------------------------------------------------------------------------------
class Session
{
	typedef StaticTable<const f8String, bool (Session::*)(const Message *)> Handlers;
	Handlers _handlers;

protected:
	tbb::atomic<bool> _shutdown;
	tbb::atomic<unsigned> _next_sender_seq, _next_target_seq;
	const F8MetaCntx& _ctx;
	Connection *_connection;
	SessionID _sid;

	Persister *_persist;
	Logger *_logger;

	Poco::Util::Timer _heartbeat_timer;
	void heartbeat_service(Poco::Util::TimerTask &);	// generate heartbeats

	virtual bool handle_logon(const Message *msg);
	virtual Message *generate_logon(const unsigned heartbeat_interval);

	virtual bool handle_logout(const Message *msg) { return false; }
	virtual Message *generate_logout();

	virtual bool handle_heartbeat(const Message *msg) { return false; }
	virtual Message *generate_heartbeat(const f8String& testReqID);

	virtual bool handle_resend_request(const Message *msg) { return false; }
	virtual Message *generate_resend_request(const unsigned begin, const unsigned end);

	virtual bool handle_sequence_reset(const Message *msg) { return false; }
	virtual Message *generate_sequence_reset(const unsigned newseqnum, const bool gapfillflag=false);

	virtual bool handle_test_request(const Message *msg) { return false; }
	virtual Message *generate_test_request(const f8String& testReqID);

	virtual bool handle_reject(const Message *msg) { return false; }
	virtual Message *generate_reject() { return 0; }

	virtual bool handle_admin(const Message *msg) { return true; }
	virtual bool handle_application(const Message *msg);

	Message *create_msg(const f8String& msg_type)
	{
		const BaseMsgEntry *bme(_ctx._bme.find_ptr(msg_type));
		if (!bme)
			throw InvalidMetadata(msg_type);
		return bme->_create();
	}

public:
	Session(const F8MetaCntx& ctx, const SessionID& sid, Persister *persist=0, Logger *logger=0);
	Session(const F8MetaCntx& ctx, Persister *persist=0, Logger *logger=0);
	virtual ~Session();

	int begin(Connection *connection, bool wait=true);
	bool process(const f8String& from);
	bool is_shutdown() const { return _shutdown; }

	typedef std::pair<const unsigned, const f8String> SequencePair;
	virtual bool retrans_callback(const SequencePair& with) { return true; }

	bool send(Message *msg);

	Connection *get_connection() { return _connection; }
	const F8MetaCntx& get_ctx() const { return _ctx; }
	bool log(const std::string& what) const;

	friend class StaticTable<const f8String, bool (Session::*)(const Message *)>;
};

//-------------------------------------------------------------------------------------------------

} // FIX8

#endif // _FIX8_SESSION_HPP_
