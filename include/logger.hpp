//-------------------------------------------------------------------------------------------------
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
//-------------------------------------------------------------------------------------------------
#ifndef _FIX8_LOGGER_HPP_
#define _FIX8_LOGGER_HPP_

#include <tbb/concurrent_queue.h>
#include <Poco/Net/IPAddress.h>
#include <Poco/Net/DatagramSocket.h>

//-------------------------------------------------------------------------------------------------
namespace FIX8 {

//-------------------------------------------------------------------------------------------------
/// File descriptor output streambuf
class fdoutbuf : public std::streambuf // inspiration from Josuttis N.M.
{
protected:
   int fd;

   virtual int_type overflow(int_type c)
   {
      if (c != traits_type::eof())
      {
         char z(c);
         if (write(fd, &z, 1) != 1)
            return traits_type::eof();
      }
      return c;
   }

   virtual std::streamsize xsputn(const char *s, std::streamsize num)
   {
      return write (fd, s, num);
   }

public:
   fdoutbuf(int _fd) : fd(_fd) {}
   virtual ~fdoutbuf () {}
};

extern "C"
{
	FILE *cfpopen(char *command, char *type);
	int cfpclose(FILE *pp);
}

/// File pointer stream
class fptrostream : public std::ostream
{
   FILE *fptr_;
   bool cf_;

protected:
   fdoutbuf buf_;

public:
	/*! Ctor.
	    \param fptr FILE*
	    \param cf if true, us cfpopen instead of popen */
   fptrostream(FILE *fptr, bool cf=true) : std::ostream(&buf_), fptr_(fptr), cf_(cf), buf_(fileno(fptr)) {}

	/// Dtor.
   virtual ~fptrostream ()
	{
		if (cf_)
			cfpclose(fptr_);
		else
			pclose(fptr_);
	}

	/*! Get the filno (fd)
	    \return fd */
   int getfileno() { return fileno(fptr_); }
};

//-------------------------------------------------------------------------------------------------
/// Socket output streambuf
class bcoutbuf : public std::streambuf // inspiration from Josuttis N.M.
{
protected:
	Poco::Net::DatagramSocket *_sock;

   virtual int_type overflow(int_type c)
   {
      if (c != traits_type::eof())
      {
         char z(c);
         _sock->sendBytes(&z, 1);
      }
      return c;
   }

   virtual std::streamsize xsputn(const char *s, std::streamsize num)
   {
      _sock->sendBytes(s, num);
      return num;
   }

public:
   bcoutbuf(Poco::Net::DatagramSocket *sock) : _sock(sock) {}
   virtual ~bcoutbuf() { _sock->close(); delete _sock; }
};

/// udp stream
class bcostream : public std::ostream
{
protected:
   bcoutbuf buf_;

public:
	/*! Ctor.
	    \param sock datagram socket */
   bcostream(Poco::Net::DatagramSocket *sock) : std::ostream(&buf_), buf_(sock) {}

	/// Dtor.
   virtual ~bcostream() {}
};

//-------------------------------------------------------------------------------------------------
class Tickval;

/// Thread delegated async logging class
class Logger
{
	Thread<Logger> _thread;
	std::ostringstream _buf;

public:
	enum Flags { append, timestamp, sequence, compress, pipe, broadcast, thread, direction, num_flags };
	enum { rotation_default = 5, max_rotation = 64} ;
	typedef ebitset<Flags> LogFlags;

protected:
	tbb::mutex _mutex;
	LogFlags _flags;
	std::ostream *_ofs;

	struct LogElement
	{
		pthread_t _tid;
		std::string _str;
		unsigned _val;
		Tickval _when;

		LogElement(const pthread_t tid, const std::string& str, const unsigned val=0)
			: _tid(tid), _str(str), _val(val), _when(true) {}
		LogElement() : _tid(), _val(), _when(true) {}
		LogElement& operator=(const LogElement& that)
		{
			if (this != &that)
			{
				_tid = that._tid;
				_str = that._str;
				_val = that._val;
				_when = that._when;
			}
			return *this;
		}
	};

	tbb::concurrent_bounded_queue<LogElement> _msg_queue;
	unsigned _sequence, _osequence;

	typedef std::
#if defined HAS_TR1_UNORDERED_MAP
		tr1::unordered_map
#else
		map
#endif
		<pthread_t, char> ThreadCodes;
	ThreadCodes _thread_codes;

