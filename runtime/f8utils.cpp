//-----------------------------------------------------------------------------------------
#if 0

Orbweb is released under the New BSD License.

Copyright (c) 2007-2010, David L. Dight <www@orbweb.org>
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

-------------------------------------------------------------------------------------------
$Id$
$LastChangedDate$
$Rev$
$URL$

#endif
//-----------------------------------------------------------------------------------------

#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <time.h>
#include <errno.h>
#include <netdb.h>
#include <cstdlib>
#include <signal.h>
#include <syslog.h>
#include <strings.h>
#include <string.h>
#include <fcntl.h>
#include <time.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <regex.h>
#include <alloca.h>
//#include <pcreposix.h>

//#include <config.h>

#include <map>
#include <set>
#include <list>
#include <vector>
#include <f8utils.hpp>

//----------------------------------------------------------------------------------------
#ifndef HAVE_GMTOFF
extern long timezone;
#endif

//-----------------------------------------------------------------------------------------
using namespace std;

namespace FIX8 {

//----------------------------------------------------------------------------------------
const std::string TRANSLATIONUNIT(__FILE__);

//-----------------------------------------------------------------------------------------
int Hex2Dec(const char src)
{
	const char sr(toupper(src));
	if (sr >= '0' && sr <= '9')
		return sr - '0';
	else if (sr >= 'A' && sr <= 'F')
		return sr - '7';
	return -1;
}

const string& GetTimeAsStringMS(string& result, timeval *tv)
{
	timeval *startTime, gotTime;
	if (tv)
		startTime = tv;
	else
	{
		gettimeofday(&gotTime, 0);
		startTime = &gotTime;
	}

	struct tm tim;
	localtime_r(&startTime->tv_sec, &tim);
	double secs(tim.tm_sec + startTime->tv_usec/1000000.);
	ostringstream oss;
	oss << setfill('0') << setw(4) << (tim.tm_year + 1900);
	oss << setw(2) << (tim.tm_mon + 1) << setw(2) << tim.tm_mday << ' ' << setw(2) << tim.tm_hour;
	oss << ':' << setw(2) << tim.tm_min << ':';
	oss.setf(ios::showpoint);
	oss.setf(ios::fixed);
	oss << setw(7) << setfill('0') << setprecision(4) << secs;
	return result = oss.str();
}

const int GetGMTOffsetInSecs()
{
	tm tim;
	time_t now;
	time(&now);
	localtime_r(&now, &tim);
	return
#ifdef HAVE_GMTOFF
	tim.tm_gmtoff
#else
	-(timezone - (tim.tm_isdst > 0 ? 60 * 60 : 0))
#endif
	;
}

//----------------------------------------------------------------------------------------
// No multiplication, algorithm by Serge Vakulenko. See http://vak.ru/doku.php/proj/hash/sources
unsigned ROT13Hash (const string& str)
{
	unsigned int hash(0);

	for (string::const_iterator itr(str.begin()); itr != str.end(); ++itr)
	{
		hash += *itr;
		hash -= rotl<unsigned>(hash, 13);
	}

	return hash;
}

//-----------------------------------------------------------------------------------------
string& CheckAddTrailingSlash(string& src)
{
	if (!src.empty() && *src.rbegin() != '/')
		src += '/';
	return src;
}

string& CheckRemoveTrailingSlash(string& src)
{
	if (!src.empty() && *src.rbegin() == '/')
		src.resize(src.size() - 1);
	return src;
}

//-----------------------------------------------------------------------------------------
string& InPlaceChrReplace(const char sch, const char rch, string& src)
{
	for (string::iterator itr(src.begin()); itr != src.end(); ++itr)
		if (*itr == sch)
			*itr = rch;
	return src;
}

string& InPlaceStrToUpper(string& src)
{
	for (string::iterator itr(src.begin()); itr != src.end(); ++itr)
		if (islower(*itr))
			*itr = toupper(*itr);
	return src;
}

//-----------------------------------------------------------------------------------------
string& InPlaceStrToLower(string& src)
{
	for (string::iterator itr(src.begin()); itr != src.end(); ++itr)
		if (isupper(*itr))
			*itr = tolower(*itr);
	return src;
}

const string& StrToLower(const string& src, string& target)
{
	target.assign (src);
	return InPlaceStrToLower(target);
}

} // namespace FIX8
