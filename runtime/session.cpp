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
const Tickval::ticks Tickval::noticks;
const Tickval::ticks Tickval::thousand;
const Tickval::ticks Tickval::million;
const Tickval::ticks Tickval::billion;

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
	_ctx(ctx), _connection(), _req_next_send_seq(), _req_next_receive_seq(),
	_sid(sid), _login_retry_interval(default_retry_interval), _login_retries(default_login_retries),
	_reset_sequence_numbers(), _persist(persist), _logger(logger), _plogger(plogger),	// initiator
	_timer(*this, 10), _hb_processor(&Session::heartbeat_service)
{
	_timer.start();
}

//-------------------------------------------------------------------------------------------------
Session::Session(const F8MetaCntx& ctx, Persister *persist, Logger *logger, Logger *plogger) :
	_ctx(ctx), _connection(), _req_next_send_seq(), _req_next_receive_seq(),
	_login_retry_interval(default_retry_interval), _login_retries(default_login_retries),
	_reset_sequence_numbers(), _persist(persist), _logger(logger), _plogger(plogger),	// acceptor
	_timer(*this, 10), _hb_processor(&Session::heartbeat_service)
{
	_timer.start();
}

//-------------------------------------------------------------------------------------------------
void Session::atomic_init(States::SessionStates st)
{
	_state = st;
	_next_send_seq = _next_receive_seq = 1;
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
int Session::start(Connection *connection, bool wait, const unsigned send_seqnum, const unsigned recv_seqnum)
{
	if (_logger)
		_logger->purge_thread_codes();
	if (_plogger)
		_plogger->purge_thread_codes();
	_control.clear(shutdown);
	log("Starting session");
	_connection = connection; // takes owership
	if (!_connection->connect()) // if already connected returns true
		return -1;;
	_connection->start();
	log("Session connected");

	if (_connection->get_role() == Connection::cn_initiator)
	{
		atomic_init(States::st_not_logged_in);
		if (_reset_sequence_numbers)
			_next_send_seq = _next_receive_seq = 1;
		else
		{
			recover_seqnums();
			if (send_seqnum)
				_next_send_seq = send_seqnum;
			if (recv_seqnum)
				_next_receive_seq = recv_seqnum;
		}

		Message *msg(generate_logon(_connection->get_hb_interval()));
		send(msg);
		_state = States::st_logon_sent;
	}
	else
	{
		atomic_init(States::st_wait_for_logon);
		if (send_seqnum)
			_req_next_send_seq = send_seqnum;
		if (recv_seqnum)
			_req_next_receive_seq = recv_seqnum;
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
	millisleep(250); // let the thread shutdowns finish
}

//-------------------------------------------------------------------------------------------------
bool Session::enforce(const unsigned seqnum, const Message *msg)
{
	if (States::is_established(_state))
	{
		if (_state != States::st_logon_received)
			compid_check(seqnum, msg);
		gap_fill_flag gff;
		if (msg->get_msgtype() != Common_MsgType_SEQUENCE_RESET && sequence_check(seqnum, msg))
			return false;
	}

	return true;
}

//-------------------------------------------------------------------------------------------------
bool Session::process(const f8String& from)
{
	unsigned seqnum(0);

	try
	{
		RegMatch match;
		if (_seq.SearchString(match, from, 2, 0) != 2)
			throw InvalidMessage(from);
		f8String seqstr;
		_seq.SubExpr(match, from, seqstr, 0, 1);
		seqnum = GetValue<unsigned>(seqstr);

		plog(from, 1);
		scoped_ptr<Message> msg(Message::factory(_ctx, from
#if defined PERMIT_CUSTOM_FIELDS
			, this, &Session::post_msg_ctor
#endif
		));
		if (_control & print)
			cout << *msg << endl;
		bool result((msg->is_admin() ? handle_admin(seqnum, msg.get()) : true)
			&& (this->*_handlers.find_ref(msg->get_msgtype()))(seqnum, msg.get()));
		++_next_receive_seq;
		if (_persist)
		{
			_persist->put(_next_send_seq, _next_receive_seq);
			//cout << "Persisted:" << _next_send_seq << " and " << _next_receive_seq << endl;
		}
		return result;

	}
	catch (f8Exception& e)
	{
		//cout << "process:: f8exception" << ' ' << seqnum << ' ' << e.what() << endl;

		log(e.what());
		if (!e.force_logoff())
		{
			Message *msg(generate_reject(seqnum, e.what()));
			send(msg);
		}
		else
		{
			if (_state != States::st_logon_received)
			{
				send(generate_logout(), 0, true); // so it won't increment
				_state = States::st_logoff_sent;
			}
			stop();
		}
	}
	catch (exception& e)	// also catches Poco::Net::NetException
	{
		//cout << "process:: std::exception" << endl;

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
	//cout << "seqnum:" << seqnum << " next_target_seq:" << _next_receive_seq
		//<< " next_sender_seq:" << _next_send_seq << " state:" << _state << " next_receive_seq:" <<  _next_receive_seq << endl;

	if (seqnum > _next_receive_seq)
	{
		//if (_state == States::st_resend_request_sent)
//			log("Resend request already sent");
		if (_state == States::st_continuous)
		{
			Message *rmsg(generate_resend_request(_next_receive_seq));
			_state = States::st_resend_request_sent;
			send(rmsg);
		}
	//	else
	//		throw InvalidMsgSequence(seqnum, _next_receive_seq);
		return false;
	}

	if (seqnum < _next_receive_seq)
	{
		poss_dup_flag pdf(false);
		msg->Header()->get(pdf);
		if (!pdf())	// poss dup not set so bad
			throw MsgSequenceTooLow(seqnum, _next_receive_seq);
		sending_time st;
		msg->Header()->get(st);
		orig_sending_time ost;
		if (msg->Header()->get(ost) && ost() > st())
		{
			ostringstream ostr;
			ost.print(ostr);
			throw BadSendingTime(ostr.str());
		}
	}

	return true;
}

//-------------------------------------------------------------------------------------------------
bool Session::handle_logon(const unsigned seqnum, const Message *msg)
{
	_state = States::st_logon_received;

	if (_connection->get_role() == Connection::cn_initiator)
	{
		enforce(seqnum, msg);
		heartbeat_interval hbi;
		msg->get(hbi);
		_connection->set_hb_interval(hbi());
		_state = States::st_continuous;
	}
	else // acceptor
	{
		if (_ctx.version() >= 4100 && msg->have(Common_ResetSeqNumFlag) && msg->get<reset_seqnum_flag>()->get())
		{
			log("Resetting sequence numbers");
			_next_send_seq = _next_receive_seq = 1;
		}
		else
		{
			recover_seqnums();
			if (_req_next_send_seq)
				_next_send_seq = _req_next_send_seq;
			if (_req_next_receive_seq)
				_next_receive_seq = _req_next_receive_seq;
		}

		sender_comp_id sci;
		msg->Header()->get(sci);
		target_comp_id tci;
		msg->Header()->get(tci);
		SessionID id(_ctx._beginStr, tci(), sci());
		if (authenticate(id, msg))
		{
			_sid = id;
			enforce(seqnum, msg);
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

	//if (_state != States::st_logoff_sent)
	//	send(generate_logout());
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
		if (nsn() > static_cast<int>(_next_send_seq))
			_next_send_seq = nsn();
		else if (nsn() < static_cast<int>(_next_send_seq))
			throw MsgSequenceTooLow(nsn(), _next_send_seq);
	}

	if (_state == States::st_resend_request_sent)
		_state = States::st_continuous;

	return true;
}

//-------------------------------------------------------------------------------------------------
bool Session::handle_resend_request(const unsigned seqnum, const Message *msg)
{
	enforce(seqnum, msg);

	if (_state != States::st_resend_request_received)
	{
		_state = States::st_resend_request_received;

		begin_seq_num begin;
		end_seq_num end;

		if (!_persist || !msg->get(begin) || !msg->get(end))
			send(generate_sequence_reset(_next_send_seq + 1, true));
		else if ((begin() > end() && end()) || begin() == 0)
			send(generate_reject(seqnum, "Invalid begin or end resend seqnum"));
		else
			_persist->get(begin(), end(), *this, &Session::retrans_callback);
	}

	return true;
}

//-------------------------------------------------------------------------------------------------
bool Session::retrans_callback(const SequencePair& with, RetransmissionContext& rctx)
{
	Message *msg(Message::factory(_ctx, with.second
#if defined PERMIT_CUSTOM_FIELDS
		, this, &Session::post_msg_ctor
#endif
		));

	//cout << "first:" << with.first << ' ' << rctx << endl;

	if (rctx._no_more_records)
	{
		if (rctx._end)
		{
			_next_send_seq = rctx._interrupted_seqnum - 1;
			send(generate_sequence_reset(rctx._interrupted_seqnum, true), rctx._last + 1);
			//cout << "#1" << endl;
		}
		else if (!rctx._last)
		{
			_next_send_seq = rctx._interrupted_seqnum;
			send(generate_sequence_reset(rctx._interrupted_seqnum, true));
			//cout << "#4" << endl;
		}
		_state = States::st_continuous;
		return true;
	}

	if (rctx._last)
	{
		if (rctx._last + 1 < with.first)
		{
			send(generate_sequence_reset(with.first, true), _next_send_seq);
			//cout << "#2" << endl;
		}
	}
	else
	{
		if (with.first > rctx._begin)
		{
			send(generate_sequence_reset(with.first, true));
			//cout << "#3" << endl;
		}
	}

	rctx._last = with.first;

	return send(msg);
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
		if (_state == States::st_test_request_sent)	// already sent
		{
			send(generate_logout(), 0, true); // so it won't increment
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
	if (_reset_sequence_numbers)
		*msg += new reset_seqnum_flag(true);

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
bool Session::send(Message *tosend, const unsigned custom_seqnum, const bool no_increment)
{
	if (custom_seqnum)
		tosend->set_custom_seqnum(custom_seqnum);
	if (no_increment)
		tosend->set_no_increment(no_increment);
	return _connection->write(tosend);
}

#if defined MSGRECYCLING
bool Session::send_wait(Message *msg, const unsigned custom_seqnum, const int waitval)
{
	if (send(msg, custom_seqnum))
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
	bool is_dup(msg->Header()->have(Common_PossDupFlag));

	if (!msg->Header()->have(Common_SenderCompID))
		*msg->Header() += new sender_comp_id(_sid.get_senderCompID());
	if (!msg->Header()->have(Common_TargetCompID))
		*msg->Header() += new target_comp_id(_sid.get_targetCompID());

	if (msg->Header()->have(Common_MsgSeqNum))
	{
		if (!is_dup)
		{
			*msg->Header() += new poss_dup_flag(true);
			is_dup = true;
		}

		sending_time sendtime;
		msg->Header()->get(sendtime);
		*msg->Header() += new orig_sending_time(sendtime());
	}
	else
		*msg->Header() += new msg_seq_num(msg->get_custom_seqnum() ? msg->get_custom_seqnum() : _next_send_seq);
	*msg->Header() += new sending_time;

	try
	{
		//cout << "Sending:" << *msg;
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

		//cout << "send_process" << endl;

		if (!is_dup)
		{
			if (_persist)
			{
				if (!msg->is_admin())
					_persist->put(_next_send_seq, output);
				_persist->put(_next_send_seq + 1, _next_receive_seq);
				//cout << "Persisted (send):" << (_next_send_seq + 1) << " and " << _next_receive_seq << endl;
			}

			if (!msg->get_custom_seqnum() && !msg->get_no_increment())
			{
				++_next_send_seq;
				//cout << "Seqnum now:" << _next_send_seq << " and " << _next_receive_seq << endl;
			}
		}
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
		unsigned send_seqnum, receive_seqnum;
		if (_persist->get(send_seqnum, receive_seqnum))
		{
			ostringstream ostr;
			ostr << "Last sent: " << send_seqnum << ", last received: " << receive_seqnum;
			log(ostr.str());
			_next_send_seq = send_seqnum ;// + 1;
			_next_receive_seq = receive_seqnum ;// + 1;
		}
	}
}