	typedef std::map<char, pthread_t> RevThreadCodes;
	RevThreadCodes _rev_thread_codes;

public:
	/*! Ctor.
	    \param flags ebitset flags */
	Logger(const LogFlags flags) : _thread(ref(*this)), _flags(flags), _ofs(), _sequence(), _osequence() { _thread.Start(); }

	/// Dtor.
	virtual ~Logger() { delete _ofs; }

	/*! Get the underlying stream object.
	    \return the stream */
	virtual std::ostream& get_stream() const { return _ofs ? *_ofs : std::cout; }

	/*! Log a string.
	    \param what the string to log
	    \param val optional value for the logger to use
	    \return true on success */
	bool send(const std::string& what, const unsigned val=0)
		{ return _msg_queue.try_push (LogElement(pthread_self(), what, val)) == 0; }

	/// Stop the logging thread.
	void stop() { send(std::string()); _thread.Join(); }

	/*! Perform logfile rotation. Only relevant for file-type loggers.
		\param force the rotation (even if the file is set to append)
	   \return true on success */
	virtual bool rotate(bool force=false) { return true; }

	/*! The logging thread entry point.
	    \return 0 on success */
	int operator()();

	/// string representation of logflags
	static const std::string _bit_names[];

	/*! Get the thread code for this thread or allocate a new code if not found.
		\param tid the thread id of the thread to get a code for */
	char get_thread_code(pthread_t tid);

	/// Remove dead threads from the thread code cache.
	void purge_thread_codes();
};

//-------------------------------------------------------------------------------------------------
/// A file logger.
class FileLogger : public Logger
{
	std::string _pathname;
	unsigned _rotnum;

public:
	/*! Ctor.
	    \param pathname pathname to log to
	    \param flags ebitset flags
	    \param rotnum number of logfile rotations to retain (default=5) */
	FileLogger(const std::string& pathname, const LogFlags flags, const unsigned rotnum=rotation_default);

	/// Dtor.
	virtual ~FileLogger() {}

	/*! Perform logfile rotation
	    \param force force the rotation (even if the file is set ti append)
	    \return true on success */
	virtual bool rotate(bool force=false);
};

//-------------------------------------------------------------------------------------------------
/// A pipe logger.
class PipeLogger : public Logger
{
public:
	/*! Ctor.
	    \param command pipe command
	    \param flags ebitset flags */
	PipeLogger(const std::string& command, const LogFlags flags);

	/// Dtor.
	virtual ~PipeLogger() {}
};

//-------------------------------------------------------------------------------------------------
/// A broadcast logger.
class BCLogger : public Logger
{
	bool _init_ok;

public:
	/*! Ctor.
	    \param sock udp socket
	    \param flags ebitset flags */
	BCLogger(Poco::Net::DatagramSocket *sock, const LogFlags flags);

	/*! Ctor.
	    \param ip ip string
	    \param port port to use
	    \param flags ebitset flags */
	BCLogger(const std::string& ip, const unsigned port, const LogFlags flags);

	/*! Check to see if a socket was successfully created.
	  \return non-zero if ok, 0 if not ok */
	operator void*() { return _init_ok ? this : 0; }

	/// Dtor.
	virtual ~BCLogger() {}
};

//-------------------------------------------------------------------------------------------------
const size_t max_global_filename_length(128);

/// A global singleton logger
/*! \tparam fn actual pathname of logfile
    \details Create a static instance of this template and set the template parameter to the desired logfile pathname */
template<char *fn>
class SingleLogger : public Singleton<SingleLogger<fn> >, public FileLogger
{
public:
	/// Ctor.
	SingleLogger() : FileLogger(fn, LogFlags() << timestamp << sequence << thread) {}

	/*! Set the global logfile name.
	    \param from name to set to */
	static void set_global_filename(const std::string& from)
	{
		CopyString(from, fn, max_global_filename_length);
	}

	/*! Send a message to the logger.
	  \param what message to log
	  \return true on success */
	static bool log(const std::string& what)
	{
		return Singleton<SingleLogger<fn> >::instance()->send(what);
	}
};

//-----------------------------------------------------------------------------------------
extern char glob_log0[max_global_filename_length];
typedef SingleLogger<glob_log0> GlobalLogger;

//-------------------------------------------------------------------------------------------------

} // FIX8

#endif // _FIX8_LOGGER_HPP_
