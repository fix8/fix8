//-----------------------------------------------------------------------------------------
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
//-----------------------------------------------------------------------------------------
#include "precomp.hpp"
#include <fix8/f8includes.hpp>

//-------------------------------------------------------------------------------------------------
using namespace FIX8;
using namespace std;


//-------------------------------------------------------------------------------------------------
namespace
{
	const string package_version { FIX8_PACKAGE_NAME " version " FIX8_PACKAGE_VERSION };
	const string copyright_short { "Copyright (c) 2010-" };
	const string copyright_short2 { ", David L. Dight <" FIX8_PACKAGE_BUGREPORT ">, All rights reserved. [" FIX8_PACKAGE_URL "]"};
}

//-------------------------------------------------------------------------------------------------
RegExp SessionID::_sid("([^:]+):([^-]+)->(.+)");

//-------------------------------------------------------------------------------------------------
#if defined(_MSC_VER) && !defined(BUILD_F8API)
// no need in definition since it is in dll already
#else
const vector<f8String> Session::_state_names
{
	"none", "continuous", "session_terminated",
	"wait_for_logon", "not_logged_in", "logon_sent", "logon_received", "logoff_sent",
	"logoff_received", "test_request_sent", "sequence_reset_sent",
	"sequence_reset_received", "resend_request_sent", "resend_request_received"
};
#endif
#if defined(_MSC_VER)
#pragma warning(push)
#pragma warning(disable: 4273)
#endif
//-------------------------------------------------------------------------------------------------
void SessionID::make_id()
{
	ostringstream ostr;
	ostr << _beginString << ':' << _senderCompID << "->" << _targetCompID;
	_id = ostr.str();
}

