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
int FIXReader::execute(f8_thread_cancellation_token& cancellation_token)
{
   unsigned processed(0), dropped(0), invalid(0);
	int retval(0);
	f8String msg;

	if (_pmodel == pm_coro)
	{
		try
		{
			reenter(_coro)
			{
				while (!_session.is_shutdown())
				{
					if (poll()) // it is a fair assumption that there is a complete message waiting to be read
					{
						if (read(msg))	// if our assumption is wrong we could block here
						{
							if (!_session.process(msg))
							{
								scout_warn << "Unhandled message: " << msg;
								++invalid;
							}
							else
								++processed;
						}
						else
							++invalid;
					}
					coro_yield;
				}
			}
		}
		catch (Poco::Net::NetException& e)
		{
			scout_error << e.what();
			retval = -1;
		}
		catch (PeerResetConnection& e)
		{
			scout_error << e.what();
			_session.do_state_change(States::st_session_terminated);
			_session.stop();
			retval = -1;
		}
		catch (exception& e)
		{
			scout_error << e.what();
			++invalid;
		}
	}
	else
	{
		while (!cancellation_token && !_session.is_shutdown())
		{
			try
			{
				if (read(msg))	// will block
				{
					if (_pmodel == pm_pipeline)
					{
						if (!_msg_queue.try_push (msg))
						{
							scout_warn << "FIXReader: message queue is full";
							++dropped;
						}
						else
							++processed;
					}
					else // _pmodel == pm_thread
					{
						if (!_session.process(msg))
						{
							scout_warn << "Unhandled message: " << msg;
							++invalid;
						}
						else
							++processed;
					}
				}
				else
					++invalid;
			}
			catch (Poco::Net::NetException& e)
			{
				scout_error << e.what();
				retval = -1;
				break;
			}
			catch (PeerResetConnection& e)
			{
				scout_error << e.what();
				_session.do_state_change(States::st_session_terminated);
				_session.stop();
				retval = -1;
				break;
			}
			catch (exception& e)
			{
				scout_error << e.what();
				++invalid;
			}
		}

		scout_info << "FIXReader: " << processed << " messages processed, " << dropped << " dropped, "
			<< invalid << " invalid";
		if (retval && errno)
		{
			scout_error << "socket error=" << errno;
		}
	}

	return retval;
}

//-------------------------------------------------------------------------------------------------
int FIXReader::callback_processor()
{
	int processed(0), ignored(0);

   for (; !_cancellation_token && !_session.is_shutdown();)
   {
		f8String *msg_ptr(0);
#if (FIX8_MPMC_SYSTEM == FIX8_MPMC_TBB)
      f8String msg;
		_msg_queue.pop (msg); // will block
      if (msg.empty())  // means exit
			break;
		msg_ptr = &msg;
#else
		_msg_queue.pop (msg_ptr); // will block
		if (msg_ptr->empty())  // means exit
			break;
#endif

      if (!_session.process(*msg_ptr))
		{
			scout_warn << "Unhandled message: " << *msg_ptr;
			++ignored;
		}
		else
			++processed;
#if (FIX8_MPMC_SYSTEM == FIX8_MPMC_FF)
		_msg_queue.release(msg_ptr);
#endif
   }

	scout_info << "FIXReaderCallback: " << processed << " messages processed, " << ignored << " ignored";

	return 0;
}

//-------------------------------------------------------------------------------------------------
void FIXReader::set_preamble_sz()
{
	_bg_sz = 2 + _session.get_ctx()._beginStr.size() + 1 + 3;
}

