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

BECAUSE THE PROGRAM IS  LICENSED FREE OF  CHARGE, THERE IS NO  WARRANTY FOR THE PROGRAM, TO
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
#ifndef FIX8_SESSION_HPP_
#define FIX8_SESSION_HPP_

#include <Poco/Net/StreamSocket.h>
//-------------------------------------------------------------------------------------------------
namespace FIX8 {

//-------------------------------------------------------------------------------------------------
/// Quickfix style sessionid.
class SessionID
{
	static RegExp _sid;

	begin_string _beginString;
	sender_comp_id _senderCompID;
	target_comp_id _targetCompID;

	f8String _id, _rid;

public:
	/*! Ctor.
	    \param beginString Fix begin string
	    \param senderCompID Fix senderCompID string
	    \param targetCompID Fix targetCompID string */
	SessionID(const f8String& beginString, const f8String& senderCompID, const f8String& targetCompID)
		: _beginString(beginString), _senderCompID(senderCompID), _targetCompID(targetCompID) { make_id(); }

	/*! Ctor.
	    \param beginString Fix begin string field
	    \param senderCompID Fix senderCompID string field
	    \param targetCompID Fix targetCompID string field */
	SessionID(const begin_string& beginString, const sender_comp_id& senderCompID, const target_comp_id& targetCompID)
		: _beginString(beginString), _senderCompID(senderCompID), _targetCompID(targetCompID) { make_id(); }

	/*! Ctor.
	    \param from SessionID string */
	SessionID(const f8String& from) { from_string(from); }

	/*! Ctor.
	    \param from SessionID field */
	SessionID(const SessionID& from) : _beginString(from._beginString), _senderCompID(from._senderCompID),
		_targetCompID(from._targetCompID), _id(from._id) {}

	SessionID() {}

	/// Dtor.
	virtual ~SessionID() {}

	/*! Create a sessionid and reverse sessionid strings. */
	F8API void make_id();

	/*! Create a reverse SessionID from the current SessionID
	    \return reverse SessionID */
	F8API SessionID make_reverse_id() const;

	/// Create a sessionid string.
	F8API void from_string(const f8String& from);

	/*! Get the beginstring field.
	    \return beginstring */
	const begin_string& get_beginString() const { return _beginString; }

	/*! Get the sender_comp_id field.
	    \return sender_comp_id */
	const sender_comp_id& get_senderCompID() const { return _senderCompID; }

	/*! Get the target_comp_id field.
	    \return target_comp_id */
	const target_comp_id& get_targetCompID() const { return _targetCompID; }

	/*! Get the target_comp_id field.
	    \return target_comp_id */
	const f8String& get_id() const { return _id; }

	/*! Sendercompid equivalence function..
	  \param targetCompID compid to check
	  \return true if both Targetcompids are the same */
	bool same_sender_comp_id(const target_comp_id& targetCompID) const { return targetCompID() == _senderCompID(); }

	/*! Sendercompid equivalence function..
	  \param senderCompID compid to check
	  \return true if both Sendercompids are the same */
	bool same_target_comp_id(const sender_comp_id& senderCompID) const { return senderCompID() == _targetCompID(); }

	/*! targetcompid equivalence function..
	  \param targetCompID compid to check
	  \return true if both Sendercompids are the same, on the same session side */
	bool same_side_target_comp_id(const target_comp_id& targetCompID) const { return targetCompID() == _targetCompID(); }

	/*! Targetcompid equivalence function..
	  \param senderCompID compid to check
	  \return true if both Targetcompids are the same, on the same session side */
	bool same_side_sender_comp_id(const sender_comp_id& senderCompID) const { return senderCompID() == _senderCompID(); }

	/*! SessionID equivalence operator
	  \param that SessionID to check
	  \return true if SessionIDS are equal */
	bool operator==(const SessionID& that)
	{
		return this != &that ? that._senderCompID() == _senderCompID() && that._targetCompID() == _targetCompID() : true;
	}

	/*! SessionID inequivalence operator
	  \param that SessionID to check
	  \return true if SessionIDS are not equal */
	bool operator!=(const SessionID& that)
	{
		return this != &that ? that._senderCompID() != _senderCompID() && that._targetCompID() != _targetCompID() : false;
	}

