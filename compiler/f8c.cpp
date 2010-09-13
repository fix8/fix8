//-----------------------------------------------------------------------------------------
#if 0

Fix8 is released under the New BSD License.

Copyright (c) 2010, David L. Dight <www@orbweb.org>
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
//#ifdef HAVE_GETOPT_H
#include <getopt.h>
//#endif

// f8 headers
#include <f8types.hpp>
#include <f8utils.hpp>
#include <traits.hpp>
#include <field.hpp>
#include <message.hpp>
#include <usage.hpp>
#include <xml.hpp>
#include <f8c.hpp>

//#include <config.h>

//-----------------------------------------------------------------------------------------
using namespace std;
using namespace FIX8;

//-----------------------------------------------------------------------------------------
static const std::string rcsid("$Id$");

//-----------------------------------------------------------------------------------------
const string GETARGLIST("hvo:p:");

//-----------------------------------------------------------------------------------------
namespace {

string inputFile, odir("./"), prefix;

}

//-----------------------------------------------------------------------------------------
// static data
#include <f8cstatic.hpp>

//-----------------------------------------------------------------------------------------
void print_usage();
int process(XmlEntity& xf, Ctxt& ctxt);
int loadfields(XmlEntity& xf, FieldSpecMap& fspec);
ostream *openofile(const string& odir, const string& fname);

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
int main(int argc, char **argv)
{
	int val;

//#ifdef HAVE_GETOPT_LONG
	option long_options[] =
	{
		{ "help",			0,	0,	'h' },
		{ "version",		0,	0,	'v' },
		{ "odir",			0,	0,	'o' },
		{ "prefix",			0,	0,	'p' },
		{ 0 },
	};

	while ((val = getopt_long (argc, argv, GETARGLIST.c_str(), long_options, 0)) != -1)
//#else
//	while ((val = getopt (argc, argv, GETARGLIST.c_str())) != -1)
//#endif
	{
      switch (val)
		{
		case 'v':
			//cout << "f8c for "PACKAGE" version "VERSION << endl;
			cout << rcsid << endl;
			return 0;
		case 'h': print_usage(); return 0;
		case ':': case '?': return 1;
		case 'o': odir = optarg; break;
		case 'p': prefix = optarg; break;
		default: break;
		}
	}

	if (optind < argc)
		inputFile = argv[optind];
	else
	{
		cerr << "no input xml file specified" << endl;
		print_usage();
		return 1;
	}

	if (prefix.empty())
		prefix = "myfix";

	cout << "f8c " << _csMap.Find_Value_Ref(cs_copyright_short) << endl;

	scoped_ptr<XmlEntity> cfr(XmlEntity::Factory(inputFile));
	if (!cfr.get())
	{
		cerr << "Error reading file \'" << inputFile << '\'';
		if	(errno)
			cerr << " (" << strerror(errno) << ')';
		cerr << endl;
		return 1;
	}

	if (cfr->GetErrorCnt())
	{
		cerr << cfr->GetErrorCnt() << " error"
			<< (cfr->GetErrorCnt() == 1 ? " " : "s ") << "found in \'" << inputFile << '\'' << endl;
		return 1;
	}

	//cout << *cfr;

	Ctxt ctxt;
	ctxt._out[Ctxt::types_cpp].first = prefix + "_types.cpp";
	ctxt._out[Ctxt::types_hpp].first = prefix + "_types.hpp";
	ctxt._out[Ctxt::classes_cpp].first = prefix + "_classes.cpp";
	ctxt._out[Ctxt::classes_hpp].first = prefix + "_classes.hpp";
	for (Ctxt::Output *itr(ctxt._out); itr < ctxt._out + Ctxt::count; ++itr)
		if (!itr->second.reset(openofile(odir, itr->first)))
			return 1;

	return process(*cfr, ctxt);
}

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
int loadfields(XmlEntity& xf, FieldSpecMap& fspec)
{
	int fieldsLoaded(0);

	XmlList flist;
	if (!xf.find("fix/fields/field", flist))
	{
		cerr << "No fields found in " << inputFile << endl;
		return 0;
	}

	for(XmlList::const_iterator itr(flist.begin()); itr != flist.end(); ++itr)
	{
		string number, name, type;
		if ((*itr)->GetAttr("number", number) && (*itr)->GetAttr("name", name) && (*itr)->GetAttr("type", type))
		{
			InPlaceStrToUpper(type);
			FieldTrait::FieldType ft(FieldSpec::_baseTypeMap.Find_Value(type));
			pair<FieldSpecMap::iterator, bool> result;
			if (ft != FieldTrait::ft_untyped)
				result = fspec.insert(FieldSpecMap::value_type(GetValue<unsigned>(number), FieldSpec(name, ft)));
			else
			{
				cerr << "Unknown field type: " << type << " in " << name << " at "
					<< inputFile << '(' << (*itr)->GetLine() << ')' << endl;
				continue;
			}

			(*itr)->GetAttr("description", result.first->second._description);
			(*itr)->GetAttr("domain", result.first->second._domain);

			++fieldsLoaded;

			XmlList domlist;
			if ((*itr)->find("field/value", domlist))
			{
				for(XmlList::const_iterator ditr(domlist.begin()); ditr != domlist.end(); ++ditr)
				{
					string enum_str, description;
					if ((*ditr)->GetAttr("enum", enum_str) && (*ditr)->GetAttr("description", description))
					{
						if (!result.first->second._dvals)
							result.first->second._dvals = new DomainMap;
						result.first->second._dvals->insert(DomainMap::value_type(enum_str, description));
					}
					else
						cerr << "Value element missing required attributes at "
							<< inputFile << '(' << (*itr)->GetLine() << ')' << endl;
				}
			}
		}
		else
			cerr << "Field element missing required attributes at "
				<< inputFile << '(' << (*itr)->GetLine() << ')' << endl;
	}

	return fieldsLoaded;
}

