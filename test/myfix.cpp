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

-------------------------------------------------------------------------------------------
$Date$
$URL$

#endif

//-----------------------------------------------------------------------------------------
/** \file myfix.cpp

  This is a complete working example of a FIX client/server using FIX8.\n
\n
<tt>
	Usage: f8test [-hlqsv]\n
		-h,--help               help, this screen\n
		-l,--log                global log filename\n
		-q,--quiet              do not print fix output\n
		-s,--server             run in server mode (default client mode)\n
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

#include <Poco/Net/ServerSocket.h>
#include "myfix.hpp"

//-----------------------------------------------------------------------------------------
using namespace std;
using namespace FIX8;

//-----------------------------------------------------------------------------------------
static const std::string rcsid("$Id$");

//-----------------------------------------------------------------------------------------
void print_usage();
const string GETARGLIST("hl:svq");
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

namespace FIX8 {
namespace TEX {

typedef Field<char, 6666> Orderbook;
typedef Field<Boolean, 6951> BrokerInitiated;
typedef Field<int, 7009> ExecOption;

namespace {

const char Orderbook_realm[] =	// the realms must be std::less sorted
   { 'A', 'C', 'I', 'P', 'X' };
const char *Orderbook_descriptions[] = // descriptions must be in the same order for the relevant realm
   { "AXCLOB", "AXCP", "CHIX", "AXPM", "CROSS" };
const char BrokerInitiated_realm[] =
   { 'N', 'Y' };
const char *BrokerInitiated_descriptions[] =
   { "NO", "YES" };
const int ExecOption_realm[] =
   { 0, 1, 2, 3, 4 };
const char *ExecOption_descriptions[] =
   { "NORMAL", "EXPEDITE", "URGENT", "REPLACEALL", "STOPALL" };

const RealmBase extra_realmbases[] =
{
   { reinterpret_cast<const void *>(Orderbook_realm), static_cast<RealmBase::RealmType>(1),
      static_cast<FieldTrait::FieldType>(7), sizeof(Orderbook_realm)/sizeof(char), Orderbook_descriptions },
   { reinterpret_cast<const void *>(BrokerInitiated_realm), static_cast<RealmBase::RealmType>(1),
      static_cast<FieldTrait::FieldType>(8), sizeof(BrokerInitiated_realm)/sizeof(char), BrokerInitiated_descriptions },
   { reinterpret_cast<const void *>(ExecOption_realm), static_cast<RealmBase::RealmType>(1),
      static_cast<FieldTrait::FieldType>(1), sizeof(ExecOption_realm)/sizeof(int), ExecOption_descriptions },
};

BaseField *Create_Orderbook(const f8String& from, const RealmBase *db) { return new Orderbook(from, db); }
BaseField *Create_BrokerInitiated(const f8String& from, const RealmBase *db) { return new BrokerInitiated(from, db); }
BaseField *Create_ExecOption(const f8String& from, const RealmBase *db) { return new ExecOption(from, db); }

} // namespace
} // namespace TEX

bool common_post_msg_ctor(Message *msg)
{
	if (msg->get_msgtype() == TEX::ExecutionReport::get_msgtype()())
	{
		const FieldTrait::TraitBase trt[] =
		{
			{ 6666, 7, 1000, 0x004 }, { 6951,  8, 5, 0x004 }, { 7009,  1, 6, 0x004 },
		};
		msg->add_trait(trt, trt + sizeof(trt)/sizeof(FieldTrait::TraitBase));
	}

	return true;
}

} // namespace FIX8
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
	bool server(false);

#ifdef HAVE_GETOPT_LONG
	option long_options[] =
	{
		{ "help",		0,	0,	'h' },
		{ "version",	0,	0,	'v' },
		{ "log",			0,	0,	'l' },
		{ "server",		0,	0,	's' },
		{ "quiet",		0,	0,	'q' },
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
		case 'l': GlobalLogger::set_global_filename(optarg); break;
		case 's': server = true; break;
		case 'q': quiet = true; break;
		default: break;
		}
	}

	RandDev::init();

	signal(SIGTERM, sig_handler);
	signal(SIGINT, sig_handler);
	signal(SIGQUIT, sig_handler);

	Logger::LogFlags logflags, plogflags;
	logflags << Logger::timestamp << Logger::sequence << Logger::thread;
	plogflags << Logger::append;
	string logname, prot_logname;
	Poco::Net::SocketAddress addr;

#if defined PERMIT_CUSTOM_FIELDS
	CustomFields custfields(true);	// will cleanup
	custfields.add(6666, BaseEntry_ctor(new BaseEntry, &TEX::Create_Orderbook,
		&TEX::extra_realmbases[0], "Orderbook", "Select downstream execution venue"));
	custfields.add(6951, BaseEntry_ctor(new BaseEntry, &TEX::Create_BrokerInitiated,
		&TEX::extra_realmbases[1], "BrokerInitiated", "Indicate if order was broker initiated"));
	custfields.add(7009, BaseEntry_ctor(new BaseEntry, &TEX::Create_ExecOption,
		&TEX::extra_realmbases[2], "ExecOption", "Broker specific option"));
	TEX::ctx.set_ube(&custfields);
#endif

	try
	{
		if (server)
		{
			const string server_conf_file("myfix_server.xml");
			Configuration conf(server_conf_file, true);
			const XmlEntity *ses(conf.get_session(0));
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
			if (!quiet)
				ms.control() |= Session::print;
			ms.start(&sc, false);
			while (!term_received)
				sleep(1);
			ms.stop();
		}
		else
		{
			const string client_conf_file("myfix_client.xml");
			Configuration conf(client_conf_file, true);
			const XmlEntity *ses(conf.get_session(0));
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
			GlobalLogger::instance()->send("established connection with server...");
			ClientConnection cc(&sock, addr, ms);
			if (!quiet)
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
#if defined PERMIT_CUSTOM_FIELDS
bool myfix_session_client::post_msg_ctor(Message *msg)
{
	return common_post_msg_ctor(msg);
}
#endif

//-----------------------------------------------------------------------------------------
bool myfix_session_server::handle_application(const unsigned seqnum, const Message *msg)
{
	return enforce(seqnum, msg) || msg->process(_router);
}

//-----------------------------------------------------------------------------------------
#if defined PERMIT_CUSTOM_FIELDS
bool myfix_session_server::post_msg_ctor(Message *msg)
{
	return common_post_msg_ctor(msg);
}
#endif

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
	UsageMan um("f8test", GETARGLIST, "");
	um.setdesc("f8test -- f8 test client/server");
	um.add('s', "server", "run in server mode (default client mode)");
	um.add('h', "help", "help, this screen");
	um.add('l', "log", "global log filename");
	um.add('q', "quiet", "do not print fix output");
	um.print(cerr);
}

//-----------------------------------------------------------------------------------------
bool tex_router_server::operator() (const TEX::NewOrderSingle *msg) const
{
	static unsigned oid(0), eoid(0);
	TEX::OrderQty qty;
	msg->get(qty);
	if (!quiet)
		cout << "Order qty:" << qty() << endl;
	TEX::Price price;
	msg->get(price);
	if (!quiet)
		cout << "price:" << price() << endl;

	//ostringstream gerr;
	//IntervalTimer itm;
#if defined MSGRECYCLING
	scoped_ptr<TEX::ExecutionReport> er(new TEX::ExecutionReport);
#if defined PERMIT_CUSTOM_FIELDS
	_session.post_msg_ctor(er.get());
#endif
	msg->copy_legal(er.get());
#else
	TEX::ExecutionReport *er(new TEX::ExecutionReport);
#if defined PERMIT_CUSTOM_FIELDS
	_session.post_msg_ctor(er);
#endif
	msg->copy_legal(er);
#endif
	if (!quiet)
		cout << endl;
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
	*er += new TEX::ReportToExch('Y');
#if defined PERMIT_CUSTOM_FIELDS
	*er += new TEX::Orderbook('X');
	*er += new TEX::BrokerInitiated(true);
	*er += new TEX::ExecOption(3);
#endif
#if defined MSGRECYCLING
	_session.send_wait(er.get());
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
#if defined PERMIT_CUSTOM_FIELDS
		_session.post_msg_ctor(er);
#endif
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

