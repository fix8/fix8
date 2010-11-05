//-----------------------------------------------------------------------------------------
#if 0

Fix8 is released under the New BSD License.

Copyright (c) 2010, David L. Dight <fix@fix8.org>
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

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS
OR  IMPLIED  WARRANTIES,  INCLUDING,  BUT  NOT  LIMITED  TO ,  THE  IMPLIED  WARRANTIES  OF
MERCHANTABILITY AND  FITNESS FOR A PARTICULAR  PURPOSE ARE  DISCLAIMED. IN  NO EVENT  SHALL
THE  COPYRIGHT  OWNER OR  CONTRIBUTORS BE  LIABLE  FOR  ANY DIRECT,  INDIRECT,  INCIDENTAL,
SPECIAL,  EXEMPLARY, OR CONSEQUENTIAL  DAMAGES (INCLUDING,  BUT NOT LIMITED TO, PROCUREMENT
OF SUBSTITUTE  GOODS OR SERVICES; LOSS OF USE, DATA,  OR PROFITS; OR BUSINESS INTERRUPTION)
HOWEVER CAUSED  AND ON ANY THEORY OF LIABILITY, WHETHER  IN CONTRACT, STRICT  LIABILITY, OR
TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE
EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

-------------------------------------------------------------------------------------------
$Id$
$Date$
$URL$

#endif
//-----------------------------------------------------------------------------------------
#include <config.h>
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

#ifdef HAS_TR1_UNORDERED_MAP
#include <tr1/unordered_map>
#endif

// f8 headers
#include <f8exception.hpp>
#include <f8utils.hpp>
#include <traits.hpp>
#include <f8types.hpp>
#include <field.hpp>
#include <message.hpp>
#include <usage.hpp>
#include <xml.hpp>
#include <f8c.hpp>

//-----------------------------------------------------------------------------------------
using namespace std;
using namespace FIX8;

//-----------------------------------------------------------------------------------------
static const std::string rcsid("$Id$");

//-----------------------------------------------------------------------------------------
extern string inputFile, odir, prefix;
extern bool verbose;
extern const string spacer, GETARGLIST;

//-----------------------------------------------------------------------------------------
void print_usage();
int process(XmlEntity& xf, Ctxt& ctxt);
int loadFixVersion (XmlEntity& xf, Ctxt& ctxt);
int loadfields(XmlEntity& xf, FieldSpecMap& fspec);
void processSpecialTraits(const unsigned short field, FieldTraits& fts);
int processMessageFields(const std::string& where, XmlEntity *xt, FieldTraits& fts,
	const FieldToNumMap& ftonSpec, const FieldSpecMap& fspec, const unsigned component);
int loadmessages(XmlEntity& xf, MessageSpecMap& mspec, const ComponentSpecMap& cspec,
	const FieldToNumMap& ftonSpec, const FieldSpecMap& fspec);
void processOrdering(MessageSpecMap& mspec);
int loadcomponents(XmlEntity& xf, ComponentSpecMap& mspec, const FieldToNumMap& ftonSpec, const FieldSpecMap& fspec);
const string flname(const string& from);
void processValueEnums(FieldSpecMap::const_iterator itr, ostream& ost_hpp, ostream& ost_cpp);
const string& mkel(const string& base, const string& compon, string& where);

//-----------------------------------------------------------------------------------------
class filestdout
{
	std::ostream *os_;
	bool del_;

public:
	filestdout(std::ostream *os, bool del=false) : os_(os), del_(del) {}
	~filestdout() { if (del_) delete os_; }

	std::ostream& operator()() { return *os_; }
};

//-----------------------------------------------------------------------------------------
ostream *openofile(const string& odir, const string& fname)
{
	ostringstream ofs;
	string odirect(odir);
	ofs << CheckAddTrailingSlash(odirect) << fname;
	ofstream *os(new ofstream(ofs.str().c_str()));
	if (!*os)
	{
		cerr << "Error opening file \'" << ofs.str() << '\'';
		if	(errno)
			cerr << " (" << strerror(errno) << ')';
		cerr << endl;
		return 0;
	}

	return os;
}

