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
#include <usage.hpp>
#include <f8c.hpp>

//-----------------------------------------------------------------------------------------
using namespace std;
using namespace FIX8;

//-----------------------------------------------------------------------------------------
extern string shortName, odir, prefix;
extern bool verbose;
extern string spacer;
extern const string GETARGLIST;
extern const CSMap _csMap;
extern unsigned glob_errors;

//-----------------------------------------------------------------------------------------
void print_usage();
int process(XmlElement& xf, Ctxt& ctxt);
int load_fix_version (XmlElement& xf, Ctxt& ctxt);
int load_fields(XmlElement& xf, FieldSpecMap& fspec);
void process_special_traits(const unsigned short field, FieldTraits& fts);
int process_message_fields(const std::string& where, XmlElement *xt, FieldTraits& fts,
	const FieldToNumMap& ftonSpec, const FieldSpecMap& fspec, const unsigned component);
int load_messages(XmlElement& xf, MessageSpecMap& mspec, const FieldToNumMap& ftonSpec, const FieldSpecMap& fspec);
void process_ordering(MessageSpecMap& mspec);
const string flname(const string& from);
void process_value_enums(FieldSpecMap::const_iterator itr, ostream& ost_hpp, ostream& ost_cpp);
const string& mkel(const string& base, const string& compon, string& where);
void process_group_ordering(const MessageSpec& ms);

//-----------------------------------------------------------------------------------------
ostream *open_ofile(const string& odir, const string& fname, string& target)
{
	ostringstream ofs;
	string odirect(odir);
	ofs << CheckAddTrailingSlash(odirect) << fname;
	target = ofs.str();
	scoped_ptr<ofstream> os(new ofstream(target.c_str()));
	if (!*os)
	{
		cerr << "Error opening file \'" << target << '\'';
		if	(errno)
			cerr << " (" << strerror(errno) << ')';
		cerr << endl;

		return 0;
	}

	return os.release();
}

//-----------------------------------------------------------------------------------------
const string& mkel(const string& base, const string& compon, string& where)
{
	ostringstream ostr;
	ostr << base << '/' << compon;
	return where = ostr.str();
}

//-----------------------------------------------------------------------------------------
const string& filepart(const string& source, string& where)
{
	string::size_type pos(source.find_last_of('/'));
	return pos == string::npos ? where = source : where = source.substr(pos + 1);
}

//-----------------------------------------------------------------------------------------
int load_fix_version (XmlElement& xf, Ctxt& ctxt)
{
	XmlElement *fix(xf.find("fix"));
	if (!fix)
	{
		cerr << "No fix header element found in " << shortName << endl;
		return -1;
	}

	string major, minor, revision("0"), type("FIX");

	if (!fix->GetAttr("major", major) || !fix->GetAttr("minor", minor))
	{
		cerr << "Missing required attributes (major/minor) from fix header in " << shortName << endl;
		return -1;
	}

	if (!fix->GetAttr("revision", revision))
		fix->GetAttr("servicepack", revision);
	fix->GetAttr("type", type);

	// fix version: <Major:1><Minor:1><Revision:2> eg. 4.2r10 is 4210
	ctxt._version = GetValue<int>(major) * 1000 + GetValue<int>(minor) * 100 + GetValue<int>(revision);
	if (type == "FIX" && ctxt._version < 4000)
	{
		cerr << "Unsupported FIX version " << ctxt._version << " from fix header in " << shortName << endl;
		return -1;
	}

	ostringstream ostr;
	ostr << type << ctxt._version;
	ctxt._systemns = ostr.str();
	if (ctxt._fixns.empty())
		ctxt._fixns = ctxt._systemns;
	ctxt._clname = prefix;

	ostr.str("");
	ostr << type << '.' << major << '.' << minor;
	ctxt._beginstr = ostr.str();

	return 0;
}

//-----------------------------------------------------------------------------------------
void process_special_traits(const unsigned short field, FieldTraits& fts)
{
	switch(field)
	{
	case Common_BeginString:
	case Common_BodyLength:
	case Common_CheckSum:
		fts.set(field, FieldTrait::suppress);	// drop through
	case Common_MsgType:
		fts.set(field, FieldTrait::automatic);
	default:
		break;
	}
}

