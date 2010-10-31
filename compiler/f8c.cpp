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
//#ifdef HAVE_GETOPT_H
#include <getopt.h>
//#endif

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

//#include <config.h>

//-----------------------------------------------------------------------------------------
using namespace std;
using namespace FIX8;

//-----------------------------------------------------------------------------------------
static const std::string rcsid("$Id$");

//-----------------------------------------------------------------------------------------
const string Ctxt::_exts[count] = { "_types.cpp", "_types.hpp", "_traits.cpp", "_classes.cpp",
	"_classes.hpp", "_router.hpp" };
template<>
const size_t GeneratedTable<unsigned int, BaseEntry>::_pairsz(0);
template<>
const GeneratedTable<unsigned int, BaseEntry>::Pair GeneratedTable<unsigned int, BaseEntry>::_pairs[] = {};
template<>
const size_t GeneratedTable<const f8String, BaseMsgEntry>::_pairsz(0);
template<>
const GeneratedTable<const f8String, BaseMsgEntry>::Pair GeneratedTable<const f8String, BaseMsgEntry>::_pairs[] = {};

//-----------------------------------------------------------------------------------------
string inputFile, odir("./"), prefix;
bool verbose(false);
extern const string GETARGLIST("hvVo:p:");
extern const string spacer(3, ' ');

//-----------------------------------------------------------------------------------------
const CSMap _csMap;

