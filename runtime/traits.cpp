//-----------------------------------------------------------------------------------------
/*

Fix8 is released under the GNU LESSER GENERAL PUBLIC LICENSE Version 3.

Fix8 Open Source FIX Engine.
Copyright (C) 2010-16 David L. Dight <fix@fix8.org>

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

*/
//-----------------------------------------------------------------------------------------
#include "precomp.hpp"
#include <fix8/f8includes.hpp>

//-------------------------------------------------------------------------------------------------
using namespace FIX8;
using namespace std;

//-------------------------------------------------------------------------------------------------
ostream& FIX8::operator<<(ostream& os, const FieldTrait& what)
{
	static const vector<string> bts { "mandatory", "present", "position", "group", "component", "suppress", "automatic" };
	os << "Tag:" << what._fnum << " Type:" << what._ftype << " (";
	string strtype;
	os << FieldTrait::get_type_string(what._ftype, strtype);
	os << ") Pos:" << what._pos;
	if (what._component)
		os << " Component:" << what._component;
	os << " Flags:";
	for (unsigned ii(what._field_traits.get()), pos(0); pos < FieldTrait::count; ++pos)
		if (ii >> pos & 0x1)
			os << bts[pos] << ' ';
	return os;
}

//-------------------------------------------------------------------------------------------------
ostream& FIX8::operator<<(ostream& os, const FieldTraits& what)
{
	copy(what.get_presence().begin(), what.get_presence().end(), ostream_iterator<FieldTrait>(os, "\n"));
	return os;
}

//-------------------------------------------------------------------------------------------------
string& FieldTrait::get_type_string(FieldTrait::FieldType ftype, string& to)
{
	return to = FieldTrait::is_int(ftype) ? "int" : FieldTrait::is_char(ftype) ? "char"
		: FieldTrait::is_float(ftype) ? "double" : FieldTrait::is_string(ftype) ? "f8String" : f8String();
}

