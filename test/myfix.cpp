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
/** \file myfix.cpp
\n
  This is a complete working example of a FIX client/server using FIX8.\n
\n
<tt>
Usage: f8test [-NRScdhlmoqrsv]\n
   -N,--session            for client, select session to use from configuration (default none)\n
   -R,--receive            set next expected receive sequence number\n
   -S,--send               set next send sequence number\n
   -c,--config             xml config (default: myfix_client.xml or myfix_server.xml)\n
   -d,--dump               dump parsed XML config file, exit\n
   -h,--help               help, this screen\n
   -l,--log                global log filename\n
   -m,--multi              run multiple server mode (default single server session at a time)\n
   -o,--once               for server, allow one client session then exit\n
   -q,--quiet              do not print fix output\n
   -r,--reliable           start in reliable mode\n
   -s,--server             run in server mode (default client mode)\n
   -v,--version            print version, exit\n
</tt>
\n
\n
  To use start the server:\n
\n
<tt>
	  % f8test -sl server\n
</tt>
\n
  In another terminal session, start the client:\n
\n
<tt>
	  % f8test -l client\n
</tt>
\n
  \b Notes \n
\n
  1. The client has a simple menu. Press ? to see options.\n
  2. The server will wait for the client to logout before exiting.\n
  3. The server uses \c myfix_client.xml and the client uses \c myfix_server.xml for configuration settings.\n
  4. The example uses the files \c FIX50SP2.xml and \c FIXT11.xml in ./schema\n
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
#include <vector>
#include <iterator>
#include <algorithm>
#include <typeinfo>
#include <thread>
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
//#include <fix8/zeromq_mbus.hpp>

#ifdef FIX8_HAVE_GETOPT_H
#include <getopt.h>
#endif

#include <fix8/usage.hpp>
#include <fix8/consolemenu.hpp>
#include <fix8/multisession.hpp>
#include "Myfix_types.hpp"
#include "Myfix_router.hpp"
#include "Myfix_classes.hpp"

#include "myfix.hpp"

//-----------------------------------------------------------------------------------------
using namespace std;
using namespace FIX8;

//-----------------------------------------------------------------------------------------
void print_usage();
const string GETARGLIST("hl:svqc:R:S:rdomN:D:");
f8_atomic<bool> term_received(false);
void server_process(ServerSessionBase *srv, int scnt, bool ismulti=false), client_process(ClientSessionBase *mc);
FIX8::tty_save_state save_tty(0);

//-----------------------------------------------------------------------------------------
const MyMenu::Handlers MyMenu::_handlers
{
	{ { 'n', "New Order Single" }, &MyMenu::new_order_single },
	{ { 'a', "New Order Single (alternate group method)" }, &MyMenu::new_order_single_alternate },
	{ { 'r', "New Order Single Recycled - 1st use send as normal then will send recycled message" },
		&MyMenu::new_order_single_recycled },
	{ { 'N', "50 New Order Singles" }, &MyMenu::new_order_single_50 },
	{ { 'T', "1000 New Order Singles" }, &MyMenu::new_order_single_1000 },
	{ { 'R', "Resend request" }, &MyMenu::resend_request },
	{ { '?', "Help" }, &MyMenu::help },
	{ { 'l', "Logout" }, &MyMenu::do_logout },
	{ { 't', "Examine static message traits" }, &MyMenu::static_probe },
	{ { 'x', "Exit" }, &MyMenu::do_exit },
};

bool quiet(false);
unsigned next_send(0), next_receive(0);

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
	int val;
	bool server(false), reliable(false), once(false), dump(false), multi(false);
	string clcf, session;

#ifdef FIX8_HAVE_GETOPT_LONG
	option long_options[]
	{
		{ "help",		0,	0,	'h' },
		{ "version",	0,	0,	'v' },
		{ "log",			1,	0,	'l' },
		{ "delimiter",	1,	0,	'D' },
		{ "config",		1,	0,	'c' },
		{ "session",	1,	0,	'N' },
		{ "once",	   0,	0,	'o' },
		{ "server",		0,	0,	's' },
		{ "multi",		0,	0,	'm' },
		{ "send",		1,	0,	'S' },
		{ "receive",	1,	0,	'R' },
		{ "quiet",		0,	0,	'q' },
		{ "reliable",	0,	0,	'r' },
		{ "dump",		0,	0,	'd' },
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
		case 'D': GlobalLogger::set_delimiter(optarg); break;
		case 'c': clcf = optarg; break;
		case 's': server = true; break;
		case 'N': session = optarg; break;
		case 'm': multi = true; break;
		case 'o': once = true; break;
		case 'S': next_send = stoul(optarg); break;
		case 'R': next_receive = stoul(optarg); break;
		case 'q': quiet = true; break;
		case 'r': reliable = true; break;
		case 'd': dump = true; break;
		default: break;
		}
	}

	RandDev::init();

	signal(SIGTERM, sig_handler);
	signal(SIGINT, sig_handler);
#ifndef _MSC_VER
	signal(SIGQUIT, sig_handler);
#endif

	bool restore_tty(false);

	try
	{
		const string conf_file(server ? clcf.empty() ? "myfix_server.xml" : clcf : clcf.empty() ? "myfix_client.xml" : clcf);
		f8_atomic<unsigned> scnt(0);

		if (dump)
		{
			XmlElement *root(XmlElement::Factory(conf_file));
			if (root)
				cout << *root << endl;
			else
				cerr << "Failed to parse " << conf_file << endl;
			return 0;
		}

		if (server)
		{
			if (multi)	// demonstrate use of multi session server manager
			{
				unique_ptr<ServerManager> sm(new ServerManager);
				sm->add(new ServerSession<myfix_session_server>(TEX::ctx(), conf_file, "TEX1"));
				sm->add(new ServerSession<myfix_session_server>(TEX::ctx(), conf_file, "TEX2"));

				vector<thread> thrds;
				while (!term_received)
				{
					ServerSessionBase *srv(sm->select());
					if (srv)
					{
						thrds.push_back(thread ([&]() { server_process(srv, ++scnt, true); }));
						hypersleep<h_seconds>(1);
					}
				}
				for_each(thrds.begin(), thrds.end(), [](thread& tt) { if (tt.joinable()) tt.join(); });
			}
			else	// serial server instances only
			{
				unique_ptr<ServerSessionBase> srv(new ServerSession<myfix_session_server>(TEX::ctx(), conf_file, "TEX"));

				while (!term_received)
				{
					if (!srv->poll())
						continue;
					server_process(srv.get(), ++scnt);
					if (once)
						break;
				}
			}
		}
		else
		{
			if (multi)	// demonstrate use of multi session client manager
			{
				const f8String cl1("DLD1"), cl2("DLD2");
				ClientManager cm;
				cm.add(cl1, reliable ? new ReliableClientSession<myfix_session_client>(TEX::ctx(), conf_file, cl1)
											: new ClientSession<myfix_session_client>(TEX::ctx(), conf_file, cl1));
				cm.add(cl2, reliable ? new ReliableClientSession<myfix_session_client>(TEX::ctx(), conf_file, cl2)
											: new ClientSession<myfix_session_client>(TEX::ctx(), conf_file, cl2));

				ClientSessionBase *csb(cm.for_each_if([&](ClientSessionBase *pp)
				{
					if (!quiet)
						pp->session_ptr()->control() |= Session::printnohb;
					pp->start(false, next_send, next_receive, pp->session_ptr()->get_login_parameters()._davi());
					// if you use ReliableClientSession, you need to return true here always
					return reliable ? true : States::is_live(pp->session_ptr()->get_session_state());
				}));

				if (csb)
					cerr << csb->_session_name << " failed to start" << endl;

				vector<thread> thrds;
				cm.for_each_if([&](ClientSessionBase *pp)
				{
					// we should be running client_process however this won't work
					// since two threads can't share the same console
					thrds.push_back(thread ([=]()		// use copy closure
					{
						MyMenu mymenu(*pp->session_ptr(), 0, cout);
						hypersleep<h_seconds>(1 + RandDev::getrandom(10));
						mymenu.new_order_single();	// send an order and then logout
						mymenu.do_logout();
					}));
					return true;
				});

				for_each(thrds.begin(), thrds.end(), [](thread& tt) { if (tt.joinable()) tt.join(); });
			}
			else	// single client only
			{
				const string my_session(session.empty() ? "DLD1" : session);
				unique_ptr<ClientSessionBase>
					mc(reliable ? new ReliableClientSession<myfix_session_client>(TEX::ctx(), conf_file, my_session)
									: new ClientSession<myfix_session_client>(TEX::ctx(), conf_file, my_session));
				if (!quiet)
					mc->session_ptr()->control() |= Session::printnohb;
				mc->start(false, next_send, next_receive, mc->session_ptr()->get_login_parameters()._davi());
				client_process(mc.get());
			}
		}
	}
	catch (f8Exception& e)
	{
		cerr << "exception: " << e.what() << endl;
		restore_tty = true;
		glout_error << e.what();
	}
	catch (exception& e)	// also catches Poco::Net::NetException
	{
		cerr << "std::exception: " << e.what() << endl;
		restore_tty = true;
		glout_error << e.what();
	}
	catch (...)
	{
		cerr << "unknown exception" << endl;
		restore_tty = true;
		glout_error << "unknown exception";
	}

	if (restore_tty && !server)
		save_tty.unset_raw_mode();

	if (term_received)
		cout << endl << "terminated." << endl;
	return 0;
}

//-----------------------------------------------------------------------------------------
void client_process(ClientSessionBase *mc)
{
	MyMenu mymenu(*mc->session_ptr(), 0, cout);
	cout << endl << "Menu started. Press '?' for help..." << endl << endl;
	char ch(0);
	mymenu.get_tty().set_raw_mode();
	save_tty = mymenu.get_tty();
	while(!mymenu.get_istr().get(ch).bad() && !mc->has_given_up() && !term_received && ch != 0x3 && mymenu.process(ch))
		;
	// don't explicitly call mc->session_ptr()->stop() with reliable sessions
	// before checking if the session is already shutdown - the framework will generally do this for you
	if (!mc->session_ptr()->is_shutdown())
		mc->session_ptr()->stop();

	mymenu.get_tty().unset_raw_mode();
}

//-----------------------------------------------------------------------------------------
void server_process(ServerSessionBase *srv, int scnt, bool ismulti)
{
	unique_ptr<SessionInstanceBase> inst(srv->create_server_instance());
	if (!quiet)
		inst->session_ptr()->control() |= Session::print;
	glout_info << "client(" << scnt << ") connection established.";
	const ProcessModel pm(srv->get_process_model(srv->_ses));
	cout << (pm == pm_pipeline ? "Pipelined" : "Threaded") << " mode." << endl;
	inst->start(pm == pm_pipeline, next_send, next_receive);
	if (inst->session_ptr()->get_connection()->is_secure())
		cout << "Session is secure (SSL)" << endl;
	if (!ismulti && !quiet)	// demonstrate use of timer events
	{
		TimerEvent<FIX8::Session> sample_callback(static_cast<bool (FIX8::Session::*)()>(&myfix_session_server::sample_scheduler_callback), true);
		inst->session_ptr()->get_timer().schedule(sample_callback, 60000); // call sample_scheduler_callback every minute forever
	}
	if (pm != pm_pipeline)
		while (!inst->session_ptr()->is_shutdown())
			FIX8::hypersleep<h_milliseconds>(100);
	cout << "Session(" << scnt << ") finished." << endl;
	inst->stop();
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
	if (enforce(seqnum, msg))
		return false;
	// this is how you take ownership of the message from the framework
	if (!msg->process(_router)) // false means I have taken ownership of the message
		detach(msg);
	return true;
}

//-----------------------------------------------------------------------------------------
void myfix_session_server::state_change(const FIX8::States::SessionStates before, const FIX8::States::SessionStates after)
{
	cout << get_session_state_string(before) << " => " << get_session_state_string(after) << endl;
}

//-----------------------------------------------------------------------------------------
bool myfix_session_server::sample_scheduler_callback()
{
	cout << "myfix_session_server::sample_scheduler_callback Hello!" << endl;
	return true;
}

//-----------------------------------------------------------------------------------------
Message *MyMenu::generate_new_order_single_alternate()
{
	static unsigned oid(0);
	ostringstream oistr;
	oistr << "ord" << ++oid;
	TEX::NewOrderSingle *nos(new TEX::NewOrderSingle(false)); // shallow construction
	*nos << new TEX::TransactTime
	     << new TEX::OrderQty(1 + RandDev::getrandom(9999))
	     << new TEX::Price(RandDev::getrandom(500.), 5)	// 5 decimal places if necessary
	     << new TEX::ClOrdID(oistr.str())
	     << new TEX::Symbol("BHP")
	     << new TEX::OrdType(TEX::OrdType_LIMIT)
	     << new TEX::Side(TEX::Side_BUY)
	     << new TEX::TimeInForce(TEX::TimeInForce_FILL_OR_KILL);

	*nos << new TEX::NoPartyIDs(unsigned(0));
	*nos << new TEX::NoUnderlyings(3);
	GroupBase *noul(nos->find_add_group<TEX::NewOrderSingle::NoUnderlyings>(nullptr)); // no parent group

	// repeating groups
	MessageBase *gr1(noul->create_group(false)); // shallow construction
	*gr1 << new TEX::UnderlyingSymbol("BLAH")
	     << new TEX::UnderlyingQty(1 + RandDev::getrandom(999));
	*noul << gr1;

	MessageBase *gr2(noul->create_group(false)); // shallow construction
	// nested repeating groups
	*gr2 << new TEX::UnderlyingSymbol("FOO")
	     << new TEX::NoUnderlyingSecurityAltID(2);
	*noul << gr2;
	GroupBase *nosai(gr2->find_add_group<TEX::NewOrderSingle::NoUnderlyings::NoUnderlyingSecurityAltID>(noul)); // parent is noul
	MessageBase *gr3(nosai->create_group(false)); // shallow construction
	*gr3 << new TEX::UnderlyingSecurityAltID("UnderBlah");
	*nosai << gr3;
	MessageBase *gr4(nosai->create_group(false)); // shallow construction
	*gr4 << new TEX::UnderlyingSecurityAltID("OverFoo");
	*nosai << gr4;

	MessageBase *gr5(noul->create_group(false)); // shallow construction
	*gr5 << new TEX::UnderlyingSymbol("BOOM");
	// nested repeating groups
	GroupBase *nus(gr5->find_add_group<TEX::NewOrderSingle::NoUnderlyings::NoUnderlyingStips>(noul)); // parent is noul
	static const char *secIDs[] { "Reverera", "Orlanda", "Withroon", "Longweed", "Blechnod" };
	*gr5 << new TEX::NoUnderlyingStips(sizeof(secIDs)/sizeof(char *));
	for (size_t ii(0); ii < sizeof(secIDs)/sizeof(char *); ++ii)
	{
		MessageBase *gr(nus->create_group(false)); // shallow construction
		*gr << new TEX::UnderlyingStipType(secIDs[ii]);
		*nus << gr;
	}
	*noul << gr5;

	// multiply nested repeating groups
	*nos << new TEX::NoAllocs(1);
	GroupBase *noall(nos->find_add_group<TEX::NewOrderSingle::NoAllocs>(nullptr)); // no parent group
	MessageBase *gr9(noall->create_group(false)); // shallow construction
	*gr9 << new TEX::AllocAccount("Account1")
	     << new TEX::NoNestedPartyIDs(1);
	*noall << gr9;
	GroupBase *nonp(gr9->find_add_group<TEX::NewOrderSingle::NoAllocs::NoNestedPartyIDs>(noall)); // parent is noall
	MessageBase *gr10(nonp->create_group(false)); // shallow construction
	*gr10 << new TEX::NestedPartyID("nestedpartyID1")
	      << new TEX::NoNestedPartySubIDs(1);
	*nonp << gr10;
	GroupBase *nonpsid(gr10->find_add_group<TEX::NewOrderSingle::NoAllocs::NoNestedPartyIDs::NoNestedPartySubIDs>(nonp)); // parent is nonp
	MessageBase *gr11(nonpsid->create_group(false)); // shallow construction
	*gr11 << new TEX::NestedPartySubID("subnestedpartyID1");
	*nonpsid << gr11;

	return nos;
}

//-----------------------------------------------------------------------------------------
Message *MyMenu::generate_new_order_single()
{
	static unsigned oid(0);
	ostringstream oistr;
	oistr << "ord" << ++oid;
	TEX::NewOrderSingle *nos(new TEX::NewOrderSingle);
	*nos << new TEX::TransactTime
	     << new TEX::OrderQty(1 + RandDev::getrandom(9999))
	     << new TEX::Price(RandDev::getrandom(500.), 5)	// 5 decimal places if necessary
	     << new TEX::ClOrdID(oistr.str())
	     << new TEX::Symbol("BHP")
	     << new TEX::OrdType(TEX::OrdType_LIMIT)
	     << new TEX::Side(TEX::Side_BUY)
	     << new TEX::TimeInForce(TEX::TimeInForce_FILL_OR_KILL);

	*nos << new TEX::NoPartyIDs(unsigned(0));
	*nos << new TEX::NoUnderlyings(3);
	GroupBase *noul(nos->find_group<TEX::NewOrderSingle::NoUnderlyings>());

	// repeating groups
	MessageBase *gr1(noul->create_group());
	*gr1 << new TEX::UnderlyingSymbol("BLAH")
	     << new TEX::UnderlyingQty(1 + RandDev::getrandom(999));
	*noul << gr1;

	MessageBase *gr2(noul->create_group());
	// nested repeating groups
	*gr2 << new TEX::UnderlyingSymbol("FOO")
	     << new TEX::NoUnderlyingSecurityAltID(2);
	*noul << gr2;
	GroupBase *nosai(gr2->find_group<TEX::NewOrderSingle::NoUnderlyings::NoUnderlyingSecurityAltID>());
	MessageBase *gr3(nosai->create_group());
	*gr3 << new TEX::UnderlyingSecurityAltID("UnderBlah");
	*nosai << gr3;
	MessageBase *gr4(nosai->create_group());
	*gr4 << new TEX::UnderlyingSecurityAltID("OverFoo");
	*nosai << gr4;

	MessageBase *gr5(noul->create_group());
	*gr5 << new TEX::UnderlyingSymbol("BOOM");
	// nested repeating groups
	GroupBase *nus(gr5->find_group<TEX::NewOrderSingle::NoUnderlyings::NoUnderlyingStips>());
	static const char *secIDs[] { "Reverera", "Orlanda", "Withroon", "Longweed", "Blechnod" };
	*gr5 << new TEX::NoUnderlyingStips(sizeof(secIDs)/sizeof(char *));
	for (size_t ii(0); ii < sizeof(secIDs)/sizeof(char *); ++ii)
	{
		MessageBase *gr(nus->create_group());
		*gr << new TEX::UnderlyingStipType(secIDs[ii]);
		*nus << gr;
	}
	*noul << gr5;

	// multiply nested repeating groups
	*nos << new TEX::NoAllocs(1);
	GroupBase *noall(nos->find_group<TEX::NewOrderSingle::NoAllocs>());
	MessageBase *gr9(noall->create_group());
	*gr9 << new TEX::AllocAccount("Account1")
	     << new TEX::NoNestedPartyIDs(1);
	*noall << gr9;
	GroupBase *nonp(gr9->find_group<TEX::NewOrderSingle::NoAllocs::NoNestedPartyIDs>());
	MessageBase *gr10(nonp->create_group());
	*gr10 << new TEX::NestedPartyID("nestedpartyID1")
	      << new TEX::NoNestedPartySubIDs(1);
	*nonp << gr10;
	GroupBase *nonpsid(gr10->find_group<TEX::NewOrderSingle::NoAllocs::NoNestedPartyIDs::NoNestedPartySubIDs>());
	MessageBase *gr11(nonpsid->create_group());
	*gr11 << new TEX::NestedPartySubID("subnestedpartyID1");
	*nonpsid << gr11;

	return nos;
}

//-----------------------------------------------------------------------------------------
bool MyMenu::new_order_single_alternate()
{
	_session.send(generate_new_order_single_alternate());
	return true;
}

//-----------------------------------------------------------------------------------------
bool MyMenu::new_order_single()
{
	_session.send(generate_new_order_single());
	return true;
}

//-----------------------------------------------------------------------------------------
bool MyMenu::new_order_single_recycled()
{
	// create a message; first time through we just send it as normal. Note the we pass the no-destroy flag;
	// second and subsequent times we just recyle the old message

	static bool first(true);
	static Message *msg(0);
	if (first)
	{
		cout << "Sending new new_order_single" << endl;
		_session.send(msg = generate_new_order_single(), first = false);
	}
	else
	{
		msg->setup_reuse();
		cout << "Sending recycled new_order_single" << endl;
		_session.send(msg, false);
	}
#if defined FIX8_RAW_MSG_SUPPORT
	// demonstrate access to outbound raw fix message and payload
	cout << msg->get_rawmsg() << endl;
	copy(msg->begin_payload(), msg->end_payload(), ostream_iterator<char>(cout, ""));
	cout << endl;
	cout << "payload begin=" << msg->get_payload_begin() << " payload len=" << msg->get_payload_len() << endl;
#endif
	return true;
}

//-----------------------------------------------------------------------------------------
bool MyMenu::static_probe()
{
#if defined FIX8_HAVE_EXTENDED_METADATA
	(cout << "Enter message tag:").flush();
	f8String result;
	const BaseMsgEntry *tbme;
	if (!get_string(result).empty() && (tbme = _session.get_ctx()._bme.find_ptr(result.c_str())))
	{
        function<void( const TraitHelper&, int )> print_traits;
		print_traits = ([&print_traits, this](const TraitHelper& tr, int depth)
		{
			const string spacer(depth * MessageBase::get_tabsize(), ' ');
			for (F8MetaCntx::const_iterator itr(F8MetaCntx::begin(tr)); itr != F8MetaCntx::end(tr); ++itr)
			{
				const BaseEntry *be(_session.get_ctx().find_be(itr->_fnum));
				cout << spacer << be->_name;
				if (be->_rlm)
					cout << " Realm:" << (be->_rlm->_dtype == RealmBase::dt_range ? "range" : "set")
						<< '(' << be->_rlm->_sz << ") ";
				cout << *itr << endl;
				if (itr->_field_traits.has(FieldTrait::group))
					print_traits(itr->_group, depth + 1);
			}
		});

		cout << tbme->_name << endl;
		print_traits(tbme->_create._get_traits(), 1);
	}
	else
		cout << "Unknown message tag: " << result << endl;
#else
	cout << "Extended metadata not available (try ./configure --enable-extended-metadata=yes)." << endl;
#endif

	return true;
}

//-----------------------------------------------------------------------------------------
bool MyMenu::new_order_single_50()
{
	vector<Message *> msgs;
	for (auto ii(0); ii < 50; ++ii)
		msgs.push_back(generate_new_order_single());

	_session.send_batch(msgs);
	return true;
}

//-----------------------------------------------------------------------------------------
bool MyMenu::new_order_single_1000()
{
	vector<Message *> msgs;
	for (auto ii(0); ii < 1000; ++ii)
		msgs.push_back(generate_new_order_single());

	_session.send_batch(msgs);
	return true;
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
bool MyMenu::resend_request()
{
	if (!_session.is_shutdown())
	{
		unsigned bnum(0), bend(0);
		cout << "Enter BeginSeqNo:" << flush;
		_tty.unset_raw_mode();
		cin >> bnum;
		cout << "Enter EndSeqNo(0=all):" << flush;
		cin >> bend;
		_tty.set_raw_mode();
		_session.send(_session.generate_resend_request(bnum, bend));
	}
	return true;
}

//-----------------------------------------------------------------------------------------
void print_usage()
{
	UsageMan um("f8test", GETARGLIST, "");
	um.setdesc("f8test -- f8 test client/server");
	um.add('s', "server", "run in server mode (default client mode)");
	um.add('m', "multi", "run in multiple server or client mode (default serial server or single client session at a time)");
	um.add('h', "help", "help, this screen");
	um.add('v', "version", "print version, exit");
	um.add('l', "log", "global log filename");
	um.add('o', "once", "for server, allow one client session then exit");
	um.add('c', "config", "xml config (default: myfix_client.xml or myfix_server.xml)");
	um.add('q', "quiet", "do not print fix output");
	um.add('R', "receive", "set next expected receive sequence number");
	um.add('S', "send", "set next send sequence number");
	um.add('D', "delimiter", "set GlobalLogger field delimiter (default ' ')");
	um.add('N', "session", "for client, select session to use from configuration (default DLD1)");
	um.add('r', "reliable", "start in reliable mode");
	um.add('d', "dump", "dump parsed XML config file, exit");
	um.add("e.g.");
	um.add("@f8test -sml server_log");
	um.add("@f8test -rl client_log");
	um.add("@f8test -rl server -S 124");
	um.print(cerr);
}

//-----------------------------------------------------------------------------------------
bool tex_router_server::operator() (const TEX::NewOrderSingle *msg) const
{
#if defined FIX8_RAW_MSG_SUPPORT
	// demonstrate access to inbound raw fix message and payload
	cout << msg->get_rawmsg() << endl;
	copy(msg->begin_payload(), msg->end_payload(), ostream_iterator<char>(cout, ""));
	cout << endl;
	cout << "payload begin=" << msg->get_payload_begin() << " payload len=" << msg->get_payload_len() << endl;
#endif
#if 0
	const Presence& pre(msg->get_fp().get_presence());
   for (Fields::const_iterator itr(msg->fields_begin()); itr != msg->fields_end(); ++itr)
   {
      const FieldTrait::FieldType trait(pre.find(itr->first)->_ftype);
      cout << itr->first << " is ";
      if (FieldTrait::is_int(trait))
         cout << "int";
      else if (FieldTrait::is_char(trait))
         cout << "char";
      else if (FieldTrait::is_string(trait))
         cout << "string";
      else if (FieldTrait::is_float(trait))
         cout << "float";
      else
         cout << "unknown";
		cout << '\t' << _session.get_ctx().find_be(itr->first)->_name << '\t' << *itr->second << endl;
   }
#endif

	static unsigned oid(0), eoid(0);

	TEX::OrderQty qty;
	TEX::Price price;

	if (!quiet)
	{
		// This is how you extract a copy of a field value
		if (msg->get(qty))
			cout << "Order qty (copy):" << qty() << endl;

		// This is how you get a field value in place
		if (msg->has<TEX::OrderQty>())
			cout << "Order qty (in place):" << msg->get<TEX::OrderQty>()->get() << endl;

		if (msg->get(price))
			cout << "price:" << price() << endl;

		// This is how you extract values from a repeating group
		const GroupBase *grnoul(msg->find_group<TEX::NewOrderSingle::NoUnderlyings>());
		if (grnoul)
		{
			for (size_t cnt(0); cnt < grnoul->size(); ++cnt)
			{
				TEX::UnderlyingSymbol unsym;
				MessageBase *me(grnoul->get_element(static_cast<unsigned>(cnt)));
				me->get(unsym);
				cout << "Underlying symbol:" << unsym() << endl;
				// This is how you extract values from a nested repeating group
				GroupBase *nus(me->find_group<TEX::NewOrderSingle::NoUnderlyings::NoUnderlyingStips>());
				if (nus)
				{
					for (size_t cnt(0); cnt < nus->size(); ++cnt)
					{
						TEX::UnderlyingStipType stipType;
						MessageBase *me(nus->get_element(static_cast<unsigned>(cnt)));
						me->get(stipType);
						cout << "Underlying StipType:" << stipType() << endl;
					}
				}
			}
		}

		const GroupBase *grallocs(msg->find_group<TEX::NewOrderSingle::NoAllocs>());
		if (grnoul)
		{
			for (size_t cnt(0); cnt < grallocs->size(); ++cnt)
			{
				TEX::AllocAccount acc;
				MessageBase *me(grallocs->get_element(static_cast<unsigned>(cnt)));
				me->get(acc);
				cout << "TEX::NewOrderSingle::NoAllocs Account:" << acc() << endl;
				// This is how you extract values from a nested repeating group
				GroupBase *nnpi(me->find_group<TEX::NewOrderSingle::NoAllocs::NoNestedPartyIDs>());
				if (nnpi)
				{
					for (size_t cnt(0); cnt < nnpi->size(); ++cnt)
					{
						TEX::NestedPartyID npi;
						MessageBase *me(nnpi->get_element(static_cast<unsigned>(cnt)));
						me->get(npi);
						cout << "TEX::NewOrderSingle::NoAllocs::NoNestedPartyIDs NestedPartyID:" << npi() << endl;
						// This is how you extract values from a nested nested repeating group
						GroupBase *nnpsi(me->find_group<TEX::NewOrderSingle::NoAllocs::NoNestedPartyIDs::NoNestedPartySubIDs>());
						if (nnpsi)
						{
							for (size_t cnt(0); cnt < nnpsi->size(); ++cnt)
							{
								TEX::NestedPartySubID npsi;
								MessageBase *me(nnpsi->get_element(static_cast<unsigned>(cnt)));
								me->get(npsi);
								cout << "TEX::NewOrderSingle::NoAllocs::NoNestedPartyIDs::NoNestedPartySubIDs NestedPartySubID:" << npsi() << endl;
							}
						}
					}
				}
			}
		}
	}

	TEX::ExecutionReport *er(new TEX::ExecutionReport);
	msg->copy_legal(er);
	if (!quiet)
		cout << endl;

	ostringstream oistr;
	oistr << "ord" << ++oid;
	*er << new TEX::OrderID(oistr.str())
	    << new TEX::ExecType(TEX::ExecType_NEW)
	    << new TEX::OrdStatus(TEX::OrdStatus_NEW)
	    << new TEX::LeavesQty(qty())
	    << new TEX::CumQty(0.)
	    << new TEX::AvgPx(0.)
	    << new TEX::LastCapacity('5')
	    << new TEX::ReportToExch('Y')
	    << new TEX::ExecID(oistr.str());
	msg->push_unknown(er);
	_session.send(er);

	unsigned remaining_qty(qty()), cum_qty(0);
	while (remaining_qty > 0)
	{
		unsigned trdqty(RandDev::getrandom(remaining_qty));
		if (!trdqty)
			trdqty = 1;
		er = new TEX::ExecutionReport;
		msg->copy_legal(er);
		ostringstream eistr;
		eistr << "exec" << ++eoid;
		remaining_qty -= trdqty;
		cum_qty += trdqty;
		*er << new TEX::OrderID(oistr.str())
		    << new TEX::ExecID(eistr.str())
		    << new TEX::ExecType(TEX::ExecType_NEW)
		    << new TEX::OrdStatus(remaining_qty == trdqty ? TEX::OrdStatus_FILLED : TEX::OrdStatus_PARTIALLY_FILLED)
		    << new TEX::LeavesQty(remaining_qty)
		    << new TEX::CumQty(cum_qty)
		    << new TEX::LastQty(trdqty)
		    << new TEX::AvgPx(price());
		_session.send(er);
	}

	return true;
}

//-----------------------------------------------------------------------------------------
bool tex_router_client::operator() (const TEX::ExecutionReport *msg) const
{
	TEX::LastCapacity lastCap;
	if (msg->get(lastCap))
	{
		// if we have set a realm range for LastCapacity, when can check it here
		if (!quiet && !lastCap.is_valid())
			cout << "TEX::LastCapacity(" << lastCap << ") is not a valid value" << endl;
	}
	return true;
}

