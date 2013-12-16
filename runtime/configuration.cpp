//-----------------------------------------------------------------------------------------
/*

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

*/
//-----------------------------------------------------------------------------------------
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <map>
#include <list>
#include <set>
#include <iterator>
#include <memory>
#include <iomanip>
#include <algorithm>
#include <numeric>

#ifndef _MSC_VER
#include <strings.h>
#endif

#ifdef HAVE_OPENSSL
#include <Poco/Net/Context.h>
#endif
#include <fix8/f8includes.hpp>

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
	load_map("fix8/server_group", _server_group);
	load_map("fix8/ssl_context", _ssl_context);

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
size_t Configuration::get_addresses(const XmlElement *from, vector<Server>& target) const
{
	string name;
	const XmlElement *which;
	if (from && from->GetAttr("server_group", name) && (which = find_server_group(name)))
	{
		XmlElement::XmlSet slist;
		if (which->find("server_group/server", slist))
		{
			const Poco::Net::SocketAddress empty_addr;
			for(XmlElement::XmlSet::const_iterator itr(slist.begin()); itr != slist.end(); ++itr)
			{
				string name;
				Poco::Net::SocketAddress addr(get_address(*itr));
				if ((*itr)->GetAttr("name", name) && addr != empty_addr && (*itr)->FindAttr("active", true))
					target.push_back(Server(name, (*itr)->FindAttr("max_retries",
						static_cast<int>(defaults::login_retries)), addr,
						(*itr)->FindAttr("reset_sequence_numbers", false)));
			}
		}
		return target.size();
	}
	else
	{
	}

	return 0;
}

//-------------------------------------------------------------------------------------------------
Persister *Configuration::create_persister(const XmlElement *from, const SessionID *sid, bool flag) const
{
	string name, type;
	const XmlElement *which;
	if (from && from->GetAttr("persist", name) && (which = find_persister(name)) && which->GetAttr("type", type))
	{
		if (type == "mem")
			return new MemoryPersister;

		string dir("./"), db("persist_db");
		which->GetAttr("dir", dir);
		which->GetAttr("db", db);

		if (sid)
			db += ('.' + sid->get_senderCompID()() + '.' + sid->get_targetCompID()());
		else if (which->FindAttr("use_session_id", false))
			db += ('.' + get_sender_comp_id(from)() + '.' + get_target_comp_id(from)());

#if defined HAVE_BDB
		if (type == "bdb")
		{
			scoped_ptr<BDBPersister> result(new BDBPersister);
			if (result->initialise(dir, db, flag))
				return result.release();
		}
		else
#endif
		if (type == "file")
		{
			scoped_ptr<FilePersister> result(new FilePersister(which->FindAttr("rotation", 0)));
			if (result->initialise(dir, db, flag))
				return result.release();
		}
	}

	return 0;
}

//-------------------------------------------------------------------------------------------------
Logger *Configuration::create_logger(const XmlElement *from, const Logtype ltype, const SessionID *sid) const
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
				which->FindAttrRef("filename", logname);
				trim(logname);

				if (logname[0] == '|')
#ifndef HAVE_POPEN
					throw f8Exception("popen not supported on your platform");
#endif
					return new PipeLogger(logname, get_logflags(which));

				RegMatch match;
				if (_ipexp.SearchString(match, logname, 3) == 3)
				{
					f8String ip, port;
					_ipexp.SubExpr(match, logname, ip, 0, 1);
					_ipexp.SubExpr(match, logname, port, 0, 2);
					BCLogger *bcl(new BCLogger(ip, get_value<unsigned>(port), get_logflags(which)));
					if (*bcl)
						return bcl;
				}

				get_logname(which, logname, sid); // only applies to file loggers
				return new FileLogger(logname, get_logflags(which), get_logfile_rotation(which));
			}
		}
	}

	return 0;
}

//-------------------------------------------------------------------------------------------------
string& Configuration::get_logname(const XmlElement *from, string& to, const SessionID *sid) const
{
	if (sid)
		to += ('.' + sid->get_senderCompID()() + '.' + sid->get_targetCompID()());
	else if (from && from->FindAttr("use_session_id", false))
		to += ('.' + get_sender_comp_id(from)() + '.' + get_target_comp_id(from)());

	return to;
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

//-------------------------------------------------------------------------------------------------
ProcessModel Configuration::get_process_model(const XmlElement *from) const
{
	static const f8String process_strings[] = { "threaded", "pipelined", "coroutine" };
	string pm;
	return from && from->GetAttr("process_model", pm)
		? enum_str_get(pm_count, process_strings, pm, pm_thread) : pm_pipeline; // default to pipelined
}

//-------------------------------------------------------------------------------------------------
#ifdef HAVE_OPENSSL
SslContext Configuration::get_ssl_context(const XmlElement *from) const
{
	SslContext target;
	string name;
	const XmlElement *which;
	if (from && from->GetAttr("ssl_context", name) && (which = find_ssl_context(name)))
	{
		static std::string empty, cipher("ALL:!ADH:!LOW:!EXP:!MD5:@STRENGTH"), relaxed("relaxed");
		target._private_key_file = which->FindAttrRef("private_key_file", empty);
		target._certificate_file = which->FindAttrRef("ceritificte_file", empty);
		target._ca_location = which->FindAttrRef("ca_location", empty);
		target._verification_depth = which->FindAttr("verification_depth", static_cast<int>(defaults::verification_depth));
		target._load_default_cas = which->FindAttr("load_default_cas", false);
		target._cipher_list = which->FindAttrRef("cipher_list", cipher);
		name = which->FindAttrRef("verification_mode", relaxed);
		if (name == "none")
			target._verification_mode = Poco::Net::Context::VERIFY_NONE;
		else if (name == "relaxed")
			target._verification_mode = Poco::Net::Context::VERIFY_RELAXED;
		else if (name == "strict")
			target._verification_mode = Poco::Net::Context::VERIFY_STRICT;
		else if (name == "once")
			target._verification_mode = Poco::Net::Context::VERIFY_ONCE;
		else
			target._verification_mode = SSL_VERIFY_PEER;
		target._valid = true;
	}
	return target;
}
#endif
