//-----------------------------------------------------------------------------------------
#if 0

FIX8 is released under the New BSD License.

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
RegExp FIXReader::_hdr("8=([^\x01]+)\x01{1}9=([^\x01]+)\x01");

//-------------------------------------------------------------------------------------------------
int FIXReader::operator()()
{
   unsigned processed(0), dropped(0), invalid(0);
	int retval(0);

   for (; !_session.is_shutdown();)
   {
		try
		{
			f8String msg;

			if (read(msg))	// will block
			{
				if (!_msg_queue.try_push (msg))
				{
					_session.log("FIXReader: message queue is full");
					++dropped;
				}
				else
					++processed;
			}
			else
				++invalid;
		}
		catch (Poco::Net::NetException& e)
		{
			_session.log(e.what());
			retval = -1;
			break;
		}
		catch (PeerResetConnection& e)
		{
			_session.log(e.what());
			retval = -1;
			break;
		}
		catch (exception& e)	// also catches Poco::Net::NetException
		{
			_session.log(e.what());
			++invalid;
		}
   }

	ostringstream ostr;
	ostr << "FIXReader: " << processed << " messages processed, " << dropped << " dropped, "
		<< invalid << " invalid";
	if (retval)
		ostr << " (socket error=" << errno << ')';
	_session.log(ostr.str());

	return retval;
}

//-------------------------------------------------------------------------------------------------
int FIXReader::callback_processor()
{
	bool stopping(false);
	int processed(0), ignored(0);

   for (; !_session.is_shutdown();)
   {
      f8String msg;

		if (stopping)	// make sure we dequeue any pending msgs before exiting
		{
			if (!_msg_queue.try_pop(msg))
				break;
		}
		else
			_msg_queue.pop (msg); // will block

      if (msg.empty())  // means exit
		{
         stopping = true;
			//continue;
			break;
		}

      if (!_session.process(msg))
		{
			ostringstream ostr;
			ostr << "Unhandled message: " << msg;
			//_session.log(ostr.str());
			++ignored;
		}
		else
			++processed;
   }

	ostringstream ostr;
	ostr << "FIXReaderCallback: " << processed << " messages processed, " << ignored << " ignored";
	_session.log(ostr.str());

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
	char msg_buf[_max_msg_len] = {};
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
				throw IllegalMessage(msg_buf);
			msg_buf[offs++] = bt;
		}
		while (bt != default_field_separator && offs < _max_msg_len);
		to.assign(msg_buf, offs);

		RegMatch match;
		if (_hdr.SearchString(match, to, 3, 0) == 3)
		{
			f8String bgstr, len;
			_hdr.SubExpr(match, to, bgstr, 0, 1);
			_hdr.SubExpr(match, to, len, 0, 2);

			if (bgstr != _session.get_ctx()._beginStr)	// invalid FIX version
				throw InvalidVersion(bgstr);

			const unsigned mlen(GetValue<unsigned>(len));
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
			return true;
		}

		throw IllegalMessage(to);
	}

	return false;
}

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
int FIXWriter::operator()()
{
	int result(0), processed(0), invalid(0);

   for (; !_session.is_shutdown();)
   {
		try
		{
			Message *inmsg;
			_msg_queue.pop (inmsg); // will block
			if (!inmsg)
				break;
#if defined MSGRECYCLING
			_session.send_process(inmsg);
			inmsg->set_in_use(false);
#else
			scoped_ptr<Message> msg(inmsg);
			_session.send_process(msg.get());
#endif
			++processed;
		}
		catch (exception& e)	// also catches Poco::Net::NetException
		{
			_session.log(e.what());
			++invalid;
			break; //?
		}
	}

	ostringstream ostr;
	ostr << "FIXWriter: " << processed << " messages processed, " << invalid << " invalid";
	_session.log(ostr.str());

	return result;
}

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
void Connection::start()
{
	_writer.start();
	_reader.start();
}

//-------------------------------------------------------------------------------------------------
void Connection::stop()
{
#if 0
	//_reader.stop();
	//_writer.stop();
	_reader.quit();
	_writer.quit();
#endif
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
	unsigned login_retry_interval, login_retries, attempts(0);;
	bool reset_sequence_numbers;
	_session.get_login_parameters(login_retry_interval, login_retries, reset_sequence_numbers);

	Poco::Timespan timeout(1000000);

	while (attempts < login_retries)
	{
		ostringstream ostr;

		try
		{
			ostr.str("");
			ostr << "Trying to connect to: " << _addr.toString() << " (" << ++attempts << ')';
			_session.log(ostr.str());
			_sock->connect(_addr, timeout);
			_sock->setLinger(false, 0);
			_sock->setNoDelay(_no_delay);
			_session.log("Connection successful");
			return _connected = true;
		}
		catch (exception& e)	// also catches Poco::Net::NetException
		{
			ostr.str("");
			ostr << "exception: ";
			if (dynamic_cast<Poco::Exception*>(&e))
				ostr << (static_cast<Poco::Exception&>(e)).displayText();
			else
				ostr << e.what();
			_session.log(ostr.str());
			millisleep(login_retry_interval);
		}
	}

	_session.log("Connection failed");
	return false;
}

