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
RegExp Configuration::_ipexp("^([^:]+):([0-9]+)$");

//-------------------------------------------------------------------------------------------------
int Configuration::process()
{
	if (!exist(_xmlfile))
		throw f8Exception("server config file not found", _xmlfile);
	if (!_root)
		throw f8Exception("could not create root xml entity");

	if (!load_map("fix8/session", _sessions, true))
		throw f8Exception("could not locate server session in config file", _xmlfile);

	load_map("fix8/persist", _persisters);
	load_map("fix8/log", _loggers);

	return _sessions.size();
}

//-------------------------------------------------------------------------------------------------
Connection::Role Configuration::get_role(const XmlElement *from) const
{
	string role;
	return from && from->GetAttr("role", role) ? role % "initiator" ? Connection::cn_initiator
		: role % "acceptor" ? Connection::cn_acceptor : Connection::cn_unknown : Connection::cn_unknown;
}

//-------------------------------------------------------------------------------------------------
Poco::Net::SocketAddress Configuration::get_address(const XmlElement *from) const
{
	Poco::Net::SocketAddress to;
	string ip, port;
	if (from && from->GetAttr("ip", ip) && from->GetAttr("port", port))
		to = Poco::Net::SocketAddress(ip, port);

	return to;
}

//-------------------------------------------------------------------------------------------------
Persister *Configuration::create_persister(const XmlElement *from) const
{
	string name;
	if (from && from->GetAttr("persist", name))
	{
		const XmlElement *which(find_persister(name));
		if (which)
		{
			string type;
			if (which->GetAttr("type", type))
			{
				if (type % "bdb")
				{
					string dir, db;
					if (which->GetAttr("dir", dir) && which->GetAttr("db", db))
					{
						scoped_ptr<BDBPersister> result(new BDBPersister);
						if (result->initialise(dir, db))
							return result.release();
					}
				}
				else if (type % "mem")
					return new MemoryPersister;
			}
		}
	}

	return 0;
}

//-------------------------------------------------------------------------------------------------
Logger *Configuration::create_logger(const XmlElement *from, const Logtype ltype) const
{
	string name;
	if (from && from->GetAttr(ltype == session_log ? "session_log" : "protocol_log", name))
	{
		const XmlElement *which(find_logger(name));
		if (which)
		{
			string type;
			if (which->GetAttr("type", type)
				&& ((type % "session" && ltype == session_log) || (type % "protocol" && ltype == protocol_log)))
			{
				string logname("logname_not_set.log");
				trim(get_logname(which, logname));

				if (logname[0] == '|' || logname[0] == '!')
					return new PipeLogger(logname, get_logflags(which));

				RegMatch match;
				if (_ipexp.SearchString(match, logname, 3) == 3)
				{
					f8String ip, port;
					_ipexp.SubExpr(match, logname, ip, 0, 1);
					_ipexp.SubExpr(match, logname, port, 0, 2);
					BCLogger *bcl(new BCLogger(ip, GetValue<unsigned>(port), get_logflags(which)));
					if (*bcl)
						return bcl;
				}

				return new FileLogger(logname, get_logflags(which), get_logfile_rotation(which));
			}
		}
	}

	return 0;
}

//-------------------------------------------------------------------------------------------------
Logger::LogFlags Configuration::get_logflags(const XmlElement *from) const
{
	Logger::LogFlags flags;
	string flags_str;
	if (from && from->GetAttr("flags", flags_str))
	{
		istringstream istr(flags_str);
		for(char extr[32]; !istr.get(extr, sizeof(extr), '|').eof() || extr[0]; istr.ignore(1))
		{
			string result(extr);
			flags.set(Logger::num_flags, Logger::_bit_names, trim(result));
		}
	}

	return flags;
}

//-------------------------------------------------------------------------------------------------
unsigned Configuration::get_all_sessions(vector<const XmlElement *>& target, const Connection::Role role) const
{
	for (vector<const XmlElement *>::const_iterator itr(_allsessions.begin()); itr != _allsessions.end(); ++itr)
		if (role == Connection::cn_unknown || get_role(*itr) == role)
			target.push_back(*itr);
	return target.size();
}