	/*! Inserter friend.
	    \param os stream to send to
	    \param what SessionID
	    \return stream */
	friend std::ostream& operator<<(std::ostream& os, const SessionID& what) { return os << what._id; }
};

//-------------------------------------------------------------------------------------------------
/// Session states and semantics.
namespace States
{
	enum SessionStates
	{
		st_none, st_continuous, st_session_terminated,
		st_wait_for_logon, st_not_logged_in, st_logon_sent, st_logon_received, st_logoff_sent, st_logoff_received,
		st_test_request_sent, st_sequence_reset_sent, st_sequence_reset_received,
		st_resend_request_sent, st_resend_request_received, st_num_states
	};

	/*! Determine if this session is live
	  \param ss SessateState to test
	  \return true if live */
	static inline bool is_live(SessionStates ss)
		{ return ss != st_none && ss != st_session_terminated; }

	/*! Determine if this session is in an established state
	  \param ss SessateState to test
	  \return true if established */
	static inline bool is_established(SessionStates ss)
		{ return ss != st_wait_for_logon && ss != st_not_logged_in && ss != st_logon_sent && is_live(ss); }
}

//-------------------------------------------------------------------------------------------------
/// Class to hold client info settings for server sessions
using Client = std::tuple<f8String, Poco::Net::IPAddress>; // name, ip
using Clients = std::unordered_map<f8String, Client>; // tci : name, ip

//-------------------------------------------------------------------------------------------------
namespace defaults
{
	enum
	{
		retry_interval=5000,
		login_retries=3,
		tabsize=3,
		hb_interval=30,
		connect_timeout=10,
		log_rotation=5,
		verification_depth=9
	};
}

//-------------------------------------------------------------------------------------------------
struct Schedule
{
	Tickval _start, _end, _duration;
	int _utc_offset, _start_day, _end_day;
	Tickval::ticks _toffset;

	Schedule() : _start(Tickval::errorticks()), _end(Tickval::errorticks()), _utc_offset(),
		_start_day(-1), _end_day(-1) {}

    Schedule(Tickval start, Tickval end, Tickval duration=Tickval(), int utc_offset=0,
			 int start_day=-1, int end_day=-1) :
		_start(start), _end(end), _duration(duration),
		_utc_offset(utc_offset), _start_day(start_day), _end_day(end_day),
		_toffset(static_cast<Tickval::ticks>(_utc_offset) * Tickval::minute)
	{
	}

	Schedule(const Schedule& from) :
		_start(from._start), _end(from._end), _duration(from._duration),
		_utc_offset(from._utc_offset), _start_day(from._start_day), _end_day(from._end_day),
		_toffset(from._toffset)
	{
	}

	Schedule& operator=(const Schedule& that)
	{
		if (this != &that)
		{
			_start = that._start;
			_end = that._end;
			_duration = that._duration;
			_utc_offset = that._utc_offset;
			_start_day = that._start_day;
			_end_day = that._end_day;
			_toffset = that._toffset;
		}

		return *this;
	}

	/*! Determine if this schdule is valid
	    \return true if this schedule is valid */
	bool is_valid() const { return !_start.is_errorval(); }

	/*! Take the current local time and test if it is within the range of this schedule
	    \param prev current bool state that we will toggle
	    \return new toggle state */
	bool test(bool prev=false) const
	{
		Tickval now(true);
		now.adjust(_toffset); // adjust for local utc offset
		const Tickval today(now.get_ticks() - (now.get_ticks() % Tickval::day));
		bool active(prev);

		if (_start_day < 0) // start/end day not specified; daily only
		{
			//cout >> now << ' ' >> (today + _start) << ' ' >> (today + _end) << endl;

			if (now.in_range(today + _start, today + _end))
			{
				if (!prev)
					active = true;
			}
			else if (prev)
				active = false;
		}
		else
		{
			const tm result(now.get_tm());

			//cout >> now << ' ' >> (today + _start) << ' ' >> (today + _end) << ' ' << result.tm_wday << endl;

			if (!prev)
			{
				if ( ((_start_day > _end_day && (result.tm_wday >= _start_day || result.tm_wday <= _end_day))
					|| (_start_day < _end_day && result.tm_wday >= _start_day && result.tm_wday <= _end_day))
					&& now.in_range(today + _start, today + _end))
						active = true;
			}
			else if ( ((_start_day > _end_day && (result.tm_wday < _start_day && result.tm_wday > _end_day))
					  || (_start_day < _end_day && result.tm_wday >= _end_day))
						 && now > today + _end)
					active = false;
		}

		return active;
	}

