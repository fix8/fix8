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

//-----------------------------------------------------------------------------------------
using namespace std;
using namespace FIX8;

//-----------------------------------------------------------------------------------------
static const std::string rcsid("$Id$");

//-----------------------------------------------------------------------------------------
void print_usage();
const string GETARGLIST("hl:s");

//-----------------------------------------------------------------------------------------
extern char glob_log0[];
typedef SingleLogger<glob_log0> GlobalLogger;

#if 0
extern const char glob_log1[] = { "f8global1.log" };

template<>
tbb::atomic<SingleLogger<glob_log1> *> Singleton<SingleLogger<glob_log1> >::_instance
	= tbb::atomic<SingleLogger<glob_log1> *>();
#endif

//-----------------------------------------------------------------------------------------
class tex_router : public TEX::Myfix_Router
{
public:
	virtual bool operator() (const TEX::NewOrderSingle *msg) const
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
};

//-----------------------------------------------------------------------------------------
class myfix_session : public Session
{
public:
	myfix_session(const F8MetaCntx& ctx, const SessionID& sid, Persister *persist, Logger *logger, Logger *plogger)
		: Session(ctx, sid, persist, logger, plogger) {}
	myfix_session(const F8MetaCntx& ctx, Persister *persist, Logger *logger, Logger *plogger)
		: Session(ctx, persist, logger, plogger) {}
	bool authenticate(SessionID& id) { return true; }
};

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

	Logger::LogFlags logflags, noflags;
	logflags << Logger::timestamp << Logger::sequence << Logger::append << Logger::thread;
	BDBPersister bdp;
	Poco::Net::SocketAddress addr("127.0.0.1:11001");

	try
	{
		if (!server)
		{
			GlobalLogger::instance()->send("test fix client starting up...");

			const SessionID id(TEX::ctx._beginStr, "DLD_TEX", "TEX_DLD");
			bdp.initialise("./run", "myfix_client.db");
			FileLogger log("./run/myfix_client.log", logflags);
			FileLogger plog("./run/myfix_client_protocol.log", noflags);
			myfix_session ms(TEX::ctx, id, &bdp, &log, &plog);
			Poco::Net::StreamSocket sock;
			ClientConnection cc(&sock, addr, ms);
			ms.start(&cc);
		}
		else
		{
			GlobalLogger::instance()->send("test fix server starting up...");

			bdp.initialise("./run", "myfix_server.db");
			FileLogger log("./run/myfix_server.log", logflags);
			FileLogger plog("./run/myfix_server_protocol.log", noflags);
			myfix_session ms(TEX::ctx, &bdp, &log, &plog);
			Poco::Net::ServerSocket ss(addr);
			Poco::Net::SocketAddress claddr;
			Poco::Net::StreamSocket sock(ss.acceptConnection(claddr));
			GlobalLogger::instance()->send("client connection established...");
			ServerConnection sc(&sock, ms, 5);
			ms.start(&sc);
		}
	}
	catch (f8Exception& e)
	{
		cerr << e.what() << endl;
		cerr << e.what() << endl;
	}

#if 0
		try
	{
		const BaseEntry& be(TEX::Myfix_BaseEntry::find_ref(35));
		//const BaseEntry& be(TEX::Myfix::find_ref(101));
		string logon("A");
		auto_ptr<BaseField> fld(be._create(logon, &be));
		if (be._comment)
			cout << be._comment << ": ";
		cout << *fld << endl;
		cout << "is valid: " << boolalpha << fld->is_valid() << endl << endl;
	}
	catch (InvalidMetadata& ex)
	{
		cerr << ex.what() << endl;
	}

	FieldTrait::TraitBase a = { 1, FieldTrait::ft_int, 0, true, false, false };

	const BaseEntry& be1(TEX::Myfix_BaseEntry::find_ref(98));
	string encryptmethod("101");
	auto_ptr<BaseField> fld1(be1._create(encryptmethod, &be1));
	if (be1._comment)
		cout << be1._comment << ": ";
	cout << *fld1 << endl;
	cout << "is valid: " << boolalpha << fld1->is_valid() << endl;
	//TEX::EncryptMethod& em(dynamic_cast<TEX::EncryptMethod&>(*fld1));
	//cout << em.get() << endl;
#endif

#if 0
	try
	{
		//FileLogger flog("myfix.log");
		Logger flog(ebitset<Logger::Flags>(0));

		TEX::NewOrderSingle *nos(new TEX::NewOrderSingle);
		*nos += new TEX::TransactTime("20110904-10:15:14");
		*nos += new TEX::OrderQty(100);
		*nos += new TEX::Price(47.78);
		*nos += new TEX::ClOrdID("ord01");
		*nos += new TEX::Symbol("BHP");
		*nos += new TEX::OrdType(TEX::OrdType_LIMIT);
		*nos += new TEX::Side(TEX::Side_BUY);
		*nos += new TEX::TimeInForce(TEX::TimeInForce_FILL_OR_KILL);
		//*nos += new TEX::Subject("blah");

		*nos += new TEX::NoUnderlyings(2);
		GroupBase *noul(nos->find_group<TEX::NewOrderSingle::NoUnderlyings>());
		MessageBase *gr1(noul->create_group());
		*gr1 += new TEX::UnderlyingSymbol("BLAH");
		*noul += gr1;
		MessageBase *gr2(noul->create_group());
		*gr2 += new TEX::UnderlyingSymbol("FOO");
		*noul += gr2;

		f8String omsg;
		nos->encode(omsg);
		cout << omsg << endl;
		nos->print(cout);
		flog.send(omsg);

		delete nos;

		cout << endl;
		//omsg[omsg.size() - 2] = '9';
		Message *mc(Message::factory(TEX::ctx, omsg));
		tex_router mr;
		mc->process(mr);
		delete mc;

		SessionID id(TEX::ctx._beginStr, "DLD_TEX", "TEX_DLD");
		cout << id << endl;
		SessionID nid(id.get_id());
		cout << nid << endl;
		flog.stop();
	}
	catch (f8Exception& e)
	{
		cerr << e.what() << endl;
	}
#endif

	return 0;
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

