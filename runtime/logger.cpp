//-----------------------------------------------------------------------------------------
#if 0

Fix8 is released under the GNU LESSER GENERAL PUBLIC LICENSE Version 3.

Fix8 Open Source FIX Engine.
Copyright (C) 2010-13 David L. Dight <fix@fix8.org>

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
#include <time.h>
#include <strings.h>
#include <regex.h>

#include <f8includes.hpp>

//-------------------------------------------------------------------------------------------------
using namespace FIX8;
using namespace std;

//-------------------------------------------------------------------------------------------------
namespace FIX8
{
	char glob_log0[max_global_filename_length] = { "global_filename_not_set.log" };

	template<>
	tbb::atomic<SingleLogger<glob_log0> *> Singleton<SingleLogger<glob_log0> >::_instance
		= tbb::atomic<SingleLogger<glob_log0> *>();
	template<>
	tbb::mutex Singleton<SingleLogger<glob_log0> >::_mutex = tbb::mutex();

	const string Logger::_bit_names[] = { "append", "timestamp", "sequence", "compress", "pipe", "broadcast", "thread", "direction", "buffer" };
}

//-------------------------------------------------------------------------------------------------
int Logger::operator()()
{
   unsigned received(0);
	bool stopping(false);

   for (;;)
   {
		LogElement msg;
		if (stopping)	// make sure we dequeue any pending msgs before exiting
		{
			if (!_msg_queue.try_pop(msg))
				break;
		}
		else
			_msg_queue.pop (msg); // will block

		++received;

      if (msg._str.empty())  // means exit
		{
         stopping = true;
			continue;
		}

		ostringstream ostr;

		if (_flags & sequence)
		{
			ostr << setw(7) << right << setfill('0');
			if (_flags & direction)
				ostr << (msg._val ? ++_sequence  : ++_osequence) << ' ';
			else
				ostr << ++_sequence << ' ';
		}

		if (_flags & thread)
			ostr << get_thread_code(msg._tid) << ' ';

		if (_flags & direction)
			ostr << (msg._val ? " in" : "out") << ' ' ;

		if (_flags & timestamp)
		{
			string ts;
			ostr << GetTimeAsStringMS(ts, &msg._when, 9) << ' ';
		}

		if (_flags & buffer)
		{
			string result(ostr.str());
			result += msg._str;
			_buffer.push_back(result);
		}
		else
		{
			tbb::mutex::scoped_lock guard(_mutex);
			get_stream() << ostr.str() << msg._str << endl;
		}
   }

   return 0;
}

//-------------------------------------------------------------------------------------------------
void Logger::flush()
{
	tbb::mutex::scoped_lock guard(_mutex);
	for (std::list<std::string>::const_iterator itr(_buffer.begin()); itr != _buffer.end(); ++itr)
		get_stream() << *itr << endl;
	_buffer.clear();
	_lines = 0;
}

//-------------------------------------------------------------------------------------------------
void Logger::purge_thread_codes()
{
	tbb::mutex::scoped_lock guard(_mutex);

	for (ThreadCodes::iterator itr(_thread_codes.begin()); itr != _thread_codes.end();)
	{
		// a little trick to see if a thread is still alive
		clockid_t clock_id;
		if (pthread_getcpuclockid(itr->first, &clock_id) == ESRCH)
		{
			_rev_thread_codes.erase(itr->second);
			_thread_codes.erase(itr++); // post inc, takes copy before incr;
		}
		else
			++itr;
	}
}

//-------------------------------------------------------------------------------------------------
char Logger::get_thread_code(pthread_t tid)
{
	tbb::mutex::scoped_lock guard(_mutex);

	ThreadCodes::const_iterator itr(_thread_codes.find(tid));
	if (itr != _thread_codes.end())
		return itr->second;

	for (char acode('A'); acode < 127; ++acode) 	// A-~ will allow for 86 threads
	{
		RevThreadCodes::const_iterator itr(_rev_thread_codes.find(acode));
		if (itr == _rev_thread_codes.end())
		{
			_thread_codes.insert(ThreadCodes::value_type(tid, acode));
			_rev_thread_codes.insert(ThreadCodes::value_type(acode, tid));
			return acode;
		}
	}

	return '?';
}