	/*! Inserter friend.
	    \param os stream to send to
	    \param what Session_Schedule reference
	    \return stream */
	friend std::ostream& operator<<(std::ostream& os, const Schedule& what)
	{
		os << "start:" >> what._start << " end:" >> what._end << " duration:" << what._duration
			<< " utc_offset:" << what._utc_offset << " start day:" << what._start_day << " end day:" << what._end_day;
		return os;
	}
};

//-------------------------------------------------------------------------------------------------
struct LoginParameters
{
	LoginParameters() = default;

	LoginParameters(unsigned login_retry_interval, unsigned login_retries,
		const default_appl_ver_id& davi, unsigned connect_timeout, bool reset_seqnum=false,
		bool always_seqnum_assign=false, bool silent_disconnect=false, bool no_chksum_flag=false,
		bool permissive_mode_flag=false, bool reliable=false, bool enforce_compids=true,
		unsigned recv_buf_sz=0, unsigned send_buf_sz=0, unsigned hb_int=defaults::hb_interval,
		const Schedule& sch=Schedule(), const Clients& clients=Clients(), const f8String& pem_path=f8String()) :
			_login_retry_interval(login_retry_interval), _login_retries(login_retries), _connect_timeout(connect_timeout),
			_reset_sequence_numbers(reset_seqnum), _always_seqnum_assign(always_seqnum_assign),
			_silent_disconnect(silent_disconnect), _no_chksum_flag(no_chksum_flag),
			_permissive_mode_flag(permissive_mode_flag), _reliable(reliable), _enforce_compids(enforce_compids),
			_davi(davi), _recv_buf_sz(recv_buf_sz), _send_buf_sz(send_buf_sz), _hb_int(hb_int),
			_login_schedule(sch), _clients(clients), _pem_path(pem_path) {}

	LoginParameters(const LoginParameters& from)
		: _login_retry_interval(from._login_retry_interval), _login_retries(from._login_retries),
		_connect_timeout(from._connect_timeout), _reset_sequence_numbers(from._reset_sequence_numbers),
		_always_seqnum_assign(from._always_seqnum_assign), _silent_disconnect(from._silent_disconnect),
		_no_chksum_flag(from._no_chksum_flag), _permissive_mode_flag(from._permissive_mode_flag),
		_reliable(from._reliable), _enforce_compids(from._enforce_compids), _davi(from._davi),
		_recv_buf_sz(from._recv_buf_sz), _send_buf_sz(from._send_buf_sz), _hb_int(from._hb_int),
		_login_schedule(from._login_schedule), _clients(from._clients), _pem_path(from._pem_path)
	{}

	LoginParameters& operator=(const LoginParameters& that)
	{
		if (this != &that)
		{
			_login_retry_interval = that._login_retry_interval;
			_login_retries = that._login_retries;
			_connect_timeout = that._connect_timeout;
			_reset_sequence_numbers = that._reset_sequence_numbers;
			_always_seqnum_assign = that._always_seqnum_assign;
			_silent_disconnect = that._silent_disconnect;
			_no_chksum_flag = that._no_chksum_flag;
			_permissive_mode_flag = that._permissive_mode_flag;
			_reliable = that._reliable;
			_enforce_compids = that._enforce_compids;
			_davi = that._davi;
			_recv_buf_sz = that._recv_buf_sz;
			_send_buf_sz = that._send_buf_sz;
			_hb_int = that._hb_int;
			_login_schedule = that._login_schedule;
			_clients = that._clients;
			_pem_path = that._pem_path;
		}
		return *this;
	}

