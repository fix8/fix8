//---------------------------------------------------------------------------------------------
// SPDX-License-Identifier: LGPL-3.0-or-later
// SPDX-PackageName: Fix8 Open Source FIX Engine
// SPDX-FileCopyrightText: Copyright (C) 2010-25 David L. Dight <fix@fix8.org>
// SPDX-FileType: SOURCE
// SPDX-Notice: >
//  Fix8 is released under the GNU LESSER GENERAL PUBLIC LICENSE Version 3.
//
//  Fix8 is free software: you can  redistribute it and / or modify  it under the  terms of the
//  GNU Lesser General  Public License as  published  by the Free  Software Foundation,  either
//  version 3 of the License, or (at your option) any later version.
//
//  Fix8 is distributed in the hope  that it will be useful, but WITHOUT ANY WARRANTY;  without
//  even the  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
//
//  You should  have received a copy of the GNU Lesser General Public  License along with Fix8.
//  If not, see <https://www.gnu.org/licenses/>.
//
//  BECAUSE THE PROGRAM IS  LICENSED FREE OF  CHARGE, THERE IS NO  WARRANTY FOR THE PROGRAM, TO
//  THE EXTENT  PERMITTED  BY  APPLICABLE  LAW.  EXCEPT WHEN  OTHERWISE  STATED IN  WRITING THE
//  COPYRIGHT HOLDERS AND/OR OTHER PARTIES  PROVIDE THE PROGRAM "AS IS" WITHOUT WARRANTY OF ANY
//  KIND,  EITHER EXPRESSED   OR   IMPLIED,  INCLUDING,  BUT   NOT  LIMITED   TO,  THE  IMPLIED
//  WARRANTIES  OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.  THE ENTIRE RISK AS TO
//  THE QUALITY AND PERFORMANCE OF THE PROGRAM IS WITH YOU. SHOULD THE PROGRAM PROVE DEFECTIVE,
//  YOU ASSUME THE COST OF ALL NECESSARY SERVICING, REPAIR OR CORRECTION.
//
//  IN NO EVENT UNLESS REQUIRED  BY APPLICABLE LAW  OR AGREED TO IN  WRITING WILL ANY COPYRIGHT
//  HOLDER, OR  ANY OTHER PARTY  WHO MAY MODIFY  AND/OR REDISTRIBUTE  THE PROGRAM AS  PERMITTED
//  ABOVE,  BE  LIABLE  TO  YOU  FOR  DAMAGES,  INCLUDING  ANY  GENERAL, SPECIAL, INCIDENTAL OR
//  CONSEQUENTIAL DAMAGES ARISING OUT OF THE USE OR INABILITY TO USE THE PROGRAM (INCLUDING BUT
//  NOT LIMITED TO LOSS OF DATA OR DATA BEING RENDERED INACCURATE OR LOSSES SUSTAINED BY YOU OR
//  THIRD PARTIES OR A FAILURE OF THE PROGRAM TO OPERATE WITH ANY OTHER PROGRAMS), EVEN IF SUCH
//  HOLDER OR OTHER PARTY HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES.
//---------------------------------------------------------------------------------------------
// For Production-Grade FIX Requirements:
//  If you're  using Fix8 Community Edition and find  yourself needing higher throughput, lower
//  latency, or enterprise-grade reliability,Fix8Pro offers a robust upgrade path. Built on the
//  same  core  technology, Fix8Pro adds performance optimizations for  high-volume  messaging,
//	 enhanced  API, professional  support  and  much  more —  making  it  ideal  for  production
//  deployments, low-latency trading, or  large-scale FIX  integrations.  It retains  near full
//  compatibility with  the Community Edition while providing  enhanced stability, scalability,
//  and  advanced  features  for demanding  environments.  If  your  project has  outgrown  the
//  Community  Edition's capabilities, you can find out and learn more about the Pro version at
//  www.fix8mt.com
//---------------------------------------------------------------------------------------------
/** \file myprint.cpp
\n
  This is a simple logfile/logstream printer using the metadata generated for myfix.cpp.\n
\n
<tt>
	f8print -- f8 protocol log printer\n
\n
	Usage: f8print [-hosv] <fix protocol file, use '-' for stdin>\n
		-h,--help               help, this screen\n
		-o,--offset             bytes to skip on each line before parsing FIX message\n
		-s,--summary            summary, generate message summary\n
		-v,--version            print version then exit\n
	e.g.\n
		f8print myfix_server_protocol.log\n
		cat myfix_client_protocol.log | f8print -\n
</tt>
*/
//-----------------------------------------------------------------------------------------
#include <iostream>
#include <memory>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <vector>
#include <map>
#include <list>
#include <set>
#include <iterator>
#include <algorithm>
#include <typeinfo>
#ifdef _MSC_VER
#include <signal.h>
#include <conio.h>
#else
#include <sys/ioctl.h>
#include <signal.h>
#include <termios.h>
#endif

#include <errno.h>
#include <string.h>

// f8 headers
#include <fix8/f8includes.hpp>