//-----------------------------------------------------------------------------------------
void process_value_enums(FieldSpecMap::const_iterator itr, ostream& ost_hpp, ostream& ost_cpp)
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
int process_message_fields(const std::string& where, XmlElement *xt, FieldTraits& fts, const FieldToNumMap& ftonSpec,
	const FieldSpecMap& fspec, const unsigned subpos)
{
	unsigned processed(0);
	XmlElement::XmlSet flist;
	if (xt->find(where, flist))
	{
		for(XmlElement::XmlSet::const_iterator fitr(flist.begin()); fitr != flist.end(); ++fitr)
		{
			string fname, required;
			if ((*fitr)->GetAttr("name", fname) && (*fitr)->GetAttr("required", required))
			{
				FieldToNumMap::const_iterator ftonItr(ftonSpec.find(fname));
				FieldSpecMap::const_iterator fs_itr;
				if (ftonItr == ftonSpec.end() || (fs_itr = fspec.find(ftonItr->second)) == fspec.end())
				{
					cerr << shortName << ':' << recover_line(**fitr) << ": error: Field element missing required attributes" << endl;
					++glob_errors;
					continue;
				}

				// add FieldTrait
				if (!fts.add(FieldTrait(fs_itr->first, fs_itr->second._ftype, (*fitr)->GetSubIdx(), required == "Y", false, subpos)))
					cerr << shortName << ':' << recover_line(**fitr) << ": error: Could not add trait object " << fname << endl;
				else
				{
					process_special_traits(fs_itr->first, fts);
					++processed;
				}
			}
			else
			{
				cerr << shortName << ':' << recover_line(**fitr) << ": error: Field element missing required attributes" << endl;
				++glob_errors;
			}
		}
	}

	return processed;
}

//-----------------------------------------------------------------------------------------
const string flname(const string& from)
{
	return from.substr(0, from.find_first_of('.'));
}

//-----------------------------------------------------------------------------------------
void process_ordering(MessageSpecMap& mspec)
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

		process_group_ordering(mitr->second);
	}
}

//-----------------------------------------------------------------------------------------
void process_group_ordering(const MessageSpec& ms)
{
	for (GroupMap::const_iterator gitr(ms._groups.begin()); gitr != ms._groups.end(); ++gitr)
	{
		FieldTraitOrder go;
		for (Presence::const_iterator flitr(gitr->second._fields.get_presence().begin());
			flitr != gitr->second._fields.get_presence().end(); ++flitr)
				go.insert(FieldTraitOrder::value_type(&*flitr));

		unsigned gcnt(0);
		for (FieldTraitOrder::iterator fto(go.begin()); fto != go.end(); ++fto)
			(*fto)->_pos = ++gcnt;

		if (!gitr->second._groups.empty())
			process_group_ordering(gitr->second);
	}
}

//-----------------------------------------------------------------------------------------
void print_usage()
{
	UsageMan um("f8c", GETARGLIST, "<input xml schema>");
	um.setdesc("f8c -- compile FIX xml schema");
	um.add('o', "odir <file>", "output target directory (default ./)");
	um.add('p', "prefix <prefix>", "output filename prefix (default Myfix)");
	um.add('d', "dump", "dump 1st pass parsed source xml file, exit");
	um.add('h', "help", "help, this screen");
	um.add('i', "ignore", "ignore errors, attempt to generate code anyhow (default no)");
	um.add('k', "keep", "retain generated temporaries even if there are errors (.*.tmp)");
	um.add('v', "version", "print version, exit");
	um.add('s', "second", "2nd pass only, no precompile (default both)");
	um.add('N', "nounique", "do not enforce unique field parsing (default false)");
	um.add('r', "retain", "retain 1st pass code (default delete)");
	um.add('t', "tabwidth", "tabwidth for generated code (default 3 spaces)");
	um.add('x', "fixt <file>", "For FIXT hosted transports or for FIX5.0 and above, the input FIXT schema file");
	um.add('V', "verbose", "be more verbose when processing");
	um.add('n', "namespace <ns>", "namespace to place generated code in (default FIXMmvv e.g. FIX4400)");
	um.add("e.g.");
	um.add("@f8c -p Texfix -n TEX myfix.xml");
	um.add("@f8c -rp Texfix -n TEX -x ../schema/FIXT11.xml myfix.xml");
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
string insert_year()
{
   struct tm tim;
	time_t now(time(0));
   localtime_r(&now, &tim);
	ostringstream ostr;
	ostr << setw(2) << (tim.tm_year - 100);
	return ostr.str();
}

//-------------------------------------------------------------------------------------------------
void generate_preamble(ostream& to)
{
	to << _csMap.find_ref(cs_divider) << endl;
	string result;
	to << _csMap.find_ref(cs_do_not_edit) << GetTimeAsStringMS(result, 0, 0) << " ***" << endl;
	to << _csMap.find_ref(cs_divider) << endl;
	to << _csMap.find_ref(cs_copyright) << insert_year() << _csMap.find_ref(cs_copyright2) << endl;
	to << _csMap.find_ref(cs_divider) << endl;
}

//-------------------------------------------------------------------------------------------------
ostream& FIX8::operator<<(ostream& os, const FIX8::MessageSpec& what)
{
	os << "Name:" << what._name;
	if (!what._description.empty())
		os << " Description:" << what._description;
	if (!what._comment.empty())
		os << " Comment:" << what._comment;
	os << " isadmin:" << boolalpha << what._is_admin << endl;
	os << "Fields:" << endl << what._fields;
	for (GroupMap::const_iterator itr(what._groups.begin()); itr != what._groups.end(); ++itr)
		os << "Group (" << itr->first << "): " << endl << itr->second << endl;

	return os;
}

