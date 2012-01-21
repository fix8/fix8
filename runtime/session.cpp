//-----------------------------------------------------------------------------------------
#if 0

Fix8 is released under the New BSD License.

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
//-----------------------------------------------------------------------------------------
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <map>
#include <set>
#include <iterator>
#include <memory>
#include <iomanip>
#include <algorithm>
#include <numeric>
#include <bitset>

#include <strings.h>
#include <regex.h>

#include <f8includes.hpp>

//-------------------------------------------------------------------------------------------------
using namespace FIX8;
using namespace std;

//-------------------------------------------------------------------------------------------------
RegExp SessionID::_sid("([^:]+):([^-]+)->(.+)");
RegExp Session::_seq("34=([^\x01]+)\x01");

//-------------------------------------------------------------------------------------------------
template<>
const Session::Handlers::TypePair Session::Handlers::_valueTable[] =
{
	Session::Handlers::TypePair(Common_MsgType_HEARTBEAT, &Session::handle_heartbeat),
	Session::Handlers::TypePair(Common_MsgType_TEST_REQUEST, &Session::handle_test_request),
	Session::Handlers::TypePair(Common_MsgType_RESEND_REQUEST, &Session::handle_resend_request),
	Session::Handlers::TypePair(Common_MsgType_REJECT, &Session::handle_reject),
	Session::Handlers::TypePair(Common_MsgType_SEQUENCE_RESET, &Session::handle_sequence_reset),
	Session::Handlers::TypePair(Common_MsgType_LOGOUT, &Session::handle_logout),
	Session::Handlers::TypePair(Common_MsgType_LOGON, &Session::handle_logon),
};
template<>
const Session::Handlers::NotFoundType Session::Handlers::_noval(&Session::handle_application);

template<>
const Session::Handlers::TypeMap Session::Handlers::_valuemap(Session::Handlers::_valueTable,
	Session::Handlers::get_table_end());

//-------------------------------------------------------------------------------------------------
const f8String& SessionID::make_id()
{
	f8ostrstream ostr;
	ostr << _beginString << ':' << _senderCompID << "->" << _targetCompID;
	return _id = ostr.str();
}

//-------------------------------------------------------------------------------------------------
void SessionID::from_string(const f8String& from)
{
	RegMatch match;
	if (_sid.SearchString(match, from, 4) == 4)
	{
		f8String bstr, scid, tcid;
		_sid.SubExpr(match, from, bstr, 0, 1);
		_beginString.set(bstr);
		_sid.SubExpr(match, from, scid, 0, 2);
		_senderCompID.set(scid);
		_sid.SubExpr(match, from, tcid, 0, 3);
		_targetCompID.set(tcid);
		make_id();
	}
}

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
Session::Session(const F8MetaCntx& ctx, const SessionID& sid, Persister *persist, Logger *logger, Logger *plogger) :
	_ctx(ctx), _connection(), _sid(sid), _persist(persist), _logger(logger), _plogger(plogger),	// initiator
	_timer(*this, 10), _hb_processor(&Session::heartbeat_service)
{
	atomic_init(States::st_not_logged_in);
	_timer.start();
}

//-------------------------------------------------------------------------------------------------
Session::Session(const F8MetaCntx& ctx, Persister *persist, Logger *logger, Logger *plogger) :
	_ctx(ctx), _connection(), _persist(persist), _logger(logger), _plogger(plogger),	// acceptor
	_timer(*this, 10), _hb_processor(&Session::heartbeat_service)
{
	atomic_init(States::st_wait_for_logon);
	_timer.start();
}

//-------------------------------------------------------------------------------------------------
void Session::atomic_init(States::SessionStates st)
{
	_state = st;
	_next_sender_seq = _next_target_seq = 1;
	_last_sent = new Tickval(true);
	_last_received = new Tickval(true);
}

//-------------------------------------------------------------------------------------------------
Session::~Session()
{
	if (_logger)
	{
		log("Session terminating");
		_logger->stop();
	}

	delete _last_sent;
	delete _last_received;
}

