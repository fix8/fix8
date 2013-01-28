//-----------------------------------------------------------------------------------------
#if 0

Fix8 is released under the GNU LESSER GENERAL PUBLIC LICENSE Version 3.

Fix8 Open Source FIX Engine.
Copyright (C) 2010-13 David L. Dight <fix@fix8.org>

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

BaseField *Create_Orderbook(const f8String& from, const RealmBase *db, const int rv) { return new Orderbook(from, db); }
BaseField *Create_BrokerInitiated(const f8String& from, const RealmBase *db, const int rv) { return new BrokerInitiated(from, db); }
BaseField *Create_ExecOption(const f8String& from, const RealmBase *db, const int rv) { return new ExecOption(from, db); }

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
