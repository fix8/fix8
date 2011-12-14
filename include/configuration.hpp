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
#ifndef _FIX8_CONFIGURATION_HPP_
#define _FIX8_CONFIGURATION_HPP_

//-------------------------------------------------------------------------------------------------
namespace FIX8 {

//-------------------------------------------------------------------------------------------------
class Configuration
{
	std::string _xmlfile;
	XmlEntity *_root;
	typedef std::map<const std::string, XmlEntity *> ConfigMap;
	ConfigMap _sessions, _persisters;
	std::vector<XmlEntity *> _allsessions;

	const XmlEntity *find_element(const std::string& tag, const ConfigMap& from) const
		{ ConfigMap::const_iterator itr(from.find(tag)); return itr != from.end() ? itr->second : 0; }
	const XmlEntity *find_persister(const std::string& tag) const { return find_element(tag, _persisters); }

	unsigned get_unsigned(const std::string& tag, const XmlEntity *from, unsigned def_val=0) const
	{
		std::string val;
		return from->GetAttr(tag, val) ? GetValue<unsigned>(val) : 0;
	}

	const std::string& get_string(const std::string& tag, const XmlEntity *from, std::string& to) const
	{
		from->GetAttr(tag, to);
		return to;
	}

	template<typename T>
	T& get_string_field(const XmlEntity *from, const std::string& tag, T& to) const
	{
		std::string val;
		if (from->GetAttr(tag, val))
			to.set(val);
		return to;
	}

public:
	Configuration(const std::string& xmlfile, bool do_process=false)
		: _xmlfile(xmlfile), _root(XmlEntity::Factory(_xmlfile)) { if (do_process) process(); }
	virtual ~Configuration() { delete _root; }

	int process();
	const XmlEntity *get_session(const unsigned num) const
		{ return num < _allsessions.size() ? _allsessions[num] : 0; }
	const XmlEntity *find_session(const std::string& tag) const { return find_element(tag, _sessions); }

	const Connection::Role get_role(const XmlEntity *from) const;
	bool get_address(const XmlEntity *from, Poco::Net::SocketAddress& to) const;
	const std::string& get_logname(const XmlEntity *from, std::string& to) const
		{ return get_string("logname", from, to); }
	const std::string& get_protocol_logname(const XmlEntity *from, std::string& to) const
		{ return get_string("protocol_logname", from, to); }
	unsigned get_version(const XmlEntity *from) const { return get_unsigned("fix_version", from); }
	unsigned get_heartbeat_interval(const XmlEntity *from) const { return get_unsigned("heartbeat_interval", from); }
	sender_comp_id& get_sender_comp_id(const XmlEntity *from, sender_comp_id& to) const
		{ return get_string_field(from, "sender_comp_id", to); }
	target_comp_id& get_target_comp_id(const XmlEntity *from, target_comp_id& to) const
		{ return get_string_field(from, "target_comp_id", to); }
	Persister *create_persister(const XmlEntity *from) const;
};

//-------------------------------------------------------------------------------------------------

} // FIX8

#endif // _FIX8_CONFIGURATION_HPP_