	unsigned _login_retry_interval = defaults::retry_interval, _login_retries = defaults::login_retries,
				_connect_timeout = defaults::connect_timeout;
	bool _reset_sequence_numbers = false, _always_seqnum_assign, _silent_disconnect = false, _no_chksum_flag = false,
		  _permissive_mode_flag = false, _reliable = false, _enforce_compids=true;
	default_appl_ver_id _davi;
	unsigned _recv_buf_sz = 0, _send_buf_sz = 0, _hb_int = defaults::hb_interval;
	Schedule _login_schedule;
	Clients _clients;
	f8String _pem_path;
};

//-------------------------------------------------------------------------------------------------
struct Session_Schedule
{
	Schedule _sch;
	int _reject_reason;
	const std::string _reject_text;

	Session_Schedule(Schedule& sch, int reject_reason=0, const std::string& reject_text="Business messages are not accepted now.") :
		_sch(sch), _reject_reason(reject_reason), _reject_text(reject_text)
	{
	}

	/*! Inserter friend.
	    \param os stream to send to
	    \param what Session_Schedule reference
	    \return stream */
	friend std::ostream& operator<<(std::ostream& os, const Session_Schedule& what)
	{
		os << what._sch << " reject_reason:" << what._reject_reason << " reject_text:" << what._reject_text;
		return os;
	}
};

//-------------------------------------------------------------------------------------------------
class Persister;
class Logger;
class Connection;

//-------------------------------------------------------------------------------------------------
/// Fix8 Base Session. User sessions are derived from this class.
class Session
{
	/*! Initialise atomic members.
	  \param st the initial session state */
	void atomic_init(States::SessionStates st);

public:
	enum SessionControl { shutdown, print, printnohb, debug, count };

	using Control = ebitset_r<SessionControl>;

protected:
	Control _control;
	f8_atomic<unsigned> _next_send_seq, _next_receive_seq;
	f8_atomic<States::SessionStates> _state;
	f8_atomic<bool> _active;
	Tickval _last_sent, _last_received;
	const F8MetaCntx& _ctx;
	sender_comp_id _sci; // used by acceptor
	Connection *_connection;
	unsigned _req_next_send_seq, _req_next_receive_seq;
	SessionID _sid;
	struct SessionConfig *_sf;

	LoginParameters _loginParameters;

	f8_spin_lock _per_spl;
	Persister *_persist;
	Logger *_logger, *_plogger;

	Timer<Session> _timer;
	TimerEvent<Session> _hb_processor, _session_scheduler;
	std::string _batchmsgs_buffer;
	Session_Schedule *_schedule;

	/// string representation of Sessionstates
	F8API static const std::vector<f8String> _state_names;

	/// Heartbeat generation service thread method.
	F8API bool heartbeat_service();

	/// Session start/stop service thread method.
	F8API bool activation_service();

	/*! Logon callback.
	    \param seqnum message sequence number
	    \param msg Message
	    \return true on success */
	F8API virtual bool handle_logon(const unsigned seqnum, const Message *msg);

	/*! Generate a logon message.
	    \param heartbeat_interval heartbeat interval
	    \param davi default appl version id (FIXT)
	    \return new Message */
	F8API virtual Message *generate_logon(const unsigned heartbeat_interval, const f8String davi=f8String());

	/*! Logout callback.
	    \param seqnum message sequence number
	    \param msg Message
	    \return true on success */
	F8API virtual bool handle_logout(const unsigned seqnum, const Message *msg);

	/*! Heartbeat callback.
	    \param seqnum message sequence number
	    \param msg Message
	    \return true on success */
	F8API virtual bool handle_heartbeat(const unsigned seqnum, const Message *msg);

	/*! Resend request callback.
	    \param seqnum message sequence number
	    \param msg Message
	    \return true on success */
	F8API virtual bool handle_resend_request(const unsigned seqnum, const Message *msg);

	/*! Sequence reset callback.
	    \param seqnum message sequence number
	    \param msg Message
	    \return true on success */
	F8API virtual bool handle_sequence_reset(const unsigned seqnum, const Message *msg);

