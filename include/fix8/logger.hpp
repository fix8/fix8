//-------------------------------------------------------------------------------------------------
/*

Fix8 is released under the GNU LESSER GENERAL PUBLIC LICENSE Version 3.

Fix8 Open Source FIX Engine.
Copyright (C) 2010-14 David L. Dight <fix@fix8.org>

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
//-------------------------------------------------------------------------------------------------
#ifndef FIX8_LOGGER_HPP_
#define FIX8_LOGGER_HPP_

//-------------------------------------------------------------------------------------------------
#include <list>
#include <Poco/Net/IPAddress.h>
#include <Poco/Net/DatagramSocket.h>

//-------------------------------------------------------------------------------------------------
namespace FIX8 {

//-------------------------------------------------------------------------------------------------
/// File descriptor output streambuf, inspiration from Josuttis N.M.
class fdoutbuf : public std::streambuf
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

/// File pointer stream
class fptrostream : public std::ostream
{
   FILE *fptr_;

protected:
   fdoutbuf buf_;

public:
	/*! Ctor.
	    \param fptr FILE* */
   fptrostream(FILE *fptr)
		: std::ostream(&buf_), fptr_(fptr), buf_(fileno(fptr)) {}

	/// Dtor.
   virtual ~fptrostream ()
   {
#ifdef _MSC_VER
	   _pclose(fptr_);
#else
	   pclose(fptr_);
#endif
   }

	/*! Get the filno (fd)
	    \return fd */
   int getfileno() { return fileno(fptr_); }
};

//-------------------------------------------------------------------------------------------------
/// Socket output streambuf, inspiration from Josuttis N.M.
class bcoutbuf : public std::streambuf
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

//-------------------------------------------------------------------------------------------------
/// f8_thread delegated async logging class
class Logger
{
	f8_thread<Logger> _thread;
	std::list<std::string> _buffer;

public:
	enum Level { Debug, Info, Warn, Error, Fatal };
	static const int Errors = (1<<Warn|1<<Error|1<<Fatal), All = (1<<Debug|1<<Info|1<<Warn|1<<Error|1<<Fatal), None = 0;
	enum Flags { mstart, sstart, sequence, thread, timestamp, minitimestamp, direction, level,
					 append, start_controls=append, buffer, compress, pipe, broadcast, nolf, inbound, outbound, num_flags };
	static const int StdFlags = (1<<sequence|1<<thread|1<<timestamp|1<<level);
	enum { rotation_default = 5, max_rotation = 1024};
	using LogFlags = ebitset<Flags>;
	using Levels = ebitset<Level>;
	using LogPositions = std::vector<int>;

protected:
	f8_mutex _mutex;
	f8_spin_lock _log_spl;
	LogFlags _flags;
	Levels _levels;
	std::ostream *_ofs = nullptr;
	size_t _lines = 0;
	f8_thread_cancellation_token _stopping;

	struct LogElement
	{
		thread_id_t _tid;
		std::string _str;
		Level _level;
		unsigned _val;
		Tickval _when{true};

		LogElement(const thread_id_t tid, const std::string& str, Level level, const unsigned val=0)
			: _tid(tid), _str(str), _level(level),_val(val) {}
		LogElement(const thread_id_t tid, const std::string& str, const unsigned val=0)
			: _tid(tid), _str(str), _level(Info),_val(val) {}
		LogElement() : _tid(), _level(Info), _val() {}
		LogElement(const LogElement& from) : _tid(from._tid), _str(from._str), _level(from._level), _val(from._val), _when(from._when) {}
		LogElement& operator=(const LogElement& that)
		{
			if (this != &that)
			{
				_tid = that._tid;
				_str = that._str;
				_level = that._level;
				_val = that._val;
				_when = that._when;
			}
			return *this;
		}
	};

	f8_concurrent_queue<LogElement> _msg_queue;
	unsigned _sequence = 0, _osequence = 0;

	using ThreadCodes = std::map<thread_id_t, char>;
	ThreadCodes _thread_codes;

	using RevThreadCodes = std::map<char, thread_id_t>;
	RevThreadCodes _rev_thread_codes;

	/// The time the entire logging system was start
	static const Tickval _started;

	LogPositions _positions;

public:
	/*! Ctor.
	    \param flags ebitset flags */
	Logger(const LogFlags flags, const Levels levels=Levels(All), const LogPositions positions=LogPositions())
		: _thread(std::ref(*this)), _flags(flags), _levels(levels), _positions(positions)
	{
		if (_positions.empty()) // setup default order
		{
			for (auto ii(mstart); ii != num_flags; ii = Flags(ii + 1))
				if (_flags & ii)
					_positions.push_back(ii);
		}

		_thread.start();
	}

