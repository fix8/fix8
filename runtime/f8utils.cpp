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
#include <fstream>
#include <sstream>
#include <iomanip>
#include <sys/types.h>
#ifndef _MSC_VER
#include <sys/time.h>
#endif
#include <sys/stat.h>
#include <time.h>
#include <errno.h>
#ifndef _MSC_VER
#include <netdb.h>
#endif
#include <cstdlib>
#include <signal.h>
#ifndef _MSC_VER
#include <syslog.h>
#include <strings.h>
#endif
#include <string.h>
#include <fcntl.h>
#include <time.h>
#ifndef _MSC_VER
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#endif
#ifndef _MSC_VER
#include <alloca.h>
#endif
#include <map>
#include <set>
#include <list>
#include <vector>
#include <algorithm>

#include <fix8/f8includes.hpp>

//----------------------------------------------------------------------------------------
#ifndef HAVE_GMTOFF
extern long timezone;
#endif

//-----------------------------------------------------------------------------------------
using namespace std;

namespace FIX8 {

//-------------------------------------------------------------------------------------------------
const string& GetTimeAsStringMS(string& result, const Tickval *tv, const unsigned dplaces)
{
   const Tickval *startTime;
   Tickval gotTime;
   if (tv)
      startTime = tv;
   else
   {
		gotTime.now();
      startTime = &gotTime;
   }

#ifdef _MSC_VER
   struct tm *ptim;
   time_t tval(startTime->secs());
   ptim = localtime (&tval);
#else
   struct tm tim, *ptim;
   time_t tval(startTime->secs());
   localtime_r(&tval, &tim);
   ptim = &tim;
#endif

   ostringstream oss;
   oss << setfill('0') << setw(4) << (ptim->tm_year + 1900) << '-';
   oss << setw(2) << (ptim->tm_mon + 1)  << '-' << setw(2) << ptim->tm_mday << ' ' << setw(2) << ptim->tm_hour;
   oss << ':' << setw(2) << ptim->tm_min << ':';
	if (dplaces)
	{
		const double secs((startTime->secs() % 60) + static_cast<double>(startTime->nsecs()) / Tickval::billion);
		oss.setf(ios::showpoint);
		oss.setf(ios::fixed);
		oss << setw(3 + dplaces) << setfill('0') << setprecision(dplaces) << secs;
	}
	else
		oss << setfill('0') << setw(2) << ptim->tm_sec;
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
#ifdef _MSC_VER
    ignore_value(strerror_s(buf, max_str - 1, err));
#else
	ignore_value(strerror_r(err, buf, max_str - 1));
#endif
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

} // namespace FIX8

