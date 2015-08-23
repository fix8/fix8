//-----------------------------------------------------------------------------------------
/*

Fix8 is released under the GNU LESSER GENERAL PUBLIC LICENSE Version 3.

Fix8 Open Source FIX Engine.
Copyright (C) 2010-15 David L. Dight <fix@fix8.org>

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
#include "precomp.hpp"
#include <fix8/f8config.h>
#ifdef FIX8_HAVE_OPENSSL
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
	if (!_root)
		throw ConfigurationError("could not create root xml entity");

	using item = tuple<f8String, group_types, bool>;
	static const item items[]
	{
		{ item("session", g_sessions, true) },					{ item("log", g_loggers, false) },
		{ item("server_group", g_server_group, false) },	{ item("client_group", g_client_group, false) },
		{ item("ssl_context", g_ssl_context, false) },		{ item("schedule", g_schedules, false) },
		{ item("login", g_logins, false) },						{ item("persist", g_persisters, false) }
	};

	for (auto &pp : items)
		if (!load_map("fix8/" + get<0>(pp), _groups[get<1>(pp)], get<2>(pp)) && get<2>(pp))
			throw ConfigurationError("could not locate server session in configuration", get<0>(pp));

	return static_cast<int>(_groups[g_sessions].size());
}

//-------------------------------------------------------------------------------------------------
Connection::Role Configuration::get_role(const XmlElement *from) const
{
	string role;
	return from_or_default(from, "role", role)
		? role % "initiator" ? Connection::cn_initiator
		: role % "acceptor" ? Connection::cn_acceptor : Connection::cn_unknown : Connection::cn_unknown;
}

//-------------------------------------------------------------------------------------------------
Poco::Net::SocketAddress Configuration::get_address(const XmlElement *from) const
{
	Poco::Net::SocketAddress to;
	string ip, port;
	if (from_or_default(from, "ip", ip) && from_or_default(from, "port", port))
		to = Poco::Net::SocketAddress(ip, port);

	return to;
}

//-------------------------------------------------------------------------------------------------
Poco::Net::IPAddress Configuration::get_ip(const XmlElement *from) const
{
	Poco::Net::IPAddress to;
	string ip;
	if (from_or_default(from, "ip", ip))
		to = Poco::Net::IPAddress(ip);

	return to;
}

//-------------------------------------------------------------------------------------------------
size_t Configuration::get_addresses(const XmlElement *from, vector<Server>& target) const
{
	string name;
	const XmlElement *which;
	if (from_or_default(from, "server_group", name) && (which = find_group(g_server_group, name)))
	{
		XmlElement::XmlSet slist;
		if (which->find("server_group/server", slist))
		{
			const Poco::Net::SocketAddress empty_addr;
			for(const auto *pp : slist)
			{
				string name;
				Poco::Net::SocketAddress addr(get_address(pp));
				if (pp->GetAttr("name", name) && addr != empty_addr && pp->FindAttr("active", true))
					target.push_back(Server(name, pp->FindAttr("max_retries", static_cast<int>(defaults::login_retries)), addr,
						pp->FindAttr("reset_sequence_numbers", false)));
			}
		}
		return target.size();
	}

	return 0;
}

//-------------------------------------------------------------------------------------------------
Schedule Configuration::create_schedule(const XmlElement *which) const
{
	Tickval start(get_time_field(which, "start_time", true));
	if (!start.is_errorval())
	{
		const int utc_offset(which->FindAttr("utc_offset_mins", 0)); // utc_offset is in minutes
		const unsigned duration(which->FindAttr("duration", 0));
		Tickval end(get_time_field(which, "end_time", true));

		if (end.is_errorval())
		{
			if (duration) // duration is in minutes
				end = start.get_ticks() + duration * Tickval::minute;
		}
		else
		{
			if (end <= start)
				throw ConfigurationError("Schedule end time cannot be equal to or before session start time");
		}
		string daytmp;
		const int start_day(which->GetAttr("start_day", daytmp) ? decode_dow(daytmp) : -1);
		const int end_day(which->GetAttr("end_day", daytmp) ? decode_dow(daytmp) : start_day < 0 ? -1 : start_day);
		return {start, end, Tickval(static_cast<Tickval::ticks>(duration)), utc_offset, start_day, end_day};
	}

	return {};
}

//-------------------------------------------------------------------------------------------------
Session_Schedule *Configuration::create_session_schedule(const XmlElement *from) const
{
	string name;
	const XmlElement *which(0);
	if (from_or_default(from, "schedule", name) && (which = find_group(g_schedules, name)))
	{
		Schedule sch(create_schedule(which));
		f8String reject_text("Business messages are not accepted now.");
		which->GetAttr("reject_text", reject_text); // can't use FindAttr since istream is delimeter sensitive
		const int reject_code(which->FindAttr("reject_code", 0));
		return new Session_Schedule(sch, reject_code, reject_text);
	}

	return nullptr;
}

//-------------------------------------------------------------------------------------------------
Schedule Configuration::create_login_schedule(const XmlElement *from) const
{
	string name;
	const XmlElement *which;
	return from_or_default(from, "login", name) && (which = find_group(g_logins, name))
		? create_schedule(which) : Schedule();
}

//-------------------------------------------------------------------------------------------------
Persister *Configuration::create_persister(const XmlElement *from, const SessionID *sid, bool flag) const
{
	string name, type;
	const XmlElement *which;
	if (from_or_default(from, "persist", name) && (which = find_group(g_persisters, name)) && which->GetAttr("type", type))
	{
		if (type == "mem")
			return new MemoryPersister;

		string dir("./"), db("persist_db");
		which->GetAttr("dir", dir);
		which->GetAttr("db", db) || which->GetAttr("session_prefix", db);

		if (sid)
			db += ('.' + sid->get_senderCompID()() + '.' + sid->get_targetCompID()());
		else if (which->FindAttr("use_session_id", false))
			db += ('.' + get_sender_comp_id(from)() + '.' + get_target_comp_id(from)());

#if defined FIX8_HAVE_LIBMEMCACHED
		if (type == "memcached")
		{
			string config_str;
			if (which->GetAttr("config_string", config_str))
			{
				unique_ptr<MemcachedPersister> result(new MemcachedPersister);
				if (result->initialise(config_str, db, flag))
					return result.release();
			}
			else
				throw ConfigurationError("memcached: config_string attribute must be given when using memcached");
		}
		else
#endif
#if defined FIX8_HAVE_LIBHIREDIS
		if (type == "redis")
		{
			string host_str;
			if (which->GetAttr("host", host_str))
			{
				unique_ptr<HiredisPersister> result(new HiredisPersister);
				if (result->initialise(host_str, which->FindAttr("port", 6379),
					which->FindAttr("connect_timeout", static_cast<unsigned>(defaults::connect_timeout)), db, flag))
						return result.release();
			}
			else
				throw ConfigurationError("redis: host attribute must be given when using redis");
		}
		else
#endif
#if defined FIX8_HAVE_BDB
		if (type == "bdb")
		{
			unique_ptr<BDBPersister> result(new BDBPersister);
			if (result->initialise(dir, db, flag))
				return result.release();
		}
		else
#endif
		if (type == "file")
		{
			unique_ptr<FilePersister> result(new FilePersister(which->FindAttr("rotation", 0)));
			if (result->initialise(dir, db, flag))
				return result.release();
		}
	}

	return nullptr;
}

//-------------------------------------------------------------------------------------------------
Logger *Configuration::create_logger(const XmlElement *from, const Logtype ltype, const SessionID *sid) const
{
	string name;
	if (from_or_default(from, ltype == session_log ? "session_log" : "protocol_log", name))
	{
		const XmlElement *which(find_group(g_loggers, name));
		if (which)
		{
			Logger::LogPositions positions;
			string type;

			if (which->GetAttr("type", type)
				&& ((type % "session" && ltype == session_log) || (type % "protocol" && ltype == protocol_log)))
			{
				string logname("logname_not_set.log"), levstr, delim(" ");
				which->FindAttrRef("filename", logname);
				trim(logname);
				if (which->GetAttr("delimiter", delim) && delim.size() > 2) // "|" or "<>" or "{}" etc
					throw ConfigurationError("invalid logging field delimiter");

				which->GetAttr("levels", levstr);
				const Logger::Levels levels(levstr.empty() || levstr % "All"
						? Logger::Levels(Logger::All) : levstr % "None"
						? Logger::Levels(Logger::None) : get_logflags<Logger::Levels>("levels", Logger::_level_names, which));

				const Logger::LogFlags flags(which->HasAttr("flags")
					? get_logflags<Logger::LogFlags>("flags", Logger::_bit_names, which, &positions) : Logger::LogFlags(Logger::StdFlags));

				if (logname[0] == '|')
				{
#ifndef FIX8_HAVE_POPEN
					throw ConfigurationError("popen not supported on your platform");
#endif
					return new PipeLogger(logname, flags, levels, delim, positions);
				}

				RegMatch match;
				if (_ipexp.SearchString(match, logname, 3) == 3)
				{
					f8String ip, port;
					_ipexp.SubExpr(match, logname, ip, 0, 1);
					_ipexp.SubExpr(match, logname, port, 0, 2);
					return new BCLogger(ip, stoul(port), flags, levels, delim, positions);
				}

				get_logname(which, logname, sid); // only applies to file loggers
				if (flags & Logger::xml)
					return new XmlFileLogger(logname, flags, levels, delim, positions, get_logfile_rotation(which));
				return new FileLogger(logname, flags, levels, delim, positions, get_logfile_rotation(which));
			}
		}
	}

	return nullptr;
}

//-------------------------------------------------------------------------------------------------
Clients Configuration::create_clients(const XmlElement *from) const
{
	string name;
	const XmlElement *which;
	Clients clients;
	if (from_or_default(from, "target_comp_id", name) && (which = find_group(g_client_group, name)))
	{
		XmlElement::XmlSet slist;
		if (which->find("client_group/client", slist))
		{
			// <client name="Goldsteins" target_comp_id="DLD_TEX" ip="192.168.0.17" active="true" />
			for(const auto *pp : slist)
			{
				string name, tci;
				const Poco::Net::IPAddress addr(get_ip(pp));
				if (pp->GetAttr("name", name) && pp->GetAttr("target_comp_id", tci) && pp->FindAttr("active", true))
					if (!clients.insert({tci, Client(name, addr)}).second)
						throw ConfigurationError("Failed to add client from client_group", tci);
			}
		}
	}

	return clients;
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
template<typename T>
T Configuration::get_logflags(const string& tag, const vector<string>& names,
	const XmlElement *from, Logger::LogPositions *positions) const
{
	T flags;
	string flags_str;
	if (from && from->GetAttr(tag, flags_str))
	{
		istringstream istr(flags_str);
		for(char extr[32]; !istr.get(extr, sizeof(extr), '|').fail(); istr.ignore(1))
		{
			string result(extr);
			const int evalue(flags.set(names, trim(result), true, true));
			if (positions && evalue >= 0)
				positions->push_back(evalue);
		}
	}

	return flags;
}

//-------------------------------------------------------------------------------------------------
unsigned Configuration::get_all_sessions(vector<const XmlElement *>& target, const Connection::Role role) const
{
	for (const auto *pp : _allsessions)
		if (role == Connection::cn_unknown || get_role(pp) == role)
			target.push_back(pp);
	return static_cast<unsigned>(target.size());
}

//-------------------------------------------------------------------------------------------------
ProcessModel Configuration::get_process_model(const XmlElement *from) const
{
	static const vector<f8String> process_strings { "threaded", "pipelined", "coroutine" };
	string pm;
	return from_or_default(from, "process_model", pm)
		? enum_str_get(process_strings, pm, pm_thread) : pm_pipeline; // default to pipelined
}

//-------------------------------------------------------------------------------------------------
#ifdef FIX8_HAVE_OPENSSL
SslContext Configuration::get_ssl_context(const XmlElement *from) const
{
	SslContext target;
	string name;
	const XmlElement *which;
	if (from_or_default(from, "ssl_context", name) && (which = find_group(g_ssl_context, name)))
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