//-----------------------------------------------------------------------------------------
int process(XmlEntity& xf, Ctxt& ctxt)
{
	ostream& ost_cpp(*ctxt._out[Ctxt::types_cpp].second.get());
	ostream& ost_hpp(*ctxt._out[Ctxt::types_hpp].second.get());
	ostream& osc_cpp(*ctxt._out[Ctxt::classes_cpp].second.get());
	ostream& osc_hpp(*ctxt._out[Ctxt::classes_hpp].second.get());
	int result(0);

	// parse fields
	FieldSpecMap fspec;
	if (!loadfields(xf, fspec))
		return result;

	// output file preambles
	ost_hpp << _csMap.Find_Value_Ref(cs_do_not_edit) << endl;
	ost_hpp << _csMap.Find_Value_Ref(cs_divider) << endl;
	ost_hpp << _csMap.Find_Value_Ref(cs_copyright) << endl;
	ost_hpp << _csMap.Find_Value_Ref(cs_divider) << endl;
	ost_hpp << "#ifndef _" << ctxt._out[Ctxt::types_hpp].first << '_' << endl;
	ost_hpp << "#define _" << ctxt._out[Ctxt::types_hpp].first << '_' << endl << endl;
	ost_hpp << _csMap.Find_Value_Ref(cs_start_namespace) << endl;

	ost_hpp << endl << _csMap.Find_Value_Ref(cs_divider) << endl;
	ost_cpp << _csMap.Find_Value_Ref(cs_do_not_edit) << endl;
	ost_cpp << _csMap.Find_Value_Ref(cs_divider) << endl;
	ost_cpp << _csMap.Find_Value_Ref(cs_copyright) << endl;
	ost_cpp << _csMap.Find_Value_Ref(cs_divider) << endl;
	ost_cpp << _csMap.Find_Value_Ref(cs_start_namespace) << endl << endl;

	// generate field types
	for (FieldSpecMap::const_iterator fitr(fspec.begin()); fitr != fspec.end(); ++fitr)
	{
		ost_hpp << "typedef Field<" << FieldSpec::_typeToCPP.Find_Value_Ref(fitr->second._ftype)
			<< ", " << fitr->first << "> " << fitr->second._name << ';' << endl;
	}

	// generate field instantiators
	ost_hpp << endl << _csMap.Find_Value_Ref(cs_divider) << endl;
	ost_cpp << endl << _csMap.Find_Value_Ref(cs_divider) << endl;
	const string spacer(3, ' ');
	for (FieldSpecMap::const_iterator fitr(fspec.begin()); fitr != fspec.end(); ++fitr)
	{
		ost_hpp << "BaseField *Create_" << fitr->second._name << "(const std::string& from);" << endl;
		ost_cpp << "BaseField *Create_" << fitr->second._name << "(const std::string& from) { return new "
			<< fitr->second._name << "(from); }" << endl;
	}

	// generate field instantiator lookup
	ost_hpp << endl << _csMap.Find_Value_Ref(cs_divider) << endl;
	ost_hpp << _csMap.Find_Value_Ref(cs_fcreate_entry_hpp) << endl;

	ost_cpp << endl << _csMap.Find_Value_Ref(cs_divider) << endl;
	ost_cpp << _csMap.Find_Value_Ref(cs_fcreate_entry_table) << endl;
	for (FieldSpecMap::const_iterator fitr(fspec.begin()); fitr != fspec.end(); ++fitr)
	{
		if (fitr != fspec.begin())
			ost_cpp << ',' << endl;
		ost_cpp << spacer << "{ " << fitr->first << ", " << "&Create_" << fitr->second._name << " }";
	}
	ost_cpp << endl << "};" << endl;
	ost_cpp << _csMap.Find_Value_Ref(cs_fcreate_entry_cpp) << endl;

	ost_hpp << endl << _csMap.Find_Value_Ref(cs_divider) << endl;
	ost_hpp << _csMap.Find_Value_Ref(cs_generated_table_def) << endl;

	// terminate files
	ost_hpp << endl << _csMap.Find_Value_Ref(cs_end_namespace) << endl;
	ost_hpp << "#endif // _" << ctxt._out[Ctxt::types_hpp].first << '_' << endl;
	ost_cpp << endl << _csMap.Find_Value_Ref(cs_end_namespace) << endl;
	return result;
}

//-----------------------------------------------------------------------------------------
void print_usage()
{
	UsageMan um("f8c", GETARGLIST, "<input xml schema>");
	um.setdesc("f8c -- compile xml schema into fix8 application classes");
	um.add('o', "odir <file>", "output target directory");
	um.add('p', "prefix <file>", "output filename prefix");
	um.add('h', "help", "help, this screen");
	um.add('v', "version", "print version, exit");
	um.add("e.g.");
	um.add("@f8c -p myfix myfix.xml");
	um.print(cerr);
}

//-------------------------------------------------------------------------------------------------

