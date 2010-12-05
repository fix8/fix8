//-----------------------------------------------------------------------------------------
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

-------------------------------------------------------------------------------------------
$Id$
$Date$
$URL$

#endif
//-----------------------------------------------------------------------------------------
#include <config.h>
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

#ifdef HAS_TR1_UNORDERED_MAP
#include <tr1/unordered_map>
#endif

#include <f8includes.hpp>

//-------------------------------------------------------------------------------------------------
const std::string TRANSLATIONUNIT(__FILE__);
using namespace FIX8;
using namespace std;

//-------------------------------------------------------------------------------------------------
char FIX8::glob_log0[max_global_filename_length] = { "global_filename_not_set.log" };

template<>
tbb::atomic<SingleLogger<glob_log0> *> Singleton<SingleLogger<glob_log0> >::_instance
	= tbb::atomic<SingleLogger<glob_log0> *>();

//-------------------------------------------------------------------------------------------------
int Logger::operator()()
{
   unsigned received(0);
	bool stopping(false);
	ThreadCodes thread_codes;

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

      if (msg.second.empty())  // means exit
		{
         stopping = true;
			continue;
		}

		++received;

		if (_flags & sequence)
			get_stream() << setw(7) << right << setfill('0') << ++_sequence << ' ';

		if (_flags & thread)
		{
			ThreadCodes::const_iterator itr(thread_codes.find(msg.first));
			char t_code;
			if (itr == thread_codes.end())
			{
				t_code = _t_code++;
				thread_codes.insert(ThreadCodes::value_type(msg.first, t_code));
			}
			else
				t_code = itr->second;
			get_stream() << t_code << ' ';
		}

		if (_flags & timestamp)
		{
			string ts;
			get_stream() << GetTimeAsStringMS(ts, 0, 9) << ' ';
		}

		get_stream() << msg.second << std::endl;
   }

   return 0;
}

//-------------------------------------------------------------------------------------------------
FileLogger::FileLogger(const std::string& fname, const ebitset<Flags> flags, const unsigned rotnum)
	: Logger(flags), _ofs(), _rotnum(rotnum)
{
   // | uses IFS safe cfpopen; ! uses old popen if available
   if (fname[0] == '|' || fname[0] == '!')
   {
      _pathname = fname.substr(1);
      FILE *pcmd(
#ifdef HAVE_POPEN
         fname[0] == '!' ? popen(_pathname.c_str(), "w") :
#endif
                        cfpopen(const_cast<char*>(_pathname.c_str()), const_cast<char*>("w")));
      if (pcmd == 0)
      {
         //SimpleLogError(TRANSLATIONUNIT, __LINE__, _pathname);
#ifdef DEBUG
         *errofs << _pathname << " failed to execute" << endl;
#endif
      }
      else if (ferror(pcmd))
#ifdef DEBUG
         *errofs << _pathname << " shows ferror" << endl
#endif
         ;
      else
      {
         _ofs = new fptrostream(pcmd, fname[0] == '|');
         _flags |= pipe;
      }
   }
   else if (!fname.empty())
   {
      _pathname = fname;
      rotate();
   }
}

//-------------------------------------------------------------------------------------------------
bool FileLogger::rotate()
{
   if (!(_flags & pipe))
   {
      string thislFile(_pathname);
#ifdef HAVE_COMPRESSION
      if (_flags & compress)
         thislFile += ".gz";
#endif
      if (_rotnum > 0 && !(_flags & append))
      {
         vector<string> rlst;
         rlst.push_back(thislFile);

         for (unsigned ii(0); ii < _rotnum && ii < max_rotation; ++ii)
         {
            ostringstream ostr;
            ostr << _pathname << '.' << (ii + 1);
            if (_flags & compress)
               ostr << ".gz";
            rlst.push_back(ostr.str());
         }

         for (int ii(_rotnum); ii; --ii)
            rename (rlst[ii - 1].c_str(), rlst[ii].c_str());   // ignore errors
      }

      delete _ofs;

#ifdef HAVE_COMPRESSION
      if (_flags & compress)
         _ofs = new ogzstream(thislFile.c_str());
      else
#endif
         _ofs = new ofstream(thislFile.c_str(),
				std::ios_base::out|(_flags & append) ? std::ios_base::app : std::ios_base::trunc);
   }

   return true;
}

