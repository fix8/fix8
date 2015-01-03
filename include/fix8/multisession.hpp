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
#ifndef FIX8_MULTISESSSION_HPP_
#define FIX8_MULTISESSSION_HPP_

//-------------------------------------------------------------------------------------------------
namespace FIX8 {

//-------------------------------------------------------------------------------------------------
/// Multi Server Manager.
class ServerManager
{
	std::map<Poco::Net::ServerSocket, ServerSessionBase *> _servermap;

public:
	/// Ctor.
	ServerManager() = default;

	/// Dtor.
	virtual ~ServerManager()
		{ std::for_each (_servermap.begin(), _servermap.end(), [](decltype(_servermap)::value_type& pp){ delete pp.second; }); }

	/*! Add a ServerSession to the manager; takes ownership of the object
	  \param what ServerSession to add
	  \return true if successful */
	bool add(ServerSessionBase *what)
	{
		if (!what || !what->_server_sock)
			throw f8Exception("bad or missing server socket");
		_servermap.insert({*what->_server_sock, what});
		return true;
	}

	/*! Check to see if there are any waiting inbound connections on any of the managed servers. Will only
		return the first ServerSession; select should be called continually to process all waiting sockets.
	  \param timeout timespan (us, default 250 ms) to wait before returning (will return immediately if connection available)
	  \return pointer to ServerSession that is ready to accept a connection or nullptr if timeout with none */
	ServerSessionBase *select(const Poco::Timespan& timeout=Poco::Timespan(250000)) const
	{
		Poco::Net::Socket::SocketList readList, writeList, exceptList;
		for (auto& pp : _servermap)
			readList.push_back(pp.first);
		return Poco::Net::Socket::select(readList, writeList, exceptList, timeout) && !readList.empty()
			? _servermap.find(readList[0])->second : nullptr;
	}

	/*! Check to see if there are any waiting inbound connections on any of the managed servers. Will add
		all ServerSession pointers to the supplied vector for all waiting sockets found.
	  \param result vector to place results in; will empty before polling
	  \param timeout timespan (us, default 250 ms) to wait before returning (will return immediately if connection available)
	  \return number of active ServerSockets returned in vector */
	size_t select_l(std::vector<ServerSessionBase *>& result, const Poco::Timespan& timeout=Poco::Timespan(250000)) const
	{
		result.clear();
		Poco::Net::Socket::SocketList readList, writeList, exceptList;
		for (auto& pp : _servermap)
			readList.push_back(pp.first);
		if (Poco::Net::Socket::select(readList, writeList, exceptList, timeout) && !readList.empty())
			std::for_each(readList.begin(), readList.end(), [&](decltype(readList)::value_type& pp)
				{ result.push_back(_servermap.find(pp)->second); });
		return result.size();
	}

	/*! Get the number of server sessions being manager by this object
	  \return count of server sessions */
	unsigned size() const { return static_cast<unsigned>(_servermap.size()); }
};

//-------------------------------------------------------------------------------------------------
/// Session Manager.
template <typename T>
class SessionManager
{
	f8_mutex _mutex;
	std::map<f8String, T*> _sessionmap;

public:
	/// Ctor.
	SessionManager() = default;

	/// Dtor.
	virtual ~SessionManager()
	{
		f8_scoped_lock guard(_mutex);
		std::for_each (_sessionmap.begin(), _sessionmap.end(), [](typename decltype(_sessionmap)::value_type& pp){ delete pp.second; });
	}

	/*! Add a T* to the manager; takes ownership of the object
	  \param name unique name for this session
	  \param what T* to add
	  \return true if successful */
	bool add(const f8String& name, T *what)
	{
		if (!what)
			throw f8Exception("bad or missing session");
		f8_scoped_lock guard(_mutex);
		return _sessionmap.insert({name, what}).second;
	}

	/*! Call supplied function on each T* in the manager
	  \param func std::function to call, which returns true on success
	  \return nullptr if all sessions executed successfully, or ptr to session that failed */
	T *for_each_if(std::function<bool(T*)> func)
	{
		for (auto& pp : _sessionmap)
		{
			f8_scoped_lock guard(_mutex);
			if (!func(pp.second))
				return pp.second;
		}
		return nullptr;
	}

	/*! Remove a T* from the manager; destroys the object
	  \param name session name
	  \return true if successful */
	bool remove(const f8String& name)
	{
		f8_scoped_lock guard(_mutex);
		auto itr(_sessionmap.find(name));
		if (itr == _sessionmap.end())
			return false;
		delete itr->second;
		_sessionmap.erase(itr);
		return true;
	}

	/*! Find a session by session name
	  \param name session name
	  \return T* or nullptr if not found */
	T *find(const f8String& name)
	{
		f8_scoped_lock guard(_mutex);
		const auto itr(_sessionmap.find(name));
		return itr == _sessionmap.cend() ? nullptr : itr->second;
	}

	/*! Find a session by session name, subscript operator version
	  \param name session name
	  \return T* or nullptr if not found */
	T *operator[] (const f8String& name) const { return find(name); }

	/*! Get the number of sessions being manager by this object
	  \return count of sessions */
	unsigned size() const { return static_cast<unsigned>(_sessionmap.size()); }
};

//-------------------------------------------------------------------------------------------------
/// Client Manager.
using ClientManager = SessionManager<ClientSessionBase>;
/// Session Instance Manager.
using SessionInstanceManager = SessionManager<SessionInstanceBase>;

}

#endif // FIX8_MULTISESSSION_HPP_
