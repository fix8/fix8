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
typedef std::map<std::string, XmlEntity *> Components;

//-----------------------------------------------------------------------------------------
void outputField(const XmlEntity& xf, const int depth, ostream& outf);
void outputAttributes(const XmlEntity& xf, ostream& outf);
void processComponent(const XmlEntity& xf, const Components& components, const int depth, ostream& outf);
void processGroup(const XmlEntity& xf, const Components& components, const int depth, ostream& outf);
void processMessages(const XmlEntity& xf, const Components& components, const string& tag, const int depth, ostream& outf);

//-----------------------------------------------------------------------------------------
int precomp(XmlEntity& xf, ostream& outf)
{
	int depth(1);
	XmlEntity::XmlSet fldlist;
	xf.find("fix/fields/field", fldlist);

	XmlEntity::XmlSet comlist;
	xf.find("fix/components/component", comlist);
	Components components;

	for(XmlEntity::XmlSet::const_iterator itr(comlist.begin()); itr != comlist.end(); ++itr)
	{
		string name;
		if ((*itr)->GetAttr("name", name))
			components.insert(Components::value_type(name, *itr));
	}

	XmlEntity::XmlSet msglist;
	xf.find("fix/messages/message", msglist);

	outf << "<?xml version='1.0' encoding='ISO-8859-1'?>" << endl;
	outf << '<' << xf.GetTag();
	outputAttributes(xf, outf);
	outf << '>' << endl;

	XmlEntity *header(xf.find("fix/header"));
	if (header)
		processMessages(*header, components, "header", 0, outf);
	XmlEntity *trailer(xf.find("fix/trailer"));
	if (trailer)
		processMessages(*trailer, components, "trailer", 0, outf);

	outf << string(depth * 2, ' ') << "<messages>" << endl;
	for(XmlEntity::XmlSet::const_iterator itr(msglist.begin()); itr != msglist.end(); ++itr)
		processMessages(**itr, components, "message", depth, outf);
	outf << string(depth * 2, ' ') << "</messages>" << endl;

	outf << string(depth * 2, ' ') << "<fields>" << endl;
	for(XmlEntity::XmlSet::const_iterator itr(fldlist.begin()); itr != fldlist.end(); ++itr)
	{
		outf << string((depth + 1) * 2, ' ') << "<field";
		outputAttributes(**itr, outf);

		if ((*itr)->GetChildCnt())
		{
			outf << '>' << endl;
			for(XmlEntity::XmlSet::const_iterator fitr((*itr)->begin()); fitr != (*itr)->end(); ++fitr)
			{
				outf << string((depth + 2) * 2, ' ') << '<' << (*fitr)->GetTag();
				outputAttributes(**fitr, outf);
				outf << "/>" << endl;
			}
			outf << string((depth + 1) * 2, ' ') << "</field>" << endl;
		}
		else
			outf << "/>" << endl;
	}
	outf << string(depth * 2, ' ') << "</fields>" << endl;

	outf << "</" << xf.GetTag() << '>' << endl;

	return 0;
}

//-----------------------------------------------------------------------------------------
void outputField(const XmlEntity& xf, const int depth, ostream& outf)
{
	outf << string(depth * 2, ' ') << "<field";
	outputAttributes(xf, outf);
	outf << "/>" << endl;
}

//-----------------------------------------------------------------------------------------
void outputAttributes(const XmlEntity& xf, ostream& outf)
{
	for (XmlEntity::XmlAttrs::const_iterator itr(xf.abegin()); itr != xf.aend(); ++itr)
		outf << ' ' << itr->first << "='" << itr->second << '\'';
	outf << " line=\'" << xf.GetLine() << '\'';
}

//-----------------------------------------------------------------------------------------
void processMessages(const XmlEntity& xf, const Components& components, const string& tag, const int depth, ostream& outf)
{
	outf << string((depth + 1) * 2, ' ') << '<' << tag;
	outputAttributes(xf, outf);
	outf << '>' << endl;

	for(XmlEntity::XmlSet::const_iterator mitr(xf.begin()); mitr != xf.end(); ++mitr)
	{
		if ((*mitr)->GetTag() == "field")
			outputField(**mitr, depth + 2, outf);
		else if ((*mitr)->GetTag() == "component")
			processComponent(**mitr, components, depth + 2, outf);
		else if ((*mitr)->GetTag() == "group")
			processGroup(**mitr, components, depth + 2, outf);
	}

	outf << string((depth + 1) * 2, ' ') << "</" << tag << '>' << endl;
}

//-----------------------------------------------------------------------------------------
void processComponent(const XmlEntity& xf, const Components& components, const int depth, ostream& outf)
{
	string name;
	xf.GetAttr("name", name);
	Components::const_iterator citr(components.find(name));
	if (citr == components.end())
		return;

	for(XmlEntity::XmlSet::const_iterator itr(citr->second->begin()); itr != citr->second->end(); ++itr)
	{
		if ((*itr)->GetTag() == "field")
			outputField(**itr, depth, outf);
		else if ((*itr)->GetTag() == "component")
			processComponent(**itr, components, depth, outf);
		else if ((*itr)->GetTag() == "group")
			processGroup(**itr, components, depth, outf);
	}
}

//-----------------------------------------------------------------------------------------
void processGroup(const XmlEntity& xf, const Components& components, const int depth, ostream& outf)
{
	outf << string(depth * 2, ' ') << "<group";
	outputAttributes(xf, outf);
	outf << '>' << endl;

	for(XmlEntity::XmlSet::const_iterator itr(xf.begin()); itr != xf.end(); ++itr)
	{
		if ((*itr)->GetTag() == "field")
			outputField(**itr, depth + 1, outf);
		else if ((*itr)->GetTag() == "component")
			processComponent(**itr, components, depth + 1, outf);
		else if ((*itr)->GetTag() == "group")
			processGroup(**itr, components, depth + 1, outf);
	}

	outf << string(depth * 2, ' ') << "</group>" << endl;
}

