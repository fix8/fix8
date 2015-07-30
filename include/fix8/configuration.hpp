//-------------------------------------------------------------------------------------------------
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
//-------------------------------------------------------------------------------------------------
#ifndef FIX8_CONFIGURATION_HPP_
#define FIX8_CONFIGURATION_HPP_

#ifdef FIX8_HAVE_OPENSSL
#include <openssl/ssl.h>
#else
#define SSL_VERIFY_PEER 0
#endif

//-------------------------------------------------------------------------------------------------
namespace FIX8 {

//-------------------------------------------------------------------------------------------------
/// Class to hold server settings for failoverable sessions
struct Server
{
	std::string _hostname;
	unsigned _max_retries, _retries;
	Poco::Net::SocketAddress _addr;
	bool _reset_sequence_numbers;

	Server(const std::string& hostname, unsigned max_retries, const Poco::Net::SocketAddress& addr, bool reset_sequence_numbers)
		: _hostname(hostname), _max_retries(max_retries), _retries(), _addr(addr), _reset_sequence_numbers(reset_sequence_numbers) {}
};

//-------------------------------------------------------------------------------------------------
/// Class to hold SSL context for failoverable sessions
struct SslContext
{
	std::string _private_key_file; ///< privateKeyFile contains the path to the private key file used for encryption.
											 ///		Can be empty if no private key file is used.
	std::string _certificate_file; ///< certificateFile contains the path to the certificate file (in PEM format).
											 ///		If the private key and the certificate are stored in the same file, this
											 ///		can be empty if privateKeyFile is given.
	std::string _ca_location;		 ///< caLocation contains the path to the file or directory containing the
											 ///		CA/root certificates. Can be empty if the OpenSSL builtin CA certificates
											  ///		 are used (see loadDefaultCAs).
	int _verification_mode;			 ///< none, relaxed, strict, once
	int _verification_depth;		 ///< 9
	bool _load_default_cas;			 ///< false
	std::string _cipher_list;		 ///< "ALL:!ADH:!LOW:!EXP:!MD5:@STRENGTH"
	bool _valid;

	SslContext(const std::string& private_key_file="", const std::string& certificate_file="", const std::string& ca_location="",
				  int verification_mode=SSL_VERIFY_PEER, int verification_depth=9, bool load_default_cas=false,
				  const std::string& cipher_list="ALL:!ADH:!LOW:!EXP:!MD5:@STRENGTH")
		: _private_key_file(private_key_file), _certificate_file(certificate_file), _ca_location(ca_location),
		  _verification_mode(verification_mode), _verification_depth(verification_depth), _load_default_cas(load_default_cas),
		  _cipher_list(cipher_list), _valid()
	{
	}
};

//-------------------------------------------------------------------------------------------------
/// Class to encapsulate a Fix8 configuration.
class Configuration
{
	static RegExp _ipexp;

	const XmlElement *_root, *_default;
	using ConfigMap = std::map<const std::string, const XmlElement *>;
	std::vector<ConfigMap> _groups;
	std::vector<const XmlElement *> _allsessions;

	/*! Find an xml entity by tag in the supplied map.
	  \param tag the tag to find
	  \param from the map to search
	  \return the found entity or 0 if not found */
	const XmlElement *find_element(const std::string& tag, const ConfigMap& from) const
		{ ConfigMap::const_iterator itr(from.find(tag)); return itr != from.end() ? itr->second : nullptr; }

public:
	enum group_types
	{
		g_sessions, g_persisters, g_loggers, g_server_group,
		g_ssl_context, g_schedules, g_logins, g_client_group,
		g_count
	};

	/*! Find an element in a specified group.
	  \param type group type enum
	  \param tag the tag to find
	  \return the found entity or 0 if not found */
	const XmlElement *find_group(group_types type, const std::string& tag) const
		{ return find_element(tag, _groups[type]); }

protected:
	/*! Search the given element for a tag or look in the default element
	  \param from the xml entity to search
	  \param tag the tag to find
	  \param target place to put the resul
	  \return true if found and stored */
	bool from_or_default(const XmlElement *from, const f8String& tag, f8String& target) const
		{ return (from && from->GetAttr(tag, target)) || (_default && _default->GetAttr(tag, target)); }

	/*! Find a fix8 field typed value by tag from an xml entity.
	  \tparam location type
	  \param from the xml entity to search
	  \param tag the tag to find
	  \param to location to store target
	  \return the target */
	template<typename T>
	T& get_string_field(const XmlElement *from, const std::string& tag, T& to) const
	{
		std::string val;
		if (from_or_default(from, tag, val))
			to.set(val);
		return to;
	}