//-------------------------------------------------------------------------------------------------
int Session::start(Connection *connection, bool wait)
{
	log("Starting session");
	_connection = connection; // takes owership
	if (!_connection->connect()) // if already connected returns true
		return -1;;
	_connection->start();
	log("Session connected");

	if (_connection->get_role() == Connection::cn_initiator)
	{
		recover_seqnums();

		Message *msg(generate_logon(_connection->get_hb_interval()));
		send(msg);
		_state = States::st_logon_sent;
	}

	if (wait)	// wait for
	{
		_connection->join();
	}

	return 0;
}

//-------------------------------------------------------------------------------------------------
void Session::stop()
{
	_control |= shutdown;
	_timer.schedule(_hb_processor, 0);
	if (_persist)
		_persist->stop();
	_connection->stop();
}

//-------------------------------------------------------------------------------------------------
bool Session::enforce(const unsigned seqnum, const Message *msg)
{
	if (States::is_established(_state))
	{
		compid_check(seqnum, msg);
		gap_fill_flag gff;
		bool ignore(msg->get_msgtype() == Common_MsgType_SEQUENCE_RESET && msg->Header()->get(gff) && gff());
		if (!ignore || !sequence_check(seqnum, msg))
			return false;
	}

	return true;
}

//-------------------------------------------------------------------------------------------------
bool Session::process(const f8String& from)
{
	RegMatch match;
	if (_seq.SearchString(match, from, 2, 0) != 2)
		throw InvalidMessage(from);
	f8String seqstr;
	_seq.SubExpr(match, from, seqstr, 0, 1);
	const unsigned seqnum(GetValue<unsigned>(seqstr));

	try
	{
		plog(from);
		scoped_ptr<Message> msg(Message::factory(_ctx, from
#if defined PERMIT_CUSTOM_FIELDS
			, this, &Session::post_msg_ctor
#endif
		));
		if (_control & print)
			cout << *msg << endl;
		bool result((msg->is_admin() ? handle_admin(seqnum, msg.get()) : true)
			&& (this->*_handlers.find_ref(msg->get_msgtype()))(seqnum, msg.get()));
		_next_target_seq = seqnum + 1;
		return result;

	}
	catch (f8Exception& e)
	{
		Message *msg(generate_reject(seqnum, e.what()));
		send(msg);
		log(e.what());
		if (e.force_logoff())
		{
			send(generate_logout());
			_state = States::st_logoff_sent;
			stop();
		}
	}
	catch (exception& e)	// also catches Poco::Net::NetException
	{
		log(e.what());
	}

	return false;
}

//-------------------------------------------------------------------------------------------------
void Session::compid_check(const unsigned seqnum, const Message *msg)
{
	sender_comp_id sci;
	msg->Header()->get(sci);
	target_comp_id tci;
	msg->Header()->get(tci);
	if (!_sid.same_sender_comp_id(tci))
		throw BadCompidId(sci());
	if (!_sid.same_target_comp_id(sci))
		throw BadCompidId(tci());
}

//-------------------------------------------------------------------------------------------------
bool Session::sequence_check(const unsigned seqnum, const Message *msg)
{
	if (seqnum > _next_target_seq)
	{
		if (_state == States::st_resend_request_sent)
			log("Resend request already sent");
		else
		{
			Message *rmsg(generate_resend_request(_next_target_seq));
			_state = States::st_resend_request_sent;
			send(rmsg);
		}
		return false;
	}

	if (seqnum < _next_target_seq)
	{
		poss_dup_flag pdf(false);
		msg->Header()->get(pdf);
		if (!pdf())	// poss dup not set so bad
			throw InvalidMsgSequence(seqnum);
		orig_sending_time ost;
		if (!msg->Header()->get(ost))
			throw MissingMandatoryField(Common_OrigSendingTime);
		sending_time st;
		msg->Header()->get(st);
		if (ost() > st())
		{
			ostringstream ostr;
			ost.print(ostr);
			throw BadSendingTime(ostr.str());
		}
	}

	if (_state == States::st_resend_request_sent)
		_state = States::st_continuous;

	return true;
}