//-----------------------------------------------------------------------------------------
const string& mkel(const string& base, const string& compon, string& where)
{
	ostringstream ostr;
	ostr << base << '/' << compon;
	return where = ostr.str();
}

//-----------------------------------------------------------------------------------------
int loadFixVersion (XmlEntity& xf, Ctxt& ctxt)
{
	XmlEntity *fix(xf.find("fix"));
	if (!fix)
	{
		cerr << "No fix header element found in " << inputFile << endl;
		return -1;
	}

	string major, minor, revision("0");

	if (!fix->GetAttr("major", major) || !fix->GetAttr("minor", minor))
	{
		cerr << "Missing required attributes (major/minor) from fix header in " << inputFile << endl;
		return -1;
	}

	fix->GetAttr("revision", revision);
	fix->GetAttr("ns", ctxt._fixns);

	// fix version: <Major:1><Minor:1><Revision:2> eg. 4.2r10 is 4210
	ctxt._version = GetValue<int>(major) * 1000 + GetValue<int>(minor) * 100 + GetValue<int>(revision);
	if (ctxt._version < 4000 || ctxt._version > 6000)
	{
		cerr << "Invalid FIX version " << ctxt._version << " from fix header in " << inputFile << endl;
		return -1;
	}

	ostringstream ostr;
	ostr << "FIX" << ctxt._version;
	ctxt._systemns = ostr.str();
	if (ctxt._fixns.empty())
		ctxt._fixns = ctxt._systemns;
	ctxt._clname = prefix;

	ostr.str("");
	if (GetValue<int>(major) > 4)
		ostr << "FIXT.1.1";
	else
		ostr << "FIX." << major << '.' << minor;
	ctxt._beginstr = ostr.str();

	return 0;
}

//-----------------------------------------------------------------------------------------
void processSpecialTraits(const unsigned short field, FieldTraits& fts)
{
	switch(field)
	{
	case Common_BeginString:
	case Common_BodyLength:
	case Common_CheckSum:
		fts.set(field, FieldTrait::suppress);	// drop through
	case Common_MsgType:
		fts.set(field, FieldTrait::automatic);
		break;
	}
}

//-----------------------------------------------------------------------------------------
void processValueEnums(FieldSpecMap::const_iterator itr, ostream& ost_hpp, ostream& ost_cpp)
{
	string typestr("const ");
	if (FieldTrait::is_int(itr->second._ftype))
		typestr += "int ";
	else if (FieldTrait::is_char(itr->second._ftype))
		typestr += "char ";
	else if (FieldTrait::is_float(itr->second._ftype))
		typestr += "double ";
	else if (FieldTrait::is_string(itr->second._ftype))
		typestr += "f8String ";
	else
		return;

	ost_cpp << typestr << itr->second._name << "_realm[] = " << endl << spacer << "{ ";
	unsigned cnt(0);
	for (RealmMap::const_iterator ditr(itr->second._dvals->begin()); ditr != itr->second._dvals->end(); ++ditr)
	{
		if (cnt)
			ost_cpp << ", ";
		ost_cpp << *ditr->first;
		ost_hpp << typestr << itr->second._name << '_' << ditr->second;
		if (ditr->first->is_range())
		{
			if (cnt == 0)
				ost_hpp << "_lower";
			else if (cnt == 1)
				ost_hpp << "_upper";
		}
		ost_hpp << '(' << *ditr->first << ");" << endl;
		++cnt;
	}
	ost_hpp << "const size_t " << itr->second._name << "_realm_els(" << itr->second._dvals->size() << ");" << endl;
	ost_cpp << " };" << endl;

	ost_cpp << "const char *" << itr->second._name << "_descriptions[] = " << endl << spacer << "{ ";
	cnt = 0;
	for (RealmMap::const_iterator ditr(itr->second._dvals->begin()); ditr != itr->second._dvals->end(); ++ditr)
	{
		if (cnt)
			ost_cpp << ", ";
		ost_cpp << '"' << ditr->second << '"';
		++cnt;
	}
	ost_cpp << " };" << endl;
}

