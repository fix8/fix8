//-----------------------------------------------------------------------------------------
/*

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

*/
//-----------------------------------------------------------------------------------------
#include <iostream>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <vector>
#include <map>
#include <list>
#include <set>
#include <iterator>
#include <algorithm>

#include <errno.h>
#include <string.h>
#include <cctype>

// f8 headers
#include <fix8/f8includes.hpp>
#include <f8c.hpp>

//-----------------------------------------------------------------------------------------
using namespace std;
using namespace FIX8;

//-----------------------------------------------------------------------------------------
const string doctype("<?xml version='1.0' encoding='ISO-8859-1'?>");

//-----------------------------------------------------------------------------------------
extern unsigned glob_errors;
extern string shortName;

//-----------------------------------------------------------------------------------------
void output_field(const XmlElement& xf, const int depth, ostream& outf, const string& compon=string(), bool required=false);
void output_attributes(const XmlElement& xf, ostream& outf, bool required=false);
void process_component(const XmlElement& xf, const Components& components, const int depth, ostream& outf, bool required=false);
void process_group(const XmlElement& xf, const Components& components, const int depth,
	ostream& outf, const string& compon=string(), bool required=false);
void process_messages(const XmlElement& xf, const Components& components, const string& tag, const int depth,
	ostream& outf, bool required=false);
void process_elements(XmlElement::XmlSet::const_iterator itr, const Components& components, const int depth,
	ostream& outf, const string& compon=string(), bool required=false);
void process_fields(const XmlElement::XmlSet& fldlist, const int depth, ostream& outf, bool required=false);
void load_components(const XmlElement::XmlSet& comlist, Components& components);
void dump_components(const Components& components, ostream& outf);
int precomp(XmlElement& xf, ostream& outf);
int precompfixt(XmlElement& xft, XmlElement& xf, ostream& outf, bool nounique);
void filter_unique(XmlElement::XmlSet& fldlist);

//-----------------------------------------------------------------------------------------
int precomp(XmlElement& xf, ostream& outf)
{
	int depth(1);
	XmlElement::XmlSet fldlist;
	xf.find("fix/fields/field", fldlist);

	XmlElement::XmlSet comlist;
	xf.find("fix/components/component", comlist);
	Components components;
	load_components(comlist, components);

	XmlElement::XmlSet msglist;
	xf.find("fix/messages/message", msglist);

	outf << doctype << endl;
	outf << '<' << xf.GetTag();
	output_attributes(xf, outf);
	outf << '>' << endl;

	const XmlElement *header(xf.find("fix/header"));
	if (header)
		process_messages(*header, components, "header", 0, outf);
	const XmlElement *trailer(xf.find("fix/trailer"));
	if (trailer)
		process_messages(*trailer, components, "trailer", 0, outf);

	outf << string(depth * 2, ' ') << "<messages>" << endl;
	for(XmlElement::XmlSet::const_iterator itr(msglist.begin()); itr != msglist.end(); ++itr)
		process_messages(**itr, components, "message", depth, outf);
	outf << string(depth * 2, ' ') << "</messages>" << endl;

	process_fields(fldlist, depth, outf);

	dump_components(components, outf);

	outf << "</" << xf.GetTag() << '>' << endl;

	return 0;
}

//-----------------------------------------------------------------------------------------
int precompfixt(XmlElement& xft, XmlElement& xf, ostream& outf, bool nounique)
{
	int depth(1);
	XmlElement::XmlSet fldlist;
	xft.find("fix/fields/field", fldlist);
	xf.find("fix/fields/field", fldlist);
	if (!nounique)
		filter_unique(fldlist);

	XmlElement::XmlSet comlist, comlistfixt;
	Components components, componentsfixt;
	xft.find("fix/components/component", comlistfixt);
	xf.find("fix/components/component", comlist);
	load_components(comlistfixt, componentsfixt);
	load_components(comlist, components);

	outf << doctype << endl;
	outf << '<' << xft.GetTag();
	output_attributes(xft, outf);
	outf << '>' << endl;

	const XmlElement *header(xft.find("fix/header"));
	if (header)
		process_messages(*header, componentsfixt, "header", 0, outf);
	const XmlElement *trailer(xft.find("fix/trailer"));
	if (trailer)
		process_messages(*trailer, componentsfixt, "trailer", 0, outf);

	outf << string(depth * 2, ' ') << "<messages>" << endl;

	XmlElement::XmlSet msglist;
	xft.find("fix/messages/message", msglist);
	for(XmlElement::XmlSet::const_iterator itr(msglist.begin()); itr != msglist.end(); ++itr)
		process_messages(**itr, componentsfixt, "message", depth, outf);

	msglist.clear();
	xf.find("fix/messages/message", msglist);
	for(XmlElement::XmlSet::const_iterator itr(msglist.begin()); itr != msglist.end(); ++itr)
		process_messages(**itr, components, "message", depth, outf);
	outf << string(depth * 2, ' ') << "</messages>" << endl;

	process_fields(fldlist, depth, outf, false);

	dump_components(components, outf);

	outf << "</" << xft.GetTag() << '>' << endl;

	return 0;
}

//-----------------------------------------------------------------------------------------
void filter_unique(XmlElement::XmlSet& fldlist)
{
	typedef map<string, const XmlElement *> UniqueFieldMap;
	UniqueFieldMap ufm;
	unsigned dupls(0);
	for(XmlElement::XmlSet::const_iterator itr(fldlist.begin()); itr != fldlist.end(); ++itr)
	{
		string name;
		(*itr)->GetAttr("name", name);
		if (!ufm.insert(UniqueFieldMap::value_type(name, *itr)).second)
			++dupls; // cerr << "Duplicate field: " << name << endl;
	}

	fldlist.clear();
	for(UniqueFieldMap::const_iterator itr(ufm.begin()); itr != ufm.end(); ++itr)
		fldlist.insert(itr->second);
}

