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
#include <bitset>

#include <regex.h>
#include <errno.h>
#include <string.h>
#include <cctype>

// f8 headers
#include <f8includes.hpp>
#include <f8c.hpp>

//-----------------------------------------------------------------------------------------
using namespace std;
using namespace FIX8;

//-----------------------------------------------------------------------------------------
typedef std::map<std::string, const XmlElement *> Components;

//-----------------------------------------------------------------------------------------
const string doctype("<?xml version='1.0' encoding='ISO-8859-1'?>");

//-----------------------------------------------------------------------------------------
extern unsigned glob_errors;
extern string shortName;

//-----------------------------------------------------------------------------------------
void output_field(const XmlElement& xf, const int depth, ostream& outf);
void output_attributes(const XmlElement& xf, ostream& outf);
void process_component(const XmlElement& xf, const Components& components, const int depth, ostream& outf);
void process_group(const XmlElement& xf, const Components& components, const int depth, ostream& outf);
void process_messages(const XmlElement& xf, const Components& components, const string& tag, const int depth, ostream& outf);
void process_elements(XmlElement::XmlSet::const_iterator itr, const Components& components, const int depth, ostream& outf);
void process_fields(const XmlElement::XmlSet& fldlist, const int depth, ostream& outf);
void load_components(const XmlElement::XmlSet& comlist, Components& components);
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

	process_fields(fldlist, depth, outf);

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
void process_fields(const XmlElement::XmlSet& fldlist, const int depth, ostream& outf)
{
	outf << string(depth * 2, ' ') << "<fields>" << endl;
	for(XmlElement::XmlSet::const_iterator itr(fldlist.begin()); itr != fldlist.end(); ++itr)
	{
		outf << string((depth + 1) * 2, ' ') << "<field";
		output_attributes(**itr, outf);

		if ((*itr)->GetChildCnt())
		{
			outf << '>' << endl;
			for(XmlElement::XmlSet::const_iterator fitr((*itr)->begin()); fitr != (*itr)->end(); ++fitr)
			{
				outf << string((depth + 2) * 2, ' ') << '<' << (*fitr)->GetTag();
				output_attributes(**fitr, outf);
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
void output_field(const XmlElement& xf, const int depth, ostream& outf)
{
	outf << string(depth * 2, ' ') << "<field";
	output_attributes(xf, outf);
	outf << "/>" << endl;
}

//-----------------------------------------------------------------------------------------
void output_attributes(const XmlElement& xf, ostream& outf)
{
	for (XmlElement::XmlAttrs::const_iterator itr(xf.abegin()); itr != xf.aend(); ++itr)
		outf << ' ' << itr->first << "='" << itr->second << '\'';
	if (xf.GetLine())
		outf << " line=\'" << xf.GetLine() << '\'';
}

//-----------------------------------------------------------------------------------------
void process_elements(XmlElement::XmlSet::const_iterator itr, const Components& components, const int depth, ostream& outf)
{
	if ((*itr)->GetTag() == "field")
		output_field(**itr, depth, outf);
	else if ((*itr)->GetTag() == "component")
		process_component(**itr, components, depth, outf);
	else if ((*itr)->GetTag() == "group")
		process_group(**itr, components, depth, outf);
}

//-----------------------------------------------------------------------------------------
void process_messages(const XmlElement& xf, const Components& components, const string& tag, const int depth, ostream& outf)
{
	outf << string((depth + 1) * 2, ' ') << '<' << tag;
	output_attributes(xf, outf);
	outf << '>' << endl;

	for(XmlElement::XmlSet::const_iterator mitr(xf.begin()); mitr != xf.end(); ++mitr)
		process_elements(mitr, components, depth + 2, outf);
	outf << string((depth + 1) * 2, ' ') << "</" << tag << '>' << endl;
}

//-----------------------------------------------------------------------------------------
void process_component(const XmlElement& xf, const Components& components, const int depth, ostream& outf)
{
	string name;
	xf.GetAttr("name", name);
	Components::const_iterator citr(components.find(name));
	if (citr == components.end())
	{
		cerr << shortName << ':' << recover_line(xf) << ": error: Could not find component " << name << endl;
		++glob_errors;
	}
	else
		for(XmlElement::XmlSet::const_iterator itr(citr->second->begin()); itr != citr->second->end(); ++itr)
			process_elements(itr, components, depth, outf);
}

//-----------------------------------------------------------------------------------------
void process_group(const XmlElement& xf, const Components& components, const int depth, ostream& outf)
{
	outf << string(depth * 2, ' ') << "<group";
	output_attributes(xf, outf);
	outf << '>' << endl;

	for(XmlElement::XmlSet::const_iterator itr(xf.begin()); itr != xf.end(); ++itr)
		process_elements(itr, components, depth + 1, outf);
	outf << string(depth * 2, ' ') << "</group>" << endl;
}