//-----------------------------------------------------------------------------------------
// static data
#include <f8cstatic.hpp>

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
ostream *openofile(const string& odir, const string& fname);
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
int main(int argc, char **argv)
{
	int val;

//#ifdef HAVE_GETOPT_LONG
	option long_options[] =
	{
		{ "help",			0,	0,	'h' },
		{ "version",		0,	0,	'v' },
		{ "verbose",		0,	0,	'V' },
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
			cout << "f8c " << _csMap.find_value_ref(cs_copyright_short) << endl;
			cout << rcsid << endl;
			return 0;
		case 'V': verbose = true; break;
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
		prefix = "Myfix";

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
	if (loadFixVersion (*cfr, ctxt) < 0)
		return 1;
	for (unsigned ii(0); ii < Ctxt::count; ++ii)
	{
		ctxt._out[ii].first = prefix + ctxt._exts[ii];
		if (!ctxt._out[ii].second.reset(openofile(odir, ctxt._out[ii].first)))
			return 1;
	}

	cout << "Compiling fix version " << ctxt._version <<  " (" << ctxt._fixns << ")." << endl;
	return process(*cfr, ctxt);
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
			FieldTrait::FieldType ft(FieldSpec::_baseTypeMap.find_value(type));
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
			(*itr)->GetAttr("comment", result.first->second._comment);

			++fieldsLoaded;

			XmlList realmlist;
			if ((*itr)->find("field/value", realmlist))
			{
				for(XmlList::const_iterator ditr(realmlist.begin()); ditr != realmlist.end(); ++ditr)
				{
					string enum_str, description;
					if ((*ditr)->GetAttr("enum", enum_str) && (*ditr)->GetAttr("description", description))
					{
						if (!result.first->second._dvals)
							result.first->second._dvals = new RealmMap;
						string lower, upper;
						bool isRange((*ditr)->GetAttr("range", lower) && (lower == "lower" || upper == "upper"));
						RealmObject *realmval(RealmObject::create(enum_str, ft, isRange));
						if (isRange)
							result.first->second._dtype = RealmBase::dt_range;
						if (realmval)
							result.first->second._dvals->insert(RealmMap::value_type(realmval, description));
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
int loadcomponents(XmlEntity& xf, ComponentSpecMap& mspec, const FieldToNumMap& ftonSpec, const FieldSpecMap& fspec)
{
	int msgssLoaded(0);

	XmlList mlist;
	if (!xf.find("fix/components/component", mlist))
	{
		cerr << "No components found in " << inputFile << endl;
		return 0;
	}

	unsigned cnum(0);
	for(XmlList::const_iterator itr(mlist.begin()); itr != mlist.end(); ++itr, ++cnum)
	{
		string name;
		if ((*itr)->GetAttr("name", name))
		{
			pair<MessageSpecMap::iterator, bool> result(
				mspec.insert(ComponentSpecMap::value_type(name, ComponentSpec(name))));
			if (!result.second)
			{
				cerr << "Could not add component " << name << " at "
					<< inputFile << '(' << (*itr)->GetLine() << ')' << endl;
				continue;
			}

			XmlList grplist;
			if ((*itr)->find("component/group", grplist))
			{
				for(XmlList::const_iterator gitr(grplist.begin()); gitr != grplist.end(); ++gitr)
				{
					string gname, required;
					if ((*gitr)->GetAttr("name", gname) && (*gitr)->GetAttr("required", required))
					{
						// add group FieldTrait
						FieldToNumMap::const_iterator ftonItr(ftonSpec.find(gname));
						FieldSpecMap::const_iterator fs_itr;
						if (ftonItr != ftonSpec.end() && (fs_itr = fspec.find(ftonItr->second)) != fspec.end())
						{
							if (!result.first->second._fields.add(FieldTrait(fs_itr->first, FieldTrait::ft_int, (*gitr)->GetSubIdx(),
								required == "Y", true, cnum)))
									cerr << "Could not add group trait object " << gname << endl;
							else
							{
								pair<GroupMap::iterator, bool> gresult(
									result.first->second._groups.insert(GroupMap::value_type(fs_itr->first, FieldTraits())));
								if (!processMessageFields("group/field", *gitr, gresult.first->second, ftonSpec, fspec, 1))
									cerr << "No fields found in component group " << gname << " at "
										<< inputFile << '(' << (*gitr)->GetLine() << ')' << endl;
								else
									gresult.first->second.set_group();
							}
						}
						else
						{
							cerr << "Could not locate group Field " << gname << " from known field types in " << inputFile << endl;
							continue;
						}
					}
					else
						cerr << "Group element missing required attributes at "
							<< inputFile << '(' << (*gitr)->GetLine() << ')' << endl;
				}
			}
			(*itr)->GetAttr("comment", result.first->second._comment);

			processMessageFields("component/field", *itr, result.first->second._fields, ftonSpec, fspec, 1);

			++msgssLoaded;
		}
		else
			cerr << "Component element missing required attributes at "
				<< inputFile << '(' << (*itr)->GetLine() << ')' << endl;
	}

	return msgssLoaded;
}

//-----------------------------------------------------------------------------------------
int loadmessages(XmlEntity& xf, MessageSpecMap& mspec, const ComponentSpecMap& cspec,
	const FieldToNumMap& ftonSpec, const FieldSpecMap& fspec)
{
	int msgssLoaded(0);

	XmlList mlist;
	if (!xf.find("fix/messages/message", mlist))
	{
		cerr << "No messages found in " << inputFile << endl;
		return 0;
	}

	if (!xf.find("fix/header", mlist))
	{
		cerr << "No header element found in " << inputFile << endl;
		return 0;
	}

	if (!xf.find("fix/trailer", mlist))
	{
		cerr << "No trailer element found in " << inputFile << endl;
		return 0;
	}

	// lookup msgtype realm - all messages must have corresponding entry here
	FieldSpecMap::const_iterator fsitr(fspec.find(35));	// always 35
	if (fsitr == fspec.end())
	{
		cerr << "Could not locate MsgType realm defintions in " << inputFile << endl;
		return 0;
	}

	for(XmlList::const_iterator itr(mlist.begin()); itr != mlist.end(); ++itr)
	{
		string msgcat, name, msgtype, elname;
		if ((*itr)->GetTag() == "header")
			msgtype = name = elname = "header";
		else if ((*itr)->GetTag() == "trailer")
			msgtype = name = elname = "trailer";
		else if ((*itr)->GetAttr("msgtype", msgtype) && (*itr)->GetAttr("name", name) && (*itr)->GetAttr("msgcat", msgcat))
		{
			StringRealm srealm(msgtype, false);
			RealmMap::const_iterator ditr(fsitr->second._dvals->find(&srealm));
			if (ditr == fsitr->second._dvals->end())
			{
				cerr << "Message " << name << " does not have corrresponding entry in MsgType field"
					" realm at " << inputFile << '(' << (*itr)->GetLine() << ')' << endl;
				continue;
			}

			elname = "message";
		}
		else
		{
			cerr << "Message element missing required attributes at "
				<< inputFile << '(' << (*itr)->GetLine() << ')' << endl;
			continue;
		}

		pair<MessageSpecMap::iterator, bool> result(
			mspec.insert(MessageSpecMap::value_type(msgtype, MessageSpec(name))));
		if (!result.second)
		{
			cerr << "Could not add message " << name << " (" << msgtype << ") at "
				<< inputFile << '(' << (*itr)->GetLine() << ')' << endl;
			continue;
		}

		string elpart;
		XmlList grplist;
		if ((*itr)->find(mkel(elname, "group", elpart), grplist))
		{
			for(XmlList::const_iterator gitr(grplist.begin()); gitr != grplist.end(); ++gitr)
			{
				string gname, required;
				if ((*gitr)->GetAttr("name", gname) && (*gitr)->GetAttr("required", required))
				{
					// add group FieldTrait
					FieldToNumMap::const_iterator ftonItr(ftonSpec.find(gname));
					FieldSpecMap::const_iterator fs_itr;
					if (ftonItr != ftonSpec.end() && (fs_itr = fspec.find(ftonItr->second)) != fspec.end())
					{
						if (!result.first->second._fields.add(FieldTrait(fs_itr->first, FieldTrait::ft_int, (*gitr)->GetSubIdx(),
							required == "Y", true, 0)))
								cerr << "Could not add group trait object " << gname << endl;
						else
						{
							pair<GroupMap::iterator, bool> gresult(
								result.first->second._groups.insert(GroupMap::value_type(fs_itr->first, FieldTraits())));
							processMessageFields("group/field", *gitr, gresult.first->second, ftonSpec, fspec, 0);
							gresult.first->second.set_group();
							XmlList comlist;
							if ((*gitr)->find("group/component", comlist))
							{
								for(XmlList::const_iterator citr(comlist.begin()); citr != comlist.end(); ++citr)
								{
									string cname, required;
									if (!(*citr)->GetAttr("name", cname) || !(*citr)->GetAttr("required", required))
										continue;
									ComponentSpecMap::const_iterator csitr(cspec.find(cname));
									if (csitr == cspec.end())
									{
										cerr << "Component not found " << name << " at "
											<< inputFile << '(' << (*citr)->GetLine() << ')' << endl;
										continue;
									}

									for (Presence::iterator deitr(csitr->second._fields.get_presence().begin());
										deitr != csitr->second._fields.get_presence().end(); ++deitr)
									{
										deitr->_field_traits.set(FieldTrait::mandatory,
											(deitr->_field_traits.has(FieldTrait::mandatory) && required == "Y")
											|| deitr->_pos == 1);
										if (!gresult.first->second.add(*deitr))
											cerr << "group Component failed to add " << name << " at "
												<< inputFile << '(' << (*citr)->GetLine() << ')' << endl;
									}
								}
							}
						}
					}
					else
					{
						cerr << "Could not locate group Field " << gname << " from known field types in " << inputFile << endl;
						continue;
					}
				}
				else
					cerr << "Group element missing required attributes at "
						<< inputFile << '(' << (*gitr)->GetLine() << ')' << endl;
			}
		}

		(*itr)->GetAttr("comment", result.first->second._comment);

		processMessageFields(mkel(elname, "field", elpart), *itr, result.first->second._fields, ftonSpec, fspec, 0);

		XmlList comlist;
		if ((*itr)->find(mkel(elname, "component", elpart), comlist))
		{
			for(XmlList::const_iterator citr(comlist.begin()); citr != comlist.end(); ++citr)
			{
				string cname, required;
				if (!(*citr)->GetAttr("name", cname) || !(*citr)->GetAttr("required", required))
					continue;
				ComponentSpecMap::const_iterator csitr(cspec.find(cname));
				if (csitr == cspec.end())
				{
					cerr << "Component not found " << name << " at "
						<< inputFile << '(' << (*citr)->GetLine() << ')' << endl;
					continue;
				}

				for (Presence::iterator deitr(csitr->second._fields.get_presence().begin());
					deitr != csitr->second._fields.get_presence().end(); ++deitr)
				{
					deitr->_field_traits.set(FieldTrait::mandatory,
						deitr->_field_traits.has(FieldTrait::mandatory) && required == "Y");
					result.first->second._fields.add(*deitr);
				}
			}
		}

		++msgssLoaded;
	}

	return msgssLoaded;
}

//-----------------------------------------------------------------------------------------
int process(XmlEntity& xf, Ctxt& ctxt)
{
	ostream& ost_cpp(*ctxt._out[Ctxt::types_cpp].second.get());
	ostream& ost_hpp(*ctxt._out[Ctxt::types_hpp].second.get());
	ostream& osr_cpp(*ctxt._out[Ctxt::traits_cpp].second.get());
	ostream& osc_hpp(*ctxt._out[Ctxt::classes_hpp].second.get());
	ostream& osc_cpp(*ctxt._out[Ctxt::classes_cpp].second.get());
	ostream& osu_hpp(*ctxt._out[Ctxt::router_hpp].second.get());
	int result(0);

// ================================= Field processing =====================================
	FieldSpecMap fspec;
	int fields(loadfields(xf, fspec));
	if (!fields)
		return 0;
	if (verbose)
		cout << fields << " fields processed" << endl;

	// output file preambles
	ost_hpp << _csMap.find_value_ref(cs_do_not_edit) << endl;
	ost_hpp << _csMap.find_value_ref(cs_divider) << endl;
	ost_hpp << _csMap.find_value_ref(cs_copyright) << endl;
	ost_hpp << _csMap.find_value_ref(cs_divider) << endl;
	ost_hpp << "#ifndef _" << flname(ctxt._out[Ctxt::types_hpp].first) << '_' << endl;
	ost_hpp << "#define _" << flname(ctxt._out[Ctxt::types_hpp].first) << '_' << endl << endl;
	ost_hpp << _csMap.find_value_ref(cs_start_namespace) << endl;
	ost_hpp << "namespace " << ctxt._fixns << " {" << endl;

	ost_hpp << endl << _csMap.find_value_ref(cs_divider) << endl;
	ost_cpp << _csMap.find_value_ref(cs_do_not_edit) << endl;
	ost_cpp << _csMap.find_value_ref(cs_divider) << endl;
	ost_cpp << _csMap.find_value_ref(cs_copyright) << endl;
	ost_cpp << _csMap.find_value_ref(cs_divider) << endl;
	ost_cpp << _csMap.find_value_ref(cs_generated_includes) << endl;
	ost_cpp << "#include \"" << ctxt._out[Ctxt::types_hpp].first << '"' << endl;
	ost_cpp << _csMap.find_value_ref(cs_divider) << endl;
	ost_cpp << _csMap.find_value_ref(cs_start_namespace) << endl;
	ost_cpp << "namespace " << ctxt._fixns << " {" << endl << endl;

	ost_cpp << _csMap.find_value_ref(cs_start_anon_namespace) << endl;
	ost_cpp << endl << _csMap.find_value_ref(cs_divider) << endl;
	// generate field types
	for (FieldSpecMap::const_iterator fitr(fspec.begin()); fitr != fspec.end(); ++fitr)
	{
		if (!fitr->second._comment.empty())
			ost_hpp << "// " << fitr->second._comment << endl;
		ost_hpp << "typedef Field<" << FieldSpec::_typeToCPP.find_value_ref(fitr->second._ftype)
			<< ", " << fitr->first << "> " << fitr->second._name << ';' << endl;
		if (fitr->second._dvals)
			processValueEnums(fitr, ost_hpp, ost_cpp);
		ost_hpp << _csMap.find_value_ref(cs_divider) << endl;
	}

	// generate realmbase objs
	ost_cpp << endl << _csMap.find_value_ref(cs_divider) << endl;
	ost_cpp << "const RealmBase realmbases[] =" << endl << '{' << endl;
	unsigned dcnt(0);
	for (FieldSpecMap::iterator fitr(fspec.begin()); fitr != fspec.end(); ++fitr)
	{
		if (!fitr->second._dvals)
			continue;
		ost_cpp << spacer << "{ reinterpret_cast<const void *>(" << fitr->second._name << "_realm), "
			<< "static_cast<RealmBase::RealmType>(" << fitr->second._dtype << "), " << endl << spacer << spacer
			<< "static_cast<FieldTrait::FieldType>(" << fitr->second._ftype << "), "
			<< fitr->second._dvals->size() << ", " << fitr->second._name << "_descriptions }," << endl;
		fitr->second._doffset = dcnt++;
	}
	ost_cpp << "};" << endl;

	// generate field instantiators
	ost_cpp << endl << _csMap.find_value_ref(cs_divider) << endl;
	for (FieldSpecMap::const_iterator fitr(fspec.begin()); fitr != fspec.end(); ++fitr)
	{
		ost_cpp << "BaseField *Create_" << fitr->second._name << "(const f8String& from, const RealmBase *db)";
		ost_cpp << " { return new " << fitr->second._name << "(from, db); }" << endl;
	}

	ost_cpp << endl << _csMap.find_value_ref(cs_end_anon_namespace) << endl;
	ost_cpp << "} // namespace " << ctxt._fixns << endl;

	// generate field instantiator lookup
	ost_hpp << "typedef GeneratedTable<unsigned, BaseEntry> " << ctxt._clname << "_BaseEntry;" << endl;

	ost_cpp << endl << _csMap.find_value_ref(cs_divider) << endl;
	ost_cpp << "template<>" << endl << "const " << ctxt._fixns << "::" << ctxt._clname << "_BaseEntry::Pair "
		<< ctxt._fixns << "::" << ctxt._clname << "_BaseEntry::_pairs[] =" << endl << '{' << endl;
	for (FieldSpecMap::const_iterator fitr(fspec.begin()); fitr != fspec.end(); ++fitr)
	{
		if (fitr != fspec.begin())
			ost_cpp << ',' << endl;
		ost_cpp << spacer << "{ " << fitr->first << ", { &" << ctxt._fixns << "::Create_" << fitr->second._name << ", ";
		if (fitr->second._dvals)
			ost_cpp << "&" << ctxt._fixns << "::realmbases[" << fitr->second._doffset << ']';
		else
			ost_cpp << '0';
		ost_cpp << ", \"" << fitr->second._name << '\"';
		if (!fitr->second._comment.empty())
			ost_cpp << ',' << endl << spacer << spacer << '"' << fitr->second._comment << '"';
		else
			ost_cpp << ", 0";
		ost_cpp << " } }";
	}
	ost_cpp << endl << "};" << endl;
	ost_cpp << "template<>" << endl << "const size_t " << ctxt._fixns << "::" << ctxt._clname << "_BaseEntry::_pairsz(sizeof(_pairs)/sizeof("
		<< ctxt._fixns << "::" << ctxt._clname << "_BaseEntry::Pair));" << endl;
	ost_cpp << "template<>" << endl << "const " << ctxt._fixns << "::" << ctxt._clname << "_BaseEntry::NoValType "
		<< ctxt._fixns << "::" << ctxt._clname << "_BaseEntry::_noval = {0, 0};" << endl;

	// terminate files
	ost_hpp << endl << "} // namespace " << ctxt._fixns << endl;
	ost_hpp << _csMap.find_value_ref(cs_end_namespace) << endl;
	ost_hpp << "#endif // _" << flname(ctxt._out[Ctxt::types_hpp].first) << '_' << endl;
	ost_cpp << endl << _csMap.find_value_ref(cs_end_namespace) << endl;

// ================================= Message processing ===================================

	FieldToNumMap ftonSpec;
	for (FieldSpecMap::const_iterator fs_itr(fspec.begin()); fs_itr != fspec.end(); ++fs_itr)
		ftonSpec.insert(FieldToNumMap::value_type(fs_itr->second._name, fs_itr->first));

	ComponentSpecMap cspec;
	int components(loadcomponents(xf, cspec, ftonSpec, fspec));
	if (verbose)
		cout << components << " components processed" << endl;

	MessageSpecMap mspec;

	int msgsloaded(loadmessages(xf, mspec, cspec, ftonSpec, fspec));
	if (!msgsloaded)
		return result;
	if (verbose)
		cout << msgsloaded << " messages processed" << endl;

	processOrdering(mspec);

	// output file preambles
	osu_hpp << _csMap.find_value_ref(cs_do_not_edit) << endl;
	osu_hpp << _csMap.find_value_ref(cs_divider) << endl;
	osu_hpp << _csMap.find_value_ref(cs_copyright) << endl;
	osu_hpp << _csMap.find_value_ref(cs_divider) << endl;
	osu_hpp << "#ifndef _" << flname(ctxt._out[Ctxt::router_hpp].first) << '_' << endl;
	osu_hpp << "#define _" << flname(ctxt._out[Ctxt::router_hpp].first) << '_' << endl << endl;
	osu_hpp << _csMap.find_value_ref(cs_start_namespace) << endl;
	osu_hpp << "namespace " << ctxt._fixns << " {" << endl;
	osu_hpp << endl << _csMap.find_value_ref(cs_divider) << endl;

	osc_hpp << _csMap.find_value_ref(cs_do_not_edit) << endl;
	osc_hpp << _csMap.find_value_ref(cs_divider) << endl;
	osc_hpp << _csMap.find_value_ref(cs_copyright) << endl;
	osc_hpp << _csMap.find_value_ref(cs_divider) << endl;
	osc_hpp << "#ifndef _" << flname(ctxt._out[Ctxt::classes_hpp].first) << '_' << endl;
	osc_hpp << "#define _" << flname(ctxt._out[Ctxt::classes_hpp].first) << '_' << endl << endl;
	osc_hpp << _csMap.find_value_ref(cs_start_namespace) << endl;
	osc_hpp << "namespace " << ctxt._fixns << " {" << endl;

	osc_hpp << endl << _csMap.find_value_ref(cs_divider) << endl;
	osc_hpp << "typedef GeneratedTable<const f8String, BaseMsgEntry> " << ctxt._clname << "_BaseMsgEntry;" << endl;
	osc_hpp << "extern F8MetaCntx ctx;" << endl;
	osc_hpp << "class " << ctxt._clname << "_Router;" << endl;
	osc_hpp << endl << _csMap.find_value_ref(cs_divider) << endl;

	osc_cpp << _csMap.find_value_ref(cs_do_not_edit) << endl;
	osc_cpp << _csMap.find_value_ref(cs_divider) << endl;
	osc_cpp << _csMap.find_value_ref(cs_copyright) << endl;
	osc_cpp << _csMap.find_value_ref(cs_divider) << endl;
	osc_cpp << _csMap.find_value_ref(cs_generated_includes) << endl;
	osc_cpp << "#include \"" << ctxt._out[Ctxt::types_hpp].first << '"' << endl;
	osc_cpp << "#include \"" << ctxt._out[Ctxt::router_hpp].first << '"' << endl;
	osc_cpp << "#include \"" << ctxt._out[Ctxt::classes_hpp].first << '"' << endl;
	osc_cpp << _csMap.find_value_ref(cs_divider) << endl;
	osc_cpp << _csMap.find_value_ref(cs_start_namespace) << endl;
	osc_cpp << "namespace " << ctxt._fixns << " {" << endl << endl;
	osc_cpp << _csMap.find_value_ref(cs_start_anon_namespace) << endl << endl;
	osc_cpp << _csMap.find_value_ref(cs_divider) << endl;

	osr_cpp << _csMap.find_value_ref(cs_do_not_edit) << endl;
	osr_cpp << _csMap.find_value_ref(cs_divider) << endl;
	osr_cpp << _csMap.find_value_ref(cs_copyright) << endl;
	osr_cpp << _csMap.find_value_ref(cs_divider) << endl;
	osr_cpp << _csMap.find_value_ref(cs_generated_includes) << endl;
	osr_cpp << "#include \"" << ctxt._out[Ctxt::types_hpp].first << '"' << endl;
	osr_cpp << "#include \"" << ctxt._out[Ctxt::router_hpp].first << '"' << endl;
	osr_cpp << "#include \"" << ctxt._out[Ctxt::classes_hpp].first << '"' << endl;
	osr_cpp << _csMap.find_value_ref(cs_divider) << endl;
	osr_cpp << _csMap.find_value_ref(cs_start_namespace) << endl;
	osr_cpp << "namespace " << ctxt._fixns << " {" << endl << endl;

	FieldSpecMap::const_iterator fsitr(fspec.find(35));	// always 35
	for (MessageSpecMap::const_iterator mitr(mspec.begin()); mitr != mspec.end(); ++mitr)
	{
		bool isTrailer(mitr->second._name == "trailer");
		bool isHeader(mitr->second._name == "header");
		if (!mitr->second._comment.empty())
			osc_hpp << "// " << mitr->second._comment << endl;
		osc_hpp << "class " << mitr->second._name << " : public "
			<< (isTrailer || isHeader ? "MessageBase" : "Message") << endl << '{' << endl;

		if (mitr->second._fields.get_presence().size())
		{
			osr_cpp << _csMap.find_value_ref(cs_divider) << endl;
			osr_cpp << "const FieldTrait::TraitBase " << mitr->second._name << "::_traits[] ="
				<< endl << '{' << endl;
			for (Presence::const_iterator flitr(mitr->second._fields.get_presence().begin());
				flitr != mitr->second._fields.get_presence().end(); ++flitr)
			{
				if (flitr != mitr->second._fields.get_presence().begin())
				{
					osr_cpp << ',';
					if (distance(mitr->second._fields.get_presence().begin(), flitr) % 3 == 0)
						osr_cpp << endl;
				}

				ostringstream tostr;
				tostr << "0x" << setw(3) << setfill('0') << hex << flitr->_field_traits.get();
				osr_cpp << spacer << "{ " << setw(4) << right << flitr->_fnum << ", "
					<< setw(3) << right << flitr->_ftype << ", " << setw(3) << right
					<< flitr->_pos << ", " << tostr.str() << " }";
			}
			osr_cpp << endl << "};" << endl;
			osr_cpp << "const size_t " << mitr->second._name << "::_fldcnt("
				<< mitr->second._fields.get_presence().size() << ");" << endl;
			osr_cpp << "const f8String " << mitr->second._name << "::_msgtype(\""
				<< mitr->first << "\");" << endl;
			osc_hpp << spacer << "static const FieldTrait::TraitBase _traits[];" << endl;
			osc_hpp << spacer << "static const size_t _fldcnt;" << endl;
			osc_hpp << spacer << "static const f8String _msgtype;" << endl << endl;
		}

		osc_hpp << "public:" << endl;
		osc_hpp << spacer << mitr->second._name << "()";
		if (mitr->second._fields.get_presence().size())
			osc_hpp << " : " << (isTrailer || isHeader ? "MessageBase" : "Message")
				<< "(ctx, _msgtype, _traits, _traits + _fldcnt)";
		if (isHeader || isTrailer)
			osc_hpp << " { add_preamble(); }" << endl;
		else if (!mitr->second._groups.empty())
		{
			osc_hpp << endl << spacer << '{' << endl;
			for (GroupMap::const_iterator gitr(mitr->second._groups.begin()); gitr != mitr->second._groups.end(); ++gitr)
			{
				FieldSpecMap::const_iterator gsitr(fspec.find(gitr->first));
				osc_hpp << spacer << spacer << "_groups[" << gitr->first << "] = new " << gsitr->second._name << ';' << endl;
			}
			osc_hpp << spacer << '}' << endl;

		}
		else
			osc_hpp << " {}" << endl;

		osc_hpp << spacer << "virtual ~" << mitr->second._name << "() {}" << endl;
		if (!isHeader && !isTrailer)
			osc_hpp << spacer << "virtual bool process(Router& rt) const { return (static_cast<"
				<< ctxt._clname << "_Router&>(rt))(this); }" << endl;

		osc_hpp << endl << spacer << "static const " << fsitr->second._name << " get_msgtype() { return " << fsitr->second._name
			<< "(_msgtype); }" << endl;
		if (isHeader)
			osc_hpp << endl << _csMap.find_value_ref(cs_header_preamble) << endl;
		else if (isTrailer)
			osc_hpp << endl << _csMap.find_value_ref(cs_trailer_preamble) << endl;

		for (GroupMap::const_iterator gitr(mitr->second._groups.begin()); gitr != mitr->second._groups.end(); ++gitr)
		{
			FieldSpecMap::const_iterator gsitr(fspec.find(gitr->first));
			osr_cpp << _csMap.find_value_ref(cs_divider) << endl;
			osr_cpp << "const FieldTrait::TraitBase " << mitr->second._name << "::" << gsitr->second._name << "::_traits[] ="
				<< endl << '{' << endl;
			for (Presence::const_iterator flitr(gitr->second.get_presence().begin());
				flitr != gitr->second.get_presence().end(); ++flitr)
			{
				if (flitr != gitr->second.get_presence().begin())
				{
					osr_cpp << ',';
					if (distance(gitr->second.get_presence().begin(), flitr) % 3 == 0)
						osr_cpp << endl;
				}

				ostringstream tostr;
				tostr << "0x" << setw(3) << setfill('0') << hex << flitr->_field_traits.get();
				osr_cpp << spacer << "{ " << setw(4) << right << flitr->_fnum << ", " << setw(3)
					<< right << flitr->_ftype << ", " << setw(3) << right << flitr->_pos << ", " << tostr.str() << " }";
			}
			osr_cpp << endl << "};" << endl;
			osr_cpp << "const size_t " << mitr->second._name << "::" << gsitr->second._name << "::_fldcnt("
				<< gitr->second.get_presence().size() << ");" << endl;
			osr_cpp << "const f8String " << mitr->second._name << "::" << gsitr->second._name << "::_msgtype(\""
				<< gsitr->second._name << "\");" << endl;
			osc_hpp << endl << spacer << "class " << gsitr->second._name
				<< " : public GroupBase" << endl << spacer << '{' << endl;
			osc_hpp << spacer << spacer << "static const FieldTrait::TraitBase _traits[];" << endl;
			osc_hpp << spacer << spacer << "static const size_t _fldcnt;" << endl;
			osc_hpp << spacer << spacer << "static const f8String _msgtype;" << endl;
			osc_hpp << spacer << spacer << "static const unsigned short _field = " << gsitr->first << ';' << endl << endl;
			osc_hpp << spacer << "public:" << endl;
			osc_hpp << spacer << spacer << gsitr->second._name << "() : GroupBase(_field) {}" << endl;
			osc_hpp << spacer << spacer << "virtual ~" << gsitr->second._name << "() {}" << endl;
			osc_hpp << spacer << spacer << "MessageBase *create_group() { return new MessageBase(ctx, _msgtype, _traits, _traits + _fldcnt); }" << endl;
			osc_hpp << endl << spacer << spacer << "static const " << fsitr->second._name
				<< " get_msgtype() { return " << fsitr->second._name << "(_msgtype); }" << endl;
			osc_hpp << spacer << spacer << "static const unsigned short get_fnum() { return _field; }" << endl;
			osc_hpp << spacer << "};" << endl;
		}
		osc_hpp << "};" << endl << endl;
		osc_hpp << _csMap.find_value_ref(cs_divider) << endl;
	}

// =============================== Message class instantiation ==============================

	for (MessageSpecMap::const_iterator mitr(mspec.begin()); mitr != mspec.end(); ++mitr)
	{
		osc_cpp << "Message *Create_" << mitr->second._name << "() { return ";
		if (mitr->second._name == "trailer" || mitr->second._name == "header")
			osc_cpp << "reinterpret_cast<Message *>(new " << mitr->second._name << "); }" << endl;
		else
			osc_cpp << "new " << mitr->second._name << "; }" << endl;
	}
	osc_cpp << endl;
	osc_cpp << "const " << ctxt._clname << "_BaseMsgEntry bme;" << endl;
	osc_cpp << "const " << ctxt._clname << "_BaseEntry be;" << endl;
	osc_cpp << endl << _csMap.find_value_ref(cs_end_anon_namespace) << endl;

	osc_cpp << endl << _csMap.find_value_ref(cs_divider) << endl;
	osc_cpp << "template<>" << endl << "const " << ctxt._fixns << "::" << ctxt._clname << "_BaseMsgEntry::Pair "
		<< ctxt._fixns << "::" << ctxt._clname << "_BaseMsgEntry::_pairs[] =" << endl << '{' << endl;
	for (MessageSpecMap::const_iterator mitr(mspec.begin()); mitr != mspec.end(); ++mitr)
	{
		if (mitr != mspec.begin())
			osc_cpp << ',' << endl;
		osc_cpp << spacer << "{ \"" << mitr->first << "\", { &" << ctxt._fixns << "::Create_" << mitr->second._name;
		osc_cpp << ", \"" << mitr->second._name << '"';
		if (!mitr->second._comment.empty())
			osc_cpp << ',' << endl << spacer << spacer << '"' << mitr->second._comment << "\" }";
		else
			osc_cpp << ", 0 }";
		osc_cpp << " }";
	}
	osc_cpp << endl << "};" << endl;
	osc_cpp << "template<>" << endl << "const size_t " << ctxt._fixns << "::" << ctxt._clname
		<< "_BaseMsgEntry::_pairsz(sizeof(_pairs)/sizeof(" << ctxt._fixns << "::"
		<< ctxt._clname << "_BaseMsgEntry::Pair));" << endl;
	osc_cpp << "template<>" << endl << "const " << ctxt._fixns << "::" << ctxt._clname << "_BaseMsgEntry::NoValType "
		<< ctxt._fixns << "::" << ctxt._clname << "_BaseMsgEntry::_noval = {0, 0};" << endl;
	osc_cpp << "F8MetaCntx ctx(" << ctxt._version << ", bme, be, \"" << ctxt._beginstr << "\");" << endl;

// ==================================== Message router ==================================

	for (MessageSpecMap::const_iterator mitr(mspec.begin()); mitr != mspec.end(); ++mitr)
		osu_hpp << "class " << mitr->second._name << ';' << endl;
	osu_hpp << endl << _csMap.find_value_ref(cs_divider) << endl;
	osu_hpp << "class " << ctxt._clname << "_Router : public Router" << endl
		<< '{' << endl << "public:" << endl;
	osu_hpp << spacer << ctxt._clname << "_Router() {}" << endl;
	osu_hpp << spacer << "virtual ~" << ctxt._clname << "_Router() {}" << endl << endl;
	for (MessageSpecMap::const_iterator mitr(mspec.begin()); mitr != mspec.end(); ++mitr)
	{
		if (mitr->second._name == "trailer" || mitr->second._name == "header")
			continue;
		osu_hpp << spacer << "virtual bool operator() (const " << mitr->second._name << " *msg) const { return false; }" << endl;
	}
	osu_hpp << "};" << endl;

	// terminate files
	osc_hpp << endl << "} // namespace " << ctxt._fixns << endl;
	osc_hpp << _csMap.find_value_ref(cs_end_namespace) << endl;
	osc_hpp << "#endif // _" << flname(ctxt._out[Ctxt::classes_hpp].first) << '_' << endl;
	osu_hpp << endl << "} // namespace " << ctxt._fixns << endl;
	osu_hpp << _csMap.find_value_ref(cs_end_namespace) << endl;
	osu_hpp << "#endif // _" << flname(ctxt._out[Ctxt::router_hpp].first) << '_' << endl;
	osr_cpp << endl << _csMap.find_value_ref(cs_end_namespace) << endl;
	osr_cpp << "} // namespace " << ctxt._fixns << endl;
	osc_cpp << endl << "} // namespace " << ctxt._fixns << endl;
	osc_cpp << _csMap.find_value_ref(cs_end_namespace) << endl;
	osc_cpp << endl;

	return result;
}

