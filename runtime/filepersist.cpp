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
#include <sstream>
#include <vector>
#include <map>
#include <set>
#include <list>
#include <iterator>
#include <memory>
#include <iomanip>
#include <algorithm>
#include <numeric>

#ifndef _MSC_VER
#include <strings.h>
#endif
#include <cerrno>
#include <stdint.h>
#ifdef _MSC_VER
#include <io.h>
#define ssize_t int
#else
#include <unistd.h>
#endif
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <fix8/f8includes.hpp>

//-------------------------------------------------------------------------------------------------
using namespace FIX8;
using namespace std;

//-------------------------------------------------------------------------------------------------
bool FilePersister::initialise(const f8String& dbDir, const f8String& dbFname, bool purge)
{
	if (_opened)
		return true;

	f8String odbdir(dbDir);
	ostringstream ostr;
	ostr << CheckAddTrailingSlash(odbdir) << dbFname;
	_dbFname = ostr.str();
	ostr << ".idx";
	_dbIname = ostr.str();

	bool nof;

	if ((nof = !exist(_dbFname)) || purge)
	{
		if (purge)
		{
			if (_rotnum > 0)
			{
				vector<string> dblst, idxlst;
				dblst.push_back(_dbFname);
				idxlst.push_back(_dbIname);

				for (unsigned ii(0); ii < _rotnum && ii < Logger::max_rotation; ++ii)
				{
					ostringstream ostr;
					ostr << _dbFname << '.' << (ii + 1);
					dblst.push_back(ostr.str());
					ostr << ".idx";
					idxlst.push_back(ostr.str());
				}

				for (unsigned ii(_rotnum); ii; --ii)
				{
					rename (dblst[ii - 1].c_str(), dblst[ii].c_str());   // ignore errors
					rename (idxlst[ii - 1].c_str(), idxlst[ii].c_str()); // ignore errors
				}
			}
		}

		if ((_fod = open(_dbFname.c_str(), O_RDWR | O_CREAT | O_TRUNC, 0600)) < 0)
		{
			ostringstream eostr;
			eostr << "Error: creating database: " << _dbFname << " (" << strerror(errno) << ')';
			GlobalLogger::log(eostr.str());
			return false;
		}
		if ((_iod = open(_dbIname.c_str(), O_RDWR | O_CREAT | O_TRUNC, 0600)) < 0)
		{
			ostringstream eostr;
			eostr << "Error: creating database index: " << _dbIname << " (" << strerror(errno) << ')';
			GlobalLogger::log(eostr.str());
			return false;
		}

		_wasCreated = true;

		if (purge && !nof)
			GlobalLogger::log(_rotnum ? "Rotated and purged perist db" : "Purged perist db");
	}
	else
	{
		if ((_fod = open(_dbFname.c_str(), O_RDWR)) < 0)
		{
			ostringstream eostr;
			eostr << "Error: opening existing database: " << _dbFname << " (" << strerror(errno) << ')';
			GlobalLogger::log(eostr.str());
			return false;
		}
		if ((_iod = open(_dbIname.c_str(), O_RDWR)) < 0)
		{
			ostringstream eostr;
			eostr << "Error: opening existing database index: " << _dbIname << " (" << strerror(errno) << ')';
			GlobalLogger::log(eostr.str());
			return false;
		}

		IPrec iprec;
		while (true)
		{
			ssize_t blrd(read(_iod, static_cast<void *>(&iprec), sizeof(IPrec)));
			if (blrd < 0)
			{
				ostringstream eostr;
				eostr << "Error: reading existing database index: " << _dbIname << " (" << strerror(errno) << ')';
				GlobalLogger::log(eostr.str());
				return false;
			}
			else if (blrd == 0)
				break; // eof

			if (iprec._seq == 0)
			{
				ostringstream eostr;
				eostr << iprec;
				GlobalLogger::log(eostr.str());
			}

			if (!_index.insert(Index::value_type(iprec._seq, iprec._prec)).second)
			{
				ostringstream eostr;
				eostr << "Warning: inserting index record into database index: " << _dbIname << " (" << iprec << "). Ignoring.";
				GlobalLogger::log(eostr.str());
			}
		}

		if (_index.size())
		{
			ostringstream eostr;
			eostr << "Database " << _dbFname << " indexed " << _index.size() << " records.";
			GlobalLogger::log(eostr.str());
		}

		unsigned last;
		if (get_last_seqnum(last))
		{
			ostringstream ostr;
			ostr << _dbFname << ": Last sequence is " << last;
			GlobalLogger::log(ostr.str());
		}
	}

	return _opened = true;
}