	/*! Find a session time field by tag from an xml entity.
	  \param from the xml entity to search
	  \param tag the tag to find
	  \param timeonly if true, only use the time part
	  \return Tickval::ticks time or errorticks if not found */
	Tickval::ticks get_time_field(const XmlElement *from, const std::string& tag, bool timeonly=false) const
	{
		std::string time_str;
		return from && from->GetAttr(tag, time_str) && time_str.size() == 8
			? time_parse(time_str.c_str(), 8, timeonly) : Tickval::errorticks();
	}

	/*! Find an attribute in the given XmlElement
	  \tparam default type
	  \param from the xml entity to search
	  \param tag the tag to find
	  \param def the default value if not found
	  \return the found attribute value or the default value if not found */
	template<typename T>
	T find_or_default(const XmlElement *from, const std::string& tag, const T def) const
	{
		if (from)
		{
			if (from->HasAttr(tag))
				return from->FindAttr(tag, def);
			if (_default)
				return _default->FindAttr(tag, def);
		}
		return def;
	}

private:
	/*! Load a repeating group into a supplied map.
	  \param tag the tag to find
	  \param map_name the target map
	  \param is_session if true, special case for session map
	  \return the number of elements inserted */
	unsigned load_map(const std::string& tag, ConfigMap& map_name, const bool is_session=false)
	{
		XmlElement::XmlSet slist;
		if (_root->find(tag, slist))
		{
			for(const auto *pp : slist)
			{
				std::string name;
				if (pp->GetAttr("name", name) && is_session ? pp->FindAttr("active", false) : true)
				{
					map_name.insert({name, pp});
					if (is_session)
						_allsessions.push_back(pp);
				}
			}
		}

		return static_cast<unsigned>(map_name.size());
	}

public:
	enum Logtype { session_log, protocol_log };

 	/*! Ctor.
	  \param xmlfile xml config filename.
	  \param do_process if true, process the file on construction */
	Configuration(const std::string& xmlfile, bool do_process=false)
		: _root(XmlElement::Factory(xmlfile)),
		_default(_root ? _root->find("fix8/default") : nullptr),
		_groups(g_count)
	{
		if (!exist(xmlfile))
			throw ConfigurationError("config file not found", xmlfile);
		if (do_process)
			process();
	}

	/*! Ctor.
	  \param istr xml stream
	  \param do_process if true, process the stream on construction */
	Configuration(std::istream& istr, bool do_process=false)
		: _root(XmlElement::Factory(istr, "stream")),
		_default(_root ? _root->find("fix8/default") : nullptr),
		_groups(g_count)
	{
		if (do_process)
			process();
	}

	/// Dtor.
	virtual ~Configuration() {}

	/*! Process the config file.
	  \return the number of sessions processed (found) */
	F8API int process();

	/*! Find a session entity by index.
	  \param num index of session
	  \return the session entity or 0 if not found */
	const XmlElement *get_session(const unsigned num) const
		{ return num < _allsessions.size() ? _allsessions[num] : nullptr; }

	/*! Extract the role from a session entity.
	  \param from xml entity to search
	  \return the connection role or Connection::cn_unknown if not found */
	F8API Connection::Role get_role(const XmlElement *from) const;

	/*! Extract the ip addresses from a server_group entity.
	  \param from xml entity to search
	  \param target target vector of Server to store addresses
	  \return number of addresses stored */
	F8API size_t get_addresses(const XmlElement *from, std::vector<Server>& target) const;

	/*! Extract the ip address and port from a session entity.
	  \param from xml entity to search
	  \return Poco::Net::SocketAddress */
	F8API Poco::Net::SocketAddress get_address(const XmlElement *from) const;

	/*! Extract the ip address from a session entity.
	  \param from xml entity to search
	  \return Poco::Net::IPAddress */
	F8API Poco::Net::IPAddress get_ip(const XmlElement *from) const;

	/*! Extract the logflags from the flags attribute in a log entity.
	  \tparam T ebitset type
	  \param tag attribute tag to search for
	  \param names vector of names for each enumeration
	  \param from xml entity to search
	  \param positions vector to place enumeration position in
	  \return LogFLags object */
	template<typename T>
	T get_logflags(const std::string& tag, const std::vector<std::string>& names,
		const XmlElement *from, Logger::LogPositions *positions=nullptr) const;

	/*! Extract the session log filename address from a session entity.
	  \param from xml entity to search
	  \param to target logfile string
	  \param sid optional session id to build name from
	  \return target string */
	F8API std::string& get_logname(const XmlElement *from, std::string& to, const SessionID *sid=nullptr) const;

	/*! Extract the connect_timeout interval (sec) from a session entity.
	  \param from xml entity to search
	  \param def default value if not found
	  \return the connect_timeout wait 10 if not found */
	unsigned get_connect_timeout(const XmlElement *from, const unsigned def=defaults::connect_timeout) const
		{ return find_or_default(from, "connect_timeout", def); }

