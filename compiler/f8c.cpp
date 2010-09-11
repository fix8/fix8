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

#include <field.hpp>
#include <traits.hpp>
#include <message.hpp>
#include <usage.hpp>
#include <f8utils.hpp>
#include <xmldecode.hpp>
#include <f8c.hpp>

//#include <config.h>
//#ifdef HAVE_GETOPT_H
#include <getopt.h>
//#endif

//-----------------------------------------------------------------------------------------
using namespace std;
using namespace FIX;

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
int process(XmlEntity& xf, ostream& os_cpp, ostream& os_hpp);
int loadfields(XmlEntity& xf, FieldSpecMap& fspec);
ostream *openofile(const string& odir, const string& prefix, const string& ext);

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

	scoped_ptr<ostream> os_cpp(openofile(odir, prefix, "cpp")), os_hpp(openofile(odir, prefix, "hpp"));
	if (!os_cpp.get() || !os_hpp.get())
		return 1;

	process(*cfr, *os_cpp.get(), *os_hpp.get());
	return 0;
}

//-----------------------------------------------------------------------------------------
ostream *openofile(const string& odir, const string& prefix, const string& ext)
{
	ostringstream ofs;
	string odirect(odir);
	ofs << CheckAddTrailingSlash(odirect) << prefix << '.' << ext;
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
			FieldTrait::FieldSubType fst(FieldSpec::_subTypeMap.Find_Value(InPlaceStrToUpper(type)));
			FieldTrait::FieldType ft(FieldSpec::_baseTypeMap.Find_Value(type));
			pair<FieldSpecMap::iterator, bool> result;
			if (fst != FieldTrait::fst_untyped)
				result = fspec.insert(FieldSpecMap::value_type(GetValue<unsigned>(number), FieldSpec(name, fst)));
			else if (ft != FieldTrait::ft_untyped)
				result = fspec.insert(FieldSpecMap::value_type(GetValue<unsigned>(number), FieldSpec(name, ft)));
			else
			{
				cerr << "Unknown field type: " << type << " in " << name << " at "
					<< inputFile << '(' << (*itr)->GetLine() << ')' << endl;
				continue;
			}

			string domain;
			if ((*itr)->GetAttr("domain", domain))
			{
			}

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
int process(XmlEntity& xf, ostream& os_cpp, ostream& os_hpp)
{
	// generate field types
	FieldSpecMap fspec;
	int fieldsLoaded(loadfields(xf, fspec));
	cout << "loaded " << fieldsLoaded << " fields" << endl;
	if (!fieldsLoaded)
		return 0;

	os_hpp << "// *** f8c generated file: Do Not Edit" << endl;
	os_hpp << "#ifndef _" << prefix << "_HPP_" << endl;
	os_hpp << "#define _" << prefix << "_HPP_" << endl << endl;
	os_hpp << "namespace FIX {" << endl << endl;

	for (FieldSpecMap::const_iterator fitr(fspec.begin()); fitr != fspec.end(); ++fitr)
	{
		os_hpp << "typedef Field<" <<
			(fitr->second._ftype != FieldTrait::ft_untyped ? FieldSpec::_typeToCPP.Find_Value_Ref(fitr->second._ftype)
																		  : FieldSpec::_subtypeToCPP.Find_Value_Ref(fitr->second._fstype))
			<< ", " << fitr->first << "> " << fitr->second._name << ';' << endl;
	}

	int result(0);
	os_hpp << endl << "} // namespace FIX" << endl;
	os_hpp << "#endif //_" << prefix << "_HPP_" << endl;
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

