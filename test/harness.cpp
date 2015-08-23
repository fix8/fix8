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
/** \file harness.cpp
\n
  This is a complete working example of a FIX client/server using FIX8.\n
\n
<tt>
Usage: harness [-LRSchlpqrsv] \n
      -L,--lines              set number of screen lines in the console menu (default 50)\n
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
#include <typeinfo>
#ifdef _MSC_VER
#include <signal.h>
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
#include "Myfix_types.hpp"
#include "Myfix_router.hpp"
#include "Myfix_classes.hpp"

#include "myfix.hpp"

//-----------------------------------------------------------------------------------------
using namespace std;
using namespace FIX8;

//-----------------------------------------------------------------------------------------
void print_usage();
const string GETARGLIST("hl:svqc:R:S:rp:L:");
bool term_received(false);

//-----------------------------------------------------------------------------------------
const MyMenu::Handlers MyMenu::_handlers
{
	{ { 'c', "Create messages" }, &MyMenu::create_msgs },
	{ { 'e', "Edit messages" }, &MyMenu::edit_msgs },
	{ { 'd', "Delete one message" }, &MyMenu::delete_msg },
	{ { 'D', "Delete all messages" }, &MyMenu::delete_msgs },
	{ { 'p', "Print messages" }, &MyMenu::print_msgs },
	{ { 's', "Send messages" }, &MyMenu::send_msgs },
	{ { 'r', "Read messages from disk" }, &MyMenu::read_msgs },
	{ { 'S', "Send one message, optionally save before send" }, &MyMenu::send_msg },
	{ { 't', "Toggle heartbeat message display" }, &MyMenu::toggle_heartbeats },
	{ { '?', "Help" }, &MyMenu::help },
	{ { 'l', "Logout" }, &MyMenu::do_logout },
	{ { 'L', "Set Lines per page" }, &MyMenu::set_lpp },
	{ { 'v', "Show version info" }, &MyMenu::version_info },
	{ { 'x', "Exit" }, &MyMenu::do_exit },
};

bool quiet(false);

//-----------------------------------------------------------------------------------------
void sig_handler(int sig)
{
   switch (sig)
   {
   case SIGTERM:
   case SIGINT:
#ifndef _MSC_VER
   case SIGQUIT:
#endif
      term_received = true;
      signal(sig, sig_handler);
      break;
   }
}

//-----------------------------------------------------------------------------------------
int main(int argc, char **argv)
{
	int val, lines(50);
	bool server(false), reliable(false);
	string clcf, replay_file;
	unsigned next_send(0), next_receive(0);

	// for xterm or if tput is available, export LINES so we can use here
	const char *gresult(getenv("LINES"));
	if (gresult && *gresult)
	{
		const string result(gresult);
		const int nlines(stoi(result));
		if (nlines > 10)
			lines = nlines - 4;
	}

#ifdef FIX8_HAVE_GETOPT_LONG
	option long_options[]
	{
		{ "help",		0,	0,	'h' },
		{ "version",	0,	0,	'v' },
		{ "log",			1,	0,	'l' },
		{ "config",		1,	0,	'c' },
		{ "replay",		1,	0,	'p' },
		{ "server",		0,	0,	's' },
		{ "send",		1,	0,	'S' },
		{ "receive",	1,	0,	'R' },
		{ "lines",		1,	0,	'L' },
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
			cout << argv[0] << " for " FIX8_PACKAGE " version " FIX8_VERSION << endl;
			cout << "Released under the GNU LESSER GENERAL PUBLIC LICENSE, Version 3. See <http://fsf.org/> for details." << endl;
			return 0;
		case ':': case '?': return 1;
		case 'h': print_usage(); return 0;
		case 'l': GlobalLogger::set_global_filename(optarg); break;
		case 'p': replay_file = optarg; break;
		case 'c': clcf = optarg; break;
		case 's': server = true; break;
		case 'S': next_send = stoul(optarg); break;
		case 'R': next_receive = stoul(optarg); break;
		case 'L': lines = stoul(optarg); break;
		case 'q': quiet = true; break;
		case 'r': reliable = true; break;
		default: break;
		}
	}

	RandDev::init();

	signal(SIGTERM, sig_handler);
	signal(SIGINT, sig_handler);
#ifndef _MSC_VER
    signal(SIGQUIT, sig_handler);
#endif

	try
	{
		const string conf_file(server ? clcf.empty() ? "myfix_server.xml" : clcf : clcf.empty() ? "myfix_client.xml" : clcf);

		if (server)
		{
			unique_ptr<ServerSessionBase> ms(new ServerSession<myfix_session_server>(TEX::ctx(), conf_file, "TEX1"));

			for (unsigned scnt(0); !term_received; )
			{
				if (!ms->poll())
					continue;
				unique_ptr<FIX8::SessionInstanceBase> inst(ms->create_server_instance());
				if (!quiet)
					inst->session_ptr()->control() |= Session::printnohb;
				glout_info << "client(" << ++scnt << ") connection established.";
				inst->start(true, next_send, next_receive);
				cout << "Session(" << scnt << ") finished." << endl;
				inst->stop();
			}
		}
		else
		{
			unique_ptr<ClientSessionBase>
				mc(reliable ? new ReliableClientSession<myfix_session_client>(TEX::ctx(), conf_file, "DLD1")
							   : new ClientSession<myfix_session_client>(TEX::ctx(), conf_file, "DLD1"));
			if (!quiet)
				mc->session_ptr()->control() |= Session::printnohb;

			if (!reliable)
			{
				const LoginParameters& lparam(mc->session_ptr()->get_login_parameters());
				mc->start(false, next_send, next_receive, lparam._davi());
			}
			else
				mc->start(false);

			ConsoleMenu cm(TEX::ctx(), cin, cout, lines);
			MyMenu mymenu(*mc->session_ptr(), 0, cout, &cm);
			mymenu.get_tty().set_raw_mode();
			hypersleep<h_seconds>(1);

			// permit replaying of test message sets
			if (!replay_file.empty() && mymenu.load_msgs(replay_file))
				mymenu.send_lst();

			for(; !term_received;)
			{
				cout << endl;
				if (mymenu.get_msg_cnt())
					cout << '[' << mymenu.get_msg_cnt() << "] msgs; ";
				cout << "?=help > " << flush;
				char ch{};
				mymenu.get_istr().get(ch);
				cout << ch << endl;
				if (mymenu.get_istr().bad() || ch == 0x3 || !mymenu.process(ch))
					break;
			}

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
bool myfix_session_client::handle_application(const unsigned seqnum, const Message *&msg)
{
	return enforce(seqnum, msg) || msg->process(_router);
}

//-----------------------------------------------------------------------------------------
void myfix_session_client::state_change(const FIX8::States::SessionStates before, const FIX8::States::SessionStates after)
{
	cout << get_session_state_string(before) << " => " << get_session_state_string(after) << endl;
}

//-----------------------------------------------------------------------------------------
bool myfix_session_server::handle_application(const unsigned seqnum, const Message *&msg)
{
	return enforce(seqnum, msg) || msg->process(_router);
}

//-----------------------------------------------------------------------------------------
void myfix_session_server::state_change(const FIX8::States::SessionStates before, const FIX8::States::SessionStates after)
{
	cout << get_session_state_string(before) << " => " << get_session_state_string(after) << endl;
}

//-----------------------------------------------------------------------------------------
bool MyMenu::help()
{
	get_ostr() << endl;
	get_ostr() << "Key\tCommand" << endl;
	get_ostr() << "===\t=======" << endl;
	for (const auto& pp : _handlers)
		get_ostr() << pp.first._key << '\t' << pp.first._help << endl;
	get_ostr() << endl;
	return true;
}

//-----------------------------------------------------------------------------------------
bool MyMenu::set_lpp()
{
	_ostr << "Enter number of lines per page (currently=" << _cm->get_lpp() << "): " << flush;
	f8String str;
	if (!_cm->GetString(_tty, str).empty())
		_cm->set_lpp(stoi(str));
	return true;
}

//-----------------------------------------------------------------------------------------
bool MyMenu::do_logout()
{
	if (!_session.is_shutdown())
	{
		_session.send(new TEX::Logout);
		get_ostr() << "logout..." << endl;
	}
	hypersleep<h_seconds>(1);
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
	um.add('p', "replay", "name of fix input file to send on connect");
	um.add('c', "config", "xml config (default: myfix_client.xml or myfix_server.xml)");
	um.add('q', "quiet", "do not print fix output");
	um.add('R', "receive", "set next expected receive sequence number");
	um.add('L', "lines", "set number of screen lines in the console menu (default 50)");
	um.add('S', "send", "set next send sequence number");
	um.add('r', "reliable", "start in reliable mode");
	um.print(cerr);
}

//-----------------------------------------------------------------------------------------
bool tex_router_server::operator() (const TEX::NewOrderSingle *msg) const
{
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
bool MyMenu::version_info()
{
	_ostr << endl;
	for (const auto& pp : package_info())
		_ostr << pp.first << ": " << pp.second << endl;
	return true;
}

//-----------------------------------------------------------------------------------------
bool MyMenu::edit_msgs()
{
	_cm->EditMsgs(_tty, _lst);
	return true;
}

//-----------------------------------------------------------------------------------------
bool MyMenu::delete_msg()
{
	_cm->DeleteMsgs(_tty, _lst);
	return true;
}

//-----------------------------------------------------------------------------------------
bool MyMenu::delete_msgs()
{
	_cm->DeleteAllMsgs(_tty, _lst);
	return true;
}

//-----------------------------------------------------------------------------------------
void MyMenu::send_lst()
{
	for (auto *pp : _lst)
		_session.send(pp);
	_lst.clear();
}

//-----------------------------------------------------------------------------------------
bool MyMenu::send_msgs()
{
	if (_lst.size() && _cm->get_yn("Send messages? (y/n):", true))
		send_lst();
	return true;
}

//-----------------------------------------------------------------------------------------
bool MyMenu::print_msgs()
{
	for (const auto *pp : _lst)
		_ostr << *pp << endl;
	return true;
}

//-----------------------------------------------------------------------------------------
bool MyMenu::send_msg()
{
	unique_ptr<Message> msg(_cm->RemoveMsg(_tty, _lst));
	if (msg.get())
	{
		string fname;
		_ostr << endl;
		bool save(_cm->get_yn("Save message after send? (y/n):", true));
		_ostr << endl;
		if (save)
		{
			_ostr << "Enter filename: " << flush;
			_cm->GetString(_tty, fname);
		}
		if (_cm->get_yn("Send message? (y/n):", true))
		{
			_session.send(msg.get(), false);
			if (save && !fname.empty())
				save_msg(fname, msg.get());
		}
	}
	return true;
}

//-----------------------------------------------------------------------------------------
bool MyMenu::save_msg(const string& fname, Message *msg)
{
#if !defined FIX8_RAW_MSG_SUPPORT
	cerr << endl << "RAW_MSG_SUPPORT support not enabled. Run configure with --enable-rawmsgsupport" << endl;
#else
	if (exist(fname))
		_ostr << endl << fname << " exists, will append message" << endl;
	ofstream ofs(fname.c_str(), ios::app);
	if (!ofs)
	{
		cerr << Str_error(errno, "Could not open file");
		return false;
	}
	ofs << msg->get_rawmsg() << endl; // requires fix8 built with --enable-rawmsgsupport
#endif
	return true;
}

//-----------------------------------------------------------------------------------------
bool MyMenu::toggle_heartbeats()
{
	if (_session.control() & Session::printnohb)
	{
		_session.control().clear(Session::printnohb);
		_session.control().set(Session::print);
		get_ostr() << "Heartbeat display on";
	}
	else
	{
		_session.control().clear(Session::print);
		_session.control().set(Session::printnohb);
		get_ostr() << "Heartbeat display off";
	}
	return true;
}

//-----------------------------------------------------------------------------------------
bool MyMenu::load_msgs(const string& fname)
{
	ifstream ifs(fname.c_str());
	if (!ifs)
	{
		cerr << Str_error(errno, "Could not open file");
		return false;
	}

	char buffer[FIX8_MAX_MSG_LENGTH];
	unsigned loaded(0), skipped(0);
	while (!ifs.eof())
	{
		ifs.getline(buffer, FIX8_MAX_MSG_LENGTH - 1);
		if (!buffer[0])
			continue;
		Message *msg(Message::factory(TEX::ctx(), buffer));
		if (msg->is_admin())
		{
			++skipped;
			continue;
		}
		sender_comp_id sci;
		msg->Header()->get(sci);
		target_comp_id tci;
		msg->Header()->get(tci);
		if (_session.get_sid().same_side_sender_comp_id(sci) && _session.get_sid().same_side_target_comp_id(tci))
		{
			++loaded;
			delete msg->Header()->remove(Common_SendingTime); // will re-add on send
			msg->setup_reuse();
			_lst.push_back(msg);
		}
		else
			++skipped;
	}

	_ostr << loaded << " msgs loaded, " << skipped << " msgs skipped" << endl;

	return true;
}

//-----------------------------------------------------------------------------------------
bool MyMenu::read_msgs()
{
   char cwd[FIX8_MAX_FLD_LENGTH];
   _ostr << "Playback (*.playback) files in ";
#ifdef _MSC_VER
	_ostr << _getcwd(cwd, sizeof(cwd)) << endl;
   if (system("dir *.playback"));
#else
	_ostr << getcwd(cwd, sizeof(cwd)) << endl;
   if (system("ls -l *.playback"));
#endif
   _ostr << endl;
	_ostr << "Enter filename: " << flush;
	string fname;
	if (!_cm->GetString(_tty, fname).empty())
		load_msgs(fname);
	return true;
}

