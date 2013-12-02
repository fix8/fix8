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
/** \file seqedit.cpp
\n
seqedit -- edit next expected send/receive
\n
<tt>
Usage: seqedit [-DRSdhiqv] \<perist file prefix\>
   -D,--rawdump            dump all the raw data records referenced in the index\n
   -R,--receive            set next expected receive sequence number\n
   -S,--send               set next send sequence number\n
   -d,--dump               dump all the records in both the index and the data file\n
   -h,--help               help, this screen\n
   -i,--index              only dump the index not the data records\n
   -q,--quiet              set the sequence numbers silently\n
   -v,--version            print version, exit\n
e.g.\n
   seqedit client.DLD_TEX.TEX_DLD\n
   seqedit -R 23417 -S 2341 client.DLD_TEX.TEX_DLD\n
   seqedit -d client.DLD_TEX.TEX_DLD\n
   seqedit -D client.DLD_TEX.TEX_DLD\n
   seqedit -id client.DLD_TEX.TEX_DLD\n
</tt>
\n
*/
//-----------------------------------------------------------------------------------------
#include <iostream>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <vector>
#include <map>
#include <list>
#include <set>
#include <iterator>
#include <algorithm>

#include <regex.h>
#include <errno.h>
#include <string.h>
#include <cctype>
#include <fcntl.h>

// f8 headers
#include <fix8/f8includes.hpp>
#include <usage.hpp>

#ifdef HAVE_GETOPT_H
#include <getopt.h>
#endif

#ifdef _MSC_VER
#define ssize_t int
#endif

//-----------------------------------------------------------------------------------------
using namespace std;
using namespace FIX8;

//-----------------------------------------------------------------------------------------
const string GETARGLIST("hvdDR:S:iq");

//-----------------------------------------------------------------------------------------
void print_usage();