//-------------------------------------------------------------------------------------------------
FilePersister::~FilePersister()
{
	close(_fod);
	close(_iod);
}

//-------------------------------------------------------------------------------------------------
unsigned FilePersister::get_last_seqnum(unsigned& sequence) const
{
	return sequence = _index.empty() ? 0 : _index.rbegin()->first;
}

//-------------------------------------------------------------------------------------------------
unsigned FilePersister::get(const unsigned from, const unsigned to, Session& session,
		bool (Session::*callback)(const Session::SequencePair& with, Session::RetransmissionContext& rctx)) const
{
	unsigned last_seq(0);
	get_last_seqnum(last_seq);
	unsigned recs_sent(0), startSeqNum(find_nearest_highest_seqnum (from, last_seq));
	const unsigned finish(to == 0 ? last_seq : to);
	Session::RetransmissionContext rctx(from, to, session.get_next_send_seq());

	if (!startSeqNum || from > finish)
	{
		GlobalLogger::log("No records found");
		rctx._no_more_records = true;
		(session.*callback)(Session::SequencePair(0, ""), rctx);
		return 0;
	}

	Index::const_iterator itr(_index.find(startSeqNum));
	if (itr != _index.end())
	{
		char buff[MAX_MSG_LENGTH];

		do
		{
			if (!itr->first || itr->first > finish)
				break;
			if (lseek(_fod, itr->second._offset, SEEK_SET) < 0)
			{
				ostringstream eostr;
				eostr << "Error: could not seek to correct index location for get: " << _dbFname;
				GlobalLogger::log(eostr.str());
				break;
			}

			if (read (_fod, buff, itr->second._size) != itr->second._size)
			{
				ostringstream eostr;
				eostr << "Error: could not read message record for seqnum " << itr->first << " from: " << _dbFname;
				GlobalLogger::log(eostr.str());
				break;
			}

			Session::SequencePair txresult(itr->first, f8String(buff, itr->second._size));
			++recs_sent;
			if (!(session.*callback)(txresult, rctx))
				break;
		}
		while(++itr != _index.end());

		rctx._no_more_records = true;
		(session.*callback)(Session::SequencePair(0, ""), rctx);
	}
	else
	{
		ostringstream ostr;
		ostr << "record not found (" << startSeqNum << ')';
		GlobalLogger::log(ostr.str());
	}

	return recs_sent;
}

//-------------------------------------------------------------------------------------------------
bool FilePersister::put(const unsigned sender_seqnum, const unsigned target_seqnum)
{
	if (!_opened)
		return false;
	IPrec iprec(0, sender_seqnum, target_seqnum);
	Index::iterator itr(_index.find(0));
	if (itr == _index.end())
		_index.insert(Index::value_type(0, iprec._prec));
	else
		itr->second = iprec._prec;

	if (lseek(_iod, 0, SEEK_SET) < 0)
	{
		ostringstream eostr;
		eostr << "Error: could not seek to 0 for seqnum persitence: " << _dbIname;
		GlobalLogger::log(eostr.str());
		return false;
	}
	return write (_iod, static_cast<void *>(&iprec), sizeof(IPrec)) == sizeof(IPrec);
}