	/*! Test request callback.
	    \param seqnum message sequence number
	    \param msg Message
	    \return true on success */
	F8API virtual bool handle_test_request(const unsigned seqnum, const Message *msg);

	/*! Reject callback.
	    \param seqnum message sequence number
	    \param msg Message
	    \return true on success */
	virtual bool handle_reject(const unsigned seqnum, const Message *msg) { return false; }

	/*! Administrative message callback. Called on receipt of all admin messages.
	    \param seqnum message sequence number
	    \param msg Message
	    \return true on success */
	virtual bool handle_admin(const unsigned seqnum, const Message *msg) { return true; }

	/*! Application message callback. Called on receipt of all non-admin messages. You must implement this method.
	  The message is passed as a reference to a pointer. Your application can detach and take ownership. If you want
	  to take ownership, take a copy of the pointer and then set msg to 0. See Session::detach()
	    \param seqnum message sequence number
	    \param msg reference to Message ptr
	    \return true on success */
	virtual bool handle_application(const unsigned seqnum, const Message *&msg) = 0;

	/*! This method id called whenever a session state change occurs
	    \param before previous session state
	    \param after new session state */
	virtual void state_change(const States::SessionStates before, const States::SessionStates after) {}

	/*! Permit modification of message just prior to sending.
	     \param msg Message */
	virtual void modify_outbound(Message *msg) {}

	/*! Call user defined authentication with logon message.
	    \param id Session id of inbound connection
	    \param msg Message
	    \return true on success */
	virtual bool authenticate(SessionID& id, const Message *msg) { return true; }

	/// Recover next expected and next to send sequence numbers from persitence layer.
	F8API virtual void recover_seqnums();

	/*! Create a new Fix message from metadata layer.
	    \param msg_type message type string
	    \return new Message */
	Message *create_msg(const f8String& msg_type) const
	{
		const BaseMsgEntry *bme(_ctx._bme.find_ptr(msg_type.c_str()));
		if (!bme)
			throw InvalidMetadata<f8String>(msg_type);
		return bme->_create._do(true);
	}

#if (FIX8_THREAD_SYSTEM == FIX8_THREAD_PTHREAD) && !defined _MSC_VER && defined _GNU_SOURCE && defined __linux__
	/*! Get a string representing the current thread policy for the given thread
	  e.g. SCHED_OTHER, SCHED_RR, SCHED_FIFO
	    \param id thread id
	    \return string */
	static f8String get_thread_policy_string(thread_id_t id);
#endif

	/*! Set the scheduling policy for the current thread
	    \param priority scheduler priority */
	void set_scheduler(int priority);

	/*! Set the CPU affinity mask for the given thread
	    \param core_id core to mask on */
	void set_affinity(int core_id);

public:
	/*! Ctor. Initiator.
	    \param ctx reference to generated metadata
	    \param sid sessionid of connecting session
		 \param persist persister for this session
		 \param logger logger for this session
		 \param plogger protocol logger for this session */
	F8API Session(const F8MetaCntx& ctx, const SessionID& sid, Persister *persist=nullptr,
		Logger *logger=nullptr, Logger *plogger=nullptr);

	/*! Ctor. Acceptor.
	    \param ctx reference to generated metadata
	    \param sci sender comp id of hosting session
		 \param persist persister for this session
		 \param logger logger for this session
		 \param plogger protocol logger for this session */
	F8API Session(const F8MetaCntx& ctx, const sender_comp_id& sci=sender_comp_id(), Persister *persist=nullptr,
		Logger *logger=nullptr, Logger *plogger=nullptr);

	/// Dtor.
	F8API virtual ~Session();

	/*! Start the session.
	    \param connection established connection
	    \param wait if true, thread will wait till session ends before returning
	    \param send_seqnum if supplied, override the send login sequence number, set next send to seqnum+1
	    \param recv_seqnum if supplied, override the receive login sequence number, set next recv to seqnum+1
	    \param davi default appl version id (FIXT)
	    \return -1 on error, 0 on success */
	F8API int start(Connection *connection, bool wait=true, const unsigned send_seqnum=0,
		const unsigned recv_seqnum=0, const f8String davi=f8String());

