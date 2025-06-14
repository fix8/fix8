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

/*! \namespace TEX
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
#if defined(FIX8_CODECTIMING)
#include <fix8/f8measure.hpp>
#endif
#include "perf_types.hpp"
#include "perf_router.hpp"
#include "perf_classes.hpp"

#include "hftest.hpp"
#if defined(__GNUC__) && !defined(__llvm__) && !defined(__INTEL_COMPILER)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wimplicit-fallthrough="
#endif

//-----------------------------------------------------------------------------------------
using namespace std;
using namespace FIX8;
using namespace PERF;

//-----------------------------------------------------------------------------------------
void print_usage();
const string GETARGLIST("hl:svqc:R:S:rb:p:u:o");
bool term_received(false);
unsigned batch_size(1000), preload_count(0), update_count(5000);

//-----------------------------------------------------------------------------------------
struct performance_metrics
{
	enum Enum
	{
		msg_create,
		msg_send,
		msg_destroy,
		//msg_ack_latency,
		//msg_fill_latency,
		_max
	};
};

#if defined(FIX8_CODECTIMING)
stop_watch perf_metric(performance_metrics::_max);
void perf_metric_start(performance_metrics::Enum what) { perf_metric.start(what); }
void perf_metric_stop(performance_metrics::Enum what) { perf_metric.stop(what); }
void perf_metric_start(performance_metrics::Enum what, stop_watch::ticks_t measure) { perf_metric.start(what, measure); }
void perf_metric_stop(performance_metrics::Enum what, stop_watch::ticks_t measure) { perf_metric.stop(what, measure); }
stop_watch::ticks_t perf_metric_measure() { return stop_watch::measure(); }
#else
void perf_metric_start(performance_metrics::Enum what) { }
void perf_metric_stop(performance_metrics::Enum what) { }
void perf_metric_start(performance_metrics::Enum what, std::int64_t measure) { }
void perf_metric_stop(performance_metrics::Enum what, std::int64_t measure) { }
std::int64_t perf_metric_measure() { return 0; }
#endif

//-----------------------------------------------------------------------------------------
void report_perf_metric(const std::string& prefix)
{
#if defined FIX8_CODECTIMING
	using namespace FIX8;

	ostringstream ostr;
	ostr.setf(std::ios::showpoint);
	ostr.setf(std::ios::fixed);

	if (perf_metric.val(performance_metrics::msg_create)[stop_watch::value::_count])
	{
		ostr << prefix << ' ';
		Message::format_codec_timings("Create", ostr, perf_metric.val(performance_metrics::msg_create));
		glout_info << ostr.str();
	}

	if (perf_metric.val(performance_metrics::msg_send)[stop_watch::value::_count])
	{
		ostr.str("");
		ostr << prefix << ' ';
		Message::format_codec_timings("Send", ostr, perf_metric.val(performance_metrics::msg_send));
		glout_info << ostr.str();
	}

	if (perf_metric.val(performance_metrics::msg_destroy)[stop_watch::value::_count])
	{
		ostr.str("");
		ostr << prefix << ' ';
		Message::format_codec_timings("Destroy", ostr, perf_metric.val(performance_metrics::msg_destroy));
		glout_info << ostr.str();
	}
#if 0
	if (perf_metric.val(performance_metrics::msg_ack_latency)[stop_watch::value::_count])
	{
		ostr.str("");
		ostr << prefix << ' ';
		Message::format_codec_timings("Ack_latency", ostr, perf_metric.val(performance_metrics::msg_ack_latency));
		glout_info << ostr.str();
	}

	if (perf_metric.val(performance_metrics::msg_fill_latency)[stop_watch::value::_count])
	{
		ostr.str("");
		ostr << prefix << ' ';
		Message::format_codec_timings("Fill_latency", ostr, perf_metric.val(performance_metrics::msg_fill_latency));
		glout_info << ostr.str();
	}
#endif
#endif
}

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
		{ "help",		0,	nullptr,	'h' },
		{ "version",	0,	nullptr,	'v' },
		{ "once",	   0,	nullptr,	'o' },
		{ "log",			1,	nullptr,	'l' },
		{ "config",		1,	nullptr,	'c' },
		{ "server",		0,	nullptr,	's' },
		{ "batch",		1,	nullptr,	'b' },
		{ "send",		1,	nullptr,	'S' },
		{ "receive",	1,	nullptr,	'R' },
		{ "quiet",		0,	nullptr,	'q' },
		{ "reliable",	0,	nullptr,	'r' },
		{ "preload",	1,	nullptr,	'p' },
		{ "update",		1,	nullptr,	'u' },
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
			cout << "hftest " << Session::copyright_string() << endl;
			cout << "Released under the GNU LESSER GENERAL PUBLIC LICENSE, Version 3. See <https://www.gnu.org/licenses/> for details." << endl;
			return 0;
		case ':': case '?': return 1;
		case 'h': print_usage(); return 0;
		case 'l': GlobalLogger::set_global_filename(optarg); break;
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
			unique_ptr<ServerSessionBase> ms(new ServerSession<hf_session_server>(ctx(), conf_file, "TEX1"));

			XmlElement::XmlSet eset;

			for (unsigned scnt(0); !term_received; )
			{
				if (!ms->poll())
					continue;
				unique_ptr<SessionInstanceBase> inst(ms->create_server_instance());
				if (!quiet)
					inst->session_ptr()->control() |= Session::print;
				ostringstream sostr;
				sostr << "client(" << ++scnt << ") connection established.";
				GlobalLogger::log(sostr.str());
				const ProcessModel pm(ms->get_process_model(ms->_ses));
				inst->start(pm == pm_pipeline, next_send, next_receive);
				cout << (pm == pm_pipeline ? "Pipelined" : "Threaded") << " mode." << endl;
				if (inst->session_ptr()->get_connection()->is_secure())
					cout << "Session is secure (SSL)" << endl;
				if (pm != pm_pipeline)
					while (!inst->session_ptr()->is_shutdown())
						hypersleep<h_milliseconds>(100);
				cout << "Session(" << scnt << ") finished." << endl;
#if defined FIX8_CODECTIMING
				Message::report_codec_timings("server");
				report_perf_metric("server");
#endif
				inst->stop();
				if (once)
					break;
			}
		}
		else
		{
			unique_ptr<ClientSessionBase>
				mc(reliable ? new ReliableClientSession<hf_session_client>(ctx(), conf_file, "DLD1")
							   : new ClientSession<hf_session_client>(ctx(), conf_file, "DLD1"));
			if (!quiet)
				mc->session_ptr()->control() |= Session::print;

			const ProcessModel pm(mc->get_process_model(mc->_ses));
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
			if (pm == pm_coro)
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
					if (select(1, &rfds, nullptr, nullptr, &tv) > 0)
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
				cout << (pm == pm_pipeline ? "Pipelined" : "Threaded") << " mode." << endl;
				while(!mymenu.get_istr().get(ch).bad() && !term_received && ch != 0x3 && mymenu.process(ch))
					;
			}
			cout << endl;
#if defined FIX8_CODECTIMING
			Message::report_codec_timings("client");
			report_perf_metric("client");
#endif
			if (!mc->session_ptr()->is_shutdown())
				mc->session_ptr()->stop();

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
void send_msg(Message* msg, Session& session)
{
	std::unique_ptr<Message> msg_ptr{ msg };
	perf_metric_start(performance_metrics::msg_send);
	session.send(msg, false);
	perf_metric_stop(performance_metrics::msg_send);

	perf_metric_start(performance_metrics::msg_destroy);
	msg_ptr.reset();
	perf_metric_stop(performance_metrics::msg_destroy);
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

			perf_metric_start(performance_metrics::msg_create);
			NewOrderSingle *ptr(new NewOrderSingle);
			*ptr << new Symbol("BHP")
				  << new HandlInst(HandlInst_AUTOMATED_EXECUTION_ORDER_PRIVATE_NO_BROKER_INTERVENTION)
				  << new OrdType(OrdType_LIMIT)
				  << new Side(Side_BUY)
				  << new TimeInForce(TimeInForce_FILL_OR_KILL)
				  << new TransactTime
				  << new ClOrdID(oistr.str())
				  << new Price(1. + RandDev::getrandom(500.), 3)
				  << new OrderQty(1 + RandDev::getrandom(10000));
			perf_metric_stop(performance_metrics::msg_create);

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

	perf_metric_start(performance_metrics::msg_create);
	NewOrderSingle *nos(new NewOrderSingle);
	*nos  << new TransactTime
			<< new OrderQty(1 + RandDev::getrandom(10000))
			<< new Price(1. + RandDev::getrandom(500.))
			<< new ClOrdID(oistr.str())
			<< new Symbol("BHP")
			<< new HandlInst(HandlInst_AUTOMATED_EXECUTION_ORDER_PRIVATE_NO_BROKER_INTERVENTION)
			<< new OrdType(OrdType_LIMIT)
			<< new Side(Side_BUY)
			<< new TimeInForce(TimeInForce_FILL_OR_KILL);
	perf_metric_stop(performance_metrics::msg_create);

	send_msg(nos, _session);
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
		NewOrderSingle *ptr(_session.pop());
		if (!ptr)
			break;
		send_msg(ptr, _session);
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
bool MyMenu::send_all_preloaded(coroutine& coro, Session *ses)
{
	unsigned snt(0);
	NewOrderSingle *ptr;

	reenter(coro)
	{
		ses->get_connection()->set_tcp_cork_flag(true);
		while (_session.cached())
		{
			if (ses->get_connection()->writer_poll())
			{
				if (!(ptr = _session.pop()))
					break;
				send_msg(ptr, _session);
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

		perf_metric_start(performance_metrics::msg_create);
		NewOrderSingle *ptr(new NewOrderSingle);

		*ptr  << new Symbol("BHP")
				<< new HandlInst(HandlInst_AUTOMATED_EXECUTION_ORDER_PRIVATE_NO_BROKER_INTERVENTION)
				<< new OrdType(OrdType_LIMIT)
				<< new Side(Side_BUY)
				<< new TimeInForce(TimeInForce_FILL_OR_KILL)
				<< new TransactTime
				<< new Price(1. + RandDev::getrandom(500.), 3) // precision=3
				<< new ClOrdID(oistr.str())
				<< new OrderQty(1 + RandDev::getrandom(10000));
		perf_metric_stop(performance_metrics::msg_create);
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
		_session.send(new Logout);
	hypersleep<h_seconds>(2);
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
bool hf_session_client::handle_application(const unsigned seqnum, const Message *&msg)
{
	return enforce(seqnum, msg) || msg->process(_router);
}

//-----------------------------------------------------------------------------------------
bool perf_router_server::operator() (const NewOrderSingle *msg)
{
	static unsigned oid(0), eoid(0);
	OrderQty qty;
	Price price;
	msg->get(qty);
	msg->get(price);

	perf_metric_start(performance_metrics::msg_create);
	ExecutionReport *er(new ExecutionReport);
	msg->copy_legal(er);

	ostringstream oistr;
	oistr << "ord" << ++oid;
	*er << new OrderID(oistr.str())
		 << new ExecType(ExecType_NEW);
	unsigned ordResult(RandDev::getrandom(3));
	switch (ordResult)
	{
	default:
	case 0:
		*er << new OrdStatus(OrdStatus_NEW);
		break;
	case 1:
		*er << new OrdStatus(OrdStatus_CANCELED);
		break;
	case 2:
		*er << new OrdStatus(OrdStatus_REJECTED);
		break;
	}

	*er   << new LeavesQty(qty())
			<< new CumQty(0.)
			<< new AvgPx(0.)
			<< new LastCapacity('5')
			<< new ReportToExch('Y')
			<< new ExecTransType(ExecTransType_NEW)
			<< new ExecID(oistr.str());
	perf_metric_stop(performance_metrics::msg_create);
	send_msg(er, _session);

	if (ordResult == 0)
	{
		unsigned remaining_qty(static_cast<unsigned>(qty())), cum_qty(0);
		while (remaining_qty > 0)
		{
			unsigned trdqty(1 + RandDev::getrandom(remaining_qty));
			remaining_qty -= trdqty;
			cum_qty += trdqty;
			perf_metric_start(performance_metrics::msg_create);
			ExecutionReport *ner(new ExecutionReport);
			msg->copy_legal(ner);
			ostringstream eistr;
			eistr << "exec" << ++eoid;

			*ner  << new ExecID(eistr.str())
					<< new OrderID(oistr.str())
					<< new ExecType(ExecType_NEW)
					<< new OrdStatus(remaining_qty == trdqty ? OrdStatus_FILLED : OrdStatus_PARTIALLY_FILLED)
					<< new LeavesQty(remaining_qty)
					<< new CumQty(cum_qty)
					<< new ExecTransType(ExecTransType_NEW)
					<< new AvgPx(price());
			perf_metric_stop(performance_metrics::msg_create);
			send_msg(ner, _session);
		}
	}

	return true;
}

//-----------------------------------------------------------------------------------------
bool perf_router_client::operator() (const ExecutionReport *msg)
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
bool hf_session_server::handle_application(const unsigned seqnum, const Message *&msg)
{
	return enforce(seqnum, msg) || msg->process(_router);
}
#if defined(__GNUC__) && !defined(__llvm__) && !defined(__INTEL_COMPILER)
#pragma GCC diagnostic pop
#endif
