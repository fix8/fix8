//-----------------------------------------------------------------------------------------
#if 0

Fix8 is released under the New BSD License.

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
#include <bitset>
#include <typeinfo>
#include <sys/ioctl.h>
#include <signal.h>
#include <termios.h>

#include <regex.h>
#include <errno.h>
#include <string.h>

// f8 headers
#include <f8includes.hpp>

#ifdef HAVE_GETOPT_H
#include <getopt.h>
#endif

#include <usage.hpp>
#include "Myfix_types.hpp"
#include "Myfix_router.hpp"
#include "Myfix_classes.hpp"
#include "myfix.hpp"

//-----------------------------------------------------------------------------------------
using namespace std;
using namespace FIX8;

//-----------------------------------------------------------------------------------------
void print_usage();
const string GETARGLIST("hsv");
bool term_received(false), summary(false);

typedef map<string, unsigned> MessageCount;

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

//----------------------------------------------------------------------------------------
class filestdin
{
   std::istream *ifs_;
   bool nodel_;

public:
   filestdin(std::istream *ifs, bool nodel=false) : ifs_(ifs), nodel_(nodel) {}
   ~filestdin() { if (!nodel_) delete ifs_; }

   std::istream& operator()() { return *ifs_; }
};

//-----------------------------------------------------------------------------------------
#if defined PERMIT_CUSTOM_FIELDS
#include "myfix_custom.hpp"
#endif

//-----------------------------------------------------------------------------------------
int main(int argc, char **argv)
{
	int val;

#ifdef HAVE_GETOPT_LONG
	option long_options[] =
	{
		{ "help",			0,	0,	'h' },
		{ "version",		0,	0,	'v' },
		{ "summary",		0,	0,	's' },
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
			cout << argv[0] << " for "PACKAGE" version "VERSION << endl;
			return 0;
		case ':': case '?': return 1;
		case 'h': print_usage(); return 0;
		case 's': summary = true; break;
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
	MessageCount *mc(summary ? new MessageCount : 0);

#if defined PERMIT_CUSTOM_FIELDS
	CustomFields custfields(true);	// will cleanup
	custfields.add(6666, BaseEntry_ctor(new BaseEntry, &TEX::Create_Orderbook,
		&TEX::extra_realmbases[0], "Orderbook", "Select downstream execution venue"));
	custfields.add(6951, BaseEntry_ctor(new BaseEntry, &TEX::Create_BrokerInitiated,
		&TEX::extra_realmbases[1], "BrokerInitiated", "Indicate if order was broker initiated"));
	custfields.add(7009, BaseEntry_ctor(new BaseEntry, &TEX::Create_ExecOption,
		&TEX::extra_realmbases[2], "ExecOption", "Broker specific option"));
	TEX::ctx.set_ube(&custfields);

	myfix_session_server ses(TEX::ctx);
#endif

	try
	{
		const int bufsz(1024);

		while (!ifs().eof() && !term_received)
		{
			char buffer[bufsz] = {};

			ifs().getline(buffer, bufsz);
			if (buffer[0])
			{
				scoped_ptr<Message> msg(Message::factory(TEX::ctx, buffer
#if defined PERMIT_CUSTOM_FIELDS
					, &ses, &Session::post_msg_ctor
#endif
				));
				if (summary)
				{
					MessageCount::iterator mitr(mc->find(msg->get_msgtype()));
					if (mitr == mc->end())
						mc->insert(MessageCount::value_type(msg->get_msgtype(), 1));
					else
						mitr->second++;
				}
				msg->print(cout);
				cout << endl;
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

	cout << msgs << " messages decoded." << endl;
	if (summary)
	{
		for (MessageCount::const_iterator mitr(mc->begin()); mitr != mc->end(); ++mitr)
		{
			const BaseMsgEntry *bme(TEX::ctx._bme.find_ptr(mitr->first));
			cout << setw(20) << left << bme->_name << " (\"" << mitr->first << "\")" << '\t' << mitr->second << endl;
		}
	}

	return 0;
}

//-----------------------------------------------------------------------------------------
#if defined PERMIT_CUSTOM_FIELDS
bool myfix_session_server::post_msg_ctor(Message *msg)
{
	return common_post_msg_ctor(msg);
}

//-----------------------------------------------------------------------------------------
bool myfix_session_server::handle_application(const unsigned seqnum, const Message *msg)
{
	return true;
}

//-----------------------------------------------------------------------------------------
bool tex_router_server::operator() (const TEX::NewOrderSingle *msg) const
{
	return true;
}

#endif

//-----------------------------------------------------------------------------------------
void print_usage()
{
	UsageMan um("f8print", GETARGLIST, "<fix protocol file, use '-' for stdin>");
	um.setdesc("f8print -- f8 protocol log printer");
	um.add('h', "help", "help, this screen");
	um.add('s', "summary", "summary, generate message summary");
	um.add("e.g.");
	um.add("@f8print myfix_server_protocol.log");
	um.add("@cat myfix_client_protocol.log | f8print -");
	um.print(cerr);
}

