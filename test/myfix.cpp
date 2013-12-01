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
/** \file myfix.cpp
\n
  This is a complete working example of a FIX client/server using FIX8.\n
\n
<tt>
	Usage: f8test [-RSchlqrsv]\n
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
  1. If you have configured with \c --enable-msgrecycle, the example will reuse allocated messages.\n
  2. The client has a simple menu. Press ? to see options.\n
  3. The server will wait for the client to logout before exiting.\n
  4. The server uses \c myfix_client.xml and the client uses \c myfix_server.xml for configuration settings.\n
  5. The example uses the files \c FIX50SP2.xml and \c FIXT11.xml in ./schema\n
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
#include <sys/ioctl.h>
#include <signal.h>
#include <termios.h>

#include <regex.h>
#include <errno.h>
#include <string.h>

// f8 headers
#include <fix8/f8includes.hpp>

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
const string GETARGLIST("hl:svqc:R:S:rdo");
bool term_received(false);

//-----------------------------------------------------------------------------------------
const MyMenu::Handlers::TypePair MyMenu::_valueTable[] =
{
	MyMenu::Handlers::TypePair(MyMenu::MenuItem('n', "New Order Single"), &MyMenu::new_order_single),
	MyMenu::Handlers::TypePair(MyMenu::MenuItem('N', "50 New Order Singles"), &MyMenu::new_order_single_50),
	MyMenu::Handlers::TypePair(MyMenu::MenuItem('T', "1000 New Order Singles"), &MyMenu::new_order_single_1000),
	MyMenu::Handlers::TypePair(MyMenu::MenuItem('R', "Resend request"), &MyMenu::resend_request),
	MyMenu::Handlers::TypePair(MyMenu::MenuItem('?', "Help"), &MyMenu::help),
	MyMenu::Handlers::TypePair(MyMenu::MenuItem('l', "Logout"), &MyMenu::do_logout),
	MyMenu::Handlers::TypePair(MyMenu::MenuItem('x', "Exit"), &MyMenu::do_exit),
};
const MyMenu::Handlers MyMenu::_handlers(MyMenu::_valueTable,
	sizeof(MyMenu::_valueTable)/sizeof(MyMenu::Handlers::TypePair), &MyMenu::nothing);

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
	bool server(false), reliable(false), once(false), dump(false);
	string clcf;
	unsigned next_send(0), next_receive(0);

#ifdef HAVE_GETOPT_LONG
	option long_options[] =
	{
		{ "help",		0,	0,	'h' },
		{ "version",	0,	0,	'v' },
		{ "log",			1,	0,	'l' },
		{ "config",		1,	0,	'c' },
		{ "once",	   0,	0,	'o' },
		{ "server",		0,	0,	's' },
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
			cout << argv[0] << " for " PACKAGE " version " VERSION << endl;
			cout << "Released under the GNU LESSER GENERAL PUBLIC LICENSE, Version 3. See <http://fsf.org/> for details." << endl;
			return 0;
		case ':': case '?': return 1;
		case 'h': print_usage(); return 0;
		case 'l': GlobalLogger::set_global_filename(optarg); break;
		case 'c': clcf = optarg; break;
		case 's': server = true; break;
		case 'o': once = true; break;
		case 'S': next_send = get_value<unsigned>(optarg); break;
		case 'R': next_receive = get_value<unsigned>(optarg); break;
		case 'q': quiet = true; break;
		case 'r': reliable = true; break;
		case 'd': dump = true; break;
		default: break;
		}
	}

	RandDev::init();

	signal(SIGTERM, sig_handler);
	signal(SIGINT, sig_handler);
	signal(SIGQUIT, sig_handler);

	FIX8::tty_save_state save_tty(0);
	bool restore_tty(false);

	try
	{
		const string conf_file(server ? clcf.empty() ? "myfix_server.xml" : clcf : clcf.empty() ? "myfix_client.xml" : clcf);

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
			ServerSession<myfix_session_server>::Server_ptr
				ms(new ServerSession<myfix_session_server>(TEX::ctx(), conf_file, "TEX1"));

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
            if (once)
               break;
			}
		}
		else
		{
			scoped_ptr<ClientSession<myfix_session_client> >
				mc(reliable ? new ReliableClientSession<myfix_session_client>(TEX::ctx(), conf_file, "DLD1")
							   : new ClientSession<myfix_session_client>(TEX::ctx(), conf_file, "DLD1"));
			if (!quiet)
				mc->session_ptr()->control() |= Session::printnohb;

			if (!reliable)
				mc->start(false, next_send, next_receive, mc->session_ptr()->get_login_parameters()._davi());
			else
				mc->start(false, next_send, next_receive);

			MyMenu mymenu(*mc->session_ptr(), 0, cout);
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
	}
	catch (f8Exception& e)
	{
		cerr << "exception: " << e.what() << endl;
		restore_tty = true;
	}
	catch (exception& e)	// also catches Poco::Net::NetException
	{
		cerr << "exception: " << e.what() << endl;
		restore_tty = true;
	}
	catch (...)
	{
		cerr << "unknown exception" << endl;
		restore_tty = true;
	}

	if (restore_tty && !server)
		save_tty.unset_raw_mode();

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
bool myfix_session_server::handle_application(const unsigned seqnum, const Message *&msg)
{
	if (enforce(seqnum, msg))
		return true;
	// this is how you take ownership of the message from the framework
	if (!msg->process(_router)) // false means I have taken ownership of the message
		detach(msg);
	return false;
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
	static const char *secIDs[] = { "Reverera", "Orlanda", "Withroon", "Longweed", "Blechnod" };
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
bool MyMenu::new_order_single()
{
	_session.send(generate_new_order_single());
	return true;
}

//-----------------------------------------------------------------------------------------
bool MyMenu::new_order_single_50()
{
	vector<Message *> msgs;
	for (int ii(0); ii < 50; ++ii)
		msgs.push_back(generate_new_order_single());

	_session.send_batch(msgs);
	return true;
}

//-----------------------------------------------------------------------------------------
bool MyMenu::new_order_single_1000()
{
	vector<Message *> msgs;
	for (int ii(0); ii < 1000; ++ii)
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
	for (Handlers::TypeMap::const_iterator itr(_handlers._valuemap.begin()); itr != _handlers._valuemap.end(); ++itr)
		get_ostr() << itr->first._key << '\t' << itr->first._help << endl;
	get_ostr() << endl;
	return true;
}

//-----------------------------------------------------------------------------------------
bool MyMenu::do_logout()
{
	if (!_session.is_shutdown())
		_session.send(new TEX::Logout);
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
	um.add('h', "help", "help, this screen");
	um.add('v', "version", "print version, exit");
	um.add('l', "log", "global log filename");
	um.add('o', "once", "for server, allow one client session then exit");
	um.add('c', "config", "xml config (default: myfix_client.xml or myfix_server.xml)");
	um.add('q', "quiet", "do not print fix output");
	um.add('R', "receive", "set next expected receive sequence number");
	um.add('S', "send", "set next send sequence number");
	um.add('r', "reliable", "start in reliable mode");
	um.add('d', "dump", "dump parsed XML config file, exit");
	um.print(cerr);
}

//-----------------------------------------------------------------------------------------
bool tex_router_server::operator() (const TEX::NewOrderSingle *msg) const
{
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
				MessageBase *me(grnoul->get_element(cnt));
				me->get(unsym);
				cout << "Underlying symbol:" << unsym() << endl;
				// This is how you extract values from a nested repeating group
				GroupBase *nus(me->find_group<TEX::NewOrderSingle::NoUnderlyings::NoUnderlyingStips>());
				if (nus)
				{
					for (size_t cnt(0); cnt < nus->size(); ++cnt)
					{
						TEX::UnderlyingStipType stipType;
						MessageBase *me(nus->get_element(cnt));
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
				MessageBase *me(grallocs->get_element(cnt));
				me->get(acc);
				cout << "TEX::NewOrderSingle::NoAllocs Account:" << acc() << endl;
				// This is how you extract values from a nested repeating group
				GroupBase *nnpi(me->find_group<TEX::NewOrderSingle::NoAllocs::NoNestedPartyIDs>());
				if (nnpi)
				{
					for (size_t cnt(0); cnt < nnpi->size(); ++cnt)
					{
						TEX::NestedPartyID npi;
						MessageBase *me(nnpi->get_element(cnt));
						me->get(npi);
						cout << "TEX::NewOrderSingle::NoAllocs::NoNestedPartyIDs NestedPartyID:" << npi() << endl;
						// This is how you extract values from a nested nested repeating group
						GroupBase *nnpsi(me->find_group<TEX::NewOrderSingle::NoAllocs::NoNestedPartyIDs::NoNestedPartySubIDs>());
						if (nnpsi)
						{
							for (size_t cnt(0); cnt < nnpsi->size(); ++cnt)
							{
								TEX::NestedPartySubID npsi;
								MessageBase *me(nnpsi->get_element(cnt));
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

