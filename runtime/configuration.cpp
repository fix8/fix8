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
				string dir, db;
				bool has_dir(which->GetAttr("dir", dir)), has_db(which->GetAttr("db", db));

				if (type == "bdb" && has_dir && has_db)
				{
					scoped_ptr<BDBPersister> result(new BDBPersister);
					if (result->initialise(dir, db))
						return result.release();
				}
#if 0
				else if (type == "file" && has_dir && has_db)
				{
					scoped_ptr<FilePersister> result(new FilePersister);
					if (result->initialise(dir, db))
						return result.release();
				}
#endif
				else if (type == "mem")
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

