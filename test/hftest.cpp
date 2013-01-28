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
/** \file hftest.cpp
\n
  This is a complete working example of a HF FIX client/server using FIX8.\n
\n
<tt>
	Usage: hftest [-RSchlqrsv]\n
		-R,--receive            set next expected receive sequence number\n
		-S,--send               set next send sequence number\n
		-c,--config             xml config (default: hf_client.xml or hf_server.xml)\n
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
	  % hftest -sl server\n
</tt>
\n
  In another terminal session, start the client:\n
\n
<tt>
	  % hftest -l client\n
</tt>
\n
  \b Notes \n
\n
  1. Configure with \c --enable-codectiming \n
  2. The client has a simple menu. Press ? to see options.\n
  3. The server will wait for the client to logout before exiting.\n
  4. Press P to preload NewOrderSingle messages, T to transmit them.\n
  5. The server uses \c hf_client.xml and the client uses \c hf_server.xml for configuration settings. \n
  6. The example uses \c FIX42.xml \n
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
#include "Perf_types.hpp"
#include "Perf_router.hpp"
#include "Perf_classes.hpp"

#include "hftest.hpp"

//-----------------------------------------------------------------------------------------
using namespace std;
using namespace FIX8;

//-----------------------------------------------------------------------------------------
void print_usage();
const string GETARGLIST("hl:svqc:R:S:rb:");
bool term_received(false);
unsigned batch_size(1000);

//-----------------------------------------------------------------------------------------
namespace FIX8
{
	template<>
	const MyMenu::Handlers::TypePair MyMenu::Handlers::_valueTable[] =
	{
		MyMenu::Handlers::TypePair(MyMenu::MenuItem('n', "Send a NewOrderSingle msg"), &MyMenu::new_order_single),
		MyMenu::Handlers::TypePair(MyMenu::MenuItem('p', "Preload n NewOrderSingle msgs"), &MyMenu::preload_new_order_single),
		MyMenu::Handlers::TypePair(MyMenu::MenuItem('b', "Batch preload and send n NewOrderSingle msgs"), &MyMenu::batch_preload_new_order_single),
		MyMenu::Handlers::TypePair(MyMenu::MenuItem('N', "Send n NewOrderSingle msgs"), &MyMenu::multi_new_order_single),
		MyMenu::Handlers::TypePair(MyMenu::MenuItem('a', "Send all Preloaded NewOrderSingle msgs"), &MyMenu::send_all_preloaded),
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

bool quiet(true);

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
	default:
		cerr << sig << endl;
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
		{ "batch",		1,	0,	'b' },
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
		case 'b': batch_size = GetValue<unsigned>(optarg); break;
		case 's': server = true; break;
		case 'S': next_send = GetValue<unsigned>(optarg); break;
		case 'R': next_receive = GetValue<unsigned>(optarg); break;
		case 'q': quiet = false; break;
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
		const string conf_file(server ? clcf.empty() ? "hf_server.xml" : clcf : clcf.empty() ? "hf_client.xml" : clcf);

		if (server)
		{
			ServerSession<hf_session_server>::Server_ptr
				ms(new ServerSession<hf_session_server>(TEX::ctx, conf_file, "TEX1"));

			XmlElement::XmlSet eset;

			for (unsigned scnt(0); !term_received; )
			{
				if (!ms->poll())
					continue;
				SessionInstance<hf_session_server>::Instance_ptr
					inst(new SessionInstance<hf_session_server>(*ms));
				if (!quiet)
					inst->session_ptr()->control() |= Session::print;
				ostringstream sostr;
				sostr << "client(" << ++scnt << ") connection established.";
				GlobalLogger::log(sostr.str());
				inst->start(true, next_send, next_receive);
				cout << "Session(" << scnt << ") finished." << endl;
				inst->stop();
#if defined BUFFERED_GLOBAL_LOGGING
				GlobalLogger::flush_log();
#endif
			}
		}
		else if (reliable)
		{
			ReliableClientSession<hf_session_client>::Client_ptr
				mc(new ReliableClientSession<hf_session_client>(TEX::ctx, conf_file, "DLD1"));
			if (!quiet)
				mc->session_ptr()->control() |= Session::print;
			mc->start(false, next_send, next_receive);
			MyMenu mymenu(*mc->session_ptr(), 0, cout);
			char ch;
			mymenu.get_tty().set_raw_mode();
			while(!mymenu.get_istr().get(ch).bad() && !term_received && ch != 0x3 && mymenu.process(ch))
				;
			// don't explicitly call mc->session_ptr()->stop() with reliable sessions
			// before checking if the session is already shutdown - the framework will generally do this for you
#if defined BUFFERED_GLOBAL_LOGGING
			GlobalLogger::flush_log();
#endif
			if (!mc->session_ptr()->is_shutdown())
				mc->session_ptr()->stop();
			mymenu.get_tty().unset_raw_mode();
		}
		else
		{
			ClientSession<hf_session_client>::Client_ptr
				mc(new ClientSession<hf_session_client>(TEX::ctx, conf_file, "DLD1"));
			if (!quiet)
				mc->session_ptr()->control() |= Session::print;
			const LoginParameters& lparam(mc->session_ptr()->get_login_parameters());
			mc->start(false, next_send, next_receive, lparam._davi());
			MyMenu mymenu(*mc->session_ptr(), 0, cout);
			char ch;
			mymenu.get_tty().set_raw_mode();
			while(!mymenu.get_istr().get(ch).bad() && !term_received && ch != 0x3 && mymenu.process(ch))
				;
#if defined BUFFERED_GLOBAL_LOGGING
			GlobalLogger::flush_log();
#endif
			if (!mc->session_ptr()->is_shutdown())
				mc->session_ptr()->stop();
			mymenu.get_tty().unset_raw_mode();
		}
	}
	catch (f8Exception& e)
	{
#if defined BUFFERED_GLOBAL_LOGGING
		GlobalLogger::flush_log();
#endif
		cerr << "exception: " << e.what() << endl;
	}
	catch (exception& e)	// also catches Poco::Net::NetException
	{
#if defined BUFFERED_GLOBAL_LOGGING
			GlobalLogger::flush_log();
#endif
		cerr << "exception: " << e.what() << endl;
	}

	if (term_received)
		cout << "terminated." << endl;
	return 0;
}

//-----------------------------------------------------------------------------------------
bool MyMenu::batch_preload_new_order_single()
{
	cout << "Enter number of NewOrderSingle msgs to batch preload:";
	cout.flush();
	unsigned num(0);
	_tty.unset_raw_mode();
	cin >> num;
	_tty.set_raw_mode();
	while (num > 0)
	{
		unsigned cnt(0);
		for (; cnt < num && cnt < batch_size; ++cnt)
		{
			static unsigned oid(10000);
			ostringstream oistr;
			oistr << "ord" << ++oid << '-' << num;

			TEX::NewOrderSingle *ptr(new TEX::NewOrderSingle);
			TEX::Price *prc(new TEX::Price(1. + RandDev::getrandom(500.)));
			prc->set_precision(3);

			*ptr << new TEX::Symbol("BHP")
				  << new TEX::HandlInst(TEX::HandlInst_AUTOMATED_EXECUTION_ORDER_PRIVATE_NO_BROKER_INTERVENTION)
				  << new TEX::OrdType(TEX::OrdType_LIMIT)
				  << new TEX::Side(TEX::Side_BUY)
				  << new TEX::TimeInForce(TEX::TimeInForce_FILL_OR_KILL)
				  << new TEX::TransactTime
				  << new TEX::ClOrdID(oistr.str())
				  << prc
				  << new TEX::OrderQty(1 + RandDev::getrandom(10000));

			_session.push(ptr);
		}
		cout << _session.cached() << " NewOrderSingle msgs preloaded." << endl;
		num -= cnt;
		send_all_preloaded();
	}

	return true;
}

//-----------------------------------------------------------------------------------------
bool MyMenu::multi_new_order_single()
{
	cout << "Enter number of NewOrderSingle msgs to send:";
	cout.flush();
	unsigned num(0);
	_tty.unset_raw_mode();
	cin >> num;
	_tty.set_raw_mode();
	for (unsigned ii(0); ii < num; ++ii)
		new_order_single();
	cout << endl << num << " NewOrderSingle msgs sent." << endl;

	return true;
}

//-----------------------------------------------------------------------------------------
bool MyMenu::new_order_single()
{
	static unsigned oid(0);
	ostringstream oistr;
	oistr << "ord" << ++oid;

	TEX::NewOrderSingle *nos(new TEX::NewOrderSingle);
	*nos  << new TEX::TransactTime
			<< new TEX::OrderQty(1 + RandDev::getrandom(10000))
			<< new TEX::Price(1. + RandDev::getrandom(500.))
			<< new TEX::ClOrdID(oistr.str())
			<< new TEX::Symbol("BHP")
			<< new TEX::HandlInst(TEX::HandlInst_AUTOMATED_EXECUTION_ORDER_PRIVATE_NO_BROKER_INTERVENTION)
			<< new TEX::OrdType(TEX::OrdType_LIMIT)
			<< new TEX::Side(TEX::Side_BUY)
			<< new TEX::TimeInForce(TEX::TimeInForce_FILL_OR_KILL);

	_session.send(nos);

	return true;
}

//-----------------------------------------------------------------------------------------
bool MyMenu::send_all_preloaded()
{
	unsigned snt(0);
	while (_session.cached())
	{
		TEX::NewOrderSingle *ptr(_session.pop());
		if (!ptr)
			break;
		_session.send(ptr);
		if (++snt % 1000 == 0)
		{
			cout << '\r' << snt << " NewOrderSingle msgs sent";
			cout.flush();
		}
	}
	cout << endl << _session.cached() << " NewOrderSingle msgs remaining." << endl;
	return true;
}

//-----------------------------------------------------------------------------------------
bool MyMenu::preload_new_order_single()
{
	cout << endl << _session.cached() << " NewOrderSingle msgs currently preloaded." << endl;
	cout << "Enter number of NewOrderSingle msgs to preload:";
	cout.flush();
	int num(0);
	_tty.unset_raw_mode();
	cin >> num;
	_tty.set_raw_mode();
	for (int ii(0); ii < num; ++ii)
	{
		static unsigned oid(10000);
		ostringstream oistr;
		oistr << "ord" << ++oid << '-' << num;

		TEX::NewOrderSingle *ptr(new TEX::NewOrderSingle);
		TEX::Price *prc(new TEX::Price(1. + RandDev::getrandom(500.)));
		prc->set_precision(3);

		*ptr  << new TEX::Symbol("BHP")
				<< new TEX::HandlInst(TEX::HandlInst_AUTOMATED_EXECUTION_ORDER_PRIVATE_NO_BROKER_INTERVENTION)
				<< new TEX::OrdType(TEX::OrdType_LIMIT)
				<< new TEX::Side(TEX::Side_BUY)
				<< new TEX::TimeInForce(TEX::TimeInForce_FILL_OR_KILL)
				<< new TEX::TransactTime
				<< prc
				<< new TEX::ClOrdID(oistr.str())
				<< new TEX::OrderQty(1 + RandDev::getrandom(10000));

		_session.push(ptr);
	}

	cout << _session.cached() << " NewOrderSingle msgs preloaded." << endl;

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
	UsageMan um("hftest", GETARGLIST, "");
	um.setdesc("hftest -- f8 HF test client/server");
	um.add('s', "server", "run in server mode (default client mode)");
	um.add('h', "help", "help, this screen");
	um.add('v', "version", "print version then exit");
	um.add('l', "log", "global log filename");
	um.add('c', "config", "xml config (default: hf_client.xml or hf_server.xml)");
	um.add('q', "quiet", "do not print fix output (default yes)");
	um.add('R', "receive", "set next expected receive sequence number");
	um.add('S', "send", "set next send sequence number");
	um.add('r', "reliable", "start in reliable mode");
	um.print(cerr);
}

//-----------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------
bool hf_session_client::handle_application(const unsigned seqnum, const FIX8::Message *msg)
{
	return enforce(seqnum, msg) || msg->process(_router);
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

	ostringstream oistr;
	oistr << "ord" << ++oid;
	*er << new TEX::OrderID(oistr.str())
		 << new TEX::ExecType(TEX::ExecType_NEW);
	unsigned ordResult(RandDev::getrandom(3));
	switch (ordResult)
	{
	default:
	case 0:
		*er << new TEX::OrdStatus(TEX::OrdStatus_NEW);
		break;
	case 1:
		*er << new TEX::OrdStatus(TEX::OrdStatus_CANCELED);
		break;
	case 2:
		*er << new TEX::OrdStatus(TEX::OrdStatus_REJECTED);
		break;
	}

	*er   << new TEX::LeavesQty(qty())
			<< new TEX::CumQty(0)
			<< new TEX::AvgPx(0)
			<< new TEX::LastCapacity('5')
			<< new TEX::ReportToExch('Y')
			<< new TEX::ExecTransType(TEX::ExecTransType_NEW)
			<< new TEX::ExecID(oistr.str());

	_session.send(er);

	if (ordResult == 0)
	{
		unsigned remaining_qty(qty()), cum_qty(0);
		while (remaining_qty > 0)
		{
			unsigned trdqty(1 + RandDev::getrandom(remaining_qty));
			remaining_qty -= trdqty;
			cum_qty += trdqty;
			TEX::ExecutionReport *ner(new TEX::ExecutionReport);
			msg->copy_legal(ner);
			ostringstream eistr;
			eistr << "exec" << ++eoid;

			*ner  << new TEX::ExecID(eistr.str())
					<< new TEX::OrderID(oistr.str())
					<< new TEX::ExecType(TEX::ExecType_NEW)
					<< new TEX::OrdStatus(remaining_qty == trdqty ? TEX::OrdStatus_FILLED : TEX::OrdStatus_PARTIALLY_FILLED)
					<< new TEX::LeavesQty(remaining_qty)
					<< new TEX::CumQty(cum_qty)
					<< new TEX::ExecTransType(TEX::ExecTransType_NEW)
					<< new TEX::AvgPx(price());

			_session.send(ner);
		}
	}

	return true;
}

//-----------------------------------------------------------------------------------------
bool tex_router_client::operator() (const TEX::ExecutionReport *msg) const
{
	static int exrecv(0);
	if (++exrecv % 5000 == 0)
	{
		cout << '\r' << exrecv << " ExecutionReport msgs received";
		cout.flush();
	}
	return true;
}

//-----------------------------------------------------------------------------------------
bool hf_session_server::handle_application(const unsigned seqnum, const FIX8::Message *msg)
{
	return enforce(seqnum, msg) || msg->process(_router);
}

