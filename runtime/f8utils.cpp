//-----------------------------------------------------------------------------------------
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
//-----------------------------------------------------------------------------------------
#include "precomp.hpp"
#include <fix8/f8includes.hpp>

//----------------------------------------------------------------------------------------
#ifndef HAVE_GMTOFF
extern long timezone;
#endif

//-----------------------------------------------------------------------------------------
using namespace std;

namespace FIX8 {

//-------------------------------------------------------------------------------------------------
const string& GetTimeAsStringMS(string& result, const Tickval *tv, const unsigned dplaces, bool use_gm)
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
   time_t tval(startTime->secs());
   struct tm *ptim(use_gm ? gmtime(&tval) : localtime (&tval));
#else
   struct tm tim, *ptim;
   time_t tval(startTime->secs());
   use_gm ? gmtime_r(&tval, &tim) : localtime_r(&tval, &tim);
   ptim = &tim;
#endif

	// 2014-07-02 23:15:51.514776595
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

//-------------------------------------------------------------------------------------------------
const string& GetTimeAsStringMini(string& result, const Tickval *tv)
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
   time_t tval(startTime->secs());
   struct tm *ptim(localtime (&tval));
#else
   struct tm tim, *ptim;
   time_t tval(startTime->secs());
   localtime_r(&tval, &tim);
   ptim = &tim;
#endif

// 14-07-02 23:15:51
   ostringstream oss;
   oss << setfill('0') << setw(2) << ((ptim->tm_year + 1900) % 100) << '-';
   oss << setw(2) << (ptim->tm_mon + 1)  << '-' << setw(2) << ptim->tm_mday << ' ' << setw(2) << ptim->tm_hour;
   oss << ':' << setw(2) << ptim->tm_min << ':' << setfill('0') << setw(2) << ptim->tm_sec;
   return result = oss.str();
}

//-------------------------------------------------------------------------------------------------
string& CheckAddTrailingSlash(string& src)
{
	if (!src.empty() && *src.rbegin() != '/')
		src += '/';
	return src;
}

//-----------------------------------------------------------------------------------------
string& InPlaceStrToUpper(string& src)
{
	//for (string::iterator itr(src.begin()); itr != src.end(); ++itr)
	for (auto& itr : src)
		if (islower(itr))
			itr = toupper(itr);
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
	for (auto& itr : src)
		if (isupper(itr))
			itr = tolower(itr);
	return src;
}

//-----------------------------------------------------------------------------------------
string StrToLower(const string& src)
{
	string result(src);
	return InPlaceStrToLower(result);
}

