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
$Id: f8c.hpp 528 2010-10-26 11:26:18Z davidd $
$Date: 2010-10-26 22:26:18 +1100 (Tue, 26 Oct 2010) $
$URL: svn://catfarm.electro.mine.nu/usr/local/repos/fix8/compiler/f8c.hpp $

#endif
//-------------------------------------------------------------------------------------------------
#ifndef _FIX8_LOGGER_HPP_
#define _FIX8_LOGGER_HPP_

#include <tbb/concurrent_queue.h>

//-------------------------------------------------------------------------------------------------
namespace FIX8 {

//-------------------------------------------------------------------------------------------------
class Logger
{
	Thread<Logger> _thread;

protected:
	tbb::concurrent_bounded_queue<std::string> _msg_queue;

public:
	Logger() : _thread(ref(*this)) { _thread.Start(); }
	virtual ~Logger() {}

	virtual std::ostream& get_stream() const { return std::cout; }
	bool send(const std::string& what) { return _msg_queue.try_push (what) == 0; }
	void stop() { send(std::string()); }

	int operator()();
};

//-------------------------------------------------------------------------------------------------
class GenericLogger : public Logger
{
	std::ostream& _os;

public:
	GenericLogger(std::ostream& os) : _os(os) {}
	virtual ~GenericLogger() {}

	virtual std::ostream& get_stream() const { return _os; }
};

//-------------------------------------------------------------------------------------------------
class FileLogger : public Logger
{
	std::ofstream *_ofs;

public:
	FileLogger(const char *fname, bool append=true)
		: _ofs(new std::ofstream(fname, std::ios_base::out|append ? std::ios_base::app : std::ios_base::trunc)) {}
	virtual ~FileLogger() { delete _ofs; }

	virtual std::ostream& get_stream() const { return *_ofs; }
};

//-------------------------------------------------------------------------------------------------

} // FIX8

#endif // _FIX8_LOGGER_HPP_