  /*! Clear reference to connection.  Called by ~Connection() to clear reference.
      \param connection being deleted */
	void clear_connection(const Connection *connection)
	{
		if (connection == _connection)
			_connection = nullptr;
	}

	/*! Process inbound messages. Called by connection object.
	    \param from raw fix message
	    \return true on success */
	F8API virtual bool process(const f8String& from);

	/// Provides context to your retrans handler.
	struct RetransmissionContext
	{
		const unsigned _begin, _end, _interrupted_seqnum;
		unsigned _last;
		bool _no_more_records;

		RetransmissionContext(const unsigned begin, const unsigned end, const unsigned interrupted_seqnum)
			: _begin(begin), _end(end), _interrupted_seqnum(interrupted_seqnum), _last(), _no_more_records() {}

		friend std::ostream& operator<<(std::ostream& os, const RetransmissionContext& what)
		{
			os << "end:" << what._end << " last:" << what._last << " interrupted seqnum:"
				<< what._interrupted_seqnum << " no_more_records:" << std::boolalpha << what._no_more_records;
			return os;
		}
	};

	using SequencePair = std::pair<const unsigned, const f8String>;

	/*! Retransmission callback. Called by framework with each message to be resent.
	    \param with pair of sequence number and raw fix message
	    \param rctx retransmission context
	    \return true on success */
	F8API virtual bool retrans_callback(const SequencePair& with, RetransmissionContext& rctx);

	/*! Send message.
	    \param msg Message
	    \param destroy if true, destroy message after send
	    \param custom_seqnum override sequence number with this value
	    \param no_increment if true, don't increment the seqnum after sending
	    \return true on success */
	F8API virtual bool send(Message *msg, bool destroy=true, const unsigned custom_seqnum=0, const bool no_increment=false);

	/*! Send message - non-pipelined version.
		 WARNING: be sure you don't inadvertently use this method. Symptoms will be out of sequence messages (seqnum==1)
		 and core dumping.
	    \param msg Message
	    \param custom_seqnum override sequence number with this value
	    \param no_increment if true, don't increment the seqnum after sending
	    \return true on success */
	F8API virtual bool send(Message& msg, const unsigned custom_seqnum=0, const bool no_increment=false);

	/*! Send a batch of messages. During this call HB and test requests are suspended.
	    \param msgs vector of Message ptrs
	    \param destroy if true, destroy message after send
	    \return size_t number of messages sent - if destroy was true those sent messages will have been destroyed
	 			with the reamining messages in the vector still allocated */
	F8API virtual size_t send_batch(const std::vector<Message *>& msgs, bool destroy=true);

	/*! Process message (encode) and send.
	    \param msg Message
	    \return true on success */
	F8API bool send_process(Message *msg);

	/*! Modify the header if desired. Called when message is sent.
	    \param msg Message
	    \return number of fields added/modifed */
	F8API virtual int modify_header(MessageBase *msg);

	/// Force persister to sync next send/receive seqnums
	F8API void update_persist_seqnums();

	/// stop the session.
	F8API void stop();

	/*! Get the connection object.
	    \return the connection object */
	Connection *get_connection() { return _connection; }

	/*! Get the timer object.
	    \return the timer object */
	Timer<Session>& get_timer() { return _timer; }

	/*! Get the metadata context object.
	    \return the context object */
	const F8MetaCntx& get_ctx() const { return _ctx; }

	/*! Check if the given log level is set for the session logger
	    \param level level to test
	    \return true if available */
	bool is_loggable(Logger::Level level) const { return _logger ? _logger->is_loggable(level) : false; }

	/*! Log a message to the session logger. Do not check for level permission
	    \param what string to log
	    \param lev log level
		 \param fl pointer to fileline
	    \param value optional value for the logger to use
	    \return true on success */
	bool enqueue(const std::string& what, Logger::Level lev, const char *fl=nullptr, unsigned value=0) const
		{ return _logger ? _logger->enqueue(what, lev, fl, value) : false; }

