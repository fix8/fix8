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
/** \file myfix.cpp

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
		-r,--reliable           start in reliable mode
		-s,--server             run in server mode (default client mode)\n
		-v,--version            print version then exit
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

  \b Notes \n
\n
  1. If you have configured with \c --enable-msgrecycle, the example will reuse allocated messages.\n
  2. If you have configured with \c --enable-customfields, the example will add custom fields\n
     defined below.\n
  3. The client has a simple menu. Press ? to see options.\n
  4. The server will wait for the client to logout before exiting.\n
  5. The server uses \c myfix_client.xml and the client uses \c myfix_server.xml for configuration settings.\n
  6. The example uses the file \c FIX44.xml in ./schema\n
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
template<>
const MyMenu::Handlers::TypePair MyMenu::Handlers::_valueTable[] =
{
	MyMenu::Handlers::TypePair(MyMenu::MenuItem('n', "New Order Single"), &MyMenu::new_order_single),
	MyMenu::Handlers::TypePair(MyMenu::MenuItem('N', "50 New Order Singles"), &MyMenu::new_order_single_50),
	MyMenu::Handlers::TypePair(MyMenu::MenuItem('?', "Help"), &MyMenu::help),
	MyMenu::Handlers::TypePair(MyMenu::MenuItem('l', "Logout"), &MyMenu::do_logout),
	MyMenu::Handlers::TypePair(MyMenu::MenuItem('x', "Exit"), &MyMenu::do_exit),
};
template<>
const MyMenu::Handlers::NotFoundType MyMenu::Handlers::_noval = &MyMenu::nothing;
template<>
const MyMenu::Handlers::TypeMap MyMenu::Handlers::_valuemap(MyMenu::Handlers::_valueTable,
	MyMenu::Handlers::get_table_end());
bool quiet(false);

//-----------------------------------------------------------------------------------------
#if defined PERMIT_CUSTOM_FIELDS
#include "myfix_custom.hpp"
#endif

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
		{ "log",			0,	0,	'l' },
		{ "config",		0,	0,	'c' },
		{ "server",		0,	0,	's' },
		{ "send",		0,	0,	'S' },
		{ "receive",	0,	0,	'R' },
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

#if defined PERMIT_CUSTOM_FIELDS
	TEX::myfix_custom custfields(true); // will cleanup; modifies ctx
	TEX::ctx.set_ube(&custfields);
#endif

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
			MyMenu mymenu(*mc->session_ptr(), 0, cout);
			char ch;
			mymenu.set_raw_mode();
			while(!mymenu.get_istr().get(ch).bad() && !term_received && ch != 0x3 && mymenu.process(ch))
				;
			mymenu.unset_raw_mode();
		}
		else
		{
			ClientSession<myfix_session_client>::Client_ptr
				mc(new ClientSession<myfix_session_client>(TEX::ctx, conf_file, "DLD1"));
			if (!quiet)
				mc->session_ptr()->control() |= Session::print;
			mc->start(false, next_send, next_receive);
			MyMenu mymenu(*mc->session_ptr(), 0, cout);
			char ch;
			mymenu.set_raw_mode();
			while(!mymenu.get_istr().get(ch).bad() && !term_received && ch != 0x3 && mymenu.process(ch))
				;
			mymenu.unset_raw_mode();
		}
	}
	catch (f8Exception& e)
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

bool myfix_session_server::handle_admin(const unsigned seqnum, const Message *msg)
{
	return msg->process(_router);
}

//-----------------------------------------------------------------------------------------
bool MyMenu::new_order_single()
{
	TEX::NewOrderSingle *nos(new TEX::NewOrderSingle);
	*nos += new TEX::TransactTime;
	*nos += new TEX::OrderQty(1 + RandDev::getrandom(9999));
	*nos += new TEX::Price(RandDev::getrandom(500.));
	static unsigned oid(0);
	ostringstream oistr;
	oistr << "ord" << ++oid;
	*nos += new TEX::ClOrdID(oistr.str());
	*nos += new TEX::Symbol("BHP");
	*nos += new TEX::OrdType(TEX::OrdType_LIMIT);
	*nos += new TEX::Side(TEX::Side_BUY);
	*nos += new TEX::TimeInForce(TEX::TimeInForce_FILLORKILL);

	*nos += new TEX::NoUnderlyings(2);
	GroupBase *noul(nos->find_group<TEX::NewOrderSingle::NoUnderlyings>());
	MessageBase *gr1(noul->create_group());
	*gr1 += new TEX::UnderlyingSymbol("BLAH");
	*noul += gr1;
	MessageBase *gr2(noul->create_group());
	*gr2 += new TEX::UnderlyingSymbol("FOO");
	*noul += gr2;

	_session.send(nos);

	return true;
}

//-----------------------------------------------------------------------------------------
bool MyMenu::new_order_single_50()
{
	for (int ii(0); ii < 50; ++ii)
		new_order_single();

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
	_session.send(new TEX::Logout);
	sleep(1);
	return false; // will exit
}

//-----------------------------------------------------------------------------------------
void print_usage()
{
	UsageMan um("f8test", GETARGLIST, "");
	um.setdesc("f8test -- f8 test client/server");
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
			}
		}
	}

	//ostringstream gerr;
	//IntervalTimer itm;
#if defined MSGRECYCLING
	scoped_ptr<TEX::ExecutionReport> er(new TEX::ExecutionReport);
	msg->copy_legal(er.get());
#else
	TEX::ExecutionReport *er(new TEX::ExecutionReport);
	msg->copy_legal(er);
#endif
	if (!quiet)
		cout << endl;
	//gerr << "encode:" << itm.Calculate();
	//GlobalLogger::log(gerr.str());

	ostringstream oistr;
	oistr << "ord" << ++oid;
	*er += new TEX::OrderID(oistr.str());
	*er += new TEX::ExecType(TEX::ExecType_NEW);
	*er += new TEX::OrdStatus(TEX::OrdStatus_NEW);
	*er += new TEX::LeavesQty(qty());
	*er += new TEX::CumQty(0);
	*er += new TEX::AvgPx(0);
	*er += new TEX::LastCapacity('5');
	*er += new TEX::ReportToExch('Y');
#if defined PERMIT_CUSTOM_FIELDS
	*er += new TEX::Orderbook('X');
	*er += new TEX::BrokerInitiated(true);
	*er += new TEX::ExecOption(3);

#endif
#if defined MSGRECYCLING
	er->set_in_use(true);	// indicate this message is in use again
	delete er->Header()->remove(Common_MsgSeqNum); // we want to reuse, not resend
	*er += new TEX::AvgPx(9999); // will replace and delete original
	_session.send_wait(er.get());
#else
	_session.send(er);
#endif

	unsigned remaining_qty(qty()), cum_qty(0);
	while (remaining_qty > 0)
	{
		unsigned trdqty(RandDev::getrandom(remaining_qty));
		if (!trdqty)
			trdqty = 1;
#if defined MSGRECYCLING
		while(er->get_in_use())
			microsleep(10);
		er->set_in_use(true);	// indicate this message is in use again
		delete er->Header()->remove(Common_MsgSeqNum); // we want to reuse, not resend
#else
		er = new TEX::ExecutionReport;
		msg->copy_legal(er);
#endif
		*er += new TEX::OrderID(oistr.str());
		ostringstream eistr;
		eistr << "exec" << ++eoid;
		*er += new TEX::ExecID(eistr.str());
		*er += new TEX::ExecType(TEX::ExecType_NEW);
		*er += new TEX::OrdStatus(remaining_qty == trdqty ? TEX::OrdStatus_FILLED : TEX::OrdStatus_PARTIAL);
		remaining_qty -= trdqty;
		cum_qty += trdqty;
		*er += new TEX::LeavesQty(remaining_qty);
		*er += new TEX::CumQty(cum_qty);
		*er += new TEX::LastQty(trdqty);
		*er += new TEX::AvgPx(price());
#if defined MSGRECYCLING
		_session.send_wait(er.get());
#else
		_session.send(er);
#endif
	}

	return true;
}

//-----------------------------------------------------------------------------------------
bool tex_router_server::operator() (const TEX::Logout *msg) const
{
	return _session.set_is_logged_out();
}

//-----------------------------------------------------------------------------------------
bool tex_router_client::operator() (const TEX::ExecutionReport *msg) const
{
	TEX::LastCapacity lastCap;
	if (msg->get(lastCap))
	{
		if (!quiet && !lastCap.is_valid())
			cout << "TEX::LastCapacity(" << lastCap << ") is not a valid value" << endl;
	}
	return true;
}