//-------------------------------------------------------------------------------------------------
bool Session::handle_logon(const unsigned seqnum, const Message *msg)
{
	enforce(seqnum, msg);

	if (_connection->get_role() == Connection::cn_initiator)
	{
		heartbeat_interval hbi;
		msg->get(hbi);
		_connection->set_hb_interval(hbi());
		_state = States::st_continuous;
	}
	else // acceptor
	{
		recover_seqnums();

		sender_comp_id sci;
		msg->Header()->get(sci);
		target_comp_id tci;
		msg->Header()->get(tci);
		SessionID id(_ctx._beginStr, tci(), sci());
		if (authenticate(id, msg))
		{
			_sid = id;
			Message *msg(generate_logon(_connection->get_hb_interval()));
			send(msg);
			_state = States::st_continuous;
		}
		else
		{
			ostringstream ostr;
			ostr << id << " failed authentication";
			log(ostr.str());
			stop();
			_state = States::st_session_terminated;
			return false;
		}
	}

	ostringstream ostr;
	ostr << "Heartbeat interval is " << _connection->get_hb_interval();
	log(ostr.str());

	_timer.schedule(_hb_processor, 1000);	// check every second

	return true;
}

//-------------------------------------------------------------------------------------------------
bool Session::handle_logout(const unsigned seqnum, const Message *msg)
{
	enforce(seqnum, msg);

	log("peer has logged out");
	stop();
	return true;
}

//-------------------------------------------------------------------------------------------------
bool Session::handle_sequence_reset(const unsigned seqnum, const Message *msg)
{
	enforce(seqnum, msg);

	new_seq_num nsn;
	if (msg->Header()->get(nsn))
	{
		if (nsn() > static_cast<int>(_next_target_seq))
			_next_target_seq = nsn();
		else if (nsn() < static_cast<int>(_next_target_seq))
			throw InvalidMsgSequence(nsn());
	}

	return true;
}

//-------------------------------------------------------------------------------------------------
bool Session::handle_resend_request(const unsigned seqnum, const Message *msg)
{
	enforce(seqnum, msg);

	begin_seq_num begin;
	msg->get(begin);
	end_seq_num end;
	msg->get(end);

	if (!_persist)
		send(generate_sequence_reset(_next_sender_seq + 1, true));
	else if ((begin() > end() && end()) || begin() == 0)
		send(generate_reject(seqnum, "Invalid begin or end resend seqnum"));

	send(generate_sequence_reset(_next_sender_seq + 1, true));
	return true;
}

//-------------------------------------------------------------------------------------------------
bool Session::retrans_callback(const SequencePair& with)
{
	return send(Message::factory(_ctx, with.second
#if defined PERMIT_CUSTOM_FIELDS
		, this, &Session::post_msg_ctor
#endif
		));
}

//-------------------------------------------------------------------------------------------------
bool Session::handle_test_request(const unsigned seqnum, const Message *msg)
{
	enforce(seqnum, msg);

	test_request_id testReqID;
	msg->get(testReqID);
	Message *trmsg(generate_heartbeat(testReqID()));
	send(trmsg);
	return true;
}

//-------------------------------------------------------------------------------------------------
bool Session::heartbeat_service()
{
	Tickval now(true);
	if ((now - *_last_sent).secs() >= _connection->get_hb_interval())
	{
		const f8String testReqID;
		Message *msg(generate_heartbeat(testReqID));
		//if (_connection->get_role() == Connection::cn_initiator)
			send(msg);
	}

	now.now();
	if ((now - *_last_received).secs() > _connection->get_hb_interval20pc())
	{
		cout << _connection->get_hb_interval20pc() << endl;
		if (_state == States::st_test_request_sent)	// already sent
		{
			send(generate_logout());
			_state = States::st_logoff_sent;
			stop();
			return true;
		}
		else
		{
			const f8String testReqID("TEST");
			send(generate_test_request(testReqID));
			_state = States::st_test_request_sent;
		}
	}

	_timer.schedule(_hb_processor, 1000);

	return true;
}

//-------------------------------------------------------------------------------------------------
bool Session::handle_heartbeat(const unsigned seqnum, const Message *msg)
{
	enforce(seqnum, msg);

	if (_state == States::st_test_request_sent)
		_state = States::st_continuous;
	return true;
}

//-------------------------------------------------------------------------------------------------
Message *Session::generate_heartbeat(const f8String& testReqID)
{
	Message *msg(create_msg(Common_MsgType_HEARTBEAT));
	if (!testReqID.empty())
		*msg += new test_request_id(testReqID);

	return msg;
}