	/*! Log a message to the session logger.
	    \param what string to log
	    \param lev log level
		 \param fl pointer to fileline
	    \param value optional value for the logger to use
	    \return true on success */
	bool log(const std::string& what, Logger::Level lev, const char *fl=nullptr, unsigned value=0) const
		{ return _logger ? _logger->send(what, lev, fl, value) : false; }

	/*! Log a message to the protocol logger.
	    \param what Fix message (string) to log
	    \param lev log level
	    \param direction 0=out, 1=in
	    \return true on success */
	bool plog(const std::string& what, Logger::Level lev, const unsigned direction=0) const
		{ return _plogger ? _plogger->send(what, lev, nullptr, direction) : false; }

	/*! Return the last received timstamp
	    \return Tickval on success */
	const Tickval& get_last_received() const { return _last_received; }

	/*! Return the last sent timstamp
	    \return Tickval on success */
	const Tickval& get_last_sent() const { return _last_sent; }

	/// Update the last sent time.
	void update_sent() { _last_sent.now(); }

	/// Update the last received time.
	void update_received() { _last_received.now(); }

	/*! Check that a message has the correct sender/target compid for this session. Throws BadCompidId on error.
	    \param seqnum message sequence number
	    \param msg Message
	    \param id Session id of inbound connection */
	F8API void compid_check(const unsigned seqnum, const Message *msg, const SessionID& id) const;

	/*! Check that a message is in the correct sequence for this session. Will generated resend request if required. Throws InvalidMsgSequence, MissingMandatoryField, BadSendingTime.
	    \param seqnum message sequence number
	    \param msg Message
	    \return true on success */
	F8API bool sequence_check(const unsigned seqnum, const Message *msg);

	/*! Check that the session is active for this application message
	    \param seqnum message sequence number
	    \param msg Message
	    \return true if active */
	virtual bool activation_check(const unsigned seqnum, const Message *msg) { return _active; }

	/*! Enforce session semantics. Checks compids, sequence numbers.
	    \param seqnum message sequence number
	    \param msg Message
	    \return true if message FAILS enforce rules */
	F8API bool enforce(const unsigned seqnum, const Message *msg);

	/*! Get the session id for this session.
	    \return the session id */
	const SessionID& get_sid() const { return _sid; }

	/*! Get the next sender sequence number
	    \return next sender sequence number */
	unsigned get_next_send_seq() const { return _next_send_seq; }

	/*! Set the LoginParameters
	    \param loginParamaters to populate from */
	void set_login_parameters(const LoginParameters& loginParamaters) { _loginParameters = loginParamaters; }

	/*! Get the LoginParameters
	    \param loginParamaters to populate */
	void get_login_parameters(LoginParameters& loginParamaters) const { loginParamaters = _loginParameters; }

	/*! Get the LoginParameters
	    \return loginParamaters */
	const LoginParameters& get_login_parameters() const { return  _loginParameters; }

	/*! Set the persister.
	    \param pst pointer to persister object  */
	void set_persister(Persister *pst) { _persist = pst; }

	/*! Get the control object for this session.
	    \return the control object */
	Control& control() { return _control; }

	/*! See if this session is being shutdown.
	    \return true if shutdown is underway */
	bool is_shutdown() { return _control.has(shutdown) || _state == States::st_session_terminated; }

	/* ! Set the SessionConfig object - only for server sessions
		\param sf pointer to SessionConfig object */
	void set_session_config(struct SessionConfig *sf) { _sf = sf; }

	/*! Generate a logout message.
	    \return new Message */
	F8API virtual Message *generate_logout(const char *msgstr=nullptr);

	/*! Generate a heartbeat message.
	    \param testReqID test request id
	    \return new Message */
	F8API virtual Message *generate_heartbeat(const f8String& testReqID);

	/*! Generate a resend request message.
	    \param begin begin sequence number
	    \param end sequence number
	    \return new Message */
	F8API virtual Message *generate_resend_request(const unsigned begin, const unsigned end=0);

	/*! Generate a sequence reset message.
	    \param newseqnum new sequence number
	    \param gapfillflag gap fill flag
	    \return new Message */
	F8API virtual Message *generate_sequence_reset(const unsigned newseqnum, const bool gapfillflag=false);