//-----------------------------------------------------------------------------------------
int processMessageFields(const std::string& where, XmlEntity *xt, FieldTraits& fts, const FieldToNumMap& ftonSpec,
	const FieldSpecMap& fspec, const unsigned subpos)
{
	unsigned processed(0);
	XmlList flist;
	if (xt->find(where, flist))
	{
		bool hasMandatory(false);

		for(XmlList::const_iterator fitr(flist.begin()); fitr != flist.end(); ++fitr)
		{
			string fname, required;
			if ((*fitr)->GetAttr("name", fname) && (*fitr)->GetAttr("required", required))
			{
				FieldToNumMap::const_iterator ftonItr(ftonSpec.find(fname));
				FieldSpecMap::const_iterator fs_itr;
				if (ftonItr == ftonSpec.end() || (fs_itr = fspec.find(ftonItr->second)) == fspec.end())
				{
					cerr << "Could not locate Field " << fname << " from known field types in " << inputFile << endl;
					continue;
				}

				// add FieldTrait
				if (required == "Y")
					hasMandatory = true;
				if (!fts.add(FieldTrait(fs_itr->first, fs_itr->second._ftype, (*fitr)->GetSubIdx(), required == "Y", false, subpos)))
					cerr << "Could not add trait object " << fname << endl;
				else
				{
					processSpecialTraits(fs_itr->first, fts);
					++processed;
				}
			}
			else
				cerr << "Field element missing required attributes at "
					<< inputFile << '(' << (*fitr)->GetLine() << ')' << endl;
		}

		if (hasMandatory)
			fts.set_mandatory();
	}

	return processed;
}

//-----------------------------------------------------------------------------------------
const string flname(const string& from)
{
	return from.substr(0, from.find_first_of('.'));
}

//-----------------------------------------------------------------------------------------
void processOrdering(MessageSpecMap& mspec)
{
	for (MessageSpecMap::const_iterator mitr(mspec.begin()); mitr != mspec.end(); ++mitr)
	{
		FieldTraitOrder mo;
		for (Presence::const_iterator flitr(mitr->second._fields.get_presence().begin());
			flitr != mitr->second._fields.get_presence().end(); ++flitr)
				mo.insert(FieldTraitOrder::value_type(&*flitr));

		unsigned cnt(0);
		for (FieldTraitOrder::iterator fto(mo.begin()); fto != mo.end(); ++fto)
			(*fto)->_pos = ++cnt;

		for (GroupMap::const_iterator gitr(mitr->second._groups.begin()); gitr != mitr->second._groups.end(); ++gitr)
		{
			FieldTraitOrder go;
			for (Presence::const_iterator flitr(gitr->second.get_presence().begin());
				flitr != gitr->second.get_presence().end(); ++flitr)
					go.insert(FieldTraitOrder::value_type(&*flitr));

			unsigned gcnt(0);
			for (FieldTraitOrder::iterator fto(go.begin()); fto != go.end(); ++fto)
				(*fto)->_pos = ++gcnt;
		}
	}
}

//-----------------------------------------------------------------------------------------
void print_usage()
{
	UsageMan um("f8c", GETARGLIST, "<input xml schema>");
	um.setdesc("f8c -- compile xml schema into fix8 application classes");
	um.add('o', "odir <file>", "output target directory");
	um.add('p', "prefix <file>", "output filename prefix");
	um.add('d', "dump", "dump parsed source xml file");
	um.add('h', "help", "help, this screen");
	um.add('v', "version", "print version, exit");
	um.add('V', "verbose", "be more verbose when processing");
	um.add("e.g.");
	um.add("@f8c -p myfix myfix.xml");
	um.print(cerr);
}

//-------------------------------------------------------------------------------------------------
RealmObject *RealmObject::create(const string& from, FieldTrait::FieldType ftype, bool isRange)
{
	if (FieldTrait::is_int(ftype))
		return new TypedRealm<int>(GetValue<int>(from), isRange);
	if (FieldTrait::is_char(ftype))
		return new CharRealm(from[0], isRange);
	if (FieldTrait::is_float(ftype))
		return new TypedRealm<double>(GetValue<double>(from), isRange);
	if (FieldTrait::is_string(ftype))
		return new StringRealm(from, isRange);
	return 0;
}

//-------------------------------------------------------------------------------------------------