//-------------------------------------------------------------------------------------------------
SessionID SessionID::make_reverse_id() const
{
	return SessionID(_beginString(), _targetCompID(), _senderCompID());
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
_state(States::st_none),
_ctx(ctx), _connection(), _req_next_send_seq(), _req_next_receive_seq(),
	_sid(sid), _sf(), _persist(persist), _logger(logger), _plogger(plogger),	// initiator
	_timer(*this, 10), _hb_processor(&Session::heartbeat_service, true),
	_session_scheduler(&Session::activation_service, true), _schedule()
{
	_timer.start();
	_batchmsgs_buffer.reserve(10 * (FIX8_MAX_MSG_LENGTH + HEADER_CALC_OFFSET));

	if (!_logger)
	{
		glout_warn << "Warning: no session logger defined for " << _sid;
	}

	if (!_plogger)
	{
		glout_warn << "Warning: no protocol logger defined for " << _sid;
	}

	if (!_persist)
	{
		glout_warn << "Warning: no persister defined for " << _sid;
	}
}

//-------------------------------------------------------------------------------------------------
Session::Session(const F8MetaCntx& ctx, const sender_comp_id& sci, Persister *persist, Logger *logger, Logger *plogger) :
_state(States::st_none),
_ctx(ctx), _sci(sci), _connection(), _req_next_send_seq(), _req_next_receive_seq(),
	_sf(), _persist(persist), _logger(logger), _plogger(plogger),	// acceptor
	_timer(*this, 10), _hb_processor(&Session::heartbeat_service, true),
	_session_scheduler(&Session::activation_service, true), _schedule()
{
	_timer.start();
	_batchmsgs_buffer.reserve(10 * (FIX8_MAX_MSG_LENGTH + HEADER_CALC_OFFSET));
}

//-------------------------------------------------------------------------------------------------
void Session::atomic_init(States::SessionStates st)
{
	do_state_change(st);
	_next_send_seq = _next_receive_seq = 1;
	_active = true;
}

//-------------------------------------------------------------------------------------------------
Session::~Session()
{
	slout_info << "Session terminating";
	if (_logger)
		_logger->stop();
	if (_plogger)
		_plogger->stop();
	hypersleep<h_seconds>(1); // needed for service threads to exit gracefully

	if (_connection && _connection->get_role() == Connection::cn_acceptor)
		{ f8_scoped_spin_lock guard(_per_spl); delete _persist; _persist = 0; }
	delete _schedule;
}

//-------------------------------------------------------------------------------------------------
int Session::start(Connection *connection, bool wait, const unsigned send_seqnum,
	const unsigned recv_seqnum, const f8String davi)
{
	glout_info << copyright_string();
	if (_logger)
		_logger->purge_thread_codes();

	if (_plogger)
		_plogger->purge_thread_codes();

	_control.clear(shutdown);
	slout_info << "Starting session";
	_connection = connection; // takes owership
	if (!_connection->connect()) // if already connected returns true
		return -1;
	if (_connection->get_role() == Connection::cn_acceptor)
		atomic_init(States::st_wait_for_logon); // important for server that this is done before connect
	_connection->start();
	slout_info << "Session connected";

	if (_connection->get_role() == Connection::cn_initiator)
	{
		atomic_init(States::st_not_logged_in);
		if (_loginParameters._reset_sequence_numbers)
			_next_send_seq = _next_receive_seq = 1;
		else
		{
			recover_seqnums();
			if (send_seqnum)
				_next_send_seq = send_seqnum;
			if (recv_seqnum)
				_next_receive_seq = recv_seqnum;
		}

		send(generate_logon(_connection->get_hb_interval(), davi));
		do_state_change(States::st_logon_sent);
	}
	else
	{
		if (send_seqnum)
			_req_next_send_seq = send_seqnum;
		if (recv_seqnum)
			_req_next_receive_seq = recv_seqnum;
	}

	if (_sf && (_schedule = _sf->create_session_schedule(_sf->_ses)))
	{
		if (_connection->get_role() == Connection::cn_initiator)
		{
			slout_info << *_schedule;
		}
		_timer.schedule(_session_scheduler, 1000);	// check every second
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
	if (_control & shutdown)
		return;
	_control |= shutdown;

	if (_connection)
	{
		if (_connection->get_role() == Connection::cn_initiator)
			_timer.clear();
		else
		{
			f8_scoped_spin_lock guard(_per_spl, _connection->get_pmodel() == pm_coro);
			if (_persist)
				_persist->stop();
		}
		_connection->stop();
	}
	hypersleep<h_milliseconds>(250);
}

//-------------------------------------------------------------------------------------------------
bool Session::enforce(const unsigned seqnum, const Message *msg)
{
	if (States::is_established(_state))
	{
		if (_state != States::st_logon_received)
			compid_check(seqnum, msg, _sid);
		if (msg->get_msgtype() != Common_MsgType_SEQUENCE_RESET && sequence_check(seqnum, msg))
			return false;
	}

	return true;
}

//-------------------------------------------------------------------------------------------------
void Session::update_persist_seqnums()
{
	if (_persist)
	{
		f8_scoped_spin_lock guard(_per_spl, _connection && _connection->get_pmodel() == pm_coro);
		_persist->put(_next_send_seq, _next_receive_seq);
		//cout << "Persisted:" << _next_send_seq << " and " << _next_receive_seq << endl;
	}
}

//-------------------------------------------------------------------------------------------------
bool Session::process(const f8String& from)
{
	unsigned seqnum(0);
	const Message *msg = nullptr;

	try
	{
		const f8String::size_type fpos(from.find("34="));
		if (fpos == f8String::npos)
		{
			slout_debug << "Session::process throwing for " << from;
			throw InvalidMessage(from, FILE_LINE);
		}

		seqnum = fast_atoi<unsigned>(from.data() + fpos + 3, default_field_separator);

		bool retry_plog(false);
		if (_plogger && _plogger->has_flag(Logger::inbound))
		{
			if (_state != States::st_wait_for_logon)
				plog(from, Logger::Info, 1);
			else
				retry_plog = true;
		}

		if (!(msg = Message::factory(_ctx, from, _loginParameters._no_chksum_flag, _loginParameters._permissive_mode_flag)))
		{
			glout_fatal << "Fatal: factory failed to generate a valid message";
			return false;
		}

		if ((_control & printnohb) && msg->get_msgtype() != Common_MsgType_HEARTBEAT)
			cout << *msg << endl;
		else if (_control & print)
			cout << *msg << endl;

		bool result(false), admin_result(msg->is_admin() ? handle_admin(seqnum, msg) : true);
		if (msg->get_msgtype().size() > 1)
			goto application_call;
		else switch(msg->get_msgtype()[0])
		{
		default:
application_call:
			if (activation_check(seqnum, msg))
				result = handle_application(seqnum, msg);
			break;
		case Common_MsgByte_HEARTBEAT:
			result = handle_heartbeat(seqnum, msg);
			break;
		case Common_MsgByte_TEST_REQUEST:
			result = handle_test_request(seqnum, msg);
			break;
		case Common_MsgByte_RESEND_REQUEST:
			result = handle_resend_request(seqnum, msg);
			break;
		case Common_MsgByte_REJECT:
			result = handle_reject(seqnum, msg);
			break;
		case Common_MsgByte_SEQUENCE_RESET:
			result = handle_sequence_reset(seqnum, msg);
			break;
		case Common_MsgByte_LOGOUT:
			result = handle_logout(seqnum, msg);
			break;
		case Common_MsgByte_LOGON:
			result = handle_logon(seqnum, msg);
			break;
		}

		++_next_receive_seq;
		if (retry_plog)
			plog(from, Logger::Info, 1);

		update_persist_seqnums();

		delete msg;
		return result && admin_result;
	}
	catch (LogfileException& e)
	{
		cerr << e.what() << endl;
	}
	catch (f8Exception& e)
	{
		slout_debug << "process: f8exception" << ' ' << seqnum << ' ' << e.what();

		if (e.force_logoff())
		{
			slout_fatal << e.what() << " - will logoff";
			if (_state == States::st_logon_received && !_loginParameters._silent_disconnect)
			{
				send(generate_logout(e.what()), true, 0, true); // so it won't increment
				do_state_change(States::st_logoff_sent);
			}
			stop();
		}
		else
		{
			slout_error << e.what() << " - message rejected";
			send(generate_reject(seqnum, e.what(), msg && !msg->get_msgtype().empty() ? msg->get_msgtype().c_str() : nullptr));
			++_next_receive_seq;
			delete msg;
			return true; // message is handled but has errors
		}
	}
	catch (Poco::Net::NetException& e)
	{
		slout_debug << "process:: Poco::Net::NetException";
		slout_error << e.what();
	}
	catch (exception& e)
	{
		slout_debug << "process:: std::exception";
		slout_error << e.what();
	}

	return false;
}

//-------------------------------------------------------------------------------------------------
void Session::compid_check(const unsigned seqnum, const Message *msg, const SessionID& id) const
{
	if (_loginParameters._enforce_compids)
	{
		if (!id.same_sender_comp_id(msg->Header()->get<target_comp_id>()->get()))
			throw BadCompidId(msg->Header()->get<target_comp_id>()->get());
		if (!id.same_target_comp_id(msg->Header()->get<sender_comp_id>()->get()))
			throw BadCompidId(msg->Header()->get<sender_comp_id>()->get());
	}
}

//-------------------------------------------------------------------------------------------------
bool Session::sequence_check(const unsigned seqnum, const Message *msg)
{
	//cout << "seqnum:" << seqnum << " next_target_seq:" << _next_receive_seq
		//<< " next_sender_seq:" << _next_send_seq << " state:" << _state << " next_receive_seq:" <<  _next_receive_seq << endl;

	if (seqnum > _next_receive_seq)
	{
		if (_state == States::st_resend_request_sent)
		{
			slout_warn << "Resend request already sent";
		}
		if (_state == States::st_continuous)
		{
			send(generate_resend_request(_next_receive_seq));
			do_state_change(States::st_resend_request_sent);
		}
		// If SessionConfig has *not* been set, assume wrong logon sequence is checked.
		else if (!_sf || !_sf->get_ignore_logon_sequence_check_flag(_sf->_ses))
			throw InvalidMsgSequence(seqnum, _next_receive_seq);
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
	do_state_change(States::st_logon_received);
	const bool reset_given(msg->have(Common_ResetSeqNumFlag) && msg->get<reset_seqnum_flag>()->get());
	sender_comp_id sci; // so this should be our tci
	msg->Header()->get(sci);
	target_comp_id tci; // so this should be our sci
	msg->Header()->get(tci);
	SessionID id(_ctx._beginStr, tci(), sci());

	if (_connection->get_role() == Connection::cn_initiator)
	{
		if (id != _sid)
		{
			glout_warn << "Inbound TargetCompID not recognised (" << tci << "), expecting (" << _sid.get_senderCompID() << ')';
			if (_loginParameters._enforce_compids)
			{
				stop();
				do_state_change(States::st_session_terminated);
				return false;
			}
		}

		enforce(seqnum, msg);
		heartbeat_interval hbi;
		msg->get(hbi);
		_connection->set_hb_interval(hbi());
		do_state_change(States::st_continuous);
		slout_info << "Client setting heartbeat interval to " << hbi();
	}
	else // acceptor
	{
		default_appl_ver_id davi;
		msg->get(davi);

		if (_sci() != tci())
		{
			glout_warn << "Inbound TargetCompID not recognised (" << tci << "), expecting (" << _sci << ')';
			if (_loginParameters._enforce_compids)
			{
				stop();
				do_state_change(States::st_session_terminated);
				return false;
			}
		}

		if (!_loginParameters._clients.empty())
		{
			auto itr(_loginParameters._clients.find(sci()));
			bool iserr(false);
			if (itr == _loginParameters._clients.cend())
			{
				glout_error << "Remote (" << sci << ") not found (" << id << "). NOT authorised to proceed.";
				iserr = true;
			}

			if (!iserr && get<1>(itr->second) != Poco::Net::IPAddress()
				&& get<1>(itr->second) != _connection->get_peer_socket_address().host())
			{
				glout_error << "Remote (" << get<0>(itr->second) << ", " << sci << ") NOT authorised to proceed ("
					<< _connection->get_peer_socket_address().toString() << ").";
				iserr = true;
			}

			if (iserr)
			{
				stop();
				do_state_change(States::st_session_terminated);
				return false;
			}

			glout_info << "Remote (" << get<0>(itr->second) << ", " << sci << ") authorised to proceed ("
				<< _connection->get_peer_socket_address().toString() << ").";
		}

		// important - these objects can't be created until we have a valid SessionID
		if (_sf)
		{
			if (!_logger && !(_logger = _sf->create_logger(_sf->_ses, Configuration::session_log, &id)))
			{
				glout_warn << "Warning: no session logger defined for " << id;
			}

			if (!_plogger && !(_plogger = _sf->create_logger(_sf->_ses, Configuration::protocol_log, &id)))
			{
				glout_warn << "Warning: no protocol logger defined for " << id;
			}

			if (!_persist)
			{
				f8_scoped_spin_lock guard(_per_spl, _connection->get_pmodel() == pm_coro);
				if (!(_persist = _sf->create_persister(_sf->_ses, &id, reset_given)))
				{
					glout_warn << "Warning: no persister defined for " << id;
				}
			}

#if 0
			if (_schedule)
				slout_info << *_schedule;
#endif
		}
		else
		{
			glout_error << "Error: SessionConfig object missing in session";
		}

		slout_info << "Connection from " << _connection->get_peer_socket_address().toString();

		if (reset_given) // ignore version restrictions on this behaviour
		{
			slout_info << "Resetting sequence numbers";
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

		if (authenticate(id, msg))
		{
			_sid = id;
			enforce(seqnum, msg);
			send(generate_logon(_connection->get_hb_interval(), davi()));
			do_state_change(States::st_continuous);
		}
		else
		{
			slout_error << id << " failed authentication";
			stop();
			do_state_change(States::st_session_terminated);
			return false;
		}

		if (_loginParameters._login_schedule.is_valid() && !_loginParameters._login_schedule.test())
		{
			slout_error << id << " Session unavailable. Login not accepted.";
			stop();
			do_state_change(States::st_session_terminated);
			return false;
		}
	}

	slout_info << "Heartbeat interval is " << _connection->get_hb_interval();

	_timer.schedule(_hb_processor, 1000);	// check every second

	return true;
}

//-------------------------------------------------------------------------------------------------
bool Session::handle_logout(const unsigned seqnum, const Message *msg)
{
	enforce(seqnum, msg);

	//if (_state != States::st_logoff_sent)
	//	send(generate_logout());
	slout_info << "peer has logged out";
	stop();
	return true;
}

//-------------------------------------------------------------------------------------------------
bool Session::handle_sequence_reset(const unsigned seqnum, const Message *msg)
{
	enforce(seqnum, msg);

	new_seq_num nsn;
	if (msg->get(nsn))
	{
		slout_debug << "newseqnum = " << nsn() << ", _next_receive_seq = " << _next_receive_seq << " seqnum:" << seqnum;
		if (nsn() >= static_cast<int>(_next_receive_seq))
			_next_receive_seq = nsn() - 1;
		else if (nsn() < static_cast<int>(_next_receive_seq))
			throw MsgSequenceTooLow(nsn(), _next_receive_seq);
	}

	if (_state == States::st_resend_request_sent)
		do_state_change(States::st_continuous);

	return true;
}

//-------------------------------------------------------------------------------------------------
bool Session::handle_resend_request(const unsigned seqnum, const Message *msg)
{
	enforce(seqnum, msg);

	if (_state != States::st_resend_request_received)
	{
		begin_seq_num begin;
		end_seq_num end;

		if (!_persist || !msg->get(begin) || !msg->get(end))
			send(generate_sequence_reset(_next_send_seq + 1, true));
		else if ((begin() > end() && end()) || begin() == 0)
			send(generate_reject(seqnum, "Invalid begin or end resend seqnum"), msg->get_msgtype().c_str());
		else
		{
			//cout << "got resend request:" << begin() << " to " << end() << endl;
			do_state_change(States::st_resend_request_received);
			//f8_scoped_spin_lock guard(_per_spl, _connection->get_pmodel() == pm_coro); // no no nanette!
			_persist->get(begin(), end(), *this, &Session::retrans_callback);
		}
	}

	return true;
}

//-------------------------------------------------------------------------------------------------
bool Session::retrans_callback(const SequencePair& with, RetransmissionContext& rctx)
{
	//cout << "first:" << with.first << ' ' << rctx << endl;

	if (rctx._no_more_records)
	{
		if (rctx._end)
		{
			_next_send_seq = rctx._interrupted_seqnum - 1;
			send(generate_sequence_reset(rctx._interrupted_seqnum, true), true, rctx._last + 1);
			//cout << "#1" << endl;
		}
		else if (!rctx._last)
		{
			_next_send_seq = rctx._interrupted_seqnum;
			send(generate_sequence_reset(rctx._interrupted_seqnum, true), true, rctx._begin);
			//cout << "#4" << endl;
		}
		do_state_change(States::st_continuous);
		return true;
	}

	if (rctx._last)
	{
		if (rctx._last + 1 < with.first)
		{
			send(generate_sequence_reset(with.first, true), true, _next_send_seq);
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

	Message *msg(Message::factory(_ctx, with.second));
	return send(msg);
}

//-------------------------------------------------------------------------------------------------
bool Session::handle_test_request(const unsigned seqnum, const Message *msg)
{
	enforce(seqnum, msg);

	test_request_id testReqID;
	msg->get(testReqID);
	send(generate_heartbeat(testReqID()));
	return true;
}


//-------------------------------------------------------------------------------------------------
bool Session::activation_service()	// called on the timer threead
{
	//cout << "activation_service()" << endl;
	if (is_shutdown())
		return false;

	if (_connection && _connection->is_connected())
	{
		const bool curr(_active);
		_active = _schedule->_sch.test(curr);
		if (curr != _active)
		{
			slout_info << "Session activation transitioned to " << (_active ? "active" : "inactive");
		}
	}

	return true;
}

//-------------------------------------------------------------------------------------------------
bool Session::heartbeat_service()
{
	//cout << "heartbeat_service()" << endl;
	if (is_shutdown())
		return false;

	if (_connection && _connection->is_connected())
	{
		Tickval now(true);
		if ((now - _last_sent).secs() >= static_cast<time_t>(_connection->get_hb_interval()))
		{
			const f8String testReqID;
			send(generate_heartbeat(testReqID));
		}

		now.now();
		if ((now - _last_received).secs() > static_cast<time_t>(_connection->get_hb_interval20pc()))
		{
			if (_state == States::st_test_request_sent)	// already sent
			{
				ostringstream ostr;
				ostr << "Remote has ignored my test request. Aborting session...";
				send(generate_logout(_loginParameters._silent_disconnect ? 0 : ostr.str().c_str()), true, 0, true); // so it won't increment
				do_state_change(States::st_logoff_sent);
				log(ostr.str(), Logger::Error);
				try
				{
					stop();
				}
				catch (Poco::Net::NetException& e)
				{
					slout_error << e.what();
				}
				catch (exception& e)
				{
					slout_error << e.what();
				}
				return true;
			}
			else if (_state != States::st_session_terminated)
			{
				ostringstream ostr;
				ostr << "Have not received anything from remote for ";
				if (_last_received.secs())
					ostr << (now - _last_received).secs();
				else
					ostr << "more than " << _connection->get_hb_interval20pc();
				ostr << " secs. Sending test request";
				log(ostr.str(), Logger::Warn);
				const f8String testReqID("TEST");
				send(generate_test_request(testReqID));
				do_state_change(States::st_test_request_sent);
			}
		}
	}

	return true;
}

//-------------------------------------------------------------------------------------------------
bool Session::handle_heartbeat(const unsigned seqnum, const Message *msg)
{
	enforce(seqnum, msg);

	if (_state == States::st_test_request_sent)
		do_state_change(States::st_continuous);
	return true;
}

//-------------------------------------------------------------------------------------------------
Message *Session::generate_heartbeat(const f8String& testReqID)
{
	Message *msg(create_msg(Common_MsgType_HEARTBEAT));
	if (!testReqID.empty())
		*msg << new test_request_id(testReqID);

	return msg;
}

//-------------------------------------------------------------------------------------------------
Message *Session::generate_reject(const unsigned seqnum, const char *what, const char *msgtype)
{
	Message *msg(create_msg(Common_MsgType_REJECT));
	*msg << new ref_seq_num(seqnum);
	if (what)
		*msg << new text(what);
	if (msgtype)
		*msg << new ref_msg_type(msgtype);

	return msg;
}

//-------------------------------------------------------------------------------------------------
Message *Session::generate_business_reject(const unsigned seqnum, const Message *imsg, const int reason, const char *what)
{
	Message *msg;
	try
	{
		msg = create_msg(Common_MsgType_BUSINESS_REJECT);
	}
	catch (InvalidMetadata<f8String>&)
	{
		// since this is an application message, it may not be supported in supplied schema
		return nullptr;
	}

	*msg << new ref_seq_num(seqnum);
	*msg << new ref_msg_type(imsg->get_msgtype());
	*msg << new business_reject_reason(reason);
	if (what)
		*msg << new text(what);

	return msg;
}

//-------------------------------------------------------------------------------------------------
Message *Session::generate_test_request(const f8String& testReqID)
{
	Message *msg(create_msg(Common_MsgType_TEST_REQUEST));
	*msg << new test_request_id(testReqID);

	return msg;
}

//-------------------------------------------------------------------------------------------------
Message *Session::generate_logon(const unsigned heartbtint, const f8String davi)
{
	Message *msg(create_msg(Common_MsgType_LOGON));
	*msg << new heartbeat_interval(heartbtint)
		  << new encrypt_method(0); // FIXME
	if (!davi.empty() && msg->is_legal<default_appl_ver_id>())
		*msg << new default_appl_ver_id(davi);
	if (_loginParameters._reset_sequence_numbers)
		*msg << new reset_seqnum_flag(true);

	return msg;
}

//-------------------------------------------------------------------------------------------------
Message *Session::generate_logout(const char *msgstr)
{
	Message *msg(create_msg(Common_MsgType_LOGOUT));
	if (msgstr)
		*msg << new text(msgstr);

	return msg;
}

//-------------------------------------------------------------------------------------------------
Message *Session::generate_resend_request(const unsigned begin, const unsigned end)
{
	Message *msg(create_msg(Common_MsgType_RESEND_REQUEST));
	*msg << new begin_seq_num(begin) << new end_seq_num(end);

	return msg;
}

//-------------------------------------------------------------------------------------------------
Message *Session::generate_sequence_reset(const unsigned newseqnum, const bool gapfillflag)
{
	Message *msg(create_msg(Common_MsgType_SEQUENCE_RESET));
	*msg << new new_seq_num(newseqnum);

	if (gapfillflag)
		*msg << new gap_fill_flag(true);

	return msg;
}

//-------------------------------------------------------------------------------------------------
bool Session::send(Message *tosend, bool destroy, const unsigned custom_seqnum, const bool no_increment)
{
	if (custom_seqnum)
		tosend->set_custom_seqnum(custom_seqnum);
	if (no_increment)
		tosend->set_no_increment(no_increment);
	return _connection && _connection->write(tosend, destroy);
}

bool Session::send(Message& tosend, const unsigned custom_seqnum, const bool no_increment)
{
	if (custom_seqnum)
		tosend.set_custom_seqnum(custom_seqnum);
	if (no_increment)
		tosend.set_no_increment(no_increment);
	return _connection && _connection->write(tosend);
}

//-------------------------------------------------------------------------------------------------
size_t Session::send_batch(const vector<Message *>& msgs, bool destroy)
{
	return _connection->write_batch(msgs, destroy);
}

//-------------------------------------------------------------------------------------------------
int Session::modify_header(MessageBase *msg)
{
	return 0;
}

//-------------------------------------------------------------------------------------------------
bool Session::send_process(Message *msg) // called from the connection (possibly on separate thread)
{
	//cout << "send_process()" << endl;
	bool is_dup(msg->Header()->have(Common_PossDupFlag));
	if (!msg->Header()->have(Common_SenderCompID))
		*msg->Header() << new sender_comp_id(_sid.get_senderCompID());
	if (!msg->Header()->have(Common_TargetCompID))
		*msg->Header() << new target_comp_id(_sid.get_targetCompID());

	if (msg->Header()->have(Common_MsgSeqNum))
	{
		if (is_dup)
		{
			if (_loginParameters._always_seqnum_assign)
				delete msg->Header()->remove(Common_PossDupFlag);
		}
		else
		{
			if (!_loginParameters._always_seqnum_assign)
			{
				*msg->Header() << new poss_dup_flag(true);
				is_dup = true;
			}
		}

		sending_time sendtime;
		msg->Header()->get(sendtime);
		*msg->Header() << new orig_sending_time(sendtime());

		if (_loginParameters._always_seqnum_assign)
		{
			slout_debug << "send_process: _next_send_seq = " << _next_send_seq;
			*msg->Header() << new msg_seq_num(msg->get_custom_seqnum() ? msg->get_custom_seqnum() : static_cast<unsigned int>(_next_send_seq));
		}
	}
	else
	{
		slout_debug << "send_process: _next_send_seq = " << _next_send_seq;
		*msg->Header() << new msg_seq_num(msg->get_custom_seqnum() ? msg->get_custom_seqnum() : static_cast<unsigned int>(_next_send_seq));
	}
	*msg->Header() << new sending_time;

	// allow session to modify the header of this message before sending
	const int fields_modified(modify_header(msg->Header()));
	if (fields_modified)
	{
		slout_debug << "send_process: " << fields_modified << " header fields added/modified";
	}

	try
	{
		slout_debug << "Sending:" << *msg;
		modify_outbound(msg);
		char output[FIX8_MAX_MSG_LENGTH + HEADER_CALC_OFFSET], *ptr(output);
		size_t enclen(msg->encode(&ptr));
		const char *optr(ptr);
		if (msg->get_end_of_batch())
		{
			if (!_batchmsgs_buffer.empty())
			{
				_batchmsgs_buffer.append(ptr);
				ptr = &_batchmsgs_buffer[0];
				enclen = _batchmsgs_buffer.size();
			}
			if (!_connection->send(ptr, enclen))
			{
				slout_error << "Message write failed: " << enclen << " bytes";
				_batchmsgs_buffer.clear();
				return false;
			}
			_last_sent.now();
			_batchmsgs_buffer.clear();
		}
		else
		{
			_batchmsgs_buffer.append(ptr);
		}

		if (_plogger && _plogger->has_flag(Logger::outbound))
			plog(optr, Logger::Info);

		//cout << "send_process" << endl;

		if (!is_dup)
		{
			if (_persist)
			{
				f8_scoped_spin_lock guard(_per_spl, _connection->get_pmodel() == pm_coro); // not needed for coroutine mode
				if (!msg->is_admin())
					_persist->put(_next_send_seq, ptr);
				_persist->put(_next_send_seq + 1, _next_receive_seq);
				//cout << "Persisted (send):" << (_next_send_seq + 1) << " and " << _next_receive_seq << endl;
			}
			if (!msg->get_custom_seqnum() && !msg->get_no_increment() && msg->get_msgtype() != Common_MsgType_SEQUENCE_RESET)
			{
				++_next_send_seq;
				//cout << "Seqnum now:" << _next_send_seq << " and " << _next_receive_seq << endl;
			}
		}
	}
	catch (f8Exception& e)
	{
		slout_error << e.what();
		return false;
	}
	catch (Poco::Exception& e)
	{
		slout_error << e.displayText();
		return false;
	}

	return true;
}

//-------------------------------------------------------------------------------------------------
void Session::recover_seqnums()
{
	f8_scoped_spin_lock guard(_per_spl, _connection->get_pmodel() == pm_coro);
	if (_persist)
	{
		unsigned send_seqnum, receive_seqnum;
		if (_persist->get(send_seqnum, receive_seqnum))
		{
			slout_info << "Last sent: " << send_seqnum << ", last received: " << receive_seqnum;
			_next_send_seq = send_seqnum; // + 1;
			_next_receive_seq = receive_seqnum; // + 1;
		}
	}
}

//-------------------------------------------------------------------------------------------------
#if (FIX8_THREAD_SYSTEM == FIX8_THREAD_PTHREAD) && !defined _MSC_VER && defined _GNU_SOURCE && defined __linux__
f8String Session::get_thread_policy_string(thread_id_t id)
{
   int policy;
	ostringstream ostr;
   sched_param param {};
   if (!pthread_getschedparam(id,  &policy, &param))
		return policy == SCHED_OTHER ? "SCHED_OTHER" : policy == SCHED_RR ? "SCHED_RR"
			  : policy == SCHED_FIFO ? "SCHED_FIFO" : "UNKNOWN";

	ostr << "Could not get scheduler parameters: " << Str_error(errno);
	return ostr.str();
}

//-------------------------------------------------------------------------------------------------
void Session::set_scheduler(int priority)
{
   pthread_t thread(pthread_self());
   sched_param param { priority };

	slout_info << "Current scheduler policy: " << get_thread_policy_string(thread);

   if (pthread_setschedparam(thread, SCHED_RR, &param))
	{
		slout_error << "Could not set new scheduler priority: " << get_thread_policy_string(thread)
			<< " (" << Str_error(errno) << ") " << priority;
		return;
   }

	slout_info << "New scheduler policy: " << get_thread_policy_string(thread);
}

//-------------------------------------------------------------------------------------------------
void Session::set_affinity(int core_id)
{
   const int num_cores(sysconf(_SC_NPROCESSORS_ONLN));
   if (core_id >= num_cores)
	{
		slout_error << "Invalid core id: " << core_id;
      return;
	}

   cpu_set_t cpuset;
   CPU_ZERO(&cpuset);
   CPU_SET(core_id, &cpuset);
   const int error(pthread_setaffinity_np(pthread_self(), sizeof(cpu_set_t), &cpuset));

	if (error)
		slout_error << "Could not set thread affinity for core " << core_id << " (" << Str_error(errno) << ')';
	else
		slout_info << "Set thread affinity to " << core_id << " core for thread " << pthread_self();
}
#else
//-------------------------------------------------------------------------------------------------
void Session::set_scheduler(int priority)
{
	slout_error << "set_scheduler: not implemented";
}

//-------------------------------------------------------------------------------------------------
void Session::set_affinity(int core_id)
{
	slout_error << "set_affinity: not implemented";
}
#endif

//-------------------------------------------------------------------------------------------------
const f8String Session::copyright_string()
{
   time_t now(time(0));
#ifdef _MSC_VER
   struct tm *ptim(localtime (&now));
#else
   struct tm tim;
   localtime_r(&now, &tim);
   struct tm *ptim(&tim);
#endif
	ostringstream ostr;
	ostr << endl << package_version << ' ' << copyright_short << setw(2) << (ptim->tm_year - 100) << copyright_short2;
	return ostr.str();
}


//-------------------------------------------------------------------------------------------------
#ifdef FIX8_HAVE_OPENSSL
void Fix8CertificateHandler::onInvalidCertificate(const void*, Poco::Net::VerificationErrorArgs& errorCert)
{
   const Poco::Net::X509Certificate& cert(errorCert.certificate());
	glout_warn << "WARNING: Certificate verification failed";
	glout_warn << "----------------------------------------";
	glout_warn << "Issuer Name:  " << cert.issuerName();
	glout_warn << "Subject Name: " << cert.subjectName();
	glout_warn << "The certificate yielded the error: " << errorCert.errorMessage();
	glout_warn << "The error occurred in the certificate chain at position " << errorCert.errorDepth();
	errorCert.setIgnoreError(true);
}

void Fix8PassPhraseHandler::onPrivateKeyRequested(const void*, std::string& privateKey)
{
	glout_warn << "warning: privatekey passphrase requested and ignored!";
}

#endif // FIX8_HAVE_OPENSSL
//-------------------------------------------------------------------------------------------------
#if defined(_MSC_VER)
#pragma warning(pop)
#endif
