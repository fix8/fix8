//-----------------------------------------------------------------------------------------
#if 0

fix8 is released under the New BSD License.

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

//----------------------------------------------------------------------------------------
const std::string TRANSLATIONUNIT(__FILE__);

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

} // namespace FIX8

