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
int main(int argc, char **argv)
{
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
	string from("35=1005=hello114=Y87=STOP47=10.239=14");
	RegExp elmnt("([0-9]+)=([^\x01]+)\x01");
	RegMatch match;
	unsigned s_offset(0);
	while (s_offset < from.size() && elmnt.SearchString(match, from, 3, s_offset) == 3)
	{
		string tag, val;
		elmnt.SubExpr(match, from, tag, s_offset, 1);
		elmnt.SubExpr(match, from, val, s_offset, 2);
		cout << tag << " => " << val << endl;
		s_offset += match.SubSize();
	}
	cout << "ol=" << from.size() << " cl=" << s_offset << endl;

	cout << TEX::ctx.version() << endl;
#endif


	try
	{
		//FileLogger flog("myfix.log");
		Logger flog(0);

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

	return 0;
}