//-----------------------------------------------------------------------------------------
void load_components(const XmlElement::XmlSet& comlist, Components& components)
{
	for(XmlElement::XmlSet::const_iterator itr(comlist.begin()); itr != comlist.end(); ++itr)
	{
		string name;
		if ((*itr)->GetAttr("name", name))
			components.insert(Components::value_type(name, *itr));
	}
}

//-----------------------------------------------------------------------------------------
void process_fields(const XmlElement::XmlSet& fldlist, const int depth, ostream& outf, bool required)
{
	outf << string(depth * 2, ' ') << "<fields>" << endl;
	for(XmlElement::XmlSet::const_iterator itr(fldlist.begin()); itr != fldlist.end(); ++itr)
	{
		outf << string((depth + 1) * 2, ' ') << "<field";
		output_attributes(**itr, outf, required);

		if ((*itr)->GetChildCnt())
		{
			outf << '>' << endl;
			for(XmlElement::XmlSet::const_iterator fitr((*itr)->begin()); fitr != (*itr)->end(); ++fitr)
			{
				outf << string((depth + 2) * 2, ' ') << '<' << (*fitr)->GetTag();
				output_attributes(**fitr, outf, required);
				outf << "/>" << endl;
			}
			outf << string((depth + 1) * 2, ' ') << "</field>" << endl;
		}
		else
			outf << "/>" << endl;
	}
	outf << string(depth * 2, ' ') << "</fields>" << endl;
}

//-----------------------------------------------------------------------------------------
void output_field(const XmlElement& xf, const int depth, ostream& outf, const string& compon, bool required)
{
	outf << string(depth * 2, ' ') << "<field";
	output_attributes(xf, outf, required);
	if (!compon.empty())
		outf << " component=\'" << compon << '\'';
	outf << "/>" << endl;
}

//-----------------------------------------------------------------------------------------
void output_attributes(const XmlElement& xf, ostream& outf, bool required)
{
	for (XmlElement::XmlAttrs::const_iterator itr(xf.abegin()); itr != xf.aend(); ++itr)
		outf << ' ' << itr->first << "='"
			<< (itr->first == "required" && itr->second == "Y" && !required ? "N" : itr->second) << '\'';
	if (xf.GetLine())
		outf << " line=\'" << xf.GetLine() << '\'';
}

//-----------------------------------------------------------------------------------------
void process_elements(XmlElement::XmlSet::const_iterator itr, const Components& components, const int depth,
	ostream& outf, const string& compon, bool required)
{
	if ((*itr)->GetTag() == "field")
		output_field(**itr, depth, outf, compon, required);
	else if ((*itr)->GetTag() == "component")
		process_component(**itr, components, depth, outf, required);
	else if ((*itr)->GetTag() == "group")
		process_group(**itr, components, depth, outf, compon, required);
}

//-----------------------------------------------------------------------------------------
void process_messages(const XmlElement& xf, const Components& components, const string& tag, const int depth, ostream& outf, bool required)
{
	outf << string((depth + 1) * 2, ' ') << '<' << tag;
	output_attributes(xf, outf);
	outf << '>' << endl;

	for(XmlElement::XmlSet::const_iterator mitr(xf.begin()); mitr != xf.end(); ++mitr)
		process_elements(mitr, components, depth + 2, outf, string(), required);
	outf << string((depth + 1) * 2, ' ') << "</" << tag << '>' << endl;
}

//-----------------------------------------------------------------------------------------
void process_component(const XmlElement& xf, const Components& components, const int depth, ostream& outf, bool required)
{
	string name;
	xf.GetAttr("name", name);
	bool comp_required(xf.FindAttr("required", false));

	Components::const_iterator citr(components.find(name));
	if (citr == components.end())
	{
		cerr << shortName << ':' << recover_line(xf) << ": error: Could not find component '" << name << '\'' << endl;
		++glob_errors;
	}
	else
	{
		for(XmlElement::XmlSet::const_iterator itr(citr->second->begin()); itr != citr->second->end(); ++itr)
			process_elements(itr, components, depth, outf, name,
				depth == 3 ? comp_required : comp_required && required);
	}
}

//-----------------------------------------------------------------------------------------
void process_group(const XmlElement& xf, const Components& components, const int depth,
	ostream& outf, const string& compon, bool required)
{
	outf << string(depth * 2, ' ') << "<group";
	output_attributes(xf, outf);
	if (!compon.empty())
		outf << " component=\'" << compon << '\'';
	outf << '>' << endl;

	for(XmlElement::XmlSet::const_iterator itr(xf.begin()); itr != xf.end(); ++itr)
		process_elements(itr, components, depth + 1, outf, string(), required);
	outf << string(depth * 2, ' ') << "</group>" << endl;
}

//-----------------------------------------------------------------------------------------
void dump_components(const Components& components, ostream& outf)
{
	if (components.empty())
		return;

	int depth(1);

	outf << string(depth * 2, ' ') << "<components>" << endl;

	for (Components::const_iterator citr(components.begin()); citr != components.end(); ++citr)
	{
		outf << string((depth + 1) * 2, ' ') << "<component name=\"";
		outf << citr->first << "\" id=\"" << (1 + distance(components.begin(), citr)) << "\"/>" << endl;
	}

	outf << string(depth * 2, ' ') << "</components>" << endl;
}
