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

-------------------------------------------------------------------------------------------
$Id$
$Date$
$URL$

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
int Configuration::process()
{
	if (!exist(_xmlfile))
		throw f8Exception("server config file not found", _xmlfile);
	if (!_root)
		throw f8Exception("could not create root xml entity");

	XmlEntity::XmlSet slist;
	if (!_root->find("fix8/session", slist))
		throw f8Exception("could not locate server session in config file", _xmlfile);

	for(XmlEntity::XmlSet::const_iterator itr(slist.begin()); itr != slist.end(); ++itr)
	{
		string name;
		if ((*itr)->GetAttr("name", name) && (*itr)->FindAttr("active", false))
		{
			_sessions.insert(ConfigMap::value_type(name, *itr));
			_allsessions.push_back(*itr);
		}
	}

	slist.clear();
	if (_root->find("fix8/persist", slist))
	{
		for(XmlEntity::XmlSet::const_iterator itr(slist.begin()); itr != slist.end(); ++itr)
		{
			string name;
			if ((*itr)->GetAttr("name", name))
				_persisters.insert(ConfigMap::value_type(name, *itr));
		}
	}

	return _sessions.size();
}

//-------------------------------------------------------------------------------------------------
const Connection::Role Configuration::get_role(const XmlEntity *from) const
{
	std::string role;
	return from->GetAttr("role", role) ? role % "initiator" ? Connection::cn_initiator
		: role % "acceptor" ? Connection::cn_acceptor : Connection::cn_unknown : Connection::cn_unknown;
}

//-------------------------------------------------------------------------------------------------
bool Configuration::get_address(const XmlEntity *from, Poco::Net::SocketAddress& to) const
{
	std::string ip, port;
	if (from->GetAttr("ip", ip) && from->GetAttr("port", port))
	{
		to = Poco::Net::SocketAddress(ip, port);
		return true;
	}

	return false;
}

//-------------------------------------------------------------------------------------------------
Persister *Configuration::create_persister(const XmlEntity *from) const
{
	string name;
	if (!from->GetAttr("persist", name))
		return 0;
	const XmlEntity *which(find_persister(name));
	if (!which)
		return 0;
	string type;
	if (!which->GetAttr("type", type))
		return 0;
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
	return 0;
}

