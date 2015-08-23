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
		-u,--update             update interval for console counters (default 5000)\n
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
#include "Perf_types.hpp"
#include "Perf_router.hpp"
#include "Perf_classes.hpp"

#include "hftest.hpp"

//-----------------------------------------------------------------------------------------
using namespace std;

//-----------------------------------------------------------------------------------------
void print_usage();
const string GETARGLIST("hl:svqc:R:S:rb:p:u:o");
bool term_received(false);
unsigned batch_size(1000), preload_count(0), update_count(5000);

//-----------------------------------------------------------------------------------------
const MyMenu::Handlers MyMenu::_handlers
{
	{ { 'n', "Send a NewOrderSingle msg" }, &MyMenu::new_order_single },
	{ { 'p', "Preload n NewOrderSingle msgs" }, &MyMenu::preload_new_order_single },
	{ { 'b', "Batch preload and send n NewOrderSingle msgs" }, &MyMenu::batch_preload_new_order_single },
	{ { 'N', "Send n NewOrderSingle msgs" }, &MyMenu::multi_new_order_single },
	{ { 'a', "Send all Preloaded NewOrderSingle msgs" }, &MyMenu::send_all_preloaded },
	{ { '?', "Help" }, &MyMenu::help },
	{ { 'l', "Logout" }, &MyMenu::do_logout },
	{ { 'x', "Exit" }, &MyMenu::do_exit },
};

bool quiet(true);

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
	default:
		cerr << sig << endl;
		break;
   }
}

