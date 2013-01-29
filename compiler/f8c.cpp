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
/** \file f8c.cpp
\n
  This is the fix8 compiler.\n
\n
f8c -- compile FIX xml schema\n
\n
<tt>
Usage: f8c [-NVcdfhiknoprstvx] \<input xml schema\>\n
   -N,--nounique           do not enforce unique field parsing (default false)\n
   -V,--verbose            be more verbose when processing\n
   -c,--classes \<server|client\> generate user session classes (default no)\n
   -f,--fields             generate code for all defined fields even if they are not used in any message (default no)\n
   -d,--dump               dump 1st pass parsed source xml file, exit\n
   -h,--help               help, this screen\n
   -i,--ignore             ignore errors, attempt to generate code anyhow (default no)\n
   -k,--keep               retain generated temporaries even if there are errors (.*.tmp)\n
   -n,--namespace \<ns\>     namespace to place generated code in (default FIXMmvv e.g. FIX4400)\n
   -o,--odir \<file\>        output target directory (default ./)\n
   -p,--prefix \<prefix\>    output filename prefix (default Myfix)\n
   -r,--retain             retain 1st pass code (default delete)\n
   -s,--second             2nd pass only, no precompile (default both)\n
   -t,--tabwidth           tabwidth for generated code (default 3 spaces)\n
   -v,--version            print version, exit\n
   -x,--fixt \<file\>        For FIXT hosted transports or for FIX5.0 and above, the input FIXT schema file\n
e.g.\n
   f8c -p Texfix -n TEX myfix.xml\n
   f8c -rp Texfix -n TEX -x ../schema/FIXT11.xml myfix.xml\n
   f8c -p Texfix -n TEX -c client -x ../schema/FIXT11.xml myfix.xml\n
</tt>
\n
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
#include <bitset>

#include <regex.h>
#include <errno.h>
#include <string.h>
#include <cctype>

// f8 headers
#include <f8includes.hpp>
#include <f8c.hpp>

#ifdef HAVE_GETOPT_H
#include <getopt.h>
#endif

//-----------------------------------------------------------------------------------------
using namespace std;
using namespace FIX8;

//-----------------------------------------------------------------------------------------
namespace FIX8
{
	template<>
	const size_t GeneratedTable<unsigned int, BaseEntry>::_pairsz(0);
	template<>
	const GeneratedTable<unsigned int, BaseEntry>::Pair GeneratedTable<unsigned int, BaseEntry>::_pairs[] = {};
	template<>
	const size_t GeneratedTable<const f8String, BaseMsgEntry>::_pairsz(0);
	template<>
	const GeneratedTable<const f8String, BaseMsgEntry>::Pair GeneratedTable<const f8String, BaseMsgEntry>::_pairs[] = {};
}

//-----------------------------------------------------------------------------------------
const string Ctxt::_exts[count] = { "_types.cpp", "_types.hpp", "_traits.cpp", "_classes.cpp",
	"_classes.hpp", "_router.hpp", "_session.hpp" };

string precompFile, spacer, inputFile, shortName, fixt, shortNameFixt, odir("./"), prefix("Myfix"), gen_classes;
bool verbose(false), error_ignore(false), gen_fields(false);
unsigned glob_errors(0), glob_warnings(0), tabsize(3);
extern unsigned glob_errors;
extern const string GETARGLIST("hvVo:p:dikn:rst:x:Nc:fb");
extern string spacer, shortName;

//-----------------------------------------------------------------------------------------
const CSMap _csMap;

//-----------------------------------------------------------------------------------------
// static data
#include <f8cstatic.hpp>

//-----------------------------------------------------------------------------------------
void print_usage();
string insert_year();
int process(XmlElement& xf, Ctxt& ctxt);
int load_fix_version (XmlElement& xf, Ctxt& ctxt);
int load_fields(XmlElement& xf, FieldSpecMap& fspec);
void process_special_traits(const unsigned short field, FieldTraits& fts);
int process_message_fields(const std::string& where, const XmlElement *xt, FieldTraits& fts,
	const FieldToNumMap& ftonSpec, FieldSpecMap& fspec, const unsigned component);
int load_messages(XmlElement& xf, MessageSpecMap& mspec, const FieldToNumMap& ftonSpec, FieldSpecMap& fspec);
void process_ordering(MessageSpecMap& mspec);
ostream *open_ofile(const string& odir, const string& fname, string& target);
const string flname(const string& from);
void process_value_enums(FieldSpecMap::const_iterator itr, ostream& ost_hpp, ostream& ost_cpp);
const string& mkel(const string& base, const string& compon, string& where);
const string& filepart(const string& source, string& where);
void generate_preamble(ostream& to, bool donotedit=true);
unsigned parse_groups(MessageSpec& ritr, XmlElement::XmlSet::const_iterator& itr, const string& name,
	const FieldToNumMap& ftonSpec, FieldSpecMap& fspec, XmlElement::XmlSet& grplist);
int precomp(XmlElement& xf, ostream& outf);
int precompfixt(XmlElement& xft, XmlElement& xf, ostream& outf, bool nounique);
void generate_group_bodies(const MessageSpec& ms, const FieldSpecMap& fspec, int depth,
	const string& msname, ostream& outp, ostream& outh, const string cls_prefix=string());
void binary_report();

//-----------------------------------------------------------------------------------------
int main(int argc, char **argv)
{
	int val;
	bool dump(false), keep_failed(false), retain_precomp(false), second_only(false), nounique(false);
	Ctxt ctxt;

#ifdef HAVE_GETOPT_LONG
	option long_options[] =
	{
		{ "help",			0,	0,	'h' },
		{ "version",		0,	0,	'v' },
		{ "verbose",		0,	0,	'V' },
		{ "nounique",		0,	0,	'N' },
		{ "odir",			1,	0,	'o' },
		{ "dump",			0,	0,	'd' },
		{ "ignore",			0,	0,	'i' },
		{ "fields",			0,	0,	'f' },
		{ "keep",			0,	0,	'k' },
		{ "retain",			0,	0,	'r' },
		{ "binary",			0,	0,	'b' },
		{ "classes",		1,	0,	'c' },
		{ "second",			0,	0,	's' },
		{ "prefix",			1,	0,	'p' },
		{ "namespace",		1,	0,	'n' },
		{ "tabsize",		1,	0,	't' },
		{ "fixt",			1,	0,	'x' },
		{ 0 },
	};

	while ((val = getopt_long (argc, argv, GETARGLIST.c_str(), long_options, 0)) != -1)
#else
	while ((val = getopt (argc, argv, GETARGLIST.c_str())) != -1)
#endif
	{
      switch (val)
		{
		case 'v':
			cout << "f8c for "PACKAGE" version "VERSION << endl;
			cout << _csMap.find_ref(cs_copyright_short) << insert_year()
				  << _csMap.find_ref(cs_copyright_short2) << endl;
			cout << "Released under the GNU LESSER GENERAL PUBLIC LICENSE, Version 3. See <http://fsf.org/> for details." << endl;
			return 0;
		case 'V': verbose = true; break;
		case 'f': gen_fields = true; break;
		case 'N': nounique = true; break;
		case 'c': gen_classes = optarg; break;
		case 'h': print_usage(); return 0;
		case ':': case '?': return 1;
		case 'o': odir = optarg; break;
		case 'd': dump = true; break;
		case 'i': error_ignore = true; break;
		case 'k': keep_failed = true; break;
		case 'r': retain_precomp = true; break;
		case 's': second_only = true; break;
		case 't': tabsize = GetValue<unsigned>(optarg); break;
		case 'p': prefix = optarg; break;
		case 'b': binary_report(); return 0;
		case 'x': fixt = optarg; break;
		case 'n': ctxt._fixns = optarg; break;
		default: break;
		}
	}

	spacer.assign(tabsize, ' ');

	if (optind < argc)
	{
		inputFile = argv[optind];
		filepart(inputFile, shortName);
		if (!fixt.empty())
			filepart(fixt, shortNameFixt);
	}
	else
	{
		cerr << "no input xml file specified" << endl;
		print_usage();
		return 1;
	}

	if (!gen_classes.empty() && gen_classes != "server" && gen_classes != "client")
	{
		cerr << "Error: " << gen_classes << " not a valid role for class generation. Choose 'server' or 'client'." << endl;
		print_usage();
		return 1;
	}

	scoped_ptr<XmlElement> cfr;
	if (second_only)
	{
		cfr.Reset(XmlElement::Factory(inputFile));
		if (!cfr.get())
		{
			cerr << "Error reading file \'" << inputFile << '\'';
			if	(errno)
				cerr << " (" << strerror(errno) << ')';
			cerr << endl;
			return 1;
		}
	}
	else
	{
		cout << "expanding " << shortName << ' ';
		cout.flush();
		scoped_ptr<ostream> pre_out(open_ofile(odir, shortName + ".p1", precompFile));
		scoped_ptr<XmlElement> pcmp(XmlElement::Factory(inputFile)), pcmpfixt;
		unsigned xmlsz(pcmp->GetLineCnt());

		unsigned fixtsz(0);
		if (!fixt.empty())
		{
			pcmpfixt.Reset(XmlElement::Factory(fixt));
			if (!pcmpfixt.get())
			{
				cerr << "Error reading file \'" << fixt << '\'';
				if	(errno)
					cerr << " (" << strerror(errno) << ')';
				cerr << endl;
				return 1;
			}
			fixtsz = pcmpfixt->GetLineCnt();
		}
		if (!pcmp.get())
		{
			cerr << "Error reading file \'" << inputFile << '\'' << " (" << precompFile << ')';
			if	(errno)
				cerr << " (" << strerror(errno) << ')';
			cerr << endl;
			return 1;
		}
		cout << (fixtsz + xmlsz) << " => ";
		cout.flush();
		if (!fixt.empty())
			precompfixt(*pcmpfixt, *pcmp, *pre_out, nounique);
		else
			precomp(*pcmp, *pre_out);
		pre_out.Reset();
		cfr.Reset(XmlElement::Factory(precompFile));
		cout << cfr->GetLineCnt() << " lines" << endl;
		if (!retain_precomp)
			remove(precompFile.c_str());
	}

	if (cfr->GetErrorCnt())
	{
		cerr << cfr->GetErrorCnt() << " error"
			<< (cfr->GetErrorCnt() == 1 ? " " : "s ") << "found in \'" << shortName << '\'' << endl;
		return 1;
	}

	if (dump)
	{
		cout << *cfr;
		return 0;
	}

	if (load_fix_version (*cfr, ctxt) < 0)
		return 1;
	for (unsigned ii(0); ii < Ctxt::count; ++ii)
	{
		ctxt._out[ii].first.second = prefix + ctxt._exts[ii];
		ctxt._out[ii].first.first = '.' + ctxt._out[ii].first.second + ".p2";
		remove(ctxt._out[ii].first.first.c_str());
		string target;
		if ((ctxt._out[ii].second = open_ofile(odir, ctxt._out[ii].first.first, target)) == 0)
			return 1;
	}

	int result(1);
	if (!glob_errors)
	{
		cout << "compiling " << shortName << endl;
		result = process(*cfr, ctxt);
		for (unsigned ii(0); ii < Ctxt::count; ++ii)
		{
			delete ctxt._out[ii].second;
			if (glob_errors && !error_ignore)
			{
				if (!keep_failed)
					remove(ctxt._out[ii].first.first.c_str());
			}
			else
			{
				string backup(ctxt._out[ii].first.second + ".old");
				remove(backup.c_str());
				rename(ctxt._out[ii].first.second.c_str(), backup.c_str());
				rename(ctxt._out[ii].first.first.c_str(), ctxt._out[ii].first.second.c_str());
			}
		}

		if (gen_classes.empty())
			remove(ctxt._out[Ctxt::session_hpp].first.second.c_str());
	}

	if (glob_errors)
		cerr << glob_errors << " error" << (glob_errors == 1 ? "." : "s.") << endl;
	if (glob_warnings)
		cerr << glob_warnings << " warning" << (glob_warnings == 1 ? "." : "s.") << endl;
	return result;
}

//-----------------------------------------------------------------------------------------
int load_fields(XmlElement& xf, FieldSpecMap& fspec)
{
	int fieldsLoaded(0);

	XmlElement::XmlSet flist;
	if (!xf.find("fix/fields/field", flist))
	{
		cerr << "error: No fields found in " << shortName << endl;
		return 0;
	}

	for(XmlElement::XmlSet::const_iterator itr(flist.begin()); itr != flist.end(); ++itr)
	{
		string number, name, type;
		if ((*itr)->GetAttr("number", number) && (*itr)->GetAttr("name", name) && (*itr)->GetAttr("type", type))
		{
			trim(number);
			trim(name);
			trim(type);
			InPlaceStrToUpper(type);
			FieldTrait::FieldType ft(FieldSpec::_baseTypeMap.find_value(type));
			pair<FieldSpecMap::iterator, bool> result;
			if (ft != FieldTrait::ft_untyped)
				result = fspec.insert(FieldSpecMap::value_type(GetValue<unsigned>(number), FieldSpec(name, ft)));
			else
			{
				cerr << shortName << ':' << recover_line(**itr) << ": warning: Unknown field type: " << type << " in " << name << endl;
				++glob_warnings;
				continue;
			}

			(*itr)->GetAttr("description", result.first->second._description);
			(*itr)->GetAttr("comment", result.first->second._comment);

			++fieldsLoaded;

			XmlElement::XmlSet realmlist;
			if ((*itr)->find("field/value", realmlist))
			{
				for(XmlElement::XmlSet::const_iterator ditr(realmlist.begin()); ditr != realmlist.end(); ++ditr)
				{
					string enum_str, description;
					if ((*ditr)->GetAttr("enum", enum_str))
					{
						if (!(*ditr)->GetAttr("description", description) || description.empty())
							description = enum_str; 	// use enum if no description supplied

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
					{
						cerr << shortName << ':' << recover_line(**itr) << ": error: Value element missing required attributes." << endl;
						++glob_errors;
					}
				}
			}
		}
		else
		{
			cerr << shortName << ':' << recover_line(**itr) << ": error: Field element missing required attributes" << endl;
			++glob_errors;
		}
	}

	return fieldsLoaded;
}

//-----------------------------------------------------------------------------------------
int load_messages(XmlElement& xf, MessageSpecMap& mspec, const FieldToNumMap& ftonSpec, FieldSpecMap& fspec)
{
	int msgssLoaded(0), grpsparsed(0);

	XmlElement::XmlSet mlist;
	if (!xf.find("fix/messages/message", mlist))
	{
		cerr << "error: No messages found in " << shortName << endl;
		++glob_errors;
		return 0;
	}

	if (!xf.find("fix/header", mlist))
	{
		cerr << "error: No header element found in " << shortName << endl;
		++glob_errors;
		return 0;
	}

	if (!xf.find("fix/trailer", mlist))
	{
		cerr << "error: No trailer element found in " << shortName << endl;
		++glob_errors;
		return 0;
	}

	// lookup msgtype realm - all messages must have corresponding entry here
	FieldSpecMap::const_iterator fsitr(fspec.find(35));	// always 35
	if (fsitr == fspec.end() || !fsitr->second._dvals)
	{
		cerr << "error: Could not locate MsgType realm defintions in " << shortName
			<< ". See FAQ for more details." << endl;
		++glob_errors;
		return 0;
	}

	for(XmlElement::XmlSet::const_iterator itr(mlist.begin()); itr != mlist.end(); ++itr)
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
				cerr << shortName << ':' << recover_line(**itr) << ": error: Message "
				  << name << " does not have corrresponding entry in MsgType field realm. See FAQ for more details." << endl;
				++glob_errors;
				continue;
			}

			elname = "message";
		}
		else
		{
			cerr << shortName << ':' << recover_line(**itr) << ": error: Message element missing required attributes" << endl;
			++glob_errors;
			continue;
		}

		pair<MessageSpecMap::iterator, bool> result(
			mspec.insert(MessageSpecMap::value_type(msgtype, MessageSpec(name, msgcat % "admin"))));
		if (!result.second)
		{
			cerr << shortName << ':' << recover_line(**itr) << ": error: Could not add message " << name << " (" << msgtype << ")" << endl;
			++glob_errors;
			continue;
		}

		string elpart;
		XmlElement::XmlSet grplist;
		if ((*itr)->find(mkel(elname, "group", elpart), grplist))
			grpsparsed += parse_groups(result.first->second, itr, name, ftonSpec, fspec, grplist);

		(*itr)->GetAttr("comment", result.first->second._comment);

		process_message_fields(mkel(elname, "field", elpart), *itr, result.first->second._fields, ftonSpec, fspec, 0);

		++msgssLoaded;
	}

	if (verbose)
		cout << grpsparsed << " groups processed" << endl;

	return msgssLoaded;
}

//-----------------------------------------------------------------------------------------
unsigned parse_groups(MessageSpec& ritr, XmlElement::XmlSet::const_iterator& itr, const string& name,
	const FieldToNumMap& ftonSpec, FieldSpecMap& fspec, XmlElement::XmlSet& grplist)
{
	unsigned result(0);

	for(XmlElement::XmlSet::const_iterator gitr(grplist.begin()); gitr != grplist.end(); ++gitr)
	{
		string gname, required;
		if ((*gitr)->GetAttr("name", gname) && (*gitr)->GetAttr("required", required))
		{
			// add group FieldTrait
			FieldToNumMap::const_iterator ftonItr(ftonSpec.find(gname));
			FieldSpecMap::const_iterator fs_itr;
			if (ftonItr != ftonSpec.end() && (fs_itr = fspec.find(ftonItr->second)) != fspec.end())
			{
				if (!ritr._fields.add(FieldTrait(fs_itr->first, FieldTrait::ft_int, (*gitr)->GetSubIdx(),
					required == "Y", true, 0)))
				{
					cerr << "error: Could not add group trait object " << gname << endl;
					++glob_errors;
				}
				else
				{
					fs_itr->second._used = true; 	// we always assume group count fields are used
					pair<GroupMap::iterator, bool> gresult(
						ritr._groups.insert(GroupMap::value_type(fs_itr->first, MessageSpec(gname))));
					process_message_fields("group/field", *gitr, gresult.first->second._fields, ftonSpec, fspec, 0);
					XmlElement::XmlSet comlist;
					++result;
					if ((*gitr)->find("group/group", comlist))
						result += parse_groups(gresult.first->second, gitr, gname, ftonSpec, fspec, comlist);
				}
			}
			else
			{
				cerr << shortName << ':' << recover_line(**itr)
					<< ": error: Could not locate group Field " << gname << " from known field types in " << shortName << endl;
				++glob_errors;
				continue;
			}
		}
		else
		{
			cerr << shortName << ':' << recover_line(**itr) << ": error: Group element missing required attributes" << endl;
			++glob_errors;
		}
	}

	return result;
}

//-----------------------------------------------------------------------------------------
void generate_group_bodies(const MessageSpec& ms, const FieldSpecMap& fspec, int depth, const string& msname,
	ostream& outp, ostream& outh, const string cls_prefix)
{
	const string dspacer(depth * tabsize, ' '), d2spacer((depth + 1) * tabsize, ' ');

	string prefix;
	if (!cls_prefix.empty())
		prefix = cls_prefix + "::";

	for (GroupMap::const_iterator gitr(ms._groups.begin()); gitr != ms._groups.end(); ++gitr)
	{
		FieldSpecMap::const_iterator gsitr(fspec.find(gitr->first));
		outp << _csMap.find_ref(cs_divider) << endl;
		outp << "const FieldTrait " << prefix << ms._name << "::" << gsitr->second._name << "::_traits[] ="
			<< endl << '{' << endl;
		for (Presence::const_iterator flitr(gitr->second._fields.get_presence().begin());
			flitr != gitr->second._fields.get_presence().end(); ++flitr)
		{
			bool spaceme(true);
			if (flitr != gitr->second._fields.get_presence().begin())
			{
				outp << ',';
				if (Presence::distance(gitr->second._fields.get_presence().begin(), flitr) % 3 == 0)
					outp << endl;
				else
					spaceme = false;
			}

			ostringstream tostr;
			tostr << "0x" << setfill('0') << hex << flitr->_field_traits.get();
			outp << (spaceme ? spacer : " ");
			outp << "FieldTrait(" << setw(4) << right << flitr->_fnum << ", " << setw(3)
				<< right << flitr->_ftype << ", " << setw(3) << right << flitr->_pos << ", " << tostr.str() << ')';
		}
		outp << endl << "};" << endl;
#if !defined PERMIT_CUSTOM_FIELDS
		outp << "const FieldTrait_Hash_Array " << prefix << ms._name << "::" << gsitr->second._name << "::_ftha("
			<< prefix << ms._name << "::" << gsitr->second._name << "::_traits, "
			<< gitr->second._fields.get_presence().size() << ");" << endl;
#endif
		outp << "const MsgType " << prefix << ms._name << "::" << gsitr->second._name << "::_msgtype(\""
			<< gsitr->second._name << "\");" << endl;
		outp << "const unsigned short " << prefix << ms._name << "::" << gsitr->second._name << "::_fnum;" << endl;

		// nested class decl.
		outh << endl << dspacer << "/// " << gitr->second._name << " (" << gitr->first << "), "
			<< (gitr->second._is_admin ? "admin" : "application") << ", " <<  gitr->second._fields.get_presence().size()
			<< " fiel" << (gitr->second._fields.get_presence().size() == 1 ? "d, " : "ds, ")
			<< gitr->second._groups.size() << " grou" << (gitr->second._groups.size() == 1 ? "p." : "ps.");
		outh << endl << dspacer << "// " << prefix << ms._name << "::" << gsitr->second._name << endl;
		outh << dspacer << "class " << gsitr->second._name
			<< " : public GroupBase // depth: " << depth << endl << dspacer << '{' << endl;
		outh << d2spacer << "static const FieldTrait _traits[];" << endl;
#if !defined PERMIT_CUSTOM_FIELDS
		outh << d2spacer << "static const FieldTrait_Hash_Array _ftha;" << endl;
#endif
		outh << d2spacer << "static const MsgType _msgtype;" << endl << endl;
		outh << dspacer << "public:" << endl;
		outh << d2spacer << "static const unsigned short _fnum = " << gsitr->first << ';' << endl << endl;
		outh << d2spacer << gsitr->second._name << "() : GroupBase(_fnum) {}" << endl;
		outh << d2spacer << "virtual ~" << gsitr->second._name << "() {}" << endl;
		if (gitr->second._groups.empty())
			outh << d2spacer << "MessageBase *create_group() const { return new MessageBase(ctx, _msgtype(), _traits, "
				<< gitr->second._fields.get_presence().size()
#if defined PERMIT_CUSTOM_FIELDS
				<< "); }" << endl;
#else
				<< ", &_ftha); }" << endl;
#endif
		else
		{
			outh << d2spacer << "MessageBase *create_group() const" << endl << d2spacer << '{' << endl;
			outh << d2spacer << spacer << "MessageBase *mb(new MessageBase(ctx, _msgtype(), _traits, "
				<< gitr->second._fields.get_presence().size()
#if defined PERMIT_CUSTOM_FIELDS
				<< "));" << endl;
#else
				<< ", &_ftha));" << endl;
#endif
			for (GroupMap::const_iterator gsitr(gitr->second._groups.begin()); gsitr != gitr->second._groups.end(); ++gsitr)
				outh << d2spacer << spacer << "mb->append_group(new " << gsitr->second._name << "); // "
					<< gsitr->first << endl;
			outh << d2spacer << spacer << "return mb;" << endl;
			outh << d2spacer << '}' << endl;
		}
		outh << endl << d2spacer << "static const " << msname << "& get_msgtype() { return _msgtype; }" << endl;

		// process nested groups
		if (!gitr->second._groups.empty())
			generate_group_bodies(gitr->second, fspec, depth + 1, msname, outp, outh, prefix + ms._name);

		outh << dspacer << "};" << endl;
	}
}

//-----------------------------------------------------------------------------------------
int process(XmlElement& xf, Ctxt& ctxt)
{
	ostream& ost_cpp(*ctxt._out[Ctxt::types_cpp].second);
	ostream& ost_hpp(*ctxt._out[Ctxt::types_hpp].second);
	ostream& osr_cpp(*ctxt._out[Ctxt::traits_cpp].second);
	ostream& osc_hpp(*ctxt._out[Ctxt::classes_hpp].second);
	ostream& osc_cpp(*ctxt._out[Ctxt::classes_cpp].second);
	ostream& osu_hpp(*ctxt._out[Ctxt::router_hpp].second);
	ostream& oss_hpp(*ctxt._out[Ctxt::session_hpp].second);
	int result(0);

// ================================= Field loading  =======================================
	FieldSpecMap fspec;
	int fields(load_fields(xf, fspec));
	if (!fields || glob_errors)
		return 0;
	if (verbose)
		cout << fields << " fields processed" << endl;

// ================================= Message processing ===================================

	FieldToNumMap ftonSpec;
	for (FieldSpecMap::const_iterator fs_itr(fspec.begin()); fs_itr != fspec.end(); ++fs_itr)
		ftonSpec.insert(FieldToNumMap::value_type(fs_itr->second._name, fs_itr->first));

	MessageSpecMap mspec;

	int msgsloaded(load_messages(xf, mspec, ftonSpec, fspec));
	if (!msgsloaded)
		return result;
	if (verbose)
		cout << msgsloaded << " messages processed" << endl;

	process_ordering(mspec);

	// output file preambles
	generate_preamble(osu_hpp);
	osu_hpp << "#ifndef _" << flname(ctxt._out[Ctxt::router_hpp].first.second) << '_' << endl;
	osu_hpp << "#define _" << flname(ctxt._out[Ctxt::router_hpp].first.second) << '_' << endl << endl;
	osu_hpp << _csMap.find_ref(cs_start_namespace) << endl;
	osu_hpp << "namespace " << ctxt._fixns << " {" << endl;
	osu_hpp << endl << _csMap.find_ref(cs_divider) << endl;

	generate_preamble(osc_hpp);
	osc_hpp << "#ifndef _" << flname(ctxt._out[Ctxt::classes_hpp].first.second) << '_' << endl;
	osc_hpp << "#define _" << flname(ctxt._out[Ctxt::classes_hpp].first.second) << '_' << endl << endl;
	osc_hpp << _csMap.find_ref(cs_start_namespace) << endl;
	osc_hpp << "namespace " << ctxt._fixns << " {" << endl;

	osc_hpp << endl << _csMap.find_ref(cs_divider) << endl;
	osc_hpp << "typedef MsgTable " << ctxt._clname << "_BaseMsgEntry;" << endl;
	osc_hpp << "extern F8MetaCntx ctx;" << endl;
	osc_hpp << "class " << ctxt._clname << "_Router;" << endl;
	osc_hpp << endl << _csMap.find_ref(cs_divider) << endl;

	generate_preamble(osc_cpp);
	osc_cpp << _csMap.find_ref(cs_generated_includes) << endl;
	osc_cpp << "#include \"" << ctxt._out[Ctxt::types_hpp].first.second << '"' << endl;
	osc_cpp << "#include \"" << ctxt._out[Ctxt::router_hpp].first.second << '"' << endl;
	osc_cpp << "#include \"" << ctxt._out[Ctxt::classes_hpp].first.second << '"' << endl;
	osc_cpp << _csMap.find_ref(cs_divider) << endl;
	osc_cpp << _csMap.find_ref(cs_start_namespace) << endl;
	osc_cpp << "namespace " << ctxt._fixns << " {" << endl << endl;
	osc_cpp << _csMap.find_ref(cs_start_anon_namespace) << endl << endl;
	osc_cpp << _csMap.find_ref(cs_divider) << endl;

	generate_preamble(osr_cpp);
	osr_cpp << _csMap.find_ref(cs_generated_includes) << endl;
	osr_cpp << "#include \"" << ctxt._out[Ctxt::types_hpp].first.second << '"' << endl;
	osr_cpp << "#include \"" << ctxt._out[Ctxt::router_hpp].first.second << '"' << endl;
	osr_cpp << "#include \"" << ctxt._out[Ctxt::classes_hpp].first.second << '"' << endl;
	osr_cpp << _csMap.find_ref(cs_divider) << endl;
	osr_cpp << _csMap.find_ref(cs_start_namespace) << endl;
	osr_cpp << "namespace " << ctxt._fixns << " {" << endl << endl;

	osr_cpp << _csMap.find_ref(cs_divider) << endl;

	FieldSpecMap::const_iterator fsitr(fspec.find(35));	// always 35
	for (MessageSpecMap::const_iterator mitr(mspec.begin()); mitr != mspec.end(); ++mitr)
	{
		bool isTrailer(mitr->second._name == "trailer");
		bool isHeader(mitr->second._name == "header");
		osc_hpp << "/// " << mitr->second._name << " (" << mitr->first << "), "
			<< (mitr->second._is_admin ? "admin" : "application")
			<< ", " <<  mitr->second._fields.get_presence().size() << " fiel"
			<< (mitr->second._fields.get_presence().size() == 1 ? "d, " : "ds, ")
			<< mitr->second._groups.size() << " grou" << (mitr->second._groups.size() == 1 ? "p." : "ps.");
		if (!mitr->second._comment.empty())
			osc_hpp << ' ' << mitr->second._comment;
		osc_hpp << endl;
		osc_hpp << "class " << mitr->second._name << " : public "
			<< (isTrailer || isHeader ? "MessageBase" : "Message") << endl << '{' << endl;

		if (mitr->second._fields.get_presence().size())
		{
			osr_cpp << _csMap.find_ref(cs_divider) << endl;
			osr_cpp << "const FieldTrait " << mitr->second._name << "::_traits[] ="
				<< endl << '{' << endl;
			for (Presence::const_iterator flitr(mitr->second._fields.get_presence().begin());
				flitr != mitr->second._fields.get_presence().end(); ++flitr)
			{
				bool spaceme(true);
				if (flitr != mitr->second._fields.get_presence().begin())
				{
					osr_cpp << ',';
					if (Presence::distance(mitr->second._fields.get_presence().begin(), flitr) % 3 == 0)
						osr_cpp << endl;
					else
						spaceme = false;
				}

				ostringstream tostr;
				tostr << "0x" << setfill('0') << hex << flitr->_field_traits.get();
				osr_cpp << (spaceme ? spacer : " ");
				osr_cpp << "FieldTrait(" << setw(4) << right << flitr->_fnum << ", "
					<< setw(3) << right << flitr->_ftype << ", " << setw(3) << right
					<< flitr->_pos << ", " << tostr.str() << ')';
			}
			osr_cpp << endl << "};" << endl;
#if !defined PERMIT_CUSTOM_FIELDS
			osr_cpp << "const FieldTrait_Hash_Array " << mitr->second._name << "::_ftha(" << mitr->second._name << "::_traits, "
				<< mitr->second._fields.get_presence().size() << ");" << endl;
#endif
			osr_cpp << "const MsgType " << mitr->second._name << "::_msgtype(\"" << mitr->first << "\");" << endl;
			osc_hpp << spacer << "static const FieldTrait _traits[];" << endl;
#if !defined PERMIT_CUSTOM_FIELDS
			osc_hpp << spacer << "static const FieldTrait_Hash_Array _ftha;" << endl;
#endif
			osc_hpp << spacer << "static const MsgType _msgtype;" << endl;
		}

		osc_hpp << endl;

		osc_hpp << "public:" << endl;
		osc_hpp << spacer << mitr->second._name << "()";
		if (mitr->second._fields.get_presence().size())
			osc_hpp << " : " << (isTrailer || isHeader ? "MessageBase" : "Message")
				<< "(ctx, _msgtype(), _traits, " << mitr->second._fields.get_presence().size()
#if defined PERMIT_CUSTOM_FIELDS
				<< ')';
#else
				<< ", &_ftha)";
#endif
		if (isHeader || isTrailer)
			osc_hpp << " { add_preamble(); }" << endl;
		else if (!mitr->second._groups.empty())
		{
			osc_hpp << endl << spacer << '{' << endl;
			for (GroupMap::const_iterator gitr(mitr->second._groups.begin()); gitr != mitr->second._groups.end(); ++gitr)
			{
				FieldSpecMap::const_iterator gsitr(fspec.find(gitr->first));
				osc_hpp << spacer << spacer;
				osc_hpp << "_groups.insert(_groups.end(), Groups::value_type("
					<< gsitr->first << ", new " << gsitr->second._name << "));" << endl;
			}
			osc_hpp << spacer << '}' << endl;
		}
		else
			osc_hpp << " {}" << endl;

		osc_hpp << spacer << "virtual ~" << mitr->second._name << "() {}" << endl;
		if (!isHeader && !isTrailer)
		{
			osc_hpp << spacer << "virtual bool process(Router& rt) const { return (static_cast<"
				<< ctxt._clname << "_Router&>(rt))(this); }" << endl;
			if (mitr->second._is_admin)
				osc_hpp << spacer << "virtual bool is_admin() const { return true; }" << endl;
		}

		osc_hpp << endl << spacer << "static const " << fsitr->second._name << "& get_msgtype() { return _msgtype; }" << endl;
		if (isHeader)
			osc_hpp << endl << _csMap.find_ref(cs_header_preamble) << endl;
		else if (isTrailer)
			osc_hpp << endl << _csMap.find_ref(cs_trailer_preamble) << endl;

// =============================== Repeating group nested classes ==============================

		generate_group_bodies(mitr->second, fspec, 1, fsitr->second._name, osr_cpp, osc_hpp);

		osc_hpp << "};" << endl << endl;
		osc_hpp << _csMap.find_ref(cs_divider) << endl;
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
	osc_cpp << endl << _csMap.find_ref(cs_end_anon_namespace) << endl;
	osc_cpp << endl << "} // namespace " << ctxt._fixns << endl;

	osc_cpp << endl << _csMap.find_ref(cs_divider) << endl;
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
	osc_cpp << "template<>" << endl << "const " << ctxt._fixns << "::" << ctxt._clname << "_BaseMsgEntry::NotFoundType "
		<< ctxt._fixns << "::" << ctxt._clname << "_BaseMsgEntry::_noval = {0, 0};" << endl;
	osc_cpp << "namespace " << ctxt._fixns << " { F8MetaCntx ctx(" << ctxt._version << ", bme, be, \"" << ctxt._beginstr << "\"); }" << endl;

// ==================================== Message router ==================================

	osu_hpp << "class " << ctxt._clname << "_Router : public Router" << endl
		<< '{' << endl << "public:" << endl;
	osu_hpp << spacer << ctxt._clname << "_Router() {}" << endl;
	osu_hpp << spacer << "virtual ~" << ctxt._clname << "_Router() {}" << endl << endl;
	for (MessageSpecMap::const_iterator mitr(mspec.begin()); mitr != mspec.end(); ++mitr)
	{
		if (mitr->second._name == "trailer" || mitr->second._name == "header")
			continue;
		osu_hpp << spacer << "virtual bool operator() (const class " << mitr->second._name
			<< " *msg) const { return " << (mitr->second._is_admin ? "true" : "false") << "; }" << endl;
	}
	osu_hpp << "};" << endl;

	// terminate files
	osc_hpp << endl << "} // namespace " << ctxt._fixns << endl;
	osc_hpp << _csMap.find_ref(cs_end_namespace) << endl;
	osc_hpp << "#endif // _" << flname(ctxt._out[Ctxt::classes_hpp].first.second) << '_' << endl;
	osu_hpp << endl << "} // namespace " << ctxt._fixns << endl;
	osu_hpp << _csMap.find_ref(cs_end_namespace) << endl;
	osu_hpp << "#endif // _" << flname(ctxt._out[Ctxt::router_hpp].first.second) << '_' << endl;
	osr_cpp << endl << _csMap.find_ref(cs_end_namespace) << endl;
	osr_cpp << "} // namespace " << ctxt._fixns << endl;
	osc_cpp << _csMap.find_ref(cs_end_namespace) << endl;
	osc_cpp << endl;

// =============================== Generate optional user session and router ==============================

	if (!gen_classes.empty())
	{
		bool is_server(gen_classes == "server");
		generate_preamble(oss_hpp, false);
		oss_hpp << "#ifndef _" << flname(ctxt._out[Ctxt::session_hpp].first.second) << '_' << endl;
		oss_hpp << "#define _" << flname(ctxt._out[Ctxt::session_hpp].first.second) << '_' << endl;
		oss_hpp << endl << _csMap.find_ref(cs_divider) << endl;

		oss_hpp << "// " << gen_classes << " session and router classes" << endl;
		oss_hpp << _csMap.find_ref(cs_divider) << endl;

		oss_hpp << "class " << ctxt._clname << "_session_" << gen_classes << ';' << endl << endl;
		oss_hpp << "class " << ctxt._clname << "_router_" << gen_classes
			<< " : public FIX8::" << ctxt._fixns << "::" << ctxt._clname << "_Router" << endl << '{' << endl;
		oss_hpp << spacer << ctxt._clname << "_session_" << gen_classes << "& _session; " << endl << endl;
		oss_hpp << "public:" << endl;
		oss_hpp << spacer << ctxt._clname << "_router_" << gen_classes
			<< '(' << ctxt._clname << "_session_" << gen_classes << "& session) : _session(session) {}" << endl;
		oss_hpp << spacer << "virtual ~" << ctxt._clname << "_router_" << gen_classes << "() {}" << endl << endl;
		oss_hpp << spacer << "// Override these methods to receive specific message callbacks." << endl;
		for (MessageSpecMap::const_iterator mitr(mspec.begin()); mitr != mspec.end(); ++mitr)
		{
			if (mitr->second._name == "trailer" || mitr->second._name == "header")
				continue;
			oss_hpp << spacer << "// bool operator() (const FIX8::"
				<< ctxt._fixns << "::" << mitr->second._name << " *msg) const;" << endl;
		}
		oss_hpp << "};" << endl;

		oss_hpp << endl << _csMap.find_ref(cs_divider) << endl;
		oss_hpp << "class " << ctxt._clname << "_session_" << gen_classes
			<< " : public FIX8::Session" << endl << '{' << endl;
		oss_hpp << spacer << ctxt._clname << "_router_" << gen_classes << " _router; " << endl << endl;
		oss_hpp << "public:" << endl;
		oss_hpp << spacer << ctxt._clname << "_session_" << gen_classes;
		if (is_server)
		{
			oss_hpp << "(const FIX8::F8MetaCntx& ctx, FIX8::Persister *persist=0," << endl;
			oss_hpp << spacer << spacer << "FIX8::Logger *logger=0, FIX8::Logger *plogger=0) : Session"
				"(ctx, persist, logger, plogger), _router(*this) {} " << endl << endl;
		}
		else
		{
			oss_hpp << "(const FIX8::F8MetaCntx& ctx, const FIX8::SessionID& sid, FIX8::Persister *persist=0," << endl;
			oss_hpp << spacer << spacer << "FIX8::Logger *logger=0, FIX8::Logger *plogger=0) : Session"
				"(ctx, sid, persist, logger, plogger), _router(*this) {} " << endl << endl;
		}

		oss_hpp << spacer << "// Override these methods if required but remember to call the base class method first." << endl
			<< spacer << "// bool handle_logon(const unsigned seqnum, const Message *msg);" << endl
			<< spacer << "// Message *generate_logon(const unsigned heartbeat_interval, const f8String davi=f8String());" << endl
			<< spacer << "// bool handle_logout(const unsigned seqnum, const Message *msg);" << endl
			<< spacer << "// Message *generate_logout();" << endl
			<< spacer << "// bool handle_heartbeat(const unsigned seqnum, const Message *msg);" << endl
			<< spacer << "// Message *generate_heartbeat(const f8String& testReqID);" << endl
			<< spacer << "// bool handle_resend_request(const unsigned seqnum, const Message *msg);" << endl
			<< spacer << "// Message *generate_resend_request(const unsigned begin, const unsigned end=0);" << endl
			<< spacer << "// bool handle_sequence_reset(const unsigned seqnum, const Message *msg);" << endl
			<< spacer << "// Message *generate_sequence_reset(const unsigned newseqnum, const bool gapfillflag=false);" << endl
			<< spacer << "// bool handle_test_request(const unsigned seqnum, const Message *msg);" << endl
			<< spacer << "// Message *generate_test_request(const f8String& testReqID);" << endl
			<< spacer << "// bool handle_reject(const unsigned seqnum, const Message *msg);" << endl
			<< spacer << "// Message *generate_reject(const unsigned seqnum, const char *what);" << endl
			<< spacer << "// bool handle_admin(const unsigned seqnum, const Message *msg);" << endl
			<< spacer << "// void modify_outbound(Message *msg);" << endl
			<< spacer << "// bool authenticate(SessionID& id, const Message *msg);" << endl << endl;

		oss_hpp << spacer << "// Override these methods to intercept admin and application methods." << endl
				<< spacer << "// bool handle_admin(const unsigned seqnum, const FIX8::Message *msg);" << endl
				<< spacer << "bool handle_application(const unsigned seqnum, const FIX8::Message *msg)" << endl
				<< spacer << '{' << endl << spacer << spacer << "return enforce(seqnum, msg) || msg->process(_router);"
				<< endl << spacer << '}' << endl;

		oss_hpp << "};" << endl;

		oss_hpp << endl << "#endif // _" << flname(ctxt._out[Ctxt::session_hpp].first.second) << '_' << endl;
	}

// ================================= Field processing =====================================

	// output file preambles
	generate_preamble(ost_hpp);
	ost_hpp << "#ifndef _" << flname(ctxt._out[Ctxt::types_hpp].first.second) << '_' << endl;
	ost_hpp << "#define _" << flname(ctxt._out[Ctxt::types_hpp].first.second) << '_' << endl << endl;
	ost_hpp << _csMap.find_ref(cs_start_namespace) << endl;
	ost_hpp << "namespace " << ctxt._fixns << " {" << endl;

	ost_hpp << endl << _csMap.find_ref(cs_divider) << endl;
	generate_preamble(ost_cpp);
	ost_cpp << _csMap.find_ref(cs_generated_includes) << endl;
	ost_cpp << "#include \"" << ctxt._out[Ctxt::types_hpp].first.second << '"' << endl;
	ost_cpp << _csMap.find_ref(cs_divider) << endl;
	ost_cpp << _csMap.find_ref(cs_start_namespace) << endl;
	ost_cpp << "namespace " << ctxt._fixns << " {" << endl << endl;

	ost_cpp << _csMap.find_ref(cs_start_anon_namespace) << endl;
	ost_cpp << endl << _csMap.find_ref(cs_divider) << endl;
	// generate field types
	for (FieldSpecMap::const_iterator fitr(fspec.begin()); fitr != fspec.end(); ++fitr)
	{
		if (!gen_fields && !fitr->second._used)
			continue;
		if (!fitr->second._comment.empty())
			ost_hpp << "// " << fitr->second._comment << endl;
		ost_hpp << "typedef Field<" << FieldSpec::_typeToCPP.find_ref(fitr->second._ftype)
			<< ", " << fitr->first << "> " << fitr->second._name << ';' << endl;
		if (fitr->second._dvals)
			process_value_enums(fitr, ost_hpp, ost_cpp);
		ost_hpp << _csMap.find_ref(cs_divider) << endl;
	}

	// generate realmbase objs
	ost_cpp << endl << _csMap.find_ref(cs_divider) << endl;
	ost_cpp << "const RealmBase realmbases[] =" << endl << '{' << endl;
	unsigned dcnt(0);
	for (FieldSpecMap::iterator fitr(fspec.begin()); fitr != fspec.end(); ++fitr)
	{
		if ((!fitr->second._used && !gen_fields) || !fitr->second._dvals)
			continue;
		ost_cpp << spacer << "{ reinterpret_cast<const void *>(" << fitr->second._name << "_realm), "
			<< "static_cast<RealmBase::RealmType>(" << fitr->second._dtype << "), " << endl << spacer << spacer
			<< "static_cast<FieldTrait::FieldType>(" << fitr->second._ftype << "), "
			<< fitr->second._dvals->size() << ", " << fitr->second._name << "_descriptions }," << endl;
		fitr->second._doffset = dcnt++;
	}
	ost_cpp << "};" << endl;

	// generate field instantiators
	ost_cpp << endl << _csMap.find_ref(cs_divider) << endl;
	for (FieldSpecMap::const_iterator fitr(fspec.begin()); fitr != fspec.end(); ++fitr)
	{
		if (!gen_fields && !fitr->second._used)
			continue;
		ost_cpp << "BaseField *Create_" << fitr->second._name << "(const f8String& from, const RealmBase *db, const int rv)";
		ost_cpp << " // " << fitr->second._name << endl;
		if (fitr->second._dvals) // generate code to create a Field using a value taken from an index into a Realm
		{
			ost_cpp << spacer << "{ return !db || rv < 0 || rv >= db->_sz || db->_dtype != RealmBase::dt_set ? new "
				<< fitr->second._name << "(from, db) : new " << fitr->second._name << "(db->get_rlm_val<";
			if (FieldTrait::is_int(fitr->second._ftype))
				ost_cpp << "int";
			else if (FieldTrait::is_char(fitr->second._ftype))
				ost_cpp << "char";
			else if (FieldTrait::is_float(fitr->second._ftype))
				ost_cpp << "double";
			else if (FieldTrait::is_string(fitr->second._ftype))
				ost_cpp << "f8String";
			else
			{
				ost_cpp << "unknown";
				cerr << shortName << ": error: unknown FieldTrait::type in realm " << fitr->second._name << endl;
				++glob_errors;
			}
			ost_cpp << ">(rv), db); }" << endl;
		}
		else
			ost_cpp << spacer << "{ return new " << fitr->second._name << "(from, db); }" << endl;
	}

	ost_cpp << endl << _csMap.find_ref(cs_end_anon_namespace) << endl;
	ost_cpp << "} // namespace " << ctxt._fixns << endl;

	// generate field instantiator lookup
	ost_hpp << "typedef FieldTable " << ctxt._clname << "_BaseEntry;" << endl;

	ost_cpp << endl << _csMap.find_ref(cs_divider) << endl;
	ost_cpp << "template<>" << endl << "const " << ctxt._fixns << "::" << ctxt._clname << "_BaseEntry::Pair "
		<< ctxt._fixns << "::" << ctxt._clname << "_BaseEntry::_pairs[] =" << endl << '{' << endl;
	for (FieldSpecMap::const_iterator fitr(fspec.begin()); fitr != fspec.end(); ++fitr)
	{
		if (!gen_fields && !fitr->second._used)
			continue;
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
	ost_cpp << "template<>" << endl << "const " << ctxt._fixns << "::" << ctxt._clname << "_BaseEntry::NotFoundType "
		<< ctxt._fixns << "::" << ctxt._clname << "_BaseEntry::_noval = {0, 0};" << endl;

	// terminate files
	ost_hpp << endl << "} // namespace " << ctxt._fixns << endl;
	ost_hpp << _csMap.find_ref(cs_end_namespace) << endl;
	ost_hpp << "#endif // _" << flname(ctxt._out[Ctxt::types_hpp].first.second) << '_' << endl;
	ost_cpp << endl << _csMap.find_ref(cs_end_namespace) << endl;

	if (verbose)
	{
		unsigned cnt(0);
		for (FieldSpecMap::const_iterator fitr(fspec.begin()); fitr != fspec.end(); ++fitr)
			if (fitr->second._used)
				++cnt;
		cout << cnt << " of " << fspec.size() << " fields used in messages." << endl;
	}
	return result;
}

//-------------------------------------------------------------------------------------------------
void binary_report()
{
#if defined __GNUG__
#if defined __GNUC_MINOR__ && __GNUC_PATCHLEVEL__
	cout << "Compiled with gcc version " << __GNUG__ << '.' << __GNUC_MINOR__ << '.' <<__GNUC_PATCHLEVEL__ << endl;
#endif
	const size_t confbufsz(256);
	char confbuf[confbufsz];
	if (confstr(_CS_GNU_LIBC_VERSION, confbuf, confbufsz))
	{
		cout << "GNU glibc version is " << confbuf << endl;
	}
	if (confstr(_CS_GNU_LIBPTHREAD_VERSION, confbuf, confbufsz))
	{
		cout << "GNU libpthread version is " << confbuf << endl;
	}
#if defined __GXX_ABI_VERSION
	cout << "GXX ABI version is " <<  __GXX_ABI_VERSION << endl;
#endif
#else
	cout << "GCC not used. No information available." << endl;
#endif
}

