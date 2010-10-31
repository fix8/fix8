#include <config.h>
#include <iostream>
#include <sstream>
#include <vector>
#include <map>
#include <set>
#include <iterator>
#include <memory>
#include <iomanip>
#include <algorithm>
#include <numeric>
#include <bitset>

#ifdef HAS_TR1_UNORDERED_MAP
#include <tr1/unordered_map>
#endif

#include <strings.h>
#include <regex.h>
#include <config.h>

#include <f8exception.hpp>
#include <f8types.hpp>
#include <f8utils.hpp>
#include <traits.hpp>
#include <field.hpp>
#include <message.hpp>
#include <session.hpp>

//-------------------------------------------------------------------------------------------------
using namespace FIX8;
using namespace std;

//-------------------------------------------------------------------------------------------------
namespace {
	const string spacer(3, ' ');
}

//-------------------------------------------------------------------------------------------------
RegExp SessionID::_sid("([^:]+):([^-]+)->(.+)");

//-------------------------------------------------------------------------------------------------
template<>
const Session::Handlers::TypePair Session::Handlers::_valueTable[] =
{
	Session::Handlers::TypePair(Common_MsgType_HEARTBEAT, &Session::Heartbeat),
	Session::Handlers::TypePair(Common_MsgType_TEST_REQUEST, &Session::TestRequest),
	Session::Handlers::TypePair(Common_MsgType_RESEND_REQUEST, &Session::ResendRequest),
	Session::Handlers::TypePair(Common_MsgType_REJECT, &Session::Reject),
	Session::Handlers::TypePair(Common_MsgType_SEQUENCE_RESET, &Session::SequenceReset),
	Session::Handlers::TypePair(Common_MsgType_LOGOUT, &Session::Logout),
	Session::Handlers::TypePair(Common_MsgType_LOGON, &Session::Logon),
};

template<>
const Session::Handlers::TypeMap Session::Handlers::_valuemap(Session::Handlers::_valueTable,
	Session::Handlers::get_table_end());
template<>
const Session::Handlers::NoValType Session::Handlers::_noval(&Session::Application);

//-------------------------------------------------------------------------------------------------
const f8String& SessionID::make_id()
{
	ostringstream ostr;
	ostr << _beginString << ':' << _senderCompID << "->" << _targetCompID;
	return _id = ostr.str();
}

//-------------------------------------------------------------------------------------------------
void SessionID::from_string(const f8String& from)
{
	RegMatch match;
	if (_sid.SearchString(match, from, 4) == 4)
	{
		f8String bstr, scid, tcid;
		_sid.SubExpr(match, from, bstr, 0, 1);
		_beginString.set(bstr);
		_sid.SubExpr(match, from, scid, 0, 2);
		_senderCompID.set(scid);
		_sid.SubExpr(match, from, tcid, 0, 3);
		_targetCompID.set(tcid);
		make_id();
	}
}

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
Session::Session()
{
}

//-------------------------------------------------------------------------------------------------
void Session::start()
{
	while(true)
	{
		Message *msg;
		(this->*_handlers.find_value_ref(msg->get_msgtype()))(msg);
	}
}