	/*! Extract the login retry wait interval (ms) from a session entity.
	  \param from xml entity to search
	  \param def default value if not found
	  \return the retry wait interval or 5000 if not found */
	unsigned get_retry_interval(const XmlElement *from, const unsigned def=defaults::retry_interval) const
		{ return find_or_default(from, "login_retry_interval", def); }

	/*! Extract the login retry count from a session entity.
	  \param from xml entity to search
	  \param def default value if not found
	  \return the retry count or 10 if not found */
	unsigned get_retry_count(const XmlElement *from, const unsigned def=defaults::login_retries) const
		{ return find_or_default(from, "login_retries", def); }

	/*! Extract the tcp recv buffer size
	  \param from xml entity to search
	  \param def default value if not found
	  \return the recv buffer size count or 0 if the default should be used */
	unsigned get_tcp_recvbuf_sz(const XmlElement *from, const unsigned def=0) const
		{ return find_or_default(from, "tcp_recv_buffer", def); }

	/*! Extract the tcp send buffer size
	  \param from xml entity to search
	  \param def default value if not found
	  \return the send buffer size count or 0 if the default should be used */
	unsigned get_tcp_sendbuf_sz(const XmlElement *from, const unsigned def=0) const
		{ return find_or_default(from, "tcp_send_buffer", def); }

	/*! Extract the FIX version from a session entity.
	  \param from xml entity to search
	  \param def default value if not found
	  \return the FIX version or 0 if not found */
	unsigned get_version(const XmlElement *from, const unsigned def=0) const
		{ return find_or_default(from, "fix_version", def); }

	/*! Extract the Message printer tabsize from a session entity.
	  \param from xml entity to search
	  \param def default value if not found
	  \return the tabsize version or defaults::tabsize if not found */
	unsigned get_tabsize(const XmlElement *from, const unsigned def=defaults::tabsize) const
		{ return find_or_default(from, "tabsize", def); }

	/*! Extract the logfile rotation count.
	  \param from xml entity to search
	  \param def default value if not found
	  \return the logfile rotation value or 5 if not found */
	unsigned get_logfile_rotation(const XmlElement *from, const unsigned def=defaults::log_rotation) const
		{ return find_or_default(from, "rotation", def); }

	/*! Extract the heartbeat interval from a session entity.
	  \param from xml entity to search
	  \param def default value if not found
	  \return the heartbeat interval version or 0 if not found */
	unsigned get_heartbeat_interval(const XmlElement *from, const unsigned def=defaults::hb_interval) const
		{ return find_or_default(from, "heartbeat_interval", def); }

	/*! Extract the tcp nodelay flag.
	  \param from xml entity to search
	  \param def default value if not found
	  \return false if nodelay flag was passed and was false */
	bool get_tcp_nodelay(const XmlElement *from, const bool def=true) const
		{ return find_or_default(from, "tcp_nodelay", def); }

	/*! Extract the tcp keepalive flag.
	  \param from xml entity to search
	  \param def default value if not found
	  \return false if keepalive flag was passed and was false */
	bool get_tcp_keepalive(const XmlElement *from, const bool def=false) const
		{ return find_or_default(from, "tcp_keepalive", def); }

	/*! Extract the tcp reuseaddr flag.
	  \param from xml entity to search
	  \param def default value if not found
	  \return false if reuseaddr flag was passed and was false */
	bool get_tcp_reuseaddr(const XmlElement *from, const bool def=false) const
		{ return find_or_default(from, "tcp_reuseaddr", def); }

	/*! Extract the socket linger setting from a session entity.
	  \param from xml entity to search
	  \param def default value if not found
	  \return the socket linger value (secs) */
	int get_tcp_linger(const XmlElement *from, const int def=-1) const
		{ return find_or_default(from, "tcp_linger", def); }

	/*! Extract the silent disconnect flag.
	  \param from xml entity to search
	  \param def default value if not found
	  \return true if silent_disconnect flag was passed and was true */
	bool get_silent_disconnect(const XmlElement *from, const bool def=false) const
		{ return find_or_default(from, "silent_disconnect", def); }

	/*! Extract the enforce_compids flag. When false, compids are not checked.
	  \param from xml entity to search
	  \param def default value if not found
	  \return true if enforce_compids flag was passed and was true */
	bool get_enforce_compids_flag(const XmlElement *from, const bool def=true) const
		{ return find_or_default(from, "enforce_compids", def); }

