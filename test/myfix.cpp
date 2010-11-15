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
#include <f8exception.hpp>
#include <thread.hpp>
#include <f8utils.hpp>
#include <traits.hpp>
#include <f8types.hpp>
#include <field.hpp>
#include <message.hpp>
#include <session.hpp>
#include <persist.hpp>
#include <logger.hpp>
#include "Myfix_types.hpp"
#include "Myfix_router.hpp"
#include "Myfix_classes.hpp"

//-----------------------------------------------------------------------------------------
using namespace std;
using namespace FIX8;

//-----------------------------------------------------------------------------------------
static const std::string rcsid("$Id$");

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

class myfix_session : public Session
{
public:
	myfix_session(const F8MetaCntx& ctx, const SessionID& sid, Persister *persist, Logger *logger)
		: Session(ctx, sid, persist, logger) {}

	Message *generate_logon(const unsigned heartbtint)
	{
		TEX::Logon *msg(static_cast<TEX::Logon *>(Session::generate_logon(10)));
		return msg;
	}
};

//-----------------------------------------------------------------------------------------
int main(int argc, char **argv)
{
	strcpy(glob_log0, argv[1]);
	GlobalLogger::instance()->send("test fix client starting up...");
	//SingleLogger<glob_log1>::instance()->send("test fix client starting up...");

	const SessionID id(TEX::ctx._beginStr, "DLD_TEX", "TEX_DLD");
	BDBPersister *bdp(new BDBPersister);
	bdp->initialise("./run", "myfix.db");
	Logger::LogFlags logflags;
	logflags << Logger::timestamp << Logger::sequence << Logger::append;
	FileLogger *log(new FileLogger("./run/myfix.log", logflags));
	myfix_session ms(TEX::ctx, id, bdp, log);
	Poco::Net::StreamSocket *sock(new Poco::Net::StreamSocket);
	Poco::Net::SocketAddress addr("127.0.0.1:11001");
	FIXReader *fr(new FIXReader(sock, ms, &Session::process));
	ClientConnection *cc(new ClientConnection(sock, addr, fr));
	ms.begin(cc);

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

