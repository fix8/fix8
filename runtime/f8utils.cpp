//-----------------------------------------------------------------------------------------
#if 0

fix8 is released under the New BSD License.

Copyright (c) 2007-2010, David L. Dight <fix@fix8.org>
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
#include <config.h>
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

#ifdef HAS_TR1_UNORDERED_MAP
#include <tr1/unordered_map>
#endif

#include <map>
#include <set>
#include <list>
#include <vector>
#include <algorithm>

#include <f8includes.hpp>

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

//-----------------------------------------------------------------------------------------
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

//-------------------------------------------------------------------------------------------------
const string& GetTimeAsStringMS(string& result, Tickval *tv, const unsigned dplaces)
{
   Tickval *startTime, gotTime;
   if (tv)
      startTime = tv;
   else
   {
		gotTime.now();
      startTime = &gotTime;
   }

   struct tm tim;
	time_t tval(startTime->secs());
   localtime_r(&tval, &tim);
	const double secs((startTime->secs() % 60) + static_cast<double>(startTime->nsecs()) / Tickval::billion);
   ostringstream oss;
   oss << setfill('0') << setw(4) << (tim.tm_year + 1900) << '-';
   oss << setw(2) << (tim.tm_mon + 1)  << '-' << setw(2) << tim.tm_mday << ' ' << setw(2) << tim.tm_hour;
   oss << ':' << setw(2) << tim.tm_min << ':';
   oss.setf(ios::showpoint);
   oss.setf(ios::fixed);
   oss << setw(3 + dplaces) << setfill('0') << setprecision(dplaces) << secs;
   return result = oss.str();
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

//----------------------------------------------------------------------------------------
string Str_error(const int err, const char *str)
{
	const size_t max_str(256);
	char buf[max_str] = {};
	strerror_r(err, buf, max_str - 1);
	if (str && *str)
	{
		ostringstream ostr;
		ostr << str << ": " << buf;
		return ostr.str();
	}
	return string(buf);
}

//----------------------------------------------------------------------------------------
RegExp::RegExp(const char *pattern, const int flags) : pattern_(pattern)
{
	if ((errCode_ = regcomp(&reg_, pattern_.c_str(), REG_EXTENDED|flags)) != 0)
	{
		char rbuf[MaxErrLen_];
		regerror(errCode_, &reg_, rbuf, MaxErrLen_);
		errString = rbuf;
	}
}

//----------------------------------------------------------------------------------------
int RegExp::SearchString(RegMatch& match, const string& source, const int subExpr, const int offset)
{
	match.subCnt_ = 0;
	if (regexec(&reg_, source.c_str() + offset, subExpr <= RegMatch::SubLimit_ ? subExpr : RegMatch::SubLimit_, match.subexprs_, 0) == 0)
		while (match.subCnt_ < subExpr && match.subexprs_[match.subCnt_].rm_so != -1)
			++match.subCnt_;
	return match.subCnt_;
}

//----------------------------------------------------------------------------------------
string& RegExp::SubExpr(RegMatch& match, const string& source, string& target, const int offset, const int num)
{
	if (num < match.subCnt_)
		target = source.substr(offset + match.subexprs_[num].rm_so, match.subexprs_[num].rm_eo - match.subexprs_[num].rm_so);
	else
		target.empty();
	return target;
}

//----------------------------------------------------------------------------------------
string& RegExp::Erase(RegMatch& match, string& source, const int num)
{
	if (num < match.subCnt_)
		source.erase(match.subexprs_[num].rm_so, match.subexprs_[num].rm_eo - match.subexprs_[num].rm_so);
	return source;
}

//----------------------------------------------------------------------------------------
string& RegExp::Replace(RegMatch& match, string& source, const string& with, const int num)
{
	if (num < match.subCnt_)
		source.replace(match.subexprs_[num].rm_so, match.subexprs_[num].rm_eo - match.subexprs_[num].rm_so, with);
	return source;
}

} // namespace FIX8

