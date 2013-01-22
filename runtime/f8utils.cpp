//-----------------------------------------------------------------------------------------
#if 0

Fix8 is released under the GNU LESSER GENERAL PUBLIC LICENSE Version 3, 29 June 2007.

Fix8 Open Source FIX Engine.
Copyright (C) 2010-12 David L. Dight <fix@fix8.org>

Fix8 is free software: you can redistribute it and/or modify  it under the terms of the GNU
General Public License as  published by the Free Software Foundation,  either version 3  of
the License, or (at your option) any later version.

Fix8 is distributed in the hope  that it will be useful, but WITHOUT ANY WARRANTY;  without
even the  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with Fix8.  If not,
see <http://www.gnu.org/licenses/>.

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
   ostringstream oss;
   oss << setfill('0') << setw(4) << (tim.tm_year + 1900) << '-';
   oss << setw(2) << (tim.tm_mon + 1)  << '-' << setw(2) << tim.tm_mday << ' ' << setw(2) << tim.tm_hour;
   oss << ':' << setw(2) << tim.tm_min << ':';
	if (dplaces)
	{
		const double secs((startTime->secs() % 60) + static_cast<double>(startTime->nsecs()) / Tickval::billion);
		oss.setf(ios::showpoint);
		oss.setf(ios::fixed);
		oss << setw(3 + dplaces) << setfill('0') << setprecision(dplaces) << secs;
	}
	else
		oss << setfill('0') << setw(2) << tim.tm_sec;
   return result = oss.str();
}

//-----------------------------------------------------------------------------------------
string& CheckAddTrailingSlash(string& src)
{
	if (!src.empty() && *src.rbegin() != '/')
		src += '/';
	return src;
}

//-----------------------------------------------------------------------------------------
string& InPlaceStrToUpper(string& src)
{
	for (string::iterator itr(src.begin()); itr != src.end(); ++itr)
		if (islower(*itr))
			*itr = toupper(*itr);
	return src;
}

//-----------------------------------------------------------------------------------------
string& InPlaceReplaceInSet(const string& iset, string& src, const char repl)
{
	for (string::iterator itr(src.begin()); itr != src.end(); ++itr)
		if (iset.find(*itr) == string::npos)
			*itr = repl;
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
const string& trim(string& source, const string& ws)
{
    const size_t bgstr(source.find_first_not_of(ws));
    return bgstr == string::npos
		 ? source : source = source.substr(bgstr, source.find_last_not_of(ws) - bgstr + 1);
}

//----------------------------------------------------------------------------------------
inline void format0(short data, char *to, int width)
{
    while(width-- > 0)
    {
        to[width] = data % 10 + '0';
        data /= 10;
    }
}

const size_t DateTimeFormat(const Poco::DateTime& dateTime, char *to, const MillisecondIndicator ind)
{
    format0(dateTime.year(), to, 4);
    format0(dateTime.month(), to + 4, 2);
    format0(dateTime.day(), to + 6, 2);
    to[8]='-';

    format0(dateTime.hour(), to + 9, 2);
    to[11]=':';

    format0(dateTime.minute(), to + 12, 2);
    to[14]=':';

    format0(dateTime.second(), to + 15, 2);

    if(ind != WithMillisecond)
    {
        //length of "YYYYMMDD-HH:MM:SS"
        return 17;
    }

    to[17]='.';
    format0(dateTime.millisecond(), to + 18, 3);

    //length of "YYYYMMDD-HH:MM:SS.MMM"
    return 21;
}

//----------------------------------------------------------------------------------------

inline void parseDate(std::string::const_iterator &begin, size_t len , short &to)
{
    while(len-- > 0)
    {
        to = (to << 3) + (to << 1) + ((*begin++) - '0');
    }
}

void DateTimeParse(const std::string& from, Poco::DateTime& dateTime, const MillisecondIndicator ind)
{
    string::const_iterator it   = from.begin();

    short year = 0;
    parseDate( it, 4, year);

    short month = 0;
    parseDate( it, 2, month);

    short day = 0;
    parseDate( it, 2, day);

    ++it;
    short hour = 0;
    parseDate( it, 2, hour);

    ++it;
    short minute = 0;
    parseDate( it, 2, minute);

    ++it;
    short second = 0;
    parseDate( it, 2, second);

    short millisecond = 0;
    if(ind == WithMillisecond)
    {
        ++it;
        parseDate( it, 3, millisecond);
    }

    if (Poco::DateTime::isValid(year, month, day, hour, minute, second, millisecond))
    {
        dateTime.assign(year, month, day, hour, minute, second, millisecond);
    }
}

} // namespace FIX8

