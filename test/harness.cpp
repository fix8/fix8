//-----------------------------------------------------------------------------------------
#if 0

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

#endif

//-----------------------------------------------------------------------------------------
/** \file harness.cpp
\n
  This is a complete working example of a FIX client/server using FIX8.\n
\n
<tt>
	Usage: harness [-RSchlqrsv]\n
		-R,--receive            set next expected receive sequence number\n
		-S,--send               set next send sequence number\n
		-c,--config             xml config (default: myfix_client.xml or myfix_server.xml)\n
		-h,--help               help, this screen\n
		-l,--log                global log filename\n
		-q,--quiet              do not print fix output\n
		-r,--reliable           start in reliable mode\n
		-s,--server             run in server mode (default client mode)\n
		-v,--version            print version then exit\n
</tt>
\n
\n
  To use start the server:\n
\n
<tt>
	  % harness -sl server\n
</tt>
\n
  In another terminal session, start the client:\n
\n
<tt>
	  % harness -l client\n
</tt>
\n
  \b Notes \n
\n
  1. If you have configured with \c --enable-msgrecycle, the example will reuse allocated messages.\n
  2. If you have configured with \c --enable-customfields, the example will add custom fields\n
     defined below.\n
  3. The client has a simple menu. Press ? to see options.\n
  4. The server will wait for the client to logout before exiting.\n
  5. The server uses \c myfix_client.xml and the client uses \c myfix_server.xml for configuration settings.\n
  6. The example uses the files \c FIX50SP2.xml and \c FIXT11.xml in ./schema\n
\n
*/

/*! \namespace FIX8
	All FIX8 classes and functions reside inside this namespace.
*/

