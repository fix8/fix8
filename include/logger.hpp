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
#ifndef _FIX8_LOGGER_HPP_
#define _FIX8_LOGGER_HPP_

#include <tbb/concurrent_queue.h>

//-------------------------------------------------------------------------------------------------
namespace FIX8 {

//------------------------------------------------------------------------------
class fdoutbuf : public std::streambuf // inspiration from Josuttis N.M.
{
protected:
   int fd;

   virtual int_type overflow (int_type c)
   {
      if (c != EOF)
      {
         char z(c);
         if (write(fd, &z, 1) != 1)
            return EOF;
      }
      return c;
   }

   virtual std::streamsize xsputn (const char *s, std::streamsize num)
   {
      return write (fd, s, num);
   }

public:
   fdoutbuf (int _fd) : fd(_fd) {}
   virtual ~fdoutbuf () {}
};

extern "C"
{
	FILE *cfpopen(char *command, char *type);
	int cfpclose(FILE *pp);
}

class fptrostream : public std::ostream
{
   FILE *fptr_;
   bool cf_;

protected:
   fdoutbuf buf_;

public:
   fptrostream (FILE *fptr, bool cf=true) : std::ostream(&buf_), fptr_(fptr), cf_(cf), buf_(fileno(fptr)) {}
   virtual ~fptrostream ()
	{
		if (cf_)
			cfpclose(fptr_);
		else
			pclose(fptr_);
	}

   int getfileno() { return fileno(fptr_); }
};

//-------------------------------------------------------------------------------------------------
class Logger
{
	Thread<Logger> _thread;

public:
	enum Flags { append, timestamp, sequence, compress, pipe };
	static const unsigned rotation_default = 5, max_rotation = 64;

protected:
	ebitset<Flags> _flags;
	tbb::concurrent_bounded_queue<std::string> _msg_queue;
	unsigned _sequence;

public:
	Logger(const ebitset<Flags> flags) : _thread(ref(*this)), _flags(flags), _sequence() { _thread.Start(); }
	virtual ~Logger() { stop(); }

	virtual std::ostream& get_stream() const { return std::cout; }
	bool send(const std::string& what) { return _msg_queue.try_push (what) == 0; }
	void stop() { send(std::string()); _thread.Join(); }
	virtual bool rotate() { return true; }

	static const std::string& GetTimeAsStringMS(std::string& result, timespec *tv=0);
	int operator()();
};

//-------------------------------------------------------------------------------------------------
class GenericLogger : public Logger
{
	std::ostream& _os;

public:
	GenericLogger(std::ostream& os, const ebitset<Flags> flags) : Logger(flags), _os(os) {}
	virtual ~GenericLogger() {}

	virtual std::ostream& get_stream() const { return _os; }
};

//-------------------------------------------------------------------------------------------------
class FileLogger : public Logger
{
	std::ostream *_ofs;
	std::string _pathname;
	unsigned _rotnum;

public:
	FileLogger(const std::string& pathname, const ebitset<Flags> flags, const unsigned rotnum=rotation_default);
	virtual ~FileLogger() { delete _ofs; }

	virtual std::ostream& get_stream() const { return *_ofs; }
	virtual bool rotate();
};

//-------------------------------------------------------------------------------------------------

} // FIX8

#endif // _FIX8_LOGGER_HPP_
