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
#ifndef _FIX8_CONFIGURATION_HPP_
#define _FIX8_CONFIGURATION_HPP_

//-------------------------------------------------------------------------------------------------
namespace FIX8 {

//-------------------------------------------------------------------------------------------------
/// Class to encapsulate a Fix8 configuration.
class Configuration
{
	std::string _xmlfile;
	XmlEntity *_root;
	typedef std::map<const std::string, XmlEntity *> ConfigMap;
	ConfigMap _sessions, _persisters;
	std::vector<XmlEntity *> _allsessions;

	/*! Find an xml entity by tag in the supplied map.
	  \param tag the tag to find
	  \param from the map to search
	  \return the found entity or 0 if not found */
	const XmlEntity *find_element(const std::string& tag, const ConfigMap& from) const
		{ ConfigMap::const_iterator itr(from.find(tag)); return itr != from.end() ? itr->second : 0; }

	/*! Find a persister by tag.
	  \param tag the tag to find
	  \return the found entity or 0 if not found */
	const XmlEntity *find_persister(const std::string& tag) const { return find_element(tag, _persisters); }

	/*! Find an unsigned value by tag from an xml entity.
	  \param tag the tag to find
	  \param from the xml entity to search
	  \param def_val value to return if not found
	  \return the found value or the def_val if not found */
	unsigned get_unsigned(const std::string& tag, const XmlEntity *from, unsigned def_val=0) const
	{
		std::string val;
		return from->GetAttr(tag, val) ? GetValue<unsigned>(val) : 0;
	}

	/*! Find a string value by tag from an xml entity.
	  \param tag the tag to find
	  \param from the xml entity to search
	  \param to location to store target
	  \return the target */
	const std::string& get_string(const std::string& tag, const XmlEntity *from, std::string& to) const
	{
		from->GetAttr(tag, to);
		return to;
	}

	/*! Find a typed value by tag from an xml entity.
	  \param tag the tag to find
	  \param from the xml entity to search
	  \param to location to store target
	  \return the target */
	template<typename T>
	T& get_string_field(const XmlEntity *from, const std::string& tag, T& to) const
	{
		std::string val;
		if (from->GetAttr(tag, val))
			to.set(val);
		return to;
	}

public:
	/*! Ctor.
	  \param xmlfile xml config filename.
	  \param do_process if true, process the file on construction */
	Configuration(const std::string& xmlfile, bool do_process=false)
		: _xmlfile(xmlfile), _root(XmlEntity::Factory(_xmlfile)) { if (do_process) process(); }

	/// Dtor.
	virtual ~Configuration() { delete _root; }

	/*! Process the config file.
	  \return the number of sessions processed (found) */
	int process();

	/*! Find a session entity by index.
	  \param num index of session
	  \return the session entity or 0 if not found */
	const XmlEntity *get_session(const unsigned num) const
		{ return num < _allsessions.size() ? _allsessions[num] : 0; }

	/*! Find a session entity by name.
	  \param tag name of session
	  \return the session entity or 0 if not found */
	const XmlEntity *find_session(const std::string& tag) const { return find_element(tag, _sessions); }

	/*! Extract the role from a session entity.
	  \param from xml entity to search
	  \return the connection role or Connection::cn_unknown if not found */
	const Connection::Role get_role(const XmlEntity *from) const;

	/*! Extract the ip address from a session entity.
	  \param from xml entity to search
	  \param to target socket address
	  \return true if found */
	bool get_address(const XmlEntity *from, Poco::Net::SocketAddress& to) const;

	/*! Extract the session log filename address from a session entity.
	  \param from xml entity to search
	  \param to target logfile string
	  \return target string */
	const std::string& get_logname(const XmlEntity *from, std::string& to) const
		{ return get_string("logname", from, to); }

	/*! Extract the protocol log filename address from a session entity.
	  \param from xml entity to search
	  \param to target logfile string
	  \return target string */
	const std::string& get_protocol_logname(const XmlEntity *from, std::string& to) const
		{ return get_string("protocol_logname", from, to); }

	/*! Extract the FIX version from a session entity.
	  \param from xml entity to search
	  \return the FIX version or 0 if not found */
	unsigned get_version(const XmlEntity *from) const { return get_unsigned("fix_version", from); }

	/*! Extract the heartbeat interval from a session entity.
	  \param from xml entity to search
	  \return the heartbeat interval version or 0 if not found */
	unsigned get_heartbeat_interval(const XmlEntity *from) const { return get_unsigned("heartbeat_interval", from); }

	/*! Extract sendercompid from a session entity.
	  \param from xml entity to search
	  \param to target sender_comp_id string
	  \return target sender_comp_id */
	sender_comp_id& get_sender_comp_id(const XmlEntity *from, sender_comp_id& to) const
		{ return get_string_field(from, "sender_comp_id", to); }

	/*! Extract targetcompid from a session entity.
	  \param from xml entity to search
	  \param to target target_comp_id string
	  \return target target_comp_id */
	target_comp_id& get_target_comp_id(const XmlEntity *from, target_comp_id& to) const
		{ return get_string_field(from, "target_comp_id", to); }

	/*! Create a new persister object a session entity.
	  \return new persister or 0 if unable to create */
	Persister *create_persister(const XmlEntity *from) const;
};

//-------------------------------------------------------------------------------------------------

} // FIX8

#endif // _FIX8_CONFIGURATION_HPP_