	/// Dtor.
	virtual ~Logger()
	{
		stop();
		delete _ofs;
	}

	/*! Check if the given log level is set for this logger
	    \param level level to test
	    \return true if available */
	bool is_loggable(Level level) const { return _levels & level; }

	/*! Set the Log Levels
	    \param levels levels to set */
	void set_levels(Levels levels) { _levels = levels; }

	/*! Set the LogFlags
	    \param flags flags to set */
	void set_flags(LogFlags flags) { _flags = flags; }

	/*! Set the log attribute positions
	    \param positions positions to set */
	void set_positions(const std::vector<int>& positions)
	{
		f8_scoped_spin_lock guard(_log_spl);
		_positions = positions;
	}

	/*! Get the underlying stream object.
	    \return the stream */
	virtual std::ostream& get_stream() const { return _ofs ? *_ofs : std::cout; }

	/*! Log a string with log level. Ignore level and always enqueue.
	    \param what the string to log
	    \param lev log level (enum)
	    \param val optional value for the logger to use
	    \return true on success */
	bool enqueue(const std::string& what, Level lev=Logger::Info, const unsigned val=0)
	{
		const LogElement le(f8_thread<Logger>::getid(), what, lev, val);
		return _msg_queue.try_push (le) == 0;
	}

	/*! Log a string with log level.
	    \param what the string to log
	    \param lev log level (enum)
	    \param val optional value for the logger to use
	    \return true on success */
	bool send(const std::string& what, Level lev=Logger::Info, const unsigned val=0)
		{ return is_loggable(lev) ? enqueue(what, lev, val) : true; }

	/// Stop the logging thread.
	void stop() { _stopping.request_stop(); enqueue(std::string()); _thread.join(); }

	/*! Perform logfile rotation. Only relevant for file-type loggers.
		\param force the rotation (even if the file is set to append)
	   \return true on success */
	virtual bool rotate(bool force=false) { return true; }

	/*! The logging thread entry point.
	    \return 0 on success */
	F8API int operator()();

	/// string representation of logflags
	static const std::vector<std::string> _bit_names;

	/// string representation of levels
	static const std::vector<std::string> _level_names;

	/*! Get te time the logging syste started (actual system startup)
		\return Tickval of start time */
	static const Tickval& get_time_started() { return _started; }

	/*! Check if the given log flag is set
		\param flg flag bit to check
		\return true if set */
	bool has_flag(const Flags flg) const { return _flags.has(flg); }

	/*! Get the thread code for this thread or allocate a new code if not found.
		\param tid the thread id of the thread to get a code for */
	F8API char get_thread_code(thread_id_t tid);

	/// Remove dead threads from the thread code cache.
	F8API void purge_thread_codes();

	/// Flush the buffer
	F8API virtual void flush();

	/*! Get the thread cancellation token
		\return reference to the cancellation token */
	f8_thread_cancellation_token& cancellation_token() { return _stopping; }
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
	F8API FileLogger(const std::string& pathname, const LogFlags flags, const Levels levels,
		const LogPositions positions=LogPositions(), const unsigned rotnum=rotation_default);

	/// Dtor.
	virtual ~FileLogger() {}

	/*! Perform logfile rotation
	    \param force force the rotation (even if the file is set ti append)
	    \return true on success */
	F8API virtual bool rotate(bool force=false);
};

//-------------------------------------------------------------------------------------------------
/// A pipe logger.
class PipeLogger : public Logger
{
public:
	/*! Ctor.
	    \param command pipe command
	    \param flags ebitset flags */
	PipeLogger(const std::string& command, const LogFlags flags, const Levels levels, LogPositions positions=LogPositions());

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
	BCLogger(Poco::Net::DatagramSocket *sock, const LogFlags flags, const Levels levels, LogPositions positions=LogPositions());

	/*! Ctor.
	    \param ip ip string
	    \param port port to use
	    \param flags ebitset flags */
	BCLogger(const std::string& ip, const unsigned port, const LogFlags flags, const Levels levels, LogPositions positions=LogPositions());

	/*! Check to see if a socket was successfully created.
	  \return non-zero if ok, 0 if not ok */
	operator void*() { return _init_ok ? this : 0; }

	/// Dtor.
	virtual ~BCLogger() {}
};

//-------------------------------------------------------------------------------------------------
const size_t max_global_filename_length(1024);

/// A global singleton logger
/*! \tparam fn actual pathname of logfile
    \details Create a static instance of this template and set the template parameter to the desired logfile pathname */
