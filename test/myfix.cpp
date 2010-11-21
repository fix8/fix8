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
const string GETARGLIST("hl:s");
bool term_received(false);

//-----------------------------------------------------------------------------------------
template<>
const MyMenu::Handlers::TypePair MyMenu::Handlers::_valueTable[] =
{
	MyMenu::Handlers::TypePair(MyMenu::MenuItem('n', "New Order Single"), &MyMenu::new_order_single),
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
		{ "help",			0,	0,	'h' },
		{ "log",				0,	0,	'l' },
		{ "server",			0,	0,	's' },
		{ 0 },
	};

	while ((val = getopt_long (argc, argv, GETARGLIST.c_str(), long_options, 0)) != -1)
#else
	while ((val = getopt (argc, argv, GETARGLIST.c_str())) != -1)
#endif
	{
      switch (val)
		{
		case ':': case '?': return 1;
		case 'h': print_usage(); return 0;
		case 'l': strcpy(glob_log0, optarg); break;
		case 's': server = true; break;
		default: break;
		}
	}

	signal(SIGTERM, sig_handler);
	signal(SIGINT, sig_handler);

	Logger::LogFlags logflags, noflags;
	logflags << Logger::timestamp << Logger::sequence << Logger::append << Logger::thread;
	BDBPersister bdp;
	Poco::Net::SocketAddress addr("127.0.0.1:11001");

	try
	{
		if (server)
		{
			GlobalLogger::instance()->send("test fix server starting up...");

			bdp.initialise("./run", "myfix_server.db");
			FileLogger log("./run/myfix_server.log", logflags, 2);
			FileLogger plog("./run/myfix_server_protocol.log", noflags, 2);
			myfix_session_server ms(TEX::ctx, &bdp, &log, &plog);
			Poco::Net::ServerSocket ss(addr);
			Poco::Net::SocketAddress claddr;
			Poco::Net::StreamSocket sock(ss.acceptConnection(claddr));
			GlobalLogger::instance()->send("client connection established...");
			ServerConnection sc(&sock, ms, 5);
			ms.control() |= Session::print;
			ms.start(&sc);
		}
		else
		{
			GlobalLogger::instance()->send("test fix client starting up...");

			const SessionID id(TEX::ctx._beginStr, "DLD_TEX", "TEX_DLD");
			bdp.initialise("./run", "myfix_client.db");
			FileLogger log("./run/myfix_client.log", logflags, 2);
			FileLogger plog("./run/myfix_client_protocol.log", noflags, 2);
			myfix_session_client ms(TEX::ctx, id, &bdp, &log, &plog);
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
bool myfix_session_client::handle_application(const Message *msg)
{
	msg->process(_router);
	return true;
}

//-----------------------------------------------------------------------------------------
bool myfix_session_server::handle_application(const Message *msg)
{
	msg->process(_router);
	return true;
}

//-----------------------------------------------------------------------------------------
bool MyMenu::new_order_single()
{
	TEX::NewOrderSingle *nos(new TEX::NewOrderSingle);
	*nos += new TEX::TransactTime("20110904-10:15:14");
	*nos += new TEX::OrderQty(100);
	*nos += new TEX::Price(47.78);
	//*nos += new TEX::ClOrdID("ord01");
	*nos += new TEX::Symbol("BHP");
	*nos += new TEX::OrdType(TEX::OrdType_LIMIT);
	*nos += new TEX::Side(TEX::Side_BUY);
	*nos += new TEX::TimeInForce(TEX::TimeInForce_FILL_OR_KILL);

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
	TEX::Logout *lo(new TEX::Logout);
	_session.send(lo);
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
	TEX::OrderQty qty;
	msg->get(qty);
	cout << "Order qty:" << qty.get() << endl;
	TEX::Price price;
	msg->get(price);
	cout << "price:" << price.get() << endl;
	msg->print(cout);
	return true;
}