//-------------------------------------------------------------------------------------------------
FileLogger::FileLogger(const string& fname, const ebitset<Flags> flags, const unsigned rotnum)
	: Logger(flags), _rotnum(rotnum)
{
   if (!fname.empty())
   {
      _pathname = fname;
      rotate();
   }
}

//-------------------------------------------------------------------------------------------------
bool FileLogger::rotate(bool force)
{
	tbb::mutex::scoped_lock guard(_mutex);

	string thislFile(_pathname);
#ifdef HAVE_COMPRESSION
	if (_flags & compress)
		thislFile += ".gz";
#endif
	if (_rotnum > 0 && (!_flags.has(append) || force))
	{
		vector<string> rlst(_rotnum);
		rlst.push_back(thislFile);

		for (unsigned ii(0); ii < _rotnum && ii < max_rotation; ++ii)
		{
			ostringstream ostr;
			ostr << _pathname << '.' << (ii + 1);
			if (_flags & compress)
				ostr << ".gz";
			rlst.push_back(ostr.str());
		}

		for (unsigned ii(_rotnum); ii; --ii)
			rename (rlst[ii - 1].c_str(), rlst[ii].c_str());   // ignore errors
	}

	delete _ofs;

	const ios_base::openmode mode (_flags & append ? ios_base::out | ios_base::app : ios_base::out);
#ifdef HAVE_COMPRESSION
	if (_flags & compress)
		_ofs = new ogzstream(thislFile.c_str(), mode);
	else
#endif
		_ofs = new ofstream(thislFile.c_str(), mode);

	return true;
}

//-------------------------------------------------------------------------------------------------
PipeLogger::PipeLogger(const string& fname, const ebitset<Flags> flags) : Logger(flags)
{
	const string pathname(fname.substr(1));

   // | uses IFS safe cfpopen; ! uses old popen if available
	FILE *pcmd(
#ifdef HAVE_POPEN
		fname[0] == '!' ? popen(pathname.c_str(), "w") :
#endif
								cfpopen(const_cast<char*>(pathname.c_str()), const_cast<char*>("w")));
	if (pcmd == 0)
#ifdef DEBUG
		*errofs << pathname << " failed to execute" << endl
#endif
		;
	else if (ferror(pcmd))
#ifdef DEBUG
		*errofs << pathname << " shows ferror" << endl
#endif
		;
	else
	{
		_ofs = new fptrostream(pcmd, fname[0] == '|');
		_flags |= pipe;
	}
}

//-------------------------------------------------------------------------------------------------
// nc -lu 127.0.0.1 -p 51000
BCLogger::BCLogger(Poco::Net::DatagramSocket *sock, const ebitset<Flags> flags) : Logger(flags), _init_ok(true)
{
	_ofs = new bcostream(sock);
	_flags |= broadcast;
}

BCLogger::BCLogger(const string& ip, const unsigned port, const LogFlags flags) : Logger(flags), _init_ok()
{
	Poco::Net::IPAddress ipaddr;
	if (Poco::Net::IPAddress::tryParse(ip, ipaddr)
		&& (ipaddr.isGlobalMC() || ipaddr.isMulticast() || ipaddr.isBroadcast()
		|| ipaddr.isLinkLocalMC() || ipaddr.isUnicast() || ipaddr.isWellKnownMC()
		|| ipaddr.isSiteLocalMC() || ipaddr.isOrgLocalMC()))
	{
		Poco::Net::SocketAddress saddr(ipaddr, port);
		Poco::Net::DatagramSocket *dgs(new Poco::Net::DatagramSocket);
		if (ipaddr.isBroadcast())
			dgs->setBroadcast(true);
		dgs->connect(saddr);
		_ofs = new bcostream(dgs);
		_flags |= broadcast;
		_init_ok = true;
	}
}