//-------------------------------------------------------------------------------------------------
bool FIXReader::read(f8String& to)	// read a complete FIX message
{
	char msg_buf[_max_msg_len] {};
	int result(sockRead(msg_buf, _bg_sz));

	if (result == static_cast<int>(_bg_sz))
	{
		char bt;
		size_t offs(_bg_sz);
		do	// get the last chrs of bodylength and ^A
		{
			if (sockRead(&bt, 1) != 1)
				return false;
			if (!isdigit(bt) && bt != default_field_separator)
				throw IllegalMessage(msg_buf, FILE_LINE);
			msg_buf[offs++] = bt;
		}
		while (bt != default_field_separator && offs < _max_msg_len);
		to.assign(msg_buf, offs);

		char tag[MAX_MSGTYPE_FIELD_LEN], val[FIX8_MAX_FLD_LENGTH];
		unsigned result;
		if ((result = MessageBase::extract_element(to.data(), static_cast<unsigned>(to.size()), tag, val)))
		{
			if (*tag != '8')
				throw IllegalMessage(to, FILE_LINE);

			if (_session.get_ctx()._beginStr.compare(val))	// invalid FIX version
				throw InvalidVersion(string(val));

			if ((result = MessageBase::extract_element(to.data() + result, static_cast<unsigned>(to.size()) - result, tag, val)))
			{
				if (*tag != '9')
					throw IllegalMessage(to, FILE_LINE);

				const unsigned mlen(fast_atoi<unsigned>(val));
				if (mlen == 0 || mlen > _max_msg_len - _bg_sz - _chksum_sz) // invalid msglen
					throw InvalidBodyLength(mlen);

				// read the body
				if ((result = sockRead(msg_buf, mlen) != static_cast<int>(mlen)))
					return false;

				// read the checksum
				if ((result = sockRead(msg_buf + mlen, _chksum_sz) != static_cast<int>(_chksum_sz)))
					return false;

				to.append(msg_buf, mlen + _chksum_sz);
				_session.update_received();
				//string ts;
				//cerr << GetTimeAsStringMS(ts, &_session.get_last_received(), 9) << endl;
				return true;
			}
		}

		throw IllegalMessage(to, FILE_LINE);
	}

	return false;
}

//-------------------------------------------------------------------------------------------------
int FIXWriter::execute(f8_thread_cancellation_token& cancellation_token)
{
	int result(0), processed(0), invalid(0);

	while (!cancellation_token && !_session.is_shutdown())
   {
		try
		{
			Message *inmsg(0);
			_msg_queue.pop (inmsg); // will block
			if (!inmsg)
				break;
			unique_ptr<Message> msg(inmsg);
			_session.send_process(msg.get());
			++processed;
		}
		catch (PeerResetConnection& e)
		{
			scout_error << e.what();
			result = -1;
			break;
		}
		catch (Poco::Net::NetException& e)
		{
			scout_error << e.what();
			++invalid;
			break;
		}
		catch (exception& e)
		{
			scout_error << e.what();
			++invalid;
			break; //?
		}
	}

	scout_info << "FIXWriter: " << processed << " messages processed, " << invalid << " invalid";

	return result;
}

//-------------------------------------------------------------------------------------------------
void Connection::start()
{
	_writer.start();
	_reader.start();
}

//-------------------------------------------------------------------------------------------------
void Connection::stop()
{
	scout_debug << "Connection::stop()";
	_writer.stop();
	_writer.join();
	_reader.stop();
	_reader.join();
	_reader.socket()->shutdownReceive();
}

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
bool ClientConnection::connect()
{
	unsigned attempts(0);
	const LoginParameters& lparam(_session.get_login_parameters());
	const Poco::Timespan timeout(lparam._connect_timeout, 0);

	while (attempts < (lparam._reliable ? 1 : lparam._login_retries))
	{
		try
		{
			if (_addr == Poco::Net::SocketAddress())
				throw Poco::Net::InvalidAddressException("empty address");

			scout_info << "Trying to connect to: " << _addr.toString() << " (" << ++attempts << ')' << ( _secured ? " secured" : " not-secured");
			_sock->connect(_addr, timeout);
			if (lparam._recv_buf_sz)
				set_recv_buf_sz(lparam._recv_buf_sz);
			if (lparam._send_buf_sz)
				set_send_buf_sz(lparam._send_buf_sz);
			_sock->setLinger(false, 0);
			_sock->setNoDelay(_no_delay);
			scout_info << "Connection successful";
			return _connected = true;
		}
		catch (Poco::Exception& e)
		{
			if (lparam._reliable)
				throw;
			scout_error << "exception: " << e.displayText();
			hypersleep<h_milliseconds>(lparam._login_retry_interval);
		}
		catch (exception& e)
		{
			if (lparam._reliable)
				throw;
			scout_error << "exception: " << e.what();
			hypersleep<h_milliseconds>(lparam._login_retry_interval);
		}
	}

	scout_error << "Connection failed";
	return false;
}