/*! \namespace FIX8::TEX
	This namespace is used by the generated classes and types, and was specified as a namespace
	to the \c f8c compiler.
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
#include <consolemenu.hpp>
#include "Myfix_types.hpp"
#include "Myfix_router.hpp"
#include "Myfix_classes.hpp"

#include "myfix.hpp"

//-----------------------------------------------------------------------------------------
using namespace std;
using namespace FIX8;

//-----------------------------------------------------------------------------------------
void print_usage();
const string GETARGLIST("hl:svqc:R:S:r");
bool term_received(false);

//-----------------------------------------------------------------------------------------
namespace FIX8
{
	template<>
	const MyMenu::Handlers::TypePair MyMenu::Handlers::_valueTable[] =
	{
		MyMenu::Handlers::TypePair(MyMenu::MenuItem('c', "Create messages"), &MyMenu::create_msgs),
		MyMenu::Handlers::TypePair(MyMenu::MenuItem('e', "Edit messages"), &MyMenu::edit_msgs),
		MyMenu::Handlers::TypePair(MyMenu::MenuItem('d', "Delete messages"), &MyMenu::delete_msgs),
		MyMenu::Handlers::TypePair(MyMenu::MenuItem('s', "Send messages"), &MyMenu::send_msgs),
		MyMenu::Handlers::TypePair(MyMenu::MenuItem('w', "Write messages to disk"), &MyMenu::write_msgs),
		MyMenu::Handlers::TypePair(MyMenu::MenuItem('r', "Read messages from disk"), &MyMenu::read_msgs),
		MyMenu::Handlers::TypePair(MyMenu::MenuItem('?', "Help"), &MyMenu::help),
		MyMenu::Handlers::TypePair(MyMenu::MenuItem('l', "Logout"), &MyMenu::do_logout),
		MyMenu::Handlers::TypePair(MyMenu::MenuItem('x', "Exit"), &MyMenu::do_exit),
	};
	template<>
	const MyMenu::Handlers::NotFoundType MyMenu::Handlers::_noval = &MyMenu::nothing;
	template<>
	const MyMenu::Handlers::TypeMap MyMenu::Handlers::_valuemap(MyMenu::Handlers::_valueTable,
		MyMenu::Handlers::get_table_end());
}

bool quiet(false);

//-----------------------------------------------------------------------------------------
void sig_handler(int sig)
{
   switch (sig)
   {
   case SIGTERM:
   case SIGINT:
   case SIGQUIT:
      term_received = true;
      signal(sig, sig_handler);
      break;
   }
}

//-----------------------------------------------------------------------------------------
int main(int argc, char **argv)
{
	int val;
	bool server(false), reliable(false);
	string clcf;
	unsigned next_send(0), next_receive(0);

#ifdef HAVE_GETOPT_LONG
	option long_options[] =
	{
		{ "help",		0,	0,	'h' },
		{ "version",	0,	0,	'v' },
		{ "log",			1,	0,	'l' },
		{ "config",		1,	0,	'c' },
		{ "server",		0,	0,	's' },
		{ "send",		1,	0,	'S' },
		{ "receive",	1,	0,	'R' },
		{ "quiet",		0,	0,	'q' },
		{ "reliable",	0,	0,	'r' },
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
			cout << "Released under the GNU LESSER GENERAL PUBLIC LICENSE, Version 3. See <http://fsf.org/> for details." << endl;
			return 0;
		case ':': case '?': return 1;
		case 'h': print_usage(); return 0;
		case 'l': GlobalLogger::set_global_filename(optarg); break;
		case 'c': clcf = optarg; break;
		case 's': server = true; break;
		case 'S': next_send = GetValue<unsigned>(optarg); break;
		case 'R': next_receive = GetValue<unsigned>(optarg); break;
		case 'q': quiet = true; break;
		case 'r': reliable = true; break;
		default: break;
		}
	}

	RandDev::init();

	signal(SIGTERM, sig_handler);
	signal(SIGINT, sig_handler);
	signal(SIGQUIT, sig_handler);

	try
	{
		const string conf_file(server ? clcf.empty() ? "myfix_server.xml" : clcf : clcf.empty() ? "myfix_client.xml" : clcf);

		if (server)
		{
			ServerSession<myfix_session_server>::Server_ptr
				ms(new ServerSession<myfix_session_server>(TEX::ctx, conf_file, "TEX1"));

			for (unsigned scnt(0); !term_received; )
			{
				if (!ms->poll())
					continue;
				SessionInstance<myfix_session_server>::Instance_ptr
					inst(new SessionInstance<myfix_session_server>(*ms));
				if (!quiet)
					inst->session_ptr()->control() |= Session::print;
				ostringstream sostr;
				sostr << "client(" << ++scnt << ") connection established.";
				GlobalLogger::log(sostr.str());
				inst->start(true, next_send, next_receive);
				cout << "Session(" << scnt << ") finished." << endl;
				inst->stop();
			}
		}
		else if (reliable)
		{
			ReliableClientSession<myfix_session_client>::Client_ptr
				mc(new ReliableClientSession<myfix_session_client>(TEX::ctx, conf_file, "DLD1"));
			if (!quiet)
				mc->session_ptr()->control() |= Session::print;
			mc->start(false);
			ConsoleMenu cm(TEX::ctx, mc->session_ptr(), cin, cout, 50);
			MyMenu mymenu(*mc->session_ptr(), 0, cout, &cm);
			char ch;
			mymenu.get_tty().set_raw_mode();
			sleep(1);
			do
			{
				cout << endl << "?=help > " << flush;
			}
			while(!mymenu.get_istr().get(ch).bad() && !term_received && ch != 0x3 && mymenu.process(ch));
			mymenu.get_tty().unset_raw_mode();
		}
		else
		{
			ClientSession<myfix_session_client>::Client_ptr
				mc(new ClientSession<myfix_session_client>(TEX::ctx, conf_file, "DLD1"));
			if (!quiet)
				mc->session_ptr()->control() |= Session::print;
			const LoginParameters& lparam(mc->session_ptr()->get_login_parameters());
			mc->start(false, next_send, next_receive, lparam._davi());
			ConsoleMenu cm(TEX::ctx, mc->session_ptr(), cin, cout, 50);
			MyMenu mymenu(*mc->session_ptr(), 0, cout, &cm);
			char ch;
			mymenu.get_tty().set_raw_mode();
			sleep(1);
			do
			{
				cout << endl << "?=help > " << flush;
			}
			while(!mymenu.get_istr().get(ch).bad() && !term_received && ch != 0x3 && mymenu.process(ch));
			mymenu.get_tty().unset_raw_mode();
		}
	}
	catch (f8Exception& e)
	{
		cerr << "exception: " << e.what() << endl;
	}
	catch (exception& e)	// also catches Poco::Net::NetException
	{
		cerr << "exception: " << e.what() << endl;
	}

	if (term_received)
		cout << "terminated." << endl;
	return 0;
}

//-----------------------------------------------------------------------------------------
bool myfix_session_client::handle_application(const unsigned seqnum, const Message *msg)
{
	return enforce(seqnum, msg) || msg->process(_router);
}

//-----------------------------------------------------------------------------------------
bool myfix_session_server::handle_application(const unsigned seqnum, const Message *msg)
{
	return enforce(seqnum, msg) || msg->process(_router);
}

//-----------------------------------------------------------------------------------------
bool MyMenu::help()
{
	get_ostr() << endl;
	get_ostr() << "Key\tCommand" << endl;
	get_ostr() << "===\t=======" << endl;
	for (Handlers::TypeMap::const_iterator itr(_handlers._valuemap.begin()); itr != _handlers._valuemap.end(); ++itr)
		get_ostr() << itr->first._key << '\t' << itr->first._help << endl;
	get_ostr() << endl;
	return true;
}

//-----------------------------------------------------------------------------------------
bool MyMenu::do_logout()
{
	_session.send(new TEX::Logout);
	sleep(1);
	return false; // will exit
}

//-----------------------------------------------------------------------------------------
void print_usage()
{
	UsageMan um("harness", GETARGLIST, "");
	um.setdesc("harness -- menu driven f8 test client/server");
	um.add('s', "server", "run in server mode (default client mode)");
	um.add('h', "help", "help, this screen");
	um.add('v', "version", "print version then exit");
	um.add('l', "log", "global log filename");
	um.add('c', "config", "xml config (default: myfix_client.xml or myfix_server.xml)");
	um.add('q', "quiet", "do not print fix output");
	um.add('R', "receive", "set next expected receive sequence number");
	um.add('S', "send", "set next send sequence number");
	um.add('r', "reliable", "start in reliable mode");
	um.print(cerr);
}

//-----------------------------------------------------------------------------------------
bool tex_router_server::operator() (const TEX::NewOrderSingle *msg) const
{
	static unsigned oid(0), eoid(0);

	TEX::OrderQty qty;
	TEX::Price price;

	msg->get(qty);
	msg->get(price);

	TEX::ExecutionReport *er(new TEX::ExecutionReport);
	msg->copy_legal(er);
	if (!quiet)
		cout << endl;

	ostringstream oistr;
	oistr << "ord" << ++oid;
	*er += new TEX::OrderID(oistr.str());
	*er += new TEX::ExecType(TEX::ExecType_NEW);
	*er += new TEX::OrdStatus(TEX::OrdStatus_NEW);
	*er += new TEX::LeavesQty(qty());
	*er += new TEX::CumQty(0);
	*er += new TEX::AvgPx(0);
	*er += new TEX::LastCapacity('1');
	*er += new TEX::ReportToExch('Y');
	*er += new TEX::ExecID(oistr.str());
	_session.send(er);

	unsigned remaining_qty(qty()), cum_qty(0);
	while (remaining_qty > 0)
	{
		unsigned trdqty(RandDev::getrandom(remaining_qty));
		if (!trdqty)
			trdqty = 1;
		er = new TEX::ExecutionReport;
		msg->copy_legal(er);
		*er += new TEX::OrderID(oistr.str());
		ostringstream eistr;
		eistr << "exec" << ++eoid;
		*er += new TEX::ExecID(eistr.str());
		*er += new TEX::ExecType(TEX::ExecType_NEW);
		*er += new TEX::OrdStatus(remaining_qty == trdqty ? TEX::OrdStatus_FILLED : TEX::OrdStatus_PARTIALLY_FILLED);
		remaining_qty -= trdqty;
		cum_qty += trdqty;
		*er += new TEX::LeavesQty(remaining_qty);
		*er += new TEX::CumQty(cum_qty);
		*er += new TEX::LastQty(trdqty);
		*er += new TEX::AvgPx(price());
		_session.send(er);
	}

	return true;
}

//-----------------------------------------------------------------------------------------
bool tex_router_client::operator() (const TEX::ExecutionReport *msg) const
{
	return true;
}

//-----------------------------------------------------------------------------------------
bool MyMenu::create_msgs()
{
	_cm->CreateMsgs(_tty, _lst);
	return true;
}

//-----------------------------------------------------------------------------------------
bool MyMenu::edit_msgs()
{
	_cm->EditMsgs(_tty, _lst);
	return true;
}

//-----------------------------------------------------------------------------------------
bool MyMenu::delete_msgs()
{
	_cm->DeleteMsgs(_tty, _lst);
	return true;
}

//-----------------------------------------------------------------------------------------
bool MyMenu::send_msgs()
{
	if (_cm->get_yn("Send messages? (y/n):", true))
	{
		for (MsgList::const_iterator itr(_lst.begin()); itr != _lst.end(); ++itr)
			_session.send(*itr);
		_lst.clear();
	}
	return true;
}

//-----------------------------------------------------------------------------------------
bool MyMenu::write_msgs()
{
	cout << "Enter filename: " << flush;
	string fname;
	if (!_cm->GetString(_tty, fname).empty())
	{
		ofstream ofs(fname.c_str());
		if (!ofs)
		{
			cout << Str_error(errno, "Could not open file");
			return false;
		}
		for (MsgList::const_iterator itr(_lst.begin()); itr != _lst.end(); ++itr)
		{
			(static_cast<MessageBase *>(*itr))->encode(ofs); // we only want the main message part, not the header/trailer
			ofs << endl;
		}
	}
	return true;
}

//-----------------------------------------------------------------------------------------
bool MyMenu::read_msgs()
{
#if 0
	cout << "Enter filename: " << flush;
	string fname;
	if (!_cm->GetString(_tty, fname).empty())
	{
		ifstream ifs(fname.c_str());
		if (!ifs)
		{
			cout << Str_error(errno, "Could not open file");
			return false;
		}
		for (MsgList::const_iterator itr(_lst.begin()); itr != _lst.end(); ++itr)
		{
			(static_cast<MessageBase *>(*itr))->decode(buf, 0);
			ofs << endl;
		}
	}
#endif
	return true;
}

