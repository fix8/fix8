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

#endif

//-----------------------------------------------------------------------------------------
/** \file myfix_custom.hpp

  This file is an example of all the necessary data structures and classes needed to
  add custom fields to a client or server.
*/

//-----------------------------------------------------------------------------------------
#ifndef _FIX8_MYFIX_CUSTOM_HPP_
#define _FIX8_MYFIX_CUSTOM_HPP_

namespace FIX8 {
namespace TEX {

typedef Field<char, 6666> Orderbook;
typedef Field<Boolean, 6951> BrokerInitiated;
typedef Field<int, 7009> ExecOption;

namespace {

const char Orderbook_realm[] =	// the realms must be alphanumerically sorted(e.g. std::less)
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

class myfix_custom : public CustomFields
{
public:
	myfix_custom(bool cleanup=true) : CustomFields(cleanup)
	{
		add(6666, BaseEntry_ctor(new BaseEntry, &Create_Orderbook,
			&extra_realmbases[0], "Orderbook", "Select downstream execution venue"));
		add(6951, BaseEntry_ctor(new BaseEntry, &Create_BrokerInitiated,
			&extra_realmbases[1], "BrokerInitiated", "Indicate if order was broker initiated"));
		add(7009, BaseEntry_ctor(new BaseEntry, &Create_ExecOption,
			&extra_realmbases[2], "ExecOption", "Broker specific option"));
	}

	virtual ~myfix_custom() {}

	virtual bool post_msg_ctor(Message *msg)
	{
		if (msg->get_msgtype() == TEX::ExecutionReport::get_msgtype()())
		{
			static const FieldTrait trt[] =
			{
				FieldTrait(6666, 7, 4, 0x004), FieldTrait(6951,  8, 5, 0x004), FieldTrait(7009,  1, 6, 0x004),
			};
			msg->add_trait(trt, sizeof(trt)/sizeof(FieldTrait));
		}

		return true;
	}
};

} // namespace TEX
} // namespace FIX8

#endif // _FIX8_MYFIX_CUSTOM_HPP_