//-------------------------------------------------------------------------------------------------
bool FilePersister::put(const unsigned seqnum, const f8String& what)
{
	if (!_opened || !seqnum)
		return false;

	if (_index.find(seqnum) != _index.end())
	{
		ostringstream eostr;
		eostr << "Error: seqnum " << seqnum << " already persisted in: " << _dbIname;
		GlobalLogger::log(eostr.str());
		return false;
	}
	if (lseek(_iod, 0, SEEK_END) < 0)
	{
		ostringstream eostr;
		eostr << "Error: could not seek to index end for seqnum persitence: " << _dbIname;
		GlobalLogger::log(eostr.str());
		return false;
	}
	off_t offset;
	if ((offset = lseek(_fod, 0, SEEK_END)) < 0)
	{
		ostringstream eostr;
		eostr << "Error: could not seek to end for seqnum persitence: " << _dbFname;
		GlobalLogger::log(eostr.str());
		return false;
	}
	IPrec iprec(seqnum, offset, what.size());
	if (write (_iod, static_cast<void *>(&iprec), sizeof(IPrec)) != sizeof(IPrec))
	{
		ostringstream eostr;
		eostr << "Error: could not write index record for seqnum " << seqnum << " to: " << _dbIname;
		GlobalLogger::log(eostr.str());
		return false;
	}
	if (write (_fod, what.data(), what.size()) != static_cast<ssize_t>(what.size()))
	{
		ostringstream eostr;
		eostr << "Error: could not write record for seqnum " << seqnum << " to: " << _dbFname;
		GlobalLogger::log(eostr.str());
		return false;
	}

	return _index.insert(Index::value_type(seqnum, iprec._prec)).second;
}

//-------------------------------------------------------------------------------------------------
bool FilePersister::get(unsigned& sender_seqnum, unsigned& target_seqnum) const
{
	if (!_opened)
		return false;

	if (_index.empty())
	{
		ostringstream eostr;
		eostr << "Warning: index is empty: " << _dbIname;
		GlobalLogger::log(eostr.str());
		return false;
	}

	Index::const_iterator itr(_index.find(0));
	if (itr == _index.end())
	{
		ostringstream eostr;
		eostr << "Error: index does not contain control record: " << _dbIname;
		GlobalLogger::log(eostr.str());
		return false;
	}

	sender_seqnum = itr->second._offset;
	target_seqnum = itr->second._size;
	return true;
}

//-------------------------------------------------------------------------------------------------
bool FilePersister::get(const unsigned seqnum, f8String& to) const
{
	if (!_opened || !seqnum || _index.empty())
		return false;
	Index::const_iterator itr(_index.find(seqnum));
	if (itr == _index.end())
	{
		ostringstream eostr;
		eostr << "Warning: index does not contain seqnum: " << seqnum << " in: " << _dbIname;
		GlobalLogger::log(eostr.str());
		return false;
	}

	if (lseek(_fod, itr->second._offset, SEEK_SET) < 0)
	{
		ostringstream eostr;
		eostr << "Error: could not seek to correct index location for get: " << _dbFname;
		GlobalLogger::log(eostr.str());
		return false;
	}

	char buff[MAX_MSG_LENGTH];
	if (read (_fod, buff, itr->second._size) != itr->second._size)
	{
		ostringstream eostr;
		eostr << "Error: could not read message record for seqnum " << seqnum << " from: " << _dbFname;
		GlobalLogger::log(eostr.str());
		return false;
	}

	to.assign(buff, itr->second._size);
	return true;
}

//---------------------------------------------------------------------------------------------------
unsigned FilePersister::find_nearest_highest_seqnum (const unsigned requested, const unsigned last) const
{
	if (last)
	{
		for (unsigned startseqnum(requested); startseqnum <= last; ++startseqnum)
		{
			Index::const_iterator itr(_index.find(startseqnum));
			if (itr != _index.end())
				return itr->first;
		}
	}

	return 0;
}

