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

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS
OR  IMPLIED  WARRANTIES,  INCLUDING,  BUT  NOT  LIMITED  TO ,  THE  IMPLIED  WARRANTIES  OF
MERCHANTABILITY AND  FITNESS FOR A PARTICULAR  PURPOSE ARE  DISCLAIMED. IN  NO EVENT  SHALL
THE  COPYRIGHT  OWNER OR  CONTRIBUTORS BE  LIABLE  FOR  ANY DIRECT,  INDIRECT,  INCIDENTAL,
SPECIAL,  EXEMPLARY, OR CONSEQUENTIAL  DAMAGES (INCLUDING,  BUT NOT LIMITED TO, PROCUREMENT
OF SUBSTITUTE  GOODS OR SERVICES; LOSS OF USE, DATA,  OR PROFITS; OR BUSINESS INTERRUPTION)
HOWEVER CAUSED  AND ON ANY THEORY OF LIABILITY, WHETHER  IN CONTRACT, STRICT  LIABILITY, OR
TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE
EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

-------------------------------------------------------------------------------------------
$Date$
$URL$

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

#include <Poco/Net/ServerSocket.h>
#include "myfix.hpp"

//-----------------------------------------------------------------------------------------
using namespace std;
using namespace FIX8;

//-----------------------------------------------------------------------------------------
static const std::string rcsid("$Id$");

//-----------------------------------------------------------------------------------------
void print_usage();
const string GETARGLIST("hl:sv");
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
	int val;
	bool server(false);

#ifdef HAVE_GETOPT_LONG
	option long_options[] =
	{
		{ "help",		0,	0,	'h' },
		{ "version",	0,	0,	'v' },
		{ "log",			0,	0,	'l' },
		{ "server",		0,	0,	's' },
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
			cout << rcsid << endl;
			return 0;
		case ':': case '?': return 1;
		case 'h': print_usage(); return 0;
		case 'l': strcpy(glob_log0, optarg); break;
		case 's': server = true; break;
		default: break;
		}
	}

	RandDev::init();

	signal(SIGTERM, sig_handler);
	signal(SIGINT, sig_handler);

	Logger::LogFlags logflags, plogflags;
	logflags << Logger::timestamp << Logger::sequence << Logger::thread;
	plogflags << Logger::append;
	string logname, prot_logname;
	Poco::Net::SocketAddress addr;

	try
	{
		if (server)
		{
			const string server_conf_file("myfix_server.xml");
			if (!exist(server_conf_file))
				throw f8Exception("server config file not found", server_conf_file);
			Configuration conf(server_conf_file, true);
			const XmlEntity *ses(conf.get_session(0));
			if (!ses)
				throw f8Exception("could not locate server session in config file", server_conf_file);
			if (conf.get_role(ses) != Connection::cn_acceptor)
				throw f8Exception("Invalid role");
			GlobalLogger::instance()->send("test fix server starting up...");

			FileLogger log(conf.get_logname(ses, logname), logflags, 2);
			FileLogger plog(conf.get_protocol_logname(ses, prot_logname), plogflags, 2);
			scoped_ptr<Persister> bdp(conf.create_persister(ses));
			myfix_session_server ms(TEX::ctx, bdp.get(), &log, &plog);
			conf.get_address(ses, addr);
			Poco::Net::ServerSocket ss(addr);
			Poco::Net::SocketAddress claddr;
			Poco::Net::StreamSocket sock(ss.acceptConnection(claddr));
			GlobalLogger::instance()->send("client connection established...");
			ServerConnection sc(&sock, ms, conf.get_heartbeat_interval(ses));
			ms.control() |= Session::print;
			ms.start(&sc);	// will wait
		}
		else
		{
			const string client_conf_file("myfix_client.xml");
			if (!exist(client_conf_file))
				throw f8Exception("client config file not found", client_conf_file);
			Configuration conf(client_conf_file, true);
			const XmlEntity *ses(conf.get_session(0));
			if (!ses)
				throw f8Exception("could not locate client session in config file", client_conf_file);
			if (conf.get_role(ses) != Connection::cn_initiator)
				throw f8Exception("Invalid role");
			GlobalLogger::instance()->send("test fix client starting up...");

			sender_comp_id sci;
			target_comp_id tci;
			const SessionID id(TEX::ctx._beginStr, conf.get_sender_comp_id(ses, sci), conf.get_target_comp_id(ses, tci));
			FileLogger log(conf.get_logname(ses, logname), logflags, 2);
			FileLogger plog(conf.get_protocol_logname(ses, prot_logname), plogflags, 2);
			scoped_ptr<Persister> bdp(conf.create_persister(ses));
			myfix_session_client ms(TEX::ctx, id, bdp.get(), &log, &plog);
			conf.get_address(ses, addr);
			Poco::Net::StreamSocket sock;
			ClientConnection cc(&sock, addr, ms);
			ms.control() |= Session::print;
			ms.start(&cc, false);

			MyMenu mymenu(ms, 0, cout);
			char ch;
			mymenu.set_raw_mode();
			while(!mymenu.get_istr().get(ch).bad() && !term_received && ch != 0x3)
			{
				if (!mymenu.process(ch))
					break;
			}
			mymenu.unset_raw_mode();

			ms.stop();
		}
	}
	catch (f8Exception& e)
	{
		cerr << "exception: " << e.what() << endl;
	}

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
	return true;
}