#ifdef FIX8_HAVE_GETOPT_H
#include <getopt.h>
#endif

#include <fix8/usage.hpp>
#include <fix8/consolemenu.hpp>
#include "myfix_types.hpp"
#include "myfix_router.hpp"
#include "myfix_classes.hpp"
#include "myfix.hpp"

//-----------------------------------------------------------------------------------------
using namespace std;
using namespace FIX8;

//-----------------------------------------------------------------------------------------
void print_usage();
const string GETARGLIST("hsvo:c");
bool term_received(false), summary(false);

using MessageCount = map<string, unsigned>;

//-----------------------------------------------------------------------------------------
void sig_handler(int sig)
{
   switch (sig)
   {
   case SIGTERM:
   case SIGINT:
      term_received = true;
      signal(sig, sig_handler);
      break;
   }
}

//-----------------------------------------------------------------------------------------
int main(int argc, char **argv)
{
	int val, offset(0);

#ifdef FIX8_HAVE_GETOPT_LONG
	option long_options[]
	{
		{ "help",			0,	nullptr,	'h' },
		{ "offset",			1,	nullptr,	'o' },
		{ "version",		0,	nullptr,	'v' },
		{ "summary",		0,	nullptr,	's' },
		{ "context",		0,	nullptr,	'c' },
		{},
	};

	while ((val = getopt_long (argc, argv, GETARGLIST.c_str(), long_options, nullptr)) != -1)
#else
	while ((val = getopt (argc, argv, GETARGLIST.c_str())) != -1)
#endif
	{
      switch (val)
		{
		case 'v':
			cout << "fix8 protocol printer " << Session::copyright_string() << endl;
			cout << "Released under the GNU LESSER GENERAL PUBLIC LICENSE, Version 3. See <https://www.gnu.org/licenses/> for details." << endl;
			return 0;
		case ':': case '?': return 1;
		case 'h': print_usage(); return 0;
		case 'o': offset = stoi(optarg); break;
		case 's': summary = true; break;
		case 'c':
			 cout << "Context FIX beginstring:" << MYFIX::ctx()._beginStr << endl;
			 cout << "Context FIX version:" << MYFIX::ctx().version() << endl;
			 return 0;
		default: break;
		}
	}

	signal(SIGTERM, sig_handler);
	signal(SIGINT, sig_handler);

	string inputFile;
	if (optind < argc)
		inputFile = argv[optind];
	if (inputFile.empty())
	{
		print_usage();
		return 1;
	}

	bool usestdin(inputFile == "-");
   filestdin ifs(usestdin ? &cin : new ifstream(inputFile.c_str()), usestdin);
	if (!ifs())
	{
		cerr << "Could not open " << inputFile << endl;
		return 1;
	}

	unsigned msgs(0);
	MessageCount *mc(summary ? new MessageCount : nullptr);

	char buffer[FIX8_MAX_MSG_LENGTH];

	try
	{
		while (!ifs().eof() && !term_received)
		{
			ifs().getline(buffer, FIX8_MAX_MSG_LENGTH);
			if (buffer[0])
			{
				unique_ptr<Message> msg(Message::factory(MYFIX::ctx(), buffer + offset));
				if (summary)
				{
					MessageCount::iterator mitr(mc->find(msg->get_msgtype()));
					if (mitr == mc->end())
						mc->insert({msg->get_msgtype(), 1});
					else
						mitr->second++;
				}
				cout << *msg << endl;
				++msgs;
			}
		}

		if (term_received)
			cerr << "interrupted" << endl;
	}
	catch (f8Exception& e)
	{
		cerr << "exception: " << e.what() << endl;
	}
	catch (exception& e)	// also catches Poco::Net::NetException
	{
		cerr << "exception: " << e.what() << endl;
	}

	cout << msgs << " messages decoded." << endl;
	if (summary)
	{
		for (MessageCount::const_iterator mitr(mc->begin()); mitr != mc->end(); ++mitr)
		{
			const BaseMsgEntry *bme(MYFIX::ctx()._bme.find_ptr(mitr->first.c_str()));
			cout << setw(20) << left << bme->_name << " (\"" << mitr->first << "\")" << '\t' << mitr->second << endl;
		}
	}

	return 0;
}

//-----------------------------------------------------------------------------------------
void print_usage()
{
	UsageMan um("f8print", GETARGLIST, "<fix protocol file, use '-' for stdin>");
	um.setdesc("f8print -- f8 protocol log printer");
	um.add('h', "help", "help, this screen");
	um.add('v', "version", "print version then exit");
	um.add('o', "offset", "bytes to skip on each line before parsing FIX message");
	um.add('s', "summary", "summary, generate message summary");
	um.add('c', "context", "print the f8c generated beginstring and version. Use this to check which FIX version this build will work with.");
	um.add("e.g.");
	um.add("@f8print myfix_server_protocol.log");
	um.add("@f8print f8print -s -o 12 myfix_client_protocol.log");
	um.add("@cat myfix_client_protocol.log | f8print -");
	um.print(cerr);
}