//-------------------------------------------------------------------------------------------------
Message *Session::generate_reject(const unsigned seqnum, const char *what)
{
	Message *msg(create_msg(Common_MsgType_REJECT));
	*msg += new ref_seq_num(seqnum);
	if (what)
		*msg += new text(what);
	return msg;
}

//-------------------------------------------------------------------------------------------------
Message *Session::generate_test_request(const f8String& testReqID)
{
	Message *msg(create_msg(Common_MsgType_TEST_REQUEST));
	*msg += new test_request_id(testReqID);

	return msg;
}

//-------------------------------------------------------------------------------------------------
Message *Session::generate_logon(const unsigned heartbtint)
{
	Message *msg(create_msg(Common_MsgType_LOGON));
	*msg += new heartbeat_interval(heartbtint);
	*msg += new encrypt_method(0); // FIXME

	return msg;
}

//-------------------------------------------------------------------------------------------------
Message *Session::generate_logout()
{
	Message *msg(create_msg(Common_MsgType_LOGOUT));
	return msg;
}

//-------------------------------------------------------------------------------------------------
Message *Session::generate_resend_request(const unsigned begin, const unsigned end)
{
	Message *msg(create_msg(Common_MsgType_RESEND_REQUEST));
	*msg += new begin_seq_num(begin);
	*msg += new end_seq_num(end);

	return msg;
}

//-------------------------------------------------------------------------------------------------
Message *Session::generate_sequence_reset(const unsigned newseqnum, const bool gapfillflag)
{
	Message *msg(create_msg(Common_MsgType_SEQUENCE_RESET));
	*msg += new new_seq_num(newseqnum);

	if (gapfillflag)
		*msg += new gap_fill_flag(true);

	return msg;
}

//-------------------------------------------------------------------------------------------------
bool Session::send(Message *tosend)
{
	return _connection->write(tosend);
}

#if defined MSGRECYCLING
bool Session::send_wait(Message *msg, const int waitval)
{
	if (send(msg))
	{
		while(msg->get_in_use())
			microsleep(waitval);
		return true;
	}
	return false;
}

#endif

//-------------------------------------------------------------------------------------------------
bool Session::send_process(Message *msg) // called from the connection thread
{
	*msg->Header() += new sender_comp_id(_sid.get_senderCompID());
	*msg->Header() += new target_comp_id(_sid.get_targetCompID());
	if (msg->Header()->have(Common_MsgSeqNum))
	{
		*msg->Header() += new poss_dup_flag(true);
		sending_time sendtime;
		msg->Header()->get(sendtime);
		*msg->Header() += new orig_sending_time(sendtime());
	}
	else
		*msg->Header() += new msg_seq_num(_next_sender_seq);
	*msg->Header() += new sending_time;

	try
	{
		modify_outbound(msg);
		f8String output;
		const unsigned enclen(msg->encode(output));
		if (!_connection->send(output))
		{
			ostringstream ostr;
			ostr << "Message write failed: " << enclen;
			log(ostr.str());
			return false;
		}
		_last_sent->now();
		plog(output);

		if (_persist)
		{
			if (!msg->is_admin())
				_persist->put(_next_sender_seq, output);
			_persist->put(_next_sender_seq + 1, _next_target_seq);
			//cout << (_next_sender_seq + 1) << ' ' << _next_target_seq << endl;
		}
		++_next_sender_seq;
	}
	catch (f8Exception& e)
	{
		log(e.what());
	}

	return true;
}

//-------------------------------------------------------------------------------------------------
bool Session::handle_application(const unsigned seqnum, const Message *msg)
{
	return false;
}

//-------------------------------------------------------------------------------------------------
void Session::recover_seqnums()
{
	if (_persist)
	{
		unsigned sender_seqnum, target_seqnum;
		if (_persist->get(sender_seqnum, target_seqnum))
		{
			ostringstream ostr;
			ostr << "Last sent: " << sender_seqnum << ", last received: " << target_seqnum;
			log(ostr.str());
			_next_sender_seq = sender_seqnum;
			_next_target_seq = target_seqnum;
		}
	}
}