//----------------------------------------------------------------------------------------
string Str_error(const int err, const char *str)
{
	const size_t max_str(256);
	char buf[max_str] {};
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
int get_umask()
{
#ifdef _MSC_VER
	const int mask(_umask(0));
	_umask(mask);
#else
	const int mask(umask(0));
	umask(mask);
#endif
	return mask;
}

//----------------------------------------------------------------------------------------
void create_path(const string& path)
{
	string new_path;
	for(string::const_iterator pos(path.begin()); pos != path.end(); ++pos)
	{
		new_path += *pos;
		if(*pos == '/' || *pos == '\\' || pos + 1 == path.end())
		{
#ifdef _MSC_VER
			_mkdir(new_path.c_str());
#else
			mkdir(new_path.c_str(), 0777); // umask applied after
#endif
		}
	}
}

//----------------------------------------------------------------------------------------
namespace
{
	using Day = pair<char, int>;
	using Daymap = multimap<char, int>;
	static const string day_names[] { "su", "mo", "tu", "we", "th", "fr", "sa" };
	static const Day days[] { {'s',0}, {'m',1}, {'t',2}, {'w',3}, {'t',4}, {'f',5}, {'s', 6} };
	static const Daymap daymap(days, days + sizeof(days)/sizeof(Day));
};

int decode_dow (const string& from)
{
	if (from.empty())
		return -1;
	const string source(StrToLower(from));
	if (isdigit(source[0]) && source.size() == 1 && source[0] >= '0' && source[0] <= '6')	// accept numeric dow
		return source[0] - '0';
	pair<Daymap::const_iterator, Daymap::const_iterator> result(daymap.equal_range(source[0]));
	switch(distance(result.first, result.second))
	{
	case 1:
		return result.first->second;
	default:
		if (source.size() == 1) // drop through
	case 0:
			return -1;
		break;
	}
	return day_names[result.first->second][1] == source[1]
		 || day_names[(++result.first)->second][1] == source[1] ? result.first->second : -1;
}

//----------------------------------------------------------------------------------------
const Package_info& package_info()
{
   //ostr << "Package info for " PACKAGE " version " VERSION;
	static const Package_info pinfo
	{
		{ "VERSION", VERSION },
		{ "PACKAGE", PACKAGE },
#if defined PACKAGE_BUGREPORT
		{ "PACKAGE_BUGREPORT", PACKAGE_BUGREPORT },
#endif
#if defined PACKAGE_URL
		{ "PACKAGE_URL", PACKAGE_URL },
#endif
		{ "MAGIC_NUM", STRINGIFY(MAGIC_NUM) },
		{ "CONFIGURE_OPTIONS", CONFIGURE_OPTIONS },
		{ "CPPFLAGS", CPPFLAGS },
		{ "LIBS", LIBS },
		{ "LDFLAGS", LDFLAGS },
		{ "CONFIGURE_SDATE", CONFIGURE_SDATE },
		{ "CONFIGURE_TIME", CONFIGURE_TIME },
		{ "MAJOR_VERSION_NUM", STRINGIFY(MAJOR_VERSION_NUM) },
		{ "MINOR_VERSION_NUM", STRINGIFY(MINOR_VERSION_NUM) },
		{ "PATCH_VERSION_NUM", STRINGIFY(PATCH_VERSION_NUM) },
		{ "CONFIGURE_TIME_NUM", STRINGIFY(CONFIGURE_TIME_NUM) },
		{ "HOST_SYSTEM", HOST_SYSTEM },
		{ "MAX_FLD_LENGTH", STRINGIFY(MAX_FLD_LENGTH) },
		{ "MAX_MSG_LENGTH", STRINGIFY(MAX_MSG_LENGTH) },
		{ "MPMC_FF", STRINGIFY(MPMC_FF) },
		{ "MPMC_TBB", STRINGIFY(MPMC_TBB) },
		{ "MPMC_SYSTEM", STRINGIFY(MPMC_SYSTEM) },
		{ "DEFAULT_PRECISION", STRINGIFY(DEFAULT_PRECISION) },
		{ "THREAD_PTHREAD", STRINGIFY(THREAD_PTHREAD) },
		{ "THREAD_STDTHREAD", STRINGIFY(THREAD_STDTHREAD) },
		{ "THREAD_SYSTEM", STRINGIFY(THREAD_SYSTEM) },
#if defined SLEEP_NO_YIELD
		{ "SLEEP_NO_YIELD", STRINGIFY(SLEEP_NO_YIELD) },
#endif
#if defined CODECTIMING
		{ "CODECTIMING", STRINGIFY(CODECTIMING) },
#endif
#if defined HAVE_OPENSSL
		{ "HAVE_OPENSSL", STRINGIFY(HAVE_OPENSSL) },
#endif
#if defined HAVE_EXTENDED_METADATA
		{ "HAVE_EXTENDED_METADATA", STRINGIFY(HAVE_EXTENDED_METADATA) },
#endif
#if defined F8_DEBUG
		{ "F8_DEBUG", STRINGIFY(F8_DEBUG) },
#endif
	};

	return pinfo;
}

f8String find_package_info_string(const f8String& what)
{
	auto itr(package_info().find(what));
	return itr != package_info().cend() ? itr->second : f8String{};
}

//----------------------------------------------------------------------------------------

} // namespace FIX8