template<char *fn>
class SingleLogger
{
	static FileLogger& instance()
	{
		static FileLogger _fl(fn, Logger::LogFlags() << Logger::timestamp << Logger::sequence << Logger::thread << Logger::level
#if defined BUFFERED_GLOBAL_LOGGING
			<< Logger::buffer
#endif
			,Logger::Levels(Logger::All));

		return _fl;
	}

public:
	/*! Set the global logfile name.
	    \param from name to set to */
	static void set_global_filename(const std::string& from)
		{ CopyString(from, fn, max_global_filename_length); }

	/*! Send a message to the logger.
	  \param what message to log
	  \param lev level to log
	  \param val extra value to log
	  \return true on success */
	static bool log(const std::string& what, Logger::Level lev=Logger::Info, unsigned int val=0)
		{ return instance().send(what, lev, val); }

	/*! Enqueue a message to the logger. Will not check log level.
	  \param what message to log
	  \param lev level to log
	  \param val extra value to log
	  \return true on success */
	static bool enqueue(const std::string& what, Logger::Level lev=Logger::Info, unsigned int val=0)
		{ return instance().enqueue(what, lev, val); }

	/*! Flush the logger */
	static void flush_log() { instance().flush(); }

	/*! Set the logflags
	  \param flags flags to set */
	static void set_flags(Logger::LogFlags flags) { instance().set_flags(flags); }

	/*! Set the log levels
	  \param levels levels to set */
	static void set_levels(Logger::Levels levels) { instance().set_levels(levels); }

	/*! Set the log positions
	  \param positions positions to set */
	static void set_positions(Logger::LogPositions positions) { instance().set_positions(positions); }

	/*! Stop the logger */
	static void stop() { instance().stop(); }

	/*! Check if the given log level is set for this logger
	    \param level level to test
	    \return true if available */
	static bool is_loggable(Logger::Level lev) { return instance().is_loggable(lev); }
};

//-----------------------------------------------------------------------------------------
class buffered_ostream : public std::ostream
{
protected:
	class tsbuf : public std::streambuf
	{
		std::string _str;

		int_type overflow(int_type c)
		{
			if (c != traits_type::eof())
			{
				char z(c);
				_str.append(&z, 1);
			}
			return c;
		}

		std::streamsize xsputn(const char *s, std::streamsize num)
			{ _str.append(s, num); return num; }

	public:
		tsbuf() = default;
		~tsbuf() = default;
		const std::string& get() const { return _str; }
	};

	tsbuf _buf;

public:
   buffered_ostream() : std::ostream(&_buf) {}
   virtual ~buffered_ostream() {}
};

//-----------------------------------------------------------------------------------------
using logger_function = std::function<bool(const std::string&, Logger::Level, const unsigned)>;

class log_stream : public buffered_ostream
{
	logger_function _logger;
	const Logger::Level _lev;
	unsigned _value;

public:
	log_stream(decltype(_logger) func, Logger::Level lev=Logger::Info, unsigned value=0)
		: _logger(func), _lev(lev), _value(value) {}
	~log_stream() { _logger(_buf.get(), _lev, _value); }
};

//-----------------------------------------------------------------------------------------
F8API extern char glob_log0[max_global_filename_length];
using GlobalLogger = SingleLogger<glob_log0>;

// our buffered RAII ostream singleton insertable log target, global ostream log target

//-------------------------------------------------------------------------------------------------

} // FIX8

#define glout_info if (!FIX8::GlobalLogger::is_loggable(FIX8::Logger::Info)); \
	else FIX8::log_stream(FIX8::logger_function(FIX8::GlobalLogger::enqueue), FIX8::Logger::Info)
#define glout glout_info
#define glout_warn if (!FIX8::GlobalLogger::is_loggable(FIX8::Logger::Warn)); \
	else FIX8::log_stream(FIX8::logger_function(FIX8::GlobalLogger::enqueue), FIX8::Logger::Warn)
#define glout_error if (!FIX8::GlobalLogger::is_loggable(FIX8::Logger::Error)); \
	else FIX8::log_stream(FIX8::logger_function(FIX8::GlobalLogger::enqueue), FIX8::Logger::Error)
#define glout_fatal if (!FIX8::GlobalLogger::is_loggable(FIX8::Logger::Fatal)); \
	else FIX8::log_stream(FIX8::logger_function(FIX8::GlobalLogger::enqueue), FIX8::Logger::Fatal)
#if defined F8_DEBUG
#define glout_debug if (!FIX8::GlobalLogger::is_loggable(FIX8::Logger::Debug)); \
	else FIX8::log_stream(FIX8::logger_function(FIX8::GlobalLogger::enqueue), FIX8::Logger::Debug) << FILE_LINE
#else
#define glout_debug true ? FIX8::null_insert() : FIX8::null_insert()
#endif

#endif // _FIX8_LOGGER_HPP_