//-----------------------------------------------------------------------------------------
void print_usage()
{
	UsageMan um("f8test", GETARGLIST, "<input xml schema>");
	um.setdesc("f8test -- f8 test client/server");
	um.add('s', "server", "run in server mode (default client mode)");
	um.add('h', "help", "help, this screen");
	um.add('l', "log", "global log filename");
	um.print(cerr);
}

//-----------------------------------------------------------------------------------------
bool tex_router_server::operator() (const TEX::NewOrderSingle *msg) const
{
	static unsigned oid(0), eoid(0);
	TEX::OrderQty qty;
	msg->get(qty);
	cout << "Order qty:" << qty() << endl;
	TEX::Price price;
	msg->get(price);
	cout << "price:" << price() << endl;

	//ostringstream gerr;
	//IntervalTimer itm;
#if defined MSGRECYCLING
	scoped_ptr<TEX::ExecutionReport> er(new TEX::ExecutionReport);
	msg->copy_legal(er.get());
#else
	TEX::ExecutionReport *er(new TEX::ExecutionReport);
	msg->copy_legal(er);
#endif
	//gerr << "encode:" << itm.Calculate();
	//GlobalLogger::instance()->send(gerr.str());

	ostringstream oistr;
	oistr << "ord" << ++oid;
	*er += new TEX::OrderID(oistr.str());
	*er += new TEX::ExecType(TEX::ExecType_NEW);
	*er += new TEX::OrdStatus(TEX::OrdStatus_NEW);
	*er += new TEX::LeavesQty(qty());
	*er += new TEX::CumQty(0);
	*er += new TEX::AvgPx(0);
	*er += new TEX::LastCapacity('5');
#if defined MSGRECYCLING
	_session.send(er.get());
	while(er->get_in_use())
		microsleep(10);
	er->set_in_use(true);
	delete er->Header()->remove(Common_MsgSeqNum); // we want to reuse, not resend
	*er += new TEX::AvgPx(9999);
	_session.send(er.get());
	while(er->get_in_use())
		microsleep(10);
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
		er.reset(new TEX::ExecutionReport);
		msg->copy_legal(er.get());
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
		_session.send(er.get());
		while(er->get_in_use())
			microsleep(10);
#else
		_session.send(er);
#endif
	}

	return true;
}

//-----------------------------------------------------------------------------------------
bool tex_router_client::operator() (const TEX::ExecutionReport *msg) const
{
	TEX::LastCapacity lastCap;
	if (msg->get(lastCap))
	{
		if (!lastCap.is_valid())
			cout << "TEX::LastCapacity(" << lastCap << ") is not a valid value" << endl;
	}
	return true;
}