//-----------------------------------------------------------------------------------------
int main(int argc, char **argv)
{
	int val;
	bool server(false), once(false), reliable(false);
	string clcf;
	unsigned next_send(0), next_receive(0);

#ifdef FIX8_HAVE_GETOPT_LONG
	option long_options[]
	{
		{ "help",		0,	0,	'h' },
		{ "version",	0,	0,	'v' },
		{ "once",	   0,	0,	'o' },
		{ "log",			1,	0,	'l' },
		{ "config",		1,	0,	'c' },
		{ "server",		0,	0,	's' },
		{ "batch",		1,	0,	'b' },
		{ "send",		1,	0,	'S' },
		{ "receive",	1,	0,	'R' },
		{ "quiet",		0,	0,	'q' },
		{ "reliable",	0,	0,	'r' },
		{ "preload",	1,	0,	'p' },
		{ "update",		1,	0,	'u' },
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
		case 'l': FIX8::GlobalLogger::set_global_filename(optarg); break;
		case 'c': clcf = optarg; break;
		case 'b': batch_size = stoul(optarg); break;
		case 'p': preload_count = stoul(optarg); break;
		case 'u': update_count = stoul(optarg); break;
		case 's': server = true; break;
		case 'o': once = true; break;
		case 'S': next_send = stoul(optarg); break;
		case 'R': next_receive = stoul(optarg); break;
		case 'q': quiet = false; break;
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
		const string conf_file(server ? clcf.empty() ? "hf_server.xml" : clcf : clcf.empty() ? "hf_client.xml" : clcf);

		if (server)
		{
			unique_ptr<FIX8::ServerSessionBase> ms(new FIX8::ServerSession<hf_session_server>(FIX8::TEX::ctx(), conf_file, "TEX1"));

			XmlElement::XmlSet eset;

			for (unsigned scnt(0); !term_received; )
			{
				if (!ms->poll())
					continue;
				unique_ptr<FIX8::SessionInstanceBase> inst(ms->create_server_instance());
				if (!quiet)
					inst->session_ptr()->control() |= FIX8::Session::print;
				ostringstream sostr;
				sostr << "client(" << ++scnt << ") connection established.";
				FIX8::GlobalLogger::log(sostr.str());
				const FIX8::ProcessModel pm(ms->get_process_model(ms->_ses));
				inst->start(pm == FIX8::pm_pipeline, next_send, next_receive);
				cout << (pm == FIX8::pm_pipeline ? "Pipelined" : "Threaded") << " mode." << endl;
				if (inst->session_ptr()->get_connection()->is_secure())
					cout << "Session is secure (SSL)" << endl;
				if (pm != FIX8::pm_pipeline)
					while (!inst->session_ptr()->is_shutdown())
						FIX8::hypersleep<FIX8::h_milliseconds>(100);
				cout << "Session(" << scnt << ") finished." << endl;
				inst->stop();
#if defined FIX8_CODECTIMING
				FIX8::Message::report_codec_timings("server");
#endif
            if (once)
               break;
			}
		}
		else
		{
			unique_ptr<FIX8::ClientSessionBase>
				mc(reliable ? new FIX8::ReliableClientSession<hf_session_client>(FIX8::TEX::ctx(), conf_file, "DLD1")
							   : new FIX8::ClientSession<hf_session_client>(FIX8::TEX::ctx(), conf_file, "DLD1"));
			if (!quiet)
				mc->session_ptr()->control() |= FIX8::Session::print;

			const FIX8::ProcessModel pm(mc->get_process_model(mc->_ses));
			if (!reliable)
				mc->start(false, next_send, next_receive, mc->session_ptr()->get_login_parameters()._davi());
			else
				mc->start(false, next_send, next_receive);

			MyMenu mymenu(*mc->session_ptr(), 0, cout);
			cout << endl << "Menu started. Press '?' for help..." << endl << endl;
			if (mc->session_ptr()->get_connection()->is_secure())
				cout << "Session is secure (SSL)" << endl;
			if (preload_count)
				mymenu.preload_new_order_single();
			char ch;
			mymenu.get_tty().set_raw_mode();
			if (pm == FIX8::pm_coro)
			{
				cout << "Coroutine mode." << endl;
				fd_set rfds;
				timeval tv {};

				while (!term_received)
				{
					mc->session_ptr()->get_connection()->reader_execute();
					char ch(0);
					FD_ZERO(&rfds);
					FD_SET(0, &rfds);
#ifdef _MSC_VER
					if (kbhit())
					{
						ch = getch();
#else
					if (select(1, &rfds, 0, 0, &tv) > 0)
					{
						if (read (0, &ch, 1) < 0)
							break;
#endif
						if (ch == 'a')
						{
							cout << "Sending messages..." << endl;
							coroutine coro;
							while(mymenu.send_all_preloaded(coro, mc->session_ptr()))
								mc->session_ptr()->get_connection()->reader_execute();
						}
						else if (ch == 0x3 || !mymenu.process(ch))
							break;
					}
				}
			}
			else
			{
				cout << (pm == FIX8::pm_pipeline ? "Pipelined" : "Threaded") << " mode." << endl;
				while(!mymenu.get_istr().get(ch).bad() && !term_received && ch != 0x3 && mymenu.process(ch))
					;
			}
			cout << endl;
#if defined FIX8_CODECTIMING
			FIX8::Message::report_codec_timings("client");
#endif
			if (!mc->session_ptr()->is_shutdown())
				mc->session_ptr()->stop();

			mymenu.get_tty().unset_raw_mode();
		}
	}
	catch (FIX8::f8Exception& e)
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
bool MyMenu::batch_preload_new_order_single()
{
	unsigned num(preload_count);
	if (!num)
	{
		cout << "Enter number of NewOrderSingle msgs to batch preload:";
		cout.flush();
		_tty.unset_raw_mode();
		cin >> num;
		_tty.set_raw_mode();
	}
	while (num > 0)
	{
		unsigned cnt(0);
		for (; cnt < num && cnt < batch_size; ++cnt)
		{
			static unsigned oid(10000);
			ostringstream oistr;
			oistr << "ord" << ++oid << '-' << num;

			FIX8::TEX::NewOrderSingle *ptr(new FIX8::TEX::NewOrderSingle);
			*ptr << new FIX8::TEX::Symbol("BHP")
				  << new FIX8::TEX::HandlInst(FIX8::TEX::HandlInst_AUTOMATED_EXECUTION_ORDER_PRIVATE_NO_BROKER_INTERVENTION)
				  << new FIX8::TEX::OrdType(FIX8::TEX::OrdType_LIMIT)
				  << new FIX8::TEX::Side(FIX8::TEX::Side_BUY)
				  << new FIX8::TEX::TimeInForce(FIX8::TEX::TimeInForce_FILL_OR_KILL)
				  << new FIX8::TEX::TransactTime
				  << new FIX8::TEX::ClOrdID(oistr.str())
				  << new FIX8::TEX::Price(1. + RandDev::getrandom(500.), 3)
				  << new FIX8::TEX::OrderQty(1 + RandDev::getrandom(10000));

#if defined FIX8_PREENCODE_MSG_SUPPORT
			ptr->preencode();
#endif
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
	cout << endl << num << " NewOrderSingle msgs sent" << endl;

	return true;
}

//-----------------------------------------------------------------------------------------
bool MyMenu::new_order_single()
{
	static unsigned oid(0);
	ostringstream oistr;
	oistr << "ord" << ++oid;

	FIX8::TEX::NewOrderSingle *nos(new FIX8::TEX::NewOrderSingle);
	*nos  << new FIX8::TEX::TransactTime
			<< new FIX8::TEX::OrderQty(1 + RandDev::getrandom(10000))
			<< new FIX8::TEX::Price(1. + RandDev::getrandom(500.))
			<< new FIX8::TEX::ClOrdID(oistr.str())
			<< new FIX8::TEX::Symbol("BHP")
			<< new FIX8::TEX::HandlInst(FIX8::TEX::HandlInst_AUTOMATED_EXECUTION_ORDER_PRIVATE_NO_BROKER_INTERVENTION)
			<< new FIX8::TEX::OrdType(FIX8::TEX::OrdType_LIMIT)
			<< new FIX8::TEX::Side(FIX8::TEX::Side_BUY)
			<< new FIX8::TEX::TimeInForce(FIX8::TEX::TimeInForce_FILL_OR_KILL);

	_session.send(nos);

	return true;
}

//-----------------------------------------------------------------------------------------
bool MyMenu::send_all_preloaded()
{
	const unsigned tosend(_session.size());
	cout << "Sending " << tosend << " NewOrderSingle msgs ..." << flush;
	unsigned snt(0);
	while (_session.cached())
	{
		FIX8::TEX::NewOrderSingle *ptr(_session.pop());
		if (!ptr)
			break;
		_session.send(ptr);
		if (++snt % update_count == 0)
		{
			cout << '\r' << snt << " NewOrderSingle msgs sent       ";
			cout.flush();
		}
	}
	cout << endl << snt << " NewOrderSingle msgs sent." << endl;
	return true;
}

//-----------------------------------------------------------------------------------------
bool MyMenu::send_all_preloaded(coroutine& coro, FIX8::Session *ses)
{
	unsigned snt(0);
	FIX8::TEX::NewOrderSingle *ptr;

	reenter(coro)
	{
		ses->get_connection()->set_tcp_cork_flag(true);
		while (_session.cached())
		{
			if (ses->get_connection()->writer_poll())
			{
				if (!(ptr = _session.pop()))
					break;
				_session.send(ptr);
				if (++snt % batch_size)
					continue;
			}
			ses->get_connection()->set_tcp_cork_flag(false);
			coro_yield;
		}
	}
	ses->get_connection()->set_tcp_cork_flag(false);
	return _session.cached();
}

//-----------------------------------------------------------------------------------------
bool MyMenu::preload_new_order_single()
{
	cout << endl;
	if (_session.size())
		cout << _session.size() << " NewOrderSingle msgs currently preloaded." << endl;
	unsigned num(preload_count);
	if (!num)
	{
		cout << "Enter number of NewOrderSingle msgs to preload:";
		cout.flush();
		_tty.unset_raw_mode();
		cin >> num;
		_tty.set_raw_mode();
	}
	else
		cout << "loading..." << endl;
	for (unsigned ii(0); ii < num; ++ii)
	{
		static unsigned oid(10000);
		ostringstream oistr;
		oistr << "ord" << ++oid << '-' << num;

		FIX8::TEX::NewOrderSingle *ptr(new FIX8::TEX::NewOrderSingle);

		*ptr  << new FIX8::TEX::Symbol("BHP")
				<< new FIX8::TEX::HandlInst(FIX8::TEX::HandlInst_AUTOMATED_EXECUTION_ORDER_PRIVATE_NO_BROKER_INTERVENTION)
				<< new FIX8::TEX::OrdType(FIX8::TEX::OrdType_LIMIT)
				<< new FIX8::TEX::Side(FIX8::TEX::Side_BUY)
				<< new FIX8::TEX::TimeInForce(FIX8::TEX::TimeInForce_FILL_OR_KILL)
				<< new FIX8::TEX::TransactTime
				<< new FIX8::TEX::Price(1. + RandDev::getrandom(500.), 3) // precision=3
				<< new FIX8::TEX::ClOrdID(oistr.str())
				<< new FIX8::TEX::OrderQty(1 + RandDev::getrandom(10000));
#if defined FIX8_PREENCODE_MSG_SUPPORT
		ptr->preencode(); // pre-encode message payload (not header or trailer)
#endif
		_session.push(ptr);
	}

	cout << _session.size() << " NewOrderSingle msgs preloaded." << endl;

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
		_session.send(new FIX8::TEX::Logout);
	FIX8::hypersleep<FIX8::h_seconds>(2);
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
	um.add('o', "once", "for server, allow one client session then exit");
	um.add('q', "quiet", "do not print fix output (default yes)");
	um.add('b', "batch", "if using batch send, number of messages in each batch (default 1000)");
	um.add('p', "preload", "if batching or preloading, default number of messages to create");
	um.add('R', "receive", "set next expected receive sequence number");
	um.add('S', "send", "set next send sequence number");
	um.add('r', "reliable", "start in reliable mode");
	um.add('u', "update", "message count update frequency (default 5000)");
	um.print(cerr);
}

//-----------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------
bool hf_session_client::handle_application(const unsigned seqnum, const FIX8::Message *&msg)
{
	return enforce(seqnum, msg) || msg->process(_router);
}

//-----------------------------------------------------------------------------------------
bool tex_router_server::operator() (const FIX8::TEX::NewOrderSingle *msg)
{
	static unsigned oid(0), eoid(0);
	FIX8::TEX::OrderQty qty;
	FIX8::TEX::Price price;
	msg->get(qty);
	msg->get(price);

	FIX8::TEX::ExecutionReport *er(new FIX8::TEX::ExecutionReport);
	msg->copy_legal(er);

	ostringstream oistr;
	oistr << "ord" << ++oid;
	*er << new FIX8::TEX::OrderID(oistr.str())
		 << new FIX8::TEX::ExecType(FIX8::TEX::ExecType_NEW);
	unsigned ordResult(RandDev::getrandom(3));
	switch (ordResult)
	{
	default:
	case 0:
		*er << new FIX8::TEX::OrdStatus(FIX8::TEX::OrdStatus_NEW);
		break;
	case 1:
		*er << new FIX8::TEX::OrdStatus(FIX8::TEX::OrdStatus_CANCELED);
		break;
	case 2:
		*er << new FIX8::TEX::OrdStatus(FIX8::TEX::OrdStatus_REJECTED);
		break;
	}

	*er   << new FIX8::TEX::LeavesQty(qty())
			<< new FIX8::TEX::CumQty(0.)
			<< new FIX8::TEX::AvgPx(0.)
			<< new FIX8::TEX::LastCapacity('5')
			<< new FIX8::TEX::ReportToExch('Y')
			<< new FIX8::TEX::ExecTransType(FIX8::TEX::ExecTransType_NEW)
			<< new FIX8::TEX::ExecID(oistr.str());

	_session.send(er);

	if (ordResult == 0)
	{
		unsigned remaining_qty(qty()), cum_qty(0);
		while (remaining_qty > 0)
		{
			unsigned trdqty(1 + RandDev::getrandom(remaining_qty));
			remaining_qty -= trdqty;
			cum_qty += trdqty;
			FIX8::TEX::ExecutionReport *ner(new FIX8::TEX::ExecutionReport);
			msg->copy_legal(ner);
			ostringstream eistr;
			eistr << "exec" << ++eoid;

			*ner  << new FIX8::TEX::ExecID(eistr.str())
					<< new FIX8::TEX::OrderID(oistr.str())
					<< new FIX8::TEX::ExecType(FIX8::TEX::ExecType_NEW)
					<< new FIX8::TEX::OrdStatus(remaining_qty == trdqty ? FIX8::TEX::OrdStatus_FILLED : FIX8::TEX::OrdStatus_PARTIALLY_FILLED)
					<< new FIX8::TEX::LeavesQty(remaining_qty)
					<< new FIX8::TEX::CumQty(cum_qty)
					<< new FIX8::TEX::ExecTransType(FIX8::TEX::ExecTransType_NEW)
					<< new FIX8::TEX::AvgPx(price());

			_session.send(ner);
		}
	}

	return true;
}

//-----------------------------------------------------------------------------------------
bool tex_router_client::operator() (const FIX8::TEX::ExecutionReport *msg)
{
	static int exrecv(0);
	if (++exrecv % update_count == 0)
	{
		cout << '\r' << exrecv << " ExecutionReport msgs received   ";
		cout.flush();
	}
	return true;
}

//-----------------------------------------------------------------------------------------
bool hf_session_server::handle_application(const unsigned seqnum, const FIX8::Message *&msg)
{
	return enforce(seqnum, msg) || msg->process(_router);
}