	/*! Generate a test request message.
	    \param testReqID test request id
	    \return new Message */
	F8API virtual Message *generate_test_request(const f8String& testReqID);

	/*! Generate a reject message.
	    \param seqnum message sequence number
	    \param what rejection text
	    \param msgtype offending msgtype
	    \return new Message */
	F8API virtual Message *generate_reject(const unsigned seqnum, const char *what, const char *msgtype=nullptr);

	/*! Generate a business_reject message.
	    \param seqnum message sequence number
	    \param msg source message
	    \param reason rejection reason code
	    \param what rejection text
	    \return new Message */
	F8API virtual Message *generate_business_reject(const unsigned seqnum, const Message *msg, const int reason, const char *what);

	/*! Call the virtual state_change method with before and after, then set the new state
	    \param new_state new session state to set */
	void do_state_change(const States::SessionStates new_state)
	{
		const States::SessionStates old_state(_state.exchange(new_state));
		if (old_state != new_state)
			state_change(old_state, new_state);
	}

	/*! Get the current session state enumeration
	    \return States::SessionStates value */
	States::SessionStates get_session_state() const { return _state; }

	/*! Detach message passed to handle_application. Will set source to 0;
	    Not thread safe and should never be called across threads. It should
		 only be called from Session::handle_application().
	    \param msg ref to ptr containing message
	    \return detached Message * */
	static const Message *detach(const Message *&msg)
	{
		const Message *tmp(msg);
		msg = 0;
		return tmp;
	}

	/*! Find the string representation for the given session state
	    \param state session state
	    \return string found or "unknown" */
	static const f8String& get_session_state_string(const States::SessionStates state)
	{
		static const f8String unknown("Unknown");
		return state < _state_names.size() ? _state_names[state] : unknown;
	}

	/*! Return the version and copyright for this version
	    \return string */
	F8API static const f8String copyright_string();
};

//-------------------------------------------------------------------------------------------------
// our buffered RAII ostream log target, ostream Session log target for specified Session ptr
#define ssout_info(x) if (!x->is_loggable(FIX8::Logger::Info)); \
    else FIX8::log_stream(FIX8::logger_function(std::bind(&FIX8::Session::enqueue, x, std::placeholders::_1, FIX8::Logger::Info, FILE_LINE, std::placeholders::_2)))
#define ssout_warn(x) if (!x->is_loggable(FIX8::Logger::Warn)); \
    else FIX8::log_stream(FIX8::logger_function(std::bind(&FIX8::Session::enqueue, x, std::placeholders::_1, FIX8::Logger::Warn, FILE_LINE, std::placeholders::_2)))
#define ssout_error(x) if (!x->is_loggable(FIX8::Logger::Error)); \
    else FIX8::log_stream(FIX8::logger_function(std::bind(&FIX8::Session::enqueue, x, std::placeholders::_1, FIX8::Logger::Error, FILE_LINE, std::placeholders::_2)))
#define ssout_fatal(x) if (!x->is_loggable(FIX8::Logger::Fatal)); \
    else FIX8::log_stream(FIX8::logger_function(std::bind(&FIX8::Session::enqueue, x, std::placeholders::_1, FIX8::Logger::Fatal, FILE_LINE, std::placeholders::_2)))
#if defined FIX8_DEBUG
#define ssout_debug(x) if (!x->is_loggable(Logger::Debug)); \
    else FIX8::log_stream(FIX8::logger_function(std::bind(&FIX8::Session::enqueue, x, std::placeholders::_1, FIX8::Logger::Debug, FILE_LINE, std::placeholders::_2)))
#else
#define ssout_debug(x) true ? null_insert() : null_insert()
#endif

#define ssout(x) ssout_info(x)
#define slout ssout(this)
#define slout_info ssout_info(this)
#define slout_warn ssout_warn(this)
#define slout_error ssout_error(this)
#define slout_fatal ssout_fatal(this)
#define slout_debug ssout_debug(this)

//-------------------------------------------------------------------------------------------------

} // FIX8

#endif // FIX8_SESSION_HPP_