//-----------------------------------------------------------------------------------------
int main(int argc, char **argv)
{
	bool dump(false), rawdump(false), indexonly(false), quiet(false);
	int val;
	unsigned next_send(0), next_receive(0);

#ifdef HAVE_GETOPT_LONG
	const option long_options[] =
	{
		{ "help",		0,	0,	'h' },
		{ "dump",		0,	0,	'd' },
		{ "version",	0,	0,	'v' },
		{ "rawdump",	0,	0,	'D' },
		{ "index",		0,	0,	'i' },
		{ "quiet",		0,	0,	'q' },
		{ "send",		1,	0,	'S' },
		{ "receive",	1,	0,	'R' },
		{ 0 },
	};

	while ((val = getopt_long (argc, argv, GETARGLIST.c_str(), long_options, 0)) != -1)
#else
	while ((val = getopt (argc, argv, GETARGLIST.c_str())) != -1)
#endif
	{
      switch (val)
		{
		case 'v':
			cout << "seqedit for " PACKAGE " version " VERSION << endl;
			cout << "Released under the GNU LESSER GENERAL PUBLIC LICENSE, Version 3. See <http://fsf.org/> for details." << endl;
			return 0;
		case 'h': print_usage(); return 0;
		case 'D': rawdump = true; // drop through
		case 'd': dump = true; break;
		case 'i': indexonly = true; break;
		case 'q': quiet = true; break;
		case 'S': next_send = get_value<unsigned>(optarg); break;
		case 'R': next_receive = get_value<unsigned>(optarg); break;
		case ':': case '?': return 1;
		default: break;
		}
	}

	if (optind >= argc)
	{
		cerr << "no input persistence file prefix specified" << endl;
		return 1;
	}

	const string dbFname(argv[optind]);
	const string dbIname(dbFname + ".idx");
	struct fdset
	{
		int fod, iod;
		fdset() : fod(), iod() {}
		~fdset() { close(iod); close(fod); }
	}
	fds;

	if ((fds.fod = open(dbFname.c_str(), O_RDONLY)) < 0)
	{
		cerr << "Error opening existing database: " << dbFname << " (" << strerror(errno) << ')' << endl;
		return 1;
	}
	if ((fds.iod = open(dbIname.c_str(), O_RDWR)) < 0)
	{
		cerr << "Error opening existing database index: " << dbIname << " (" << strerror(errno) << ')' << endl;
		return 1;
	}

	IPrec iprec;
	ssize_t blrd(read(fds.iod, static_cast<void *>(&iprec), sizeof(IPrec)));
	if (blrd <= 0)
	{
		cerr << "Error reading existing database index: " << dbIname << " (" << strerror(errno) << ')' << endl;
		return 1;
	}

	if (!rawdump && !quiet)
	{
		cout << endl;
		cout << "Next      send         receive" << endl;
		cout << "==============================" << endl;
		cout << "Current   " << left << setw(10) << iprec._prec._offset << right << setw(10) << iprec._prec._size << endl;
	}

	if (!dump)
	{
		if (next_send > 0 || next_receive > 0)
		{
			if (next_send)
				iprec._prec._offset = next_send;
			if (next_receive)
				iprec._prec._size = next_receive;
			if (lseek(fds.iod, 0, SEEK_SET) < 0)
			{
				ostringstream eostr;
				cerr << "Error could not seek to 0 for seqnum persitence: " << dbIname << endl;
				return 1;
			}
			if (write (fds.iod, static_cast<void *>(&iprec), sizeof(iprec)) != sizeof(iprec))
			{
				ostringstream eostr;
				cerr << "Error writing seqnum persitence: " << dbIname << endl;
				return 1;
			}

			if (!quiet)
				cout << "New       " << left << setw(10) << iprec._prec._offset << right << setw(10) << iprec._prec._size << endl;
		}
		cout << endl;
	}
	else for(;;)
	{
		blrd = read(fds.iod, static_cast<void *>(&iprec), sizeof(IPrec));
		if (blrd < 0)
		{
			cerr << "Error reading existing database index: " << dbIname << " (" << strerror(errno) << ')' << endl;
			return 1;
		}
		else if (blrd == 0)
			break; // eof

		if (!rawdump)
			cout << iprec << endl;

		if (iprec._seq == 0)
			continue;

		char buff[MAX_MSG_LENGTH] = {};

		if (lseek(fds.fod, iprec._prec._offset, SEEK_SET) < 0)
		{
			cerr << "Error could not seek to correct index location " << iprec._prec._offset << " for get: " << dbFname << endl;
			return 1;
		}

		if (read(fds.fod, buff, iprec._prec._size) != iprec._prec._size)
		{
			cerr << "Error could not read message record for seqnum " << iprec._seq << " from: " << dbFname << endl;
			return 1;
		}

		if (rawdump)
			cout.write(buff, iprec._prec._size);
		else if (!indexonly)
			cout << buff << endl;
	}

	return 0;
}

//-----------------------------------------------------------------------------------------
void print_usage()
{
	UsageMan um("seqedit", GETARGLIST, "<perist file prefix>");
	um.setdesc("seqedit -- edit next expected send/receive");
	um.add('R', "receive", "set next expected receive sequence number");
	um.add('S', "send", "set next send sequence number");
	um.add('d', "dump", "dump all the records in both the index and the data file");
	um.add('D', "rawdump", "dump all the raw data records referenced in the index");
	um.add('h', "help", "help, this screen");
	um.add('i', "index", "only dump the index not the data records");
	um.add('q', "quiet", "set the sequence numbers silently");
	um.add('v', "version", "print version, exit");
	um.add("e.g.");
	um.add("@seqedit client.DLD_TEX.TEX_DLD");
	um.add("@seqedit -R 23417 -S 2341 client.DLD_TEX.TEX_DLD");
	um.add("@seqedit -d client.DLD_TEX.TEX_DLD");
	um.add("@seqedit -D client.DLD_TEX.TEX_DLD");
	um.add("@seqedit -id client.DLD_TEX.TEX_DLD");
	um.add("@seqedit -S 2005 server.TEX_DLD.DLD_TEX");
	um.print(cerr);
}

