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
/** \file f8c.cpp
\n
  This is the fix8 compiler.\n
\n
f8c -- compile FIX xml schema\n
\n
<tt>
Usage: f8c [-CINPRVWbcdfhiknoprstvx] \<input xml schema\>\n
   -C,--nocheck            do not embed version checking in generated code (default false)\n
   -I,--info               print package info, exit\n
   -N,--nounique           do not enforce unique field parsing (default false)\n
   -P,--incpath            prefix system include path with "fix8" in generated compilation units (default yes)\n
   -R,--norealm            do not generate realm constructed field instantiators (default false)\n
   -W,--nowarn             suppress warning messages (default false)\n
   -V,--verbose            be more verbose when processing\n
   -c,--classes \<server|client\> generate user session classes (default no)\n
   -d,--dump               dump 1st pass parsed source xml file, exit\n
   -f,--fields             generate code for all defined fields even if they are not used in any message (default no)\n
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
   f8c -Vp Texfix -n TEX myfix.xml\n
   f8c -Vrp Texfix -n TEX -x ../schema/FIXT11.xml myfix.xml\n
   f8c -Vp Texfix -n TEX -c client -x ../schema/FIXT11.xml myfix.xml\n
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

#include <errno.h>
#include <string.h>
#include <cctype>

// f8 headers
#include <fix8/f8includes.hpp>
#include <f8c.hpp>

#ifdef HAVE_GETOPT_H
#include <getopt.h>
#endif

//-----------------------------------------------------------------------------------------
using namespace std;
using namespace FIX8;

//-----------------------------------------------------------------------------------------
const string Ctxt::_exts[count] = { "_types.cpp", "_types.hpp", "_traits.cpp", "_classes.cpp",
	"_classes.hpp", "_router.hpp", "_session.hpp" };

string precompFile, spacer, inputFile, shortName, fixt, shortNameFixt, odir("./"), prefix("Myfix"), gen_classes;
bool verbose(false), error_ignore(false), gen_fields(false), norealm(false), nocheck(false), nowarn(false), incpath(true);
unsigned glob_errors(0), glob_warnings(0), tabsize(3);
extern unsigned glob_errors;
extern const string GETARGLIST("hvVo:p:dikn:rst:x:NRc:fbCIWP");
extern string spacer, shortName;

//-----------------------------------------------------------------------------------------
// static data
#include <f8cstatic.hpp>
extern const CSMap _csMap;
const CSMap _csMap(cs_valueTable, sizeof(cs_valueTable)/sizeof(CSMap::TypePair), "not found");

//-----------------------------------------------------------------------------------------
void print_usage();
string insert_year();
int process(XmlElement& xf, Ctxt& ctxt);
int load_fix_version (XmlElement& xf, Ctxt& ctxt);
int load_fields(XmlElement& xf, FieldSpecMap& fspec);
void process_special_traits(const unsigned short field, FieldTraits& fts);
int process_message_fields(const std::string& where, const XmlElement *xt, FieldTraits& fts,
	const FieldToNumMap& ftonSpec, FieldSpecMap& fspec, const Components& compon);
int load_messages(XmlElement& xf, MessageSpecMap& mspec, const FieldToNumMap& ftonSpec,
	FieldSpecMap& fspec, Components& compon, CommonGroupMap& globmap);
void process_ordering(MessageSpecMap& mspec);
ostream *open_ofile(const string& odir, const string& fname, string& target);
void process_value_enums(FieldSpecMap::const_iterator itr, ostream& ost_hpp, ostream& ost_cpp);
const string& mkel(const string& base, const string& compon, string& where);
const string& filepart(const string& source, string& where);
void generate_preamble(ostream& to, const string& fname, bool donotedit=true);
void generate_includes(ostream& to);
unsigned parse_groups(MessageSpec& ritr, const string& name,
	const FieldToNumMap& ftonSpec, FieldSpecMap& fspec, XmlElement::XmlSet& grplist,
   const Components& compon, CommonGroupMap& globmap);
int precomp(XmlElement& xf, ostream& outf);
void process_group_ordering(const CommonGroupMap& gm);
int precompfixt(XmlElement& xft, XmlElement& xf, ostream& outf, bool nounique);
void generate_group_bodies(const MessageSpec& ms, const FieldSpecMap& fspec, int depth,
	const string& msname, ostream& outp, ostream& outh, const CommonGroupMap& globmap, const string cls_prefix=string());
void generate_common_group_bodies(const FieldSpecMap& fspec, ostream& outp, CommonGroupMap& globmap);
void load_components(const XmlElement::XmlSet& comlist, Components& components);
unsigned lookup_component(const Components& compon, const f8String& name);
void binary_report();
string bintoaschex(const string& from);
uint32_t group_hash(const MessageSpec& p1);
const MessageSpec *find_group(const CommonGroupMap& globmap, int& vers, unsigned tp, uint32_t key);
void generate_group_traits(const MessageSpec& ms, const string& gname, const string& prefix, ostream& outp);

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
		{ "norealm",		0,	0,	'R' },
		{ "incpath",		0,	0,	'P' },
		{ "nowarn",		   0,	0,	'W' },
		{ "odir",			1,	0,	'o' },
		{ "dump",			0,	0,	'd' },
		{ "ignore",			0,	0,	'i' },
		{ "nocheck",		0,	0,	'C' },
		{ "info",		   0,	0,	'I' },
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
			cout << "f8c for " PACKAGE " version " VERSION << endl;
			cout << _csMap.find_ref(cs_copyright_short) << insert_year()
				  << _csMap.find_ref(cs_copyright_short2) << endl;
			cout << "Released under the GNU LESSER GENERAL PUBLIC LICENSE, Version 3. See <http://fsf.org/> for details." << endl;
			return 0;
		case 'I':
			cout << "Package info for " PACKAGE " version " VERSION << endl;
         cout << "MAGIC_NUM: " << MAGIC_NUM << endl;
         cout << "CONFIGURE_OPTIONS:" << CONFIGURE_OPTIONS << endl;
         cout << "CPPFLAGS:" << CPPFLAGS << endl;
         cout << "LIBS:" << LIBS << endl;
         cout << "LDFLAGS:" << LDFLAGS << endl;
         cout << "CONFIGURE_SDATE: " << CONFIGURE_SDATE << endl;
         cout << "CONFIGURE_TIME: " << CONFIGURE_TIME << endl;
         cout << "CONFIGURE_TIME_NUM: " << CONFIGURE_TIME_NUM << endl;
         cout << "HOST_SYSTEM: " << HOST_SYSTEM << endl;
         cout << "MAX_FLD_LENGTH: " << MAX_FLD_LENGTH << endl;
         cout << "MAX_MSG_LENGTH: " << MAX_MSG_LENGTH << endl;
         cout << "MPMC_FF: " << MPMC_FF << endl;
         cout << "MPMC_TBB: " << MPMC_TBB << endl;
         cout << "MPMC_SYSTEM: " << MPMC_SYSTEM << endl;
#if defined SLEEP_NO_YIELD
         cout << "SLEEP_NO_YIELD: " << SLEEP_NO_YIELD << endl;
#endif
#if defined CODECTIMING
         cout << "CODECTIMING: " << CODECTIMING << endl;
#endif
#if defined HAVE_OPENSSL
         cout << "HAVE_OPENSSL: " << HAVE_OPENSSL << endl;
#endif
			return 0;
		case 'V': verbose = true; break;
		case 'f': gen_fields = true; break;
		case 'N': nounique = true; break;
		case 'R': norealm = true; break;
		case 'C': nocheck = true; break;
		case 'c': gen_classes = optarg; break;
		case 'h': print_usage(); return 0;
		case ':': case '?': return 1;
		case 'o': CheckAddTrailingSlash(odir = optarg); break;
		case 'd': dump = true; break;
		case 'i': error_ignore = true; break;
		case 'P': incpath = false; break;
		case 'k': keep_failed = true; break;
		case 'r': retain_precomp = true; break;
		case 's': second_only = true; break;
		case 't': tabsize = get_value<unsigned>(optarg); break;
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
		if (!pre_out.get())
		{
			cerr << endl << "Error opening: " << odir << '/' << shortName;
			if	(errno)
				cerr << " (" << strerror(errno) << ')';
			cerr << endl;
			return 1;
		}
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
		ctxt._out[ii].first.first = ctxt._out[ii].first.second + ".p2";
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
				try
				{
					push_dir pd(odir);
					string backup(ctxt._out[ii].first.second + ".old");
					remove(backup.c_str());
					rename(ctxt._out[ii].first.second.c_str(), backup.c_str());
					if (rename(ctxt._out[ii].first.first.c_str(), ctxt._out[ii].first.second.c_str()))
					{
						cerr << "Error renaming files \'" << ctxt._out[ii].first.first << "' to '" <<  ctxt._out[ii].first.second;
						if	(errno)
							cerr << " (" << strerror(errno) << ')';
						cerr << endl;
					}

					if (gen_classes.empty())
						remove(ctxt._out[Ctxt::session_hpp].first.second.c_str());
				}
				catch (f8Exception& e)
				{
					cerr << "exception: " << e.what() << endl;
				}
			}
		}
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
				result = fspec.insert(FieldSpecMap::value_type(get_value<unsigned>(number), FieldSpec(name, ft)));
			else
			{
            if (!nowarn)
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
						cerr << shortName << ':' << recover_line(**itr) << ": error: field value element missing required attribute (enum)." << endl;
						++glob_errors;
					}
				}
			}
		}
		else
		{
			cerr << shortName << ':' << recover_line(**itr) << ": error: field definition element missing required attributes (number, name or type)" << endl;
			++glob_errors;
		}
	}

	return fieldsLoaded;
}

//-----------------------------------------------------------------------------------------
int load_messages(XmlElement& xf, MessageSpecMap& mspec, const FieldToNumMap& ftonSpec,
	FieldSpecMap& fspec, Components& compon, CommonGroupMap& globmap)
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
		cerr << "error: Could not locate MsgType realm defintions in '" << shortName
			<< "'. See FAQ for more details." << endl;
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
				cerr << shortName << ':' << recover_line(**itr) << ": error: Message '"
				  << name << "' does not have corrresponding entry in MsgType field realm. See FAQ for more details." << endl;
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
			cerr << shortName << ':' << recover_line(**itr) << ": error: Could not add message '" << name << "' (" << msgtype << ")" << endl;
			++glob_errors;
			continue;
		}

		string elpart;
		XmlElement::XmlSet grplist;
		if ((*itr)->find(mkel(elname, "group", elpart), grplist))
			grpsparsed += parse_groups(result.first->second, name, ftonSpec, fspec, grplist, compon, globmap);

		(*itr)->GetAttr("comment", result.first->second._comment);

		process_message_fields(mkel(elname, "field", elpart), *itr, result.first->second._fields, ftonSpec, fspec, compon);

		++msgssLoaded;
	}

	if (verbose)
		cout << grpsparsed << " repeating groups defined" << endl;

	return msgssLoaded;
}

//-----------------------------------------------------------------------------------------
unsigned parse_groups(MessageSpec& ritr, const string& name,
	const FieldToNumMap& ftonSpec, FieldSpecMap& fspec, XmlElement::XmlSet& grplist,
   const Components& compon, CommonGroupMap& globmap)
{
	unsigned result(0);

	for(XmlElement::XmlSet::const_iterator gitr(grplist.begin()); gitr != grplist.end(); ++gitr)
	{
		string gname, required;
		if ((*gitr)->GetAttr("name", gname) && (*gitr)->GetAttr("required", required))
		{
			string compname;
			unsigned compidx((*gitr)->GetAttr("component", compname) ? lookup_component(compon, compname) : 0);

			// add group FieldTrait
			FieldToNumMap::const_iterator ftonItr(ftonSpec.find(gname));
			FieldSpecMap::const_iterator fs_itr;
			if (ftonItr != ftonSpec.end() && (fs_itr = fspec.find(ftonItr->second)) != fspec.end())
			{
				if (!ritr._fields.add(FieldTrait(fs_itr->first, FieldTrait::ft_int, (*gitr)->GetSubIdx(),
					required == "Y", true, compidx)))
				{
					if (!nowarn)
                  cerr << "warning: Could not add group trait object '" << gname << "' (duplicate ?)" << endl;
					++glob_warnings;
				}
				else
				{
					fs_itr->second._used = true; 	// we always assume group count fields are used
					pair<GroupMap::iterator, bool> gresult(
						ritr._groups.insert(GroupMap::value_type(fs_itr->first, MessageSpec(gname))));
               if (gresult.second)
               {
                  process_message_fields("group/field", *gitr, gresult.first->second._fields, ftonSpec, fspec, compon);
                  XmlElement::XmlSet comlist;
                  ++result;
                  if ((*gitr)->find("group/group", comlist))
                     result += parse_groups(gresult.first->second, gname, ftonSpec, fspec, comlist, compon, globmap);
                  CommonGroupMap::iterator cgitr(globmap.find(fs_itr->first));
                  if (cgitr == globmap.end())
                     cgitr = globmap.insert(make_pair(fs_itr->first, CommonGroups())).first;
                  const uint32_t hv(group_hash(gresult.first->second));
                  gresult.first->second._hash = hv;
                  cgitr->second.insert(make_pair(hv, gresult.first->second));
                  CommonGroups::iterator cghitr(cgitr->second.find(hv));
                  if (cghitr != cgitr->second.end())
                  {
                     cghitr->second._group_refcnt++;
                     cghitr->second._hash = hv;
                  }
               }
               else
               {
                  if (!nowarn)
                     cerr << "warning: Could not add group map '" << fs_itr->first << "' (duplicate ?)" << endl;
                  ++glob_warnings;
               }
				}
			}
			else
			{
				cerr << shortName << ':' << recover_line(**gitr)
					<< ": error: Could not locate group Field '" << gname << "' from known field types in " << shortName << endl;
				++glob_errors;
				continue;
			}
		}
		else
		{
			cerr << shortName << ':' << recover_line(**gitr) << ": error: group element missing required attributes (name or required)" << endl;
			++glob_errors;
		}
	}
   ritr._hash = group_hash(ritr);

	return result;
}

//-----------------------------------------------------------------------------------------
void generate_group_bodies(const MessageSpec& ms, const FieldSpecMap& fspec, int depth, const string& msname,
	ostream& outp, ostream& outh, const CommonGroupMap& globmap, const string cls_prefix)
{
	const string dspacer(depth * tabsize, ' '), d2spacer((depth + 1) * tabsize, ' ');

	string prefix;
	if (!cls_prefix.empty())
		prefix = cls_prefix + "::";

	for (GroupMap::const_iterator gitr(ms._groups.begin()); gitr != ms._groups.end(); ++gitr)
	{
		FieldSpecMap::const_iterator gsitr(fspec.find(gitr->first));
		outp << _csMap.find_ref(cs_divider) << endl;

      int vers(0);
      const MessageSpec *tgroup (find_group(globmap, vers, gitr->first, gitr->second._hash));
      if (!tgroup)
      {
         cout << gitr->first << " not found" << endl;
         continue;
      }

      ostringstream rnme;
      rnme << gsitr->second._name;
      if (tgroup->_group_refcnt > 1)
         rnme << 'V' << vers;

      if (tgroup->_group_refcnt > 1)
         outp << "const FieldTrait *" << prefix << ms._name << "::" << gsitr->second._name << "::_traits("
            << rnme.str() << "_traits);" << endl;
      else
         generate_group_traits(*tgroup, rnme.str(), prefix + ms._name + "::", outp);

      if (tgroup->_group_refcnt > 1)
      {
         outp << "const FieldTrait_Hash_Array" << "& " << prefix << ms._name << "::" << gsitr->second._name << "::_ftha("
            << rnme.str() << "_ftha);" << endl;
         outp << "const MsgType" << "& " << prefix << ms._name << "::" << gsitr->second._name << "::_msgtype("
            << rnme.str() << "_msgtype);" << endl;
      }
      else
         outp << "const MsgType " << prefix << ms._name << "::" << gsitr->second._name << "::_msgtype("
            << '"' << gsitr->second._name << "\");" << endl;

#ifdef _MSC_VER
		outp << "#ifndef _MSC_EXTENSIONS" << endl;
#endif
		outp << "const unsigned short " << prefix << ms._name << "::" << gsitr->second._name << "::_fnum;" << endl;
#ifdef _MSC_VER
		outp << "#endif // _MSC_EXTENSIONS" << endl;
#endif

		// nested class decl.
		outh << endl << dspacer << "/// " << tgroup->_name << " (" << gitr->first << "), "
			<< (tgroup->_is_admin ? "admin" : "application") << ", " <<  tgroup->_fields.get_presence().size()
			<< " fiel" << (tgroup->_fields.get_presence().size() == 1 ? "d, " : "ds, ")
			<< tgroup->_groups.size() << " grou" << (tgroup->_groups.size() == 1 ? "p, " : "ps, ")
         << (tgroup->_group_refcnt > 1 ? "shares static data" : "is unique") << ", hash: 0x"
         << hex << tgroup->_hash << dec << endl;
		outh << dspacer << "// " << prefix << ms._name << "::" << gsitr->second._name << endl;
		outh << dspacer << "class " << gsitr->second._name
			<< " : public GroupBase // depth: " << depth << endl << dspacer << '{' << endl;
      if (tgroup->_group_refcnt > 1)
      {
         outh << d2spacer << "static const FieldTrait *_traits;" << endl;
         outh << d2spacer << "static const FieldTrait_Hash_Array& _ftha;" << endl;
         outh << d2spacer << "static const MsgType& _msgtype;" << endl << endl;
      }
      else
      {
         outh << d2spacer << "static const FieldTrait _traits[];" << endl;
         outh << d2spacer << "static const FieldTrait_Hash_Array _ftha;" << endl;
         outh << d2spacer << "static const MsgType _msgtype;" << endl << endl;
      }
		outh << dspacer << "public:" << endl;
		outh << d2spacer << "static const unsigned short _fnum = " << gsitr->first << ';' << endl << endl;
		outh << d2spacer << gsitr->second._name << "() : GroupBase(_fnum) {}" << endl;
		outh << d2spacer << "~" << gsitr->second._name << "() {}" << endl;
		if (tgroup->_groups.empty())
			outh << d2spacer << "MessageBase *create_group() const { return new MessageBase(ctx(), _msgtype(), _traits, "
				<< tgroup->_fields.get_presence().size()
				<< ", &_ftha); }" << endl;
		else
		{
			outh << d2spacer << "MessageBase *create_group() const" << endl << d2spacer << '{' << endl;
			outh << d2spacer << spacer << "MessageBase *mb(new MessageBase(ctx(), _msgtype(), _traits, "
				<< tgroup->_fields.get_presence().size()
				<< ", &_ftha));" << endl;
			for (GroupMap::const_iterator gsitr(tgroup->_groups.begin()); gsitr != tgroup->_groups.end(); ++gsitr)
				outh << d2spacer << spacer << "mb->append_group(new " << gsitr->second._name << "); // "
					<< gsitr->first << endl;
			outh << d2spacer << spacer << "return mb;" << endl;
			outh << d2spacer << '}' << endl;
		}
		outh << endl << d2spacer << "static const " << msname << "& get_msgtype() { return _msgtype; }" << endl;

		// process nested groups
		if (!tgroup->_groups.empty())
			generate_group_bodies(*tgroup, fspec, depth + 1, msname, outp, outh, globmap, prefix + ms._name);

		outh << dspacer << "};" << endl;
	}
}

//-----------------------------------------------------------------------------------------
void generate_group_traits(const MessageSpec& ms, const string& gname, const string& prefix, ostream& outp)
{
   if (prefix.empty())
      outp << "const FieldTrait " << gname << "_traits[] ="
         << " // refs:" << ms._group_refcnt << endl << '{' << endl;
   else
      outp << "const FieldTrait " << prefix << gname << "::_traits[] =" << endl << '{' << endl;
   for (Presence::const_iterator flitr(ms._fields.get_presence().begin());
      flitr != ms._fields.get_presence().end(); ++flitr)
   {
      bool spaceme(true);
      if (flitr != ms._fields.get_presence().begin())
      {
         outp << ',';
         if (Presence::distance(ms._fields.get_presence().begin(), flitr) % 3 == 0)
            outp << endl;
         else
            spaceme = false;
      }

      ostringstream tostr;
      tostr << "0x" << setfill('0') << setw(2) << hex << flitr->_field_traits.get();
      outp << (spaceme ? spacer : " ");
      outp << "FieldTrait(" << setw(4) << right << flitr->_fnum << ',' << setw(2)
         << right << flitr->_ftype << ',' << setw(3) << right << flitr->_pos <<
         ',' << setw(3) << right << flitr->_component << ',' << tostr.str() << ')';
   }
   outp << endl << "};" << endl;
   if (prefix.empty())
   {
      outp << "const FieldTrait_Hash_Array " << gname << "_ftha(" << gname << "_traits, "
         << ms._fields.get_presence().size() << ");" << endl;
      outp << "const MsgType " << gname << "_msgtype(\"" << gname << "\");" << endl;
   }
   else
      outp << "const FieldTrait_Hash_Array " << endl << spacer << prefix << gname
         << "::_ftha(" << prefix << gname << "::_traits, " << ms._fields.get_presence().size() << ");" << endl;
}

//-----------------------------------------------------------------------------------------
void generate_common_group_bodies(const FieldSpecMap& fspec, ostream& outp, CommonGroupMap& globmap)
{
	for (CommonGroupMap::const_iterator gcitr(globmap.begin()); gcitr != globmap.end(); ++gcitr)
	{
      int vers(1);
      for (CommonGroups::const_iterator gitr(gcitr->second.begin()); gitr != gcitr->second.end(); ++gitr, ++vers)
      {
         if (gitr->second._group_refcnt <= 1)   // if there is only one variant, don't generate code in common section
            continue;
         FieldSpecMap::const_iterator gsitr(fspec.find(gcitr->first));
         outp << _csMap.find_ref(cs_divider) << endl;
         ostringstream gostr;
         gostr << gsitr->second._name << 'V' << vers;

         generate_group_traits(gitr->second, gostr.str(), string(), outp);
      }
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
		cout << fields << " fields defined" << endl;

// ================================= Message processing ===================================

	FieldToNumMap ftonSpec;
	for (FieldSpecMap::const_iterator fs_itr(fspec.begin()); fs_itr != fspec.end(); ++fs_itr)
		ftonSpec.insert(FieldToNumMap::value_type(fs_itr->second._name, fs_itr->first));

	XmlElement::XmlSet comlist;
	Components components;
	xf.find("fix/components/component", comlist);
	load_components(comlist, components);

	MessageSpecMap mspec;
   CommonGroupMap globmap;

	int msgsloaded(load_messages(xf, mspec, ftonSpec, fspec, components, globmap));
	if (!msgsloaded)
		return result;
	if (verbose)
	{
		cout << msgsloaded << " messages defined" << endl;
		if (components.size())
			cout << components.size() << " components defined" << endl;
		if (globmap.size())
      {
         unsigned cgs(0), ugs(0), vars(0);
         for (CommonGroupMap::const_iterator gcitr(globmap.begin()); gcitr != globmap.end(); ++gcitr)
         {
            vars += gcitr->second.size();
            for (CommonGroups::const_iterator gitr(gcitr->second.begin()); gitr != gcitr->second.end(); ++gitr)
            {
               if (gitr->second._group_refcnt > 1)
                  ugs += gitr->second._group_refcnt;
               else
                  ++cgs;
            }
         }
         cout << globmap.size() << " repeating group catagories (" << vars << " variants, "
            << ugs << " common, " << cgs << " unique) processed" << endl;
      }
	}

	process_ordering(mspec);
   process_group_ordering(globmap);

	// output file preambles
	generate_preamble(osu_hpp, ctxt._out[Ctxt::router_hpp].first.second);
	osu_hpp << "#ifndef " << bintoaschex(ctxt._out[Ctxt::router_hpp].first.second) << endl;
	osu_hpp << "#define " << bintoaschex(ctxt._out[Ctxt::router_hpp].first.second) << endl << endl;
	osu_hpp << _csMap.find_ref(cs_start_namespace) << endl;
	osu_hpp << "namespace " << ctxt._fixns << " {" << endl;
	osu_hpp << endl << _csMap.find_ref(cs_divider) << endl;

	generate_preamble(osc_hpp, ctxt._out[Ctxt::classes_hpp].first.second);
	osc_hpp << "#ifndef " << bintoaschex(ctxt._out[Ctxt::classes_hpp].first.second) << endl;
	osc_hpp << "#define " << bintoaschex(ctxt._out[Ctxt::classes_hpp].first.second) << endl << endl;
	osc_hpp << _csMap.find_ref(cs_start_namespace) << endl;
	osc_hpp << "namespace " << ctxt._fixns << " {" << endl;

	osc_hpp << endl << _csMap.find_ref(cs_divider) << endl;
	osc_hpp << "typedef MsgTable " << ctxt._clname << "_BaseMsgEntry;" << endl;
	osc_hpp << "/// Compiler generated metadata object, accessed through this function" << endl;
	osc_hpp << "const F8MetaCntx& ctx();" << endl;
	osc_hpp << "class " << ctxt._clname << "_Router;" << endl;
	osc_hpp << endl << _csMap.find_ref(cs_divider) << endl;

	generate_preamble(osc_cpp, ctxt._out[Ctxt::classes_cpp].first.second);
	osc_cpp << _csMap.find_ref(cs_generated_includes) << endl;
   generate_includes(osc_cpp);
	osc_cpp << "#include \"" << ctxt._out[Ctxt::types_hpp].first.second << '"' << endl;
	osc_cpp << "#include \"" << ctxt._out[Ctxt::router_hpp].first.second << '"' << endl;
	osc_cpp << "#include \"" << ctxt._out[Ctxt::classes_hpp].first.second << '"' << endl;
	osc_cpp << _csMap.find_ref(cs_divider) << endl;
	osc_cpp << _csMap.find_ref(cs_start_namespace) << endl;
	osc_cpp << "namespace " << ctxt._fixns << " {" << endl << endl;
	osc_cpp << _csMap.find_ref(cs_start_anon_namespace) << endl << endl;
	osc_cpp << _csMap.find_ref(cs_divider) << endl;

	generate_preamble(osr_cpp, ctxt._out[Ctxt::traits_cpp].first.second);
	osr_cpp << _csMap.find_ref(cs_generated_includes) << endl;
   generate_includes(osr_cpp);
	osr_cpp << "#include \"" << ctxt._out[Ctxt::types_hpp].first.second << '"' << endl;
	osr_cpp << "#include \"" << ctxt._out[Ctxt::router_hpp].first.second << '"' << endl;
	osr_cpp << "#include \"" << ctxt._out[Ctxt::classes_hpp].first.second << '"' << endl;
	osr_cpp << _csMap.find_ref(cs_divider) << endl;
	osr_cpp << _csMap.find_ref(cs_start_namespace) << endl;
	osr_cpp << "namespace " << ctxt._fixns << " {" << endl << endl;

	osr_cpp << _csMap.find_ref(cs_divider) << endl;

// =============================== Message class definitions ==============================

	osr_cpp << "// Common group traits" << endl;
	osr_cpp << "namespace {" << endl;
	generate_common_group_bodies(fspec, osr_cpp, globmap);
	osr_cpp << "} // namespace" << endl << endl;
	osr_cpp << _csMap.find_ref(cs_divider) << endl;
	osr_cpp << "// Message traits" << endl;

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
				tostr << "0x" << setfill('0') << setw(2) << hex << flitr->_field_traits.get();
				osr_cpp << (spaceme ? spacer : " ");
				osr_cpp << "FieldTrait(" << setw(4) << right << flitr->_fnum << ','
					<< setw(2) << right << flitr->_ftype << ',' << setw(3) << right
					<< flitr->_pos << ',' << setw(3) << right << flitr->_component << ',' << tostr.str() << ')';
			}
			osr_cpp << endl << "};" << endl;
			osr_cpp << "const FieldTrait_Hash_Array " << mitr->second._name << "::_ftha(" << mitr->second._name << "::_traits, "
				<< mitr->second._fields.get_presence().size() << ");" << endl;
			osr_cpp << "const MsgType " << mitr->second._name << "::_msgtype(\"" << mitr->first << "\");" << endl;
			osc_hpp << spacer << "static const FieldTrait _traits[];" << endl;
			osc_hpp << spacer << "static const FieldTrait_Hash_Array _ftha;" << endl;
			osc_hpp << spacer << "static const MsgType _msgtype;" << endl;
		}

		if (isHeader)
		{
			osc_hpp << endl << spacer << "begin_string *_begin_string;" << endl;
			osc_hpp << spacer << "body_length *_body_length;" << endl;
			osc_hpp << spacer << "msg_type *_msg_type;" << endl;
		}
		else if (isTrailer)
			osc_hpp << endl << spacer << "check_sum *_check_sum;" << endl;

		osc_hpp << endl;

		osc_hpp << "public:" << endl;
		osc_hpp << spacer << mitr->second._name << "()";
		if (mitr->second._fields.get_presence().size())
			osc_hpp << " : " << (isTrailer || isHeader ? "MessageBase" : "Message")
				<< "(ctx(), _msgtype(), _traits, " << mitr->second._fields.get_presence().size()
				<< ", &_ftha)";
		if (isHeader || isTrailer)
		{
			osc_hpp << ',' << endl << spacer << spacer;
			if (isHeader)
				osc_hpp << "_begin_string(new begin_string(ctx()._beginStr)), _body_length(new body_length), _msg_type(new msg_type)";
			else
				osc_hpp << "_check_sum(new check_sum)";
			osc_hpp << " { add_preamble(); }" << endl;
		}
		else if (!mitr->second._groups.empty())
		{
			osc_hpp << endl << spacer << '{' << endl;
			for (GroupMap::const_iterator gitr(mitr->second._groups.begin()); gitr != mitr->second._groups.end(); ++gitr)
			{
				FieldSpecMap::const_iterator gsitr(fspec.find(gitr->first));
				osc_hpp << spacer << spacer;
				osc_hpp << "_groups.insert(";
				if (gitr != mitr->second._groups.begin())
					osc_hpp << "_groups.end(), ";
				osc_hpp << "Groups::value_type(" << gsitr->first << ", new " << gsitr->second._name << "));" << endl;
			}
			osc_hpp << spacer << '}' << endl;
		}
		else
			osc_hpp << " {}" << endl;

		osc_hpp << spacer << "~" << mitr->second._name << "() {}" << endl;
		if (!isHeader && !isTrailer)
		{
			osc_hpp << spacer << "bool process(Router& rt) const { return (static_cast<"
				<< ctxt._clname << "_Router&>(rt))(this); }" << endl;
			if (mitr->second._is_admin)
				osc_hpp << spacer << "bool is_admin() const { return true; }" << endl;
		}

		osc_hpp << endl << spacer << "static const " << fsitr->second._name << "& get_msgtype() { return _msgtype; }" << endl;
		if (isHeader)
			osc_hpp << endl << _csMap.find_ref(cs_header_preamble) << endl;
		else if (isTrailer)
			osc_hpp << endl << _csMap.find_ref(cs_trailer_preamble) << endl;

// =============================== Repeating group nested classes ==============================

		generate_group_bodies(mitr->second, fspec, 1, fsitr->second._name, osr_cpp, osc_hpp, globmap);

		osc_hpp << "};" << endl << endl;
		osc_hpp << _csMap.find_ref(cs_divider) << endl;
	}

// =============================== Message class instantiation ==============================

	osc_cpp << endl;

	osc_cpp << "const char *cn[] = // Component names" << endl << '{' << endl;
	osc_cpp << spacer << "\"\"," << endl;
	for (Components::iterator citr(components.begin()); citr != components.end(); ++citr)
		osc_cpp << spacer << '"' << citr->first << "\", // " << (1 + distance(components.begin(), citr)) << endl;
	osc_cpp << "};" << endl;

	osc_cpp << endl << _csMap.find_ref(cs_end_anon_namespace) << endl;

	osc_cpp << endl << _csMap.find_ref(cs_divider) << endl;
	osc_cpp << "const " << ctxt._fixns << "::" << ctxt._clname << "_BaseMsgEntry::Pair "
		<< "msgpairs[] =" << endl << '{' << endl;
	for (MessageSpecMap::const_iterator mitr(mspec.begin()); mitr != mspec.end(); ++mitr)
	{
		if (mitr != mspec.begin())
			osc_cpp << ',' << endl;
		osc_cpp << spacer << "{ \"" << mitr->first << "\", { ";
		if (mitr->second._name == "trailer" || mitr->second._name == "header")
         osc_cpp << "Minst(Type2Types<" << ctxt._fixns << "::" << mitr->second._name << ", bool>())";
      else
         osc_cpp << "Minst(Type2Type<" << ctxt._fixns << "::" << mitr->second._name << ">())";
		osc_cpp << ", \"" << mitr->second._name << '"';
		if (!mitr->second._comment.empty())
			osc_cpp << ',' << endl << spacer << spacer << '"' << mitr->second._comment << "\" }";
		else
			osc_cpp << ", 0 }";
		osc_cpp << " }";
	}
	osc_cpp << endl << "}; // " << mspec.size() << endl;

   size_t fields_generated(0);
	for (FieldSpecMap::const_iterator fitr(fspec.begin()); fitr != fspec.end(); ++fitr)
		if (gen_fields || fitr->second._used)
         ++fields_generated;

	osc_cpp << endl << "extern const " << ctxt._clname << "_BaseEntry::Pair fldpairs[];" << endl << endl
      << "/// Compiler generated metadata object, accessed through this function" << endl
      << "const F8MetaCntx& ctx() // avoid SIOF" << endl << '{' << endl
      << spacer << "static const BaseMsgEntry nvbme = { Minst(Type2Type<void *>()) };" << endl
      << spacer << "static const " << ctxt._clname << "_BaseMsgEntry "
         << "bme(msgpairs, " << mspec.size() << ", nvbme);" << endl
      << spacer << "static const BaseEntry nvbe = { Inst(Type2Type<void *>()) };" << endl
      << spacer << "static const " << ctxt._clname << "_BaseEntry "
         << "be(fldpairs, " << fields_generated << ", nvbe);" << endl
      << spacer << "static const F8MetaCntx _ctx(" << ctxt._version << ", bme, be, cn, \"" << ctxt._beginstr << "\");" << endl
      << spacer << "return _ctx;" << endl << '}' << endl;
	osc_cpp << endl << "} // namespace " << ctxt._fixns << endl;

// ==================================== Message router ==================================

	osu_hpp << "class " << ctxt._clname << "_Router : public Router" << endl
		<< '{' << endl << "public:" << endl;
	osu_hpp << spacer << ctxt._clname << "_Router() {}" << endl;
	osu_hpp << spacer << "virtual ~" << ctxt._clname << "_Router() {}" << endl << endl;
        osu_hpp << spacer << "virtual bool operator() (const class Message *msg) const { return false; }" << endl;
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
	osc_hpp << "#endif // " << bintoaschex(ctxt._out[Ctxt::classes_hpp].first.second) << endl;
	osu_hpp << endl << "} // namespace " << ctxt._fixns << endl;
	osu_hpp << _csMap.find_ref(cs_end_namespace) << endl;
	osu_hpp << "#endif // " << bintoaschex(ctxt._out[Ctxt::router_hpp].first.second) << endl;
	osr_cpp << endl << "} // namespace " << ctxt._fixns << endl;
	osr_cpp << _csMap.find_ref(cs_end_namespace) << endl;
	osc_cpp << _csMap.find_ref(cs_end_namespace) << endl;
	osc_cpp << endl;

// =============================== Generate optional user session and router ==============================

	if (!gen_classes.empty())
	{
		bool is_server(gen_classes == "server");
		generate_preamble(oss_hpp, ctxt._out[Ctxt::session_hpp].first.second, false);
		oss_hpp << "#ifndef " << bintoaschex(ctxt._out[Ctxt::session_hpp].first.second) << endl;
		oss_hpp << "#define " << bintoaschex(ctxt._out[Ctxt::session_hpp].first.second) << endl;
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
			<< spacer << "// bool handle_logon(const unsigned seqnum, const FIX8::Message *msg);" << endl
			<< spacer << "// Message *generate_logon(const unsigned heartbeat_interval, const f8String davi=f8String());" << endl
			<< spacer << "// bool handle_logout(const unsigned seqnum, const FIX8::Message *msg);" << endl
			<< spacer << "// Message *generate_logout();" << endl
			<< spacer << "// bool handle_heartbeat(const unsigned seqnum, const FIX8::Message *msg);" << endl
			<< spacer << "// Message *generate_heartbeat(const f8String& testReqID);" << endl
			<< spacer << "// bool handle_resend_request(const unsigned seqnum, const FIX8::Message *msg);" << endl
			<< spacer << "// Message *generate_resend_request(const unsigned begin, const unsigned end=0);" << endl
			<< spacer << "// bool handle_sequence_reset(const unsigned seqnum, const FIX8::Message *msg);" << endl
			<< spacer << "// Message *generate_sequence_reset(const unsigned newseqnum, const bool gapfillflag=false);" << endl
			<< spacer << "// bool handle_test_request(const unsigned seqnum, const FIX8::Message *msg);" << endl
			<< spacer << "// Message *generate_test_request(const f8String& testReqID);" << endl
			<< spacer << "// bool handle_reject(const unsigned seqnum, const FIX8::Message *msg);" << endl
			<< spacer << "// Message *generate_reject(const unsigned seqnum, const char *what);" << endl
			<< spacer << "// bool handle_admin(const unsigned seqnum, const FIX8::Message *msg);" << endl
			<< spacer << "// void modify_outbound(FIX8::Message *msg);" << endl
			<< spacer << "// bool authenticate(SessionID& id, const FIX8::Message *msg);" << endl << endl;

		oss_hpp << spacer << "// Override these methods to intercept admin and application methods." << endl
				<< spacer << "// bool handle_admin(const unsigned seqnum, const FIX8::Message *msg);" << endl
				<< spacer << "bool handle_application(const unsigned seqnum, const FIX8::Message *&msg)" << endl
				<< spacer << '{' << endl << spacer << spacer << "return enforce(seqnum, msg) || msg->process(_router);"
				<< endl << spacer << '}' << endl;

		oss_hpp << "};" << endl;

		oss_hpp << endl << "#endif // " << bintoaschex(ctxt._out[Ctxt::session_hpp].first.second) << endl;
	}

// ================================= Field processing =====================================

	// output file preambles
	generate_preamble(ost_hpp, ctxt._out[Ctxt::types_hpp].first.second);
	ost_hpp << "#ifndef " << bintoaschex(ctxt._out[Ctxt::types_hpp].first.second) << endl;
	ost_hpp << "#define " << bintoaschex(ctxt._out[Ctxt::types_hpp].first.second) << endl << endl;
	ost_hpp << _csMap.find_ref(cs_start_namespace) << endl;
	ost_hpp << "namespace " << ctxt._fixns << " {" << endl;

	ost_hpp << endl << _csMap.find_ref(cs_divider) << endl;
	generate_preamble(ost_cpp, ctxt._out[Ctxt::types_cpp].first.second);
	ost_cpp << _csMap.find_ref(cs_generated_includes) << endl;
   generate_includes(ost_cpp);
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
	ost_cpp << endl << _csMap.find_ref(cs_end_anon_namespace) << endl;

	// generate field instantiator lookup
	ost_hpp << "typedef FieldTable " << ctxt._clname << "_BaseEntry;" << endl;

	ost_cpp << endl << _csMap.find_ref(cs_divider) << endl;
	ost_cpp << "extern const " << ctxt._clname << "_BaseEntry::Pair fldpairs[];" << endl;
	ost_cpp << "const " << ctxt._clname << "_BaseEntry::Pair fldpairs[] ="
      << endl << '{' << endl;
	for (FieldSpecMap::const_iterator fitr(fspec.begin()); fitr != fspec.end(); ++fitr)
	{
		if (!gen_fields && !fitr->second._used)
			continue;
		if (fitr != fspec.begin())
			ost_cpp << ',' << endl;
		ost_cpp << spacer << "{ " << fitr->first << ", { ";
		if (fitr->second._dvals && !norealm) // generate code to create a Field using a value taken from an index into a Realm
		{
			ost_cpp << "Inst(Type2Types<" << ctxt._fixns << "::" << fitr->second._name << ", ";
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
				cerr << shortName << ": error: unknown FieldTrait::type in realm '" << fitr->second._name << '\'' << endl;
				++glob_errors;
			}
      }
      else
			ost_cpp << "Inst(Type2Type<" << ctxt._fixns << "::" << fitr->second._name;
      ost_cpp << ">()), ";

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
	ost_cpp << endl << "}; // " << fields_generated << endl;

	// terminate files
	ost_cpp << "} // namespace " << ctxt._fixns << endl;
	ost_hpp << endl << "} // namespace " << ctxt._fixns << endl;
	ost_hpp << _csMap.find_ref(cs_end_namespace) << endl;
	ost_hpp << "#endif // " << bintoaschex(ctxt._out[Ctxt::types_hpp].first.second) << endl;
	ost_cpp << endl << _csMap.find_ref(cs_end_namespace) << endl;

	if (verbose)
	{
		unsigned cnt(0);
		for (FieldSpecMap::const_iterator fitr(fspec.begin()); fitr != fspec.end(); ++fitr)
			if (fitr->second._used)
				++cnt;
		cout << cnt << " of " << fspec.size() << " fields used in messages" << endl;
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
#ifndef __APPLE__
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
#endif
#if defined __GXX_ABI_VERSION
	cout << "GXX ABI version is " <<  __GXX_ABI_VERSION << endl;
#endif
#else
	cout << "GCC not used. No information available." << endl;
#endif
}

//-------------------------------------------------------------------------------------------------
unsigned lookup_component(const Components& compon, const f8String& name)
{
	Components::const_iterator citr(compon.find(name));
	return citr != compon.end() ? 1 + distance(compon.begin(), citr) : 0;
}

//-------------------------------------------------------------------------------------------------
uint32_t group_hash(const MessageSpec& p1)
{
   uint32_t result(0);

	for (Presence::const_iterator flitr(p1._fields.get_presence().begin());
		flitr != p1._fields.get_presence().end(); ++flitr)
         result = rothash(result, flitr->_fnum);
   for (GroupMap::const_iterator gmitr(p1._groups.begin()); gmitr != p1._groups.end(); ++gmitr)
      result = rothash(result, group_hash(gmitr->second));

   return result;
}

//-------------------------------------------------------------------------------------------------
const MessageSpec *find_group(const CommonGroupMap& globmap, int& vers, unsigned tp, uint32_t key)
{
   CommonGroupMap::const_iterator tp_result(globmap.find(tp));
   if (tp_result == globmap.end())
      return 0;
   CommonGroups::const_iterator key_result(tp_result->second.find(key));
   if (key_result == tp_result->second.end())
      return 0;
   vers = 1 + distance(tp_result->second.begin(), key_result);
   return &key_result->second;
}

/* vim: set ts=3 sw=3 tw=0 et :*/