	/*! Extract the ignore_logon_sequence_check flag from a session entity.
	  \param from xml entity to search
	  \param def default value if not found
	  \return true if ignore_logon_sequence_check flag was passed and was true */
	bool get_ignore_logon_sequence_check_flag(const XmlElement *from, const bool def=false) const
		{ return find_or_default(from, "ignore_logon_sequence_check", def); }

	/*! Extract the get_no_chksum_flag flag from a session entity.
	  \param from xml entity to search
	  \param def default value if not found
	  \return true if get_no_chksum_flag flag was passed and was true */
	bool get_no_chksum_flag(const XmlElement *from, const bool def=false) const
		{ return find_or_default(from, "no_chksum", def); }

	/*! Extract the get_permissive_mode_flag flag from a session entity.
	  \param from xml entity to search
	  \param def default value if not found
	  \return true if get_permissive_mode_flag flag was passed and was true */
	bool get_permissive_mode_flag(const XmlElement *from, const bool def=false) const
		{ return find_or_default(from, "permissive_mode", def); }

	/*! Extract the reset_sequence_number flag from a session entity.
	  \param from xml entity to search
	  \param def default value if not found
	  \return true if reset_sequence_number flag was passed and was true */
	bool get_reset_sequence_number_flag(const XmlElement *from, const bool def=false) const
		{ return find_or_default(from, "reset_sequence_numbers", def); }

	/*! Extract the always_seqnum_assign flag from a session entity.
	  \param from xml entity to search
	  \param def default value if not found
	  \return true if always_seqnum_assign flag was passed and was true */
	bool get_always_seqnum_assign(const XmlElement *from, const bool def=false) const
		{ return find_or_default(from, "always_seqnum_assign", def); }

	/*! Extract process model.
	  \param from xml entity to search
	  \return pm_thread, pm_pipeline or pm_coro */
	F8API ProcessModel get_process_model(const XmlElement *from) const;

	/*! Extract default_appl_ver_id from a session entity.
	  \param from xml entity to search
	  \return target default_appl_ver_id */
	default_appl_ver_id get_default_appl_ver_id(const XmlElement *from) const
		{ default_appl_ver_id to; return get_string_field(from, "default_appl_ver_id", to); }

	/*! Extract sendercompid from a session entity.
	  \param from xml entity to search
	  \return target sender_comp_id */
	sender_comp_id get_sender_comp_id(const XmlElement *from) const
		{ sender_comp_id to; return get_string_field(from, "sender_comp_id", to); }

	/*! Extract targetcompid from a session entity.
	  \param from xml entity to search
	  \return target target_comp_id */
	target_comp_id get_target_comp_id(const XmlElement *from) const
		{ target_comp_id to; return get_string_field(from, "target_comp_id", to); }

#ifdef FIX8_HAVE_OPENSSL
	/*! Extract the SSL context from a ssl_context entity.
	  \param from xml entity to search
	  \return ssl context */
	F8API SslContext get_ssl_context(const XmlElement *from) const;
#endif

	/*! Create a new persister object from a session entity.
	  \param from xml entity to search
	  \param sid optional session id to build name from
	  \param flag additional flag for persister use
	  \return new persister or 0 if unable to create */
	F8API Persister *create_persister(const XmlElement *from, const SessionID *sid=nullptr, bool flag=false) const;

	/*! Create a new logger object from a session entity.
	  \param from xml entity to search
	  \param ltype log type
	  \param sid optional session id to build name from
	  \return new logger or 0 if unable to create */
	F8API Logger *create_logger(const XmlElement *from, const Logtype ltype, const SessionID *sid=nullptr) const;

	/*! Create schedule object from a session entity.
	  \param from xml entity to search
	  \return Schedule */
	F8API Schedule create_schedule(const XmlElement *from) const;

	/*! Create clients object from a session entity.
	  \param from xml entity to search
	  \return Clients */
	F8API Clients create_clients(const XmlElement *from) const;

	/*! Create login schedule object from a session entity.
	  \param from xml entity to search
	  \return login Schedule */
	F8API Schedule create_login_schedule(const XmlElement *from) const;

	/*! Create a new session schedule object from a session entity.
	  \param from xml entity to search
	  \return new Session_Schedule or 0 if unable to create */
	F8API Session_Schedule *create_session_schedule(const XmlElement *from) const;

	/*! Get all active sessions that have been read; filter by role if desired.
	  \param target vector to place results
	  \param role role to filter (cn_unknown means all)
	  \return number of sessions found */
	F8API unsigned get_all_sessions(std::vector<const XmlElement *>& target, const Connection::Role role=Connection::cn_unknown) const;

	/*! Return ptr to the root XmlElement
	  \return root element */
	const XmlElement *get_root() const { return _root; }
};

//-------------------------------------------------------------------------------------------------

} // FIX8

#endif // FIX8_CONFIGURATION_HPP_
