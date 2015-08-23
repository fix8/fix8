//-----------------------------------------------------------------------------------------
/*

Fix8 is released under the GNU LESSER GENERAL PUBLIC LICENSE Version 3.

Fix8 Open Source FIX Engine.
Copyright (C) 2010-15 David L. Dight <fix@fix8.org>

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
Usage: f8c [-CFHINPRSUVWbcdefhiknoprstvx] \<input xml schema\>\n
   -C,--nocheck            do not embed version checking in generated code (default false)\n
   -F,--xfields            specify additional fields with associated messages (see documentation for details)\n
   -H,--pch <filename>     use specified precompiled header name for Windows (default none)\n
   -I,--info               print package info, exit\n
   -N,--nounique           do not enforce unique field parsing (default false)\n
   -P,--incpath            prefix system include path with "fix8" in generated compilation units (default yes)\n
   -R,--norealm            do not generate realm constructed field instantiators (default false)\n
   -U,--noconst            Generate non-const Router method declarations (default false, const)")\n
   -S,--noshared           Treat every group as unique and expose all static traits. Do not share metadata in message classes (default shared)\n
   -W,--nowarn             suppress warning messages (default false)\n
   -V,--verbose            be more verbose when processing\n
   -c,--classes \<server|client\> generate user session classes (default no)\n
   -d,--dump               dump 1st pass parsed source xml file, exit\n
   -e,--extension          Generate with .cxx/.hxx extensions (default .cpp/.hpp)\n
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
   f8c -p Texfix -n TEX -c client -x ../schema/FIXT11.xml myfix.xml -F "<field number='9999' name='SampleUserField' type='STRING' messages='NewOrderSingle:Y ExecutionReport:Y OrderCancelRequest:N' />\n
</tt>
\n
*/
//-----------------------------------------------------------------------------------------
#include "precomp.hpp"
// f8 headers
#include <fix8/f8includes.hpp>
#include <f8c.hpp>

#ifdef FIX8_HAVE_GETOPT_H
#include <getopt.h>
#endif

//-----------------------------------------------------------------------------------------
using namespace std;
using namespace FIX8;

//-----------------------------------------------------------------------------------------
const string Ctxt::_exts[count] { "_types.c", "_types.h", "_traits.c", "_classes.c",
                                    "_classes.h", "_router.h", "_session.h" },
                  Ctxt::_exts_ver[2] { "pp", "xx" };
string precompFile, spacer, inputFile, precompHdr, shortName, fixt, shortNameFixt, odir("./"),
       prefix("Myfix"), gen_classes, extra_fields;
bool verbose(false), error_ignore(false), gen_fields(false), norealm(false), nocheck(false), nowarn(false),
     incpath(true), nconst_router(false), no_shared_groups(false), no_default_routers(false);
unsigned glob_errors(0), glob_warnings(0), tabsize(3), ext_ver(0);
extern unsigned glob_errors;
extern const string GETARGLIST("hvVo:p:dikn:rst:x:NRc:fbCIWPF:UeH:SD");
extern string spacer, shortName, precompHdr;

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
	const FieldToNumMap& ftonSpec, FieldSpecMap& fspec, const Components& compon);
int load_messages(XmlElement& xf, MessageSpecMap& mspec, const FieldToNumMap& ftonSpec,
	FieldSpecMap& fspec, Components& compon, CommonGroupMap& globmap);
void process_ordering(MessageSpecMap& mspec);
ostream *open_ofile(const string& odir, const string& fname, string& target);
void process_value_enums(FieldSpecMap::const_iterator itr, ostream& ost_hpp, ostream& ost_cpp);
const string& mkel(const string& base, const string& compon, string& where);
const string& filepart(const string& source, string& where);
void generate_preamble(ostream& to, const string& fname, bool isheader, bool donotedit=true);
void generate_includes(ostream& to);
unsigned parse_groups(MessageSpec& ritr, const string& name,
	const FieldToNumMap& ftonSpec, FieldSpecMap& fspec, XmlElement::XmlSet& grplist,
   const Components& compon, CommonGroupMap& globmap);
int precomp(XmlElement& xf, ostream& outf);
void process_group_ordering(const CommonGroupMap& gm);
int precompfixt(XmlElement& xft, XmlElement& xf, ostream& outf, bool nounique);
void generate_group_bodies(const MessageSpec& ms, const FieldSpecMap& fspec, int depth,
	const string& msname, ostream& outp, ostream& outh, const CommonGroupMap& globmap, const std::string& fixns, const string cls_prefix=string());
void generate_nested_group(const MessageSpec& ms, const FieldSpecMap& fspec, int depth, ostream& outh, const std::string& fixns);
void generate_common_group_bodies(const FieldSpecMap& fspec, ostream& outp, CommonGroupMap& globmap);
void load_components(const XmlElement::XmlSet& comlist, Components& components);
unsigned lookup_component(const Components& compon, const f8String& name);
void binary_report();
string bintoaschex(const string& from);
uint32_t group_hash(const MessageSpec& p1);
const MessageSpec *find_group(const CommonGroupMap& globmap, int& vers, unsigned tp, uint32_t key);
void generate_group_traits(const FieldSpecMap& fspec, const MessageSpec& ms, const string& gname, const string& prefix, ostream& outp);
void generate_export( ostream& to, const string& ns );

//-----------------------------------------------------------------------------------------
int main(int argc, char **argv)
{
	int val;
	bool dump(false), keep_failed(false), retain_precomp(false), second_only(false), nounique(false);
	Ctxt ctxt;

#ifdef FIX8_HAVE_GETOPT_LONG
	option long_options[]
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
		{ "extension",		0,	0,	'e' },
		{ "ignore",			0,	0,	'i' },
		{ "nocheck",		0,	0,	'C' },
		{ "noconst",		0,	0,	'U' },
		{ "info",		   0,	0,	'I' },
		{ "fields",			0,	0,	'f' },
		{ "xfields",		1,	0,	'F' },
		{ "keep",			0,	0,	'k' },
		{ "retain",			0,	0,	'r' },
		{ "binary",			0,	0,	'b' },
		{ "classes",		1,	0,	'c' },
		{ "pch",		      1,	0,	'H' },
		{ "second",			0,	0,	's' },
		{ "defaulted",		0,	0,	'D' },
		{ "noshared",		0,	0,	'S' },
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
			cout << "f8c for " << Session::copyright_string() << endl;
			return 0;
		case 'I':
         for (const auto& pp : package_info())
            cout << pp.first << ": " << pp.second << endl;
         return 0;
		case 'V': verbose = true; break;
		case 'f': gen_fields = true; break;
		case 'N': nounique = true; break;
		case 'R': norealm = true; break;
		case 'C': nocheck = true; break;
		case 'U': nconst_router = true; break;
		case 'c': gen_classes = optarg; break;
		case 'F': extra_fields = optarg; break;
		case 'h': print_usage(); return 0;
		case ':': case '?': return 1;
		case 'o': CheckAddTrailingSlash(odir = optarg); break;
		case 'd': dump = true; break;
		case 'e': ext_ver = 1; break;
		case 'i': error_ignore = true; break;
		case 'P': incpath = false; break;
		case 'k': keep_failed = true; break;
		case 'r': retain_precomp = true; break;
		case 's': second_only = true; break;
		case 'S': no_shared_groups = true; break;
		case 'D': no_default_routers = true; break;
		case 't': tabsize = stoul(optarg); break;
		case 'p': prefix = optarg; break;
		case 'H': precompHdr = optarg; break;
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

	unique_ptr<XmlElement> cfr;
	if (second_only)
	{
		cfr.reset(XmlElement::Factory(inputFile));
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
		unique_ptr<ostream> pre_out(open_ofile(odir, shortName + ".p1", precompFile));
		if (!pre_out.get())
		{
			cerr << endl << "Error opening: " << odir << '/' << shortName;
			if	(errno)
				cerr << " (" << strerror(errno) << ')';
			cerr << endl;
			return 1;
		}
		unique_ptr<XmlElement> pcmp(XmlElement::Factory(inputFile)), pcmpfixt;
		if (!pcmp.get())
		{
			cerr << "Error reading file \'" << inputFile << '\'' << " (" << precompFile << ')';
			if	(errno)
				cerr << " (" << strerror(errno) << ')';
			cerr << endl;
			return 1;
		}
		unsigned xmlsz(pcmp->GetLineCnt()), fixtsz(0);
		if (!fixt.empty())
		{
			pcmpfixt.reset(XmlElement::Factory(fixt));
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
		cout << (fixtsz + xmlsz) << " => ";
		cout.flush();
		if (!fixt.empty())
			precompfixt(*pcmpfixt, *pcmp, *pre_out, nounique);
		else
			precomp(*pcmp, *pre_out);
		pre_out.reset();
		cfr.reset(XmlElement::Factory(precompFile));
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
		ctxt._out[ii].first.second = prefix + ctxt._exts[ii] + ctxt._exts_ver[ext_ver];
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

      // insert any fields passed on the command line into the DOM
      if (!extra_fields.empty())
      {
         XmlElement *flds(const_cast<XmlElement*>(cfr->find("fix/fields"))),
                    *msgs(const_cast<XmlElement*>(cfr->find("fix/messages")));
         if (flds && msgs)
         {
            const RegExp rMS("([^:]+):(Y|N)");
            istringstream istr(extra_fields);
            size_t added(0);
            while (istr.good())
            {
               unique_ptr<XmlElement> nel(new XmlElement(istr, 0, flds));
               string what;
               if (nel->GetTag().empty() || !nel->GetAttr("messages", what) || !flds->Insert(nel.get()))
                  continue;

               // messages='NewOrderSingle:Y ExecutionReport:Y OrderCancelRequest:N'
               RegMatch match;
               while (rMS.SearchString(match, what, 3) == 3)
               {
                  string msg_name, mandatory;
                  rMS.SubExpr(match, what, msg_name, 0, 1);
                  rMS.SubExpr(match, what, mandatory, 0, 2);
                  trim(msg_name);
                  rMS.Erase(match, what);
                  const string name_tag("name");
                  XmlElement *tmsg(const_cast<XmlElement*>(msgs->find("messages/message", &name_tag, &msg_name)));
                  if (!tmsg)
                  {
                     if (!nowarn)
                        cerr << "message: " << msg_name << " not found for custom specification" << endl;
                     ++glob_warnings;
                     continue;
                  }
                  ostringstream ostr;
                  string field_name;
                  nel->GetAttr(name_tag, field_name);
                  ostr << "<field name='" << field_name << "' required='" << mandatory << "' />";
                  istringstream mistr(ostr.str());
                  unique_ptr<XmlElement> mel(new XmlElement(mistr, 0, msgs));
                  if (!mel->GetTag().empty() && tmsg->Insert(mel.get()))
                     mel.release();
               }

               ++added;
               nel.release();

            }
            if (verbose)
               cout << added << " candidate custom fields found" << endl;
         }
      }

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
	XmlElement::XmlSet flist;
	if (!xf.find("fix/fields/field", flist))
	{
		cerr << "error: No fields found in " << shortName << endl;
		return 0;
	}

	int fieldsLoaded(0);

	for(const auto *pp : flist)
	{
		string number, name, type;
		if (pp->GetAttr("number", number) && pp->GetAttr("name", name) && pp->GetAttr("type", type))
		{
			trim(number);
			trim(name);
			trim(type);
			InPlaceStrToUpper(type);
         const auto bmitr(FieldSpec::_baseTypeMap.find(type));
			FieldTrait::FieldType ft(bmitr == FieldSpec::_baseTypeMap.end() ? FieldTrait::ft_untyped : bmitr->second);
			pair<FieldSpecMap::iterator, bool> result;
			if (ft != FieldTrait::ft_untyped)
				result = fspec.insert({stoul(number), FieldSpec(name, ft)});
			else
			{
            if (!nowarn)
               cerr << shortName << ':' << recover_line(*pp) << ": warning: Unknown field type: " << type << " in " << name << endl;
				++glob_warnings;
				continue;
			}

			pp->GetAttr("description", result.first->second._description);
			pp->GetAttr("comment", result.first->second._comment);

			++fieldsLoaded;

			XmlElement::XmlSet realmlist;
			if (pp->find("field/value", realmlist))
			{
				for(const auto *pp : realmlist)
				{
					string enum_str, description;
					if (pp->GetAttr("enum", enum_str))
					{
						if (!pp->GetAttr("description", description) || description.empty())
							description = enum_str; 	// use enum if no description supplied

						if (!result.first->second._dvals)
							result.first->second._dvals = new RealmMap;
						string rangeend;
						bool isRange(pp->GetAttr("range", rangeend) && (rangeend == "lower" || rangeend == "upper"));
						RealmObject *realmval(RealmObject::create(enum_str, ft, isRange));
						if (isRange)
							result.first->second._dtype = RealmBase::dt_range;
						if (realmval)
							result.first->second._dvals->insert({realmval, description});
					}
					else
					{
						cerr << shortName << ':' << recover_line(*pp) << ": error: field value element missing required attribute (enum)." << endl;
						++glob_errors;
					}
				}
			}
		}
		else
		{
			cerr << shortName << ':' << recover_line(*pp) << ": error: field definition element missing required attributes (number, name or type)" << endl;
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

	for(const auto *pp : mlist)
	{
		string msgcat, name, msgtype, elname;
		if (pp->GetTag() == "header")
			msgtype = name = elname = "header";
		else if (pp->GetTag() == "trailer")
			msgtype = name = elname = "trailer";
		else if (pp->GetAttr("msgtype", msgtype) && pp->GetAttr("name", name) && pp->GetAttr("msgcat", msgcat))
		{
			StringRealm srealm(msgtype, false);
			RealmMap::const_iterator ditr(fsitr->second._dvals->find(&srealm));
			if (ditr == fsitr->second._dvals->end())
			{
				cerr << shortName << ':' << recover_line(*pp) << ": error: Message '"
				  << name << "' does not have corrresponding entry in MsgType field realm. See FAQ for more details." << endl;
				++glob_errors;
				continue;
			}

			elname = "message";
		}
		else
		{
			cerr << shortName << ':' << recover_line(*pp) << ": error: Message element missing required attributes" << endl;
			++glob_errors;
			continue;
		}

		pair<MessageSpecMap::iterator, bool> result(mspec.insert({msgtype, MessageSpec(name, msgcat % "admin")}));
		if (!result.second)
		{
			cerr << shortName << ':' << recover_line(*pp) << ": error: Could not add message '" << name << "' (" << msgtype << ")" << endl;
			++glob_errors;
			continue;
		}

		string elpart;
		XmlElement::XmlSet grplist;
		if (pp->find(mkel(elname, "group", elpart), grplist))
			grpsparsed += parse_groups(result.first->second, name, ftonSpec, fspec, grplist, compon, globmap);

		pp->GetAttr("comment", result.first->second._comment);

		process_message_fields(mkel(elname, "field", elpart), pp, result.first->second._fields, ftonSpec, fspec, compon);

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

	for(const auto *pp : grplist)
	{
		string gname, required;
		if (pp->GetAttr("name", gname) && pp->GetAttr("required", required))
		{
			string compname;
			unsigned compidx(pp->GetAttr("component", compname) ? lookup_component(compon, compname) : 0);

			// add group FieldTrait
			FieldToNumMap::const_iterator ftonItr(ftonSpec.find(gname));
			FieldSpecMap::const_iterator fs_itr;
			if (ftonItr != ftonSpec.end() && (fs_itr = fspec.find(ftonItr->second)) != fspec.end())
			{
				if (!ritr._fields.add(FieldTrait(fs_itr->first, FieldTrait::ft_int, pp->GetSubIdx(),
					required == "Y", true, compidx)))
				{
					if (!nowarn)
                  cerr << "warning: Could not add group trait object '" << gname << "' (duplicate ?)" << endl;
					++glob_warnings;
				}
				else
				{
					fs_itr->second._used = true; 	// we always assume group count fields are used
					pair<GroupMap::iterator, bool> gresult(ritr._groups.insert({fs_itr->first, MessageSpec(gname)}));
               if (gresult.second)
               {
                  process_message_fields("group/field", pp, gresult.first->second._fields, ftonSpec, fspec, compon);
                  XmlElement::XmlSet comlist;
                  ++result;
                  if (pp->find("group/group", comlist))
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
				cerr << shortName << ':' << recover_line(*pp)
					<< ": error: Could not locate group Field '" << gname << "' from known field types in " << shortName << endl;
				++glob_errors;
				continue;
			}
		}
		else
		{
			cerr << shortName << ':' << recover_line(*pp) << ": error: group element missing required attributes (name or required)" << endl;
			++glob_errors;
		}
	}
   ritr._hash = group_hash(ritr);

	return result;
}

//-----------------------------------------------------------------------------------------
void generate_nested_group(const MessageSpec& ms, const FieldSpecMap& fspec, int depth, ostream& outh, const std::string& fixns)
{
   if (ms._groups.size())
   {
      const string dspacer(depth * tabsize, ' '), d2spacer((depth + 1) * tabsize, ' ');

      outh << dspacer << "GroupBase *create_nested_group(unsigned short fnum) const" << endl;
      if (ms._groups.size() > 1)
      {
         outh << dspacer << '{' << endl;
         outh << dspacer << spacer << "switch(fnum)" << endl << dspacer << spacer << '{' << endl;
         for (const auto& pp : ms._groups)
         {
            FieldSpecMap::const_iterator gsitr(fspec.find(pp.first));
            outh << dspacer << spacer << "case " << gsitr->first << ": return new " << gsitr->second._name << ';' << endl;
         }
         outh << dspacer << spacer << "default: return nullptr;" << endl;
         outh << dspacer << spacer << '}' << endl;
         outh << dspacer << '}' << endl;
      }
      else
      {
         const auto& pp(*ms._groups.cbegin());
         FieldSpecMap::const_iterator gsitr(fspec.find(pp.first));
         outh << dspacer << spacer << "{ return fnum == " << gsitr->first << " ? new "
            << gsitr->second._name << " : nullptr; }" << endl;
      }
   }
}

//-----------------------------------------------------------------------------------------
void generate_group_bodies(const MessageSpec& ms, const FieldSpecMap& fspec, int depth, const string& msname,
	ostream& outp, ostream& outh, const CommonGroupMap& globmap, const std::string& fixns, const string cls_prefix)
{
	const string dspacer(depth * tabsize, ' '), d2spacer((depth + 1) * tabsize, ' ');

	string prefix;
	if (!cls_prefix.empty())
		prefix = cls_prefix + "::";

	for (const auto& pp : ms._groups)
	{
		FieldSpecMap::const_iterator gsitr(fspec.find(pp.first));
		outp << _csMap.find(cs_divider)->second << endl;

      int vers(0);
      const MessageSpec *tgroup (find_group(globmap, vers, pp.first, pp.second._hash));
      if (!tgroup)
      {
         cout << pp.first << " not found" << endl;
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
         generate_group_traits(fspec, *tgroup, rnme.str(), prefix + ms._name + "::", outp);

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

		// nested class decl.
		outh << endl << dspacer << "/// " << tgroup->_name << " (" << pp.first << "), "
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
         outh << d2spacer << "static F8_" << fixns << "_API const FieldTrait *_traits;" << endl;
         outh << d2spacer << "static F8_" << fixns << "_API const FieldTrait_Hash_Array& _ftha;" << endl;
         outh << d2spacer << "static F8_" << fixns << "_API const MsgType& _msgtype;" << endl;
      }
      else
      {
          outh << d2spacer << "static F8_" << fixns << "_API const FieldTrait _traits[];" << endl;
          outh << d2spacer << "static F8_" << fixns << "_API const FieldTrait_Hash_Array _ftha;" << endl;
          outh << d2spacer << "static F8_" << fixns << "_API const MsgType _msgtype;" << endl;
      }
      outh << d2spacer << "static const unsigned _fieldcnt = " << tgroup->_fields.get_presence().size() << ';' << endl << endl;
		outh << dspacer << "public:" << endl;
      outh << d2spacer << "enum { _fnum = " << gsitr->first << " };" << endl << endl;
		outh << d2spacer << gsitr->second._name << "() : GroupBase(_fnum) {}" << endl;
		outh << d2spacer << "~" << gsitr->second._name << "() = default;" << endl;
		if (tgroup->_groups.empty())
			outh << d2spacer << "MessageBase *create_group(bool) const { return new MessageBase(ctx(), _msgtype(), _traits, _fieldcnt, &_ftha); }" << endl;
		else
		{
			outh << d2spacer << "MessageBase *create_group(bool deepctor) const" << endl << d2spacer << '{' << endl;
			outh << d2spacer << spacer << "MessageBase *mb(new MessageBase(ctx(), _msgtype(), _traits, _fieldcnt, &_ftha));" << endl;
         outh << d2spacer << spacer << "if (deepctor)" << endl;
         outh << d2spacer << spacer << spacer << "mb->get_groups().insert({";
         if (tgroup->_groups.size() == 1)
            outh << tgroup->_groups.begin()->first << ", new " << tgroup->_groups.begin()->second._name << " });" << endl;
         else
         {
            outh << endl;
            for (const auto& qq : tgroup->_groups)
               outh << d2spacer << spacer << spacer << spacer << "{ " << qq.first << ", new " << qq.second._name << " }," << endl;
            outh << d2spacer << spacer << spacer << "});" << endl;
         }

			outh << d2spacer << spacer << "return mb;" << endl;
			outh << d2spacer << '}' << endl;
		}
		outh << endl << d2spacer << "static const " << msname << "& get_msgtype() { return _msgtype; }" << endl;
#if defined FIX8_HAVE_EXTENDED_METADATA
      outh << d2spacer << "static const FieldTrait *get_traits() { return _traits; };" << endl;
      outh << d2spacer << "static const unsigned get_fieldcnt() { return _fieldcnt; };" << endl;
#endif

		// process nested groups
		if (!tgroup->_groups.empty())
      {
         outh << endl;
         generate_nested_group(*tgroup, fspec, depth + 1, outh, fixns);
         generate_group_bodies(*tgroup, fspec, depth + 1, msname, outp, outh, globmap, fixns, prefix + ms._name);
      }

		outh << dspacer << "};" << endl;
	}
}

//-----------------------------------------------------------------------------------------
void generate_group_traits(const FieldSpecMap& fspec, const MessageSpec& ms, const string& gname, const string& prefix, ostream& outp)
{
   if (prefix.empty())
      outp << "const FieldTrait " << gname << "_traits[]"
         << " // refs:" << ms._group_refcnt << endl << '{' << endl;
   else
      outp << "const FieldTrait " << prefix << gname << "::_traits[]" << endl << '{' << endl;
   int felpos(0);
   for (Presence::const_iterator flitr(ms._fields.get_presence().begin());
      flitr != ms._fields.get_presence().end(); ++flitr, ++felpos)
   {
      bool spaceme(true);
      if (flitr != ms._fields.get_presence().begin())
      {
         outp << ',';
         if (felpos % 4 == 0)
            outp << endl;
         else
            spaceme = false;
      }

      ostringstream tostr;
      tostr << "0x" << setfill('0') << setw(2) << hex << flitr->_field_traits.get();
      outp << (spaceme ? spacer : " ");
      outp << '{' << setw(4) << right << flitr->_fnum << ',' << setw(2)
         << right << flitr->_ftype << ',' << setw(3) << right << flitr->_pos <<
         ',' << setw(3) << right << flitr->_component << ',' << tostr.str();
#if defined FIX8_HAVE_EXTENDED_METADATA
      if (no_shared_groups && flitr->_field_traits.has(FieldTrait::group))
      {
         outp << ", " << endl << spacer << spacer;
         FieldSpecMap::const_iterator gsitr(fspec.find(flitr->_fnum));
         outp << gsitr->second._name << "::get_traits(), " << gsitr->second._name << "::get_fieldcnt()";
         outp << endl << spacer << '}';
         felpos = -1;
      }
      else
#endif
         outp << '}';
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
         << "::_ftha(" << prefix << gname << "::_traits, " << gname << "::_fieldcnt);" << endl;
}

//-----------------------------------------------------------------------------------------
void generate_common_group_bodies(const FieldSpecMap& fspec, ostream& outp, CommonGroupMap& globmap)
{
	for (const auto& pp : globmap)
	{
      int vers(1);
      for (const auto& ii : pp.second)
      {
         if (ii.second._group_refcnt > 1)   // if there is only one variant, don't generate code in common section
         {
            FieldSpecMap::const_iterator gsitr(fspec.find(pp.first));
            outp << _csMap.find(cs_divider)->second << endl;
            ostringstream gostr;
            gostr << gsitr->second._name << 'V' << vers;

            generate_group_traits(fspec, ii.second, gostr.str(), string(), outp);
         }
         ++vers;
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
	for (const auto& pp : fspec)
		ftonSpec.insert({pp.second._name, pp.first});

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
         for (const auto& pp : globmap)
         {
            vars += static_cast<unsigned>(pp.second.size());
            for (const auto& ii : pp.second)
            {
               if (ii.second._group_refcnt > 1)
                  ugs += ii.second._group_refcnt;
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
	generate_preamble(osu_hpp, ctxt._out[Ctxt::router_hpp].first.second, true);
	osu_hpp << "#ifndef " << bintoaschex(ctxt._out[Ctxt::router_hpp].first.second) << endl;
	osu_hpp << "#define " << bintoaschex(ctxt._out[Ctxt::router_hpp].first.second) << endl << endl;
	osu_hpp << _csMap.find(cs_start_namespace)->second << endl;
	osu_hpp << "namespace " << ctxt._fixns << " {" << endl;
	osu_hpp << endl << _csMap.find(cs_divider)->second << endl;

	generate_preamble(osc_hpp, ctxt._out[Ctxt::classes_hpp].first.second, true);
	osc_hpp << "#ifndef " << bintoaschex(ctxt._out[Ctxt::classes_hpp].first.second) << endl;
	osc_hpp << "#define " << bintoaschex(ctxt._out[Ctxt::classes_hpp].first.second) << endl << endl;
    generate_export(osc_hpp, ctxt._fixns);
    osc_hpp << _csMap.find(cs_start_namespace)->second << endl;
	osc_hpp << endl << "extern \"C\"" << endl << '{' << endl
      << spacer << "F8_" << ctxt._fixns << "_API const F8MetaCntx& " << ctxt._fixns << "_ctx();" << endl << '}' << endl << endl;
	osc_hpp << "namespace " << ctxt._fixns << " {" << endl;

	osc_hpp << endl << _csMap.find(cs_divider)->second << endl;
	osc_hpp << "using " << ctxt._clname << "_BaseMsgEntry = MsgTable;" << endl;
	osc_hpp << "/// Compiler generated metadata object, accessed through this function." << endl;
	osc_hpp << "F8_" << ctxt._fixns << "_API " << "const F8MetaCntx& ctx();" << endl;
	osc_hpp << "class " << ctxt._clname << "_Router;" << endl;
	osc_hpp << endl << _csMap.find(cs_divider)->second << endl;

	generate_preamble(osc_cpp, ctxt._out[Ctxt::classes_cpp].first.second, false);
	osc_cpp << _csMap.find(cs_generated_includes)->second << endl;
   generate_includes(osc_cpp);
	osc_cpp << "#include \"" << ctxt._out[Ctxt::types_hpp].first.second << '"' << endl;
	osc_cpp << "#include \"" << ctxt._out[Ctxt::router_hpp].first.second << '"' << endl;
	osc_cpp << "#include \"" << ctxt._out[Ctxt::classes_hpp].first.second << '"' << endl;
	osc_cpp << _csMap.find(cs_divider)->second << endl;
	osc_cpp << _csMap.find(cs_start_namespace)->second << endl;
	osc_cpp << "namespace " << ctxt._fixns << " {" << endl << endl;
	osc_cpp << _csMap.find(cs_start_anon_namespace)->second << endl << endl;
	osc_cpp << _csMap.find(cs_divider)->second << endl;

	generate_preamble(osr_cpp, ctxt._out[Ctxt::traits_cpp].first.second, false);
	osr_cpp << _csMap.find(cs_generated_includes)->second << endl;
   generate_includes(osr_cpp);
	osr_cpp << "#include \"" << ctxt._out[Ctxt::types_hpp].first.second << '"' << endl;
	osr_cpp << "#include \"" << ctxt._out[Ctxt::router_hpp].first.second << '"' << endl;
	osr_cpp << "#include \"" << ctxt._out[Ctxt::classes_hpp].first.second << '"' << endl;
	osr_cpp << _csMap.find(cs_divider)->second << endl;
	osr_cpp << _csMap.find(cs_start_namespace)->second << endl;
	osr_cpp << "namespace " << ctxt._fixns << " {" << endl << endl;

	osr_cpp << _csMap.find(cs_divider)->second << endl;

// =============================== Message class definitions ==============================

   if (!no_shared_groups)
   {
      osr_cpp << "// Common group traits" << endl;
      osr_cpp << "namespace {" << endl;
      generate_common_group_bodies(fspec, osr_cpp, globmap);
      osr_cpp << "} // namespace" << endl << endl;
   }
#if defined FIX8_HAVE_EXTENDED_METADATA
   else
   {
      osr_cpp << "//" << endl;
      osr_cpp << "// Shared groups disabled (--noshared). Static traits are now exposed." << endl;
      osr_cpp << "//" << endl;
   }
#endif
   osr_cpp << _csMap.find(cs_divider)->second << endl;
	osr_cpp << "// Message traits" << endl;

	FieldSpecMap::const_iterator fsitr(fspec.find(35));	// always 35
	for (const auto& pp : mspec)
	{
		bool isTrailer(pp.second._name == "trailer");
		bool isHeader(pp.second._name == "header");
		osc_hpp << "/// " << pp.second._name << " (" << pp.first << "), "
			<< (pp.second._is_admin ? "admin" : "application")
			<< ", " <<  pp.second._fields.get_presence().size() << " fiel"
			<< (pp.second._fields.get_presence().size() == 1 ? "d, " : "ds, ")
			<< pp.second._groups.size() << " grou" << (pp.second._groups.size() == 1 ? "p." : "ps.");
		if (!pp.second._comment.empty())
			osc_hpp << ' ' << pp.second._comment;
		osc_hpp << endl;
		osc_hpp << "class " << pp.second._name << " : public "
			<< (isTrailer || isHeader ? "MessageBase" : "Message") << endl << '{' << endl;

		if (pp.second._fields.get_presence().size())
		{
			osr_cpp << _csMap.find(cs_divider)->second << endl;
			osr_cpp << "const FieldTrait " << pp.second._name << "::_traits[]"
				<< endl << '{' << endl;
         int felpos(0);
			for (Presence::const_iterator flitr(pp.second._fields.get_presence().begin());
				flitr != pp.second._fields.get_presence().end(); ++flitr, ++felpos)
			{
				bool spaceme(true);
				if (flitr != pp.second._fields.get_presence().begin())
				{
					osr_cpp << ',';
               if (felpos % 4 == 0)
						osr_cpp << endl;
					else
						spaceme = false;
				}

				ostringstream tostr;
				tostr << "0x" << setfill('0') << setw(2) << hex << flitr->_field_traits.get();
				osr_cpp << (spaceme ? spacer : " ");
				osr_cpp << '{' << setw(4) << right << flitr->_fnum << ','
					<< setw(2) << right << flitr->_ftype << ',' << setw(3) << right
					<< flitr->_pos << ',' << setw(3) << right << flitr->_component << ',' << tostr.str();
#if defined FIX8_HAVE_EXTENDED_METADATA
            if (no_shared_groups && flitr->_field_traits.has(FieldTrait::group))
            {
               osr_cpp << ", " << endl << spacer << spacer;
               FieldSpecMap::const_iterator gsitr(fspec.find(flitr->_fnum));
               osr_cpp << gsitr->second._name << "::get_traits(), " << gsitr->second._name << "::get_fieldcnt()";
               osr_cpp << endl << spacer << '}';
               felpos = -1;
            }
            else
#endif
               osr_cpp << '}';
			}
			osr_cpp << endl << "};" << endl;
			osr_cpp << "const FieldTrait_Hash_Array " << pp.second._name << "::_ftha(" << pp.second._name << "::_traits, "
            << pp.second._name << "::_fieldcnt);" << endl;
			osr_cpp << "const MsgType " << pp.second._name << "::_msgtype(\"" << pp.first << "\");" << endl;
            osc_hpp << spacer << "static F8_" << ctxt._fixns << "_API const FieldTrait _traits[];" << endl;
            osc_hpp << spacer << "static F8_" << ctxt._fixns << "_API const FieldTrait_Hash_Array _ftha; " << endl;
            osc_hpp << spacer << "static F8_" << ctxt._fixns << "_API const MsgType _msgtype;" << endl;
            osc_hpp << spacer << "static F8_" << ctxt._fixns << "_API const unsigned _fieldcnt = " << pp.second._fields.get_presence().size() << ';' << endl;
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
		osc_hpp << spacer << "explicit " << pp.second._name << "(bool deepctor=true)";
		if (pp.second._fields.get_presence().size())
			osc_hpp << " : " << (isTrailer || isHeader ? "MessageBase" : "Message")
				<< "(ctx(), _msgtype(), _traits, _fieldcnt, &_ftha)";
		if (isHeader || isTrailer)
		{
			osc_hpp << ',' << endl << spacer << spacer;
			if (isHeader)
				osc_hpp << "_begin_string(new begin_string(ctx()._beginStr)), _body_length(new body_length), _msg_type(new msg_type)";
			else
				osc_hpp << "_check_sum(new check_sum)";
			osc_hpp << " { add_preamble(); }" << endl;
		}
		else if (!pp.second._groups.empty())
		{
			osc_hpp << endl << spacer << '{' << endl;
			osc_hpp << spacer << spacer << "if (deepctor)" << endl;
         osc_hpp << spacer << spacer << spacer << "_groups.insert({";
         if (pp.second._groups.size() == 1)
            osc_hpp << pp.second._groups.begin()->first << ", new " << pp.second._groups.begin()->second._name << " });" << endl;
         else
         {
            osc_hpp << endl;
            for (const auto& ii : pp.second._groups)
            {
               FieldSpecMap::const_iterator gsitr(fspec.find(ii.first));
               osc_hpp << spacer << spacer << spacer << spacer << "{ " << gsitr->first << ", new " << gsitr->second._name << " }," << endl;
            }
            osc_hpp << spacer << spacer << spacer << "});" << endl;
         }
         osc_hpp << spacer << '}' << endl;
		}
		else
			osc_hpp << " {}" << endl;

		osc_hpp << spacer << "~" << pp.second._name << "() = default;" << endl;
		if (!isHeader && !isTrailer)
		{
			osc_hpp << spacer << "bool process(Router& rt) const { return (static_cast<" << ctxt._clname << "_Router&>(rt))(this); }" << endl;
			if (pp.second._is_admin)
				osc_hpp << spacer << "bool is_admin() const { return true; }" << endl;
		}

		osc_hpp << endl << spacer << "static const " << fsitr->second._name << "& get_msgtype() { return _msgtype; }" << endl;
#if defined FIX8_HAVE_EXTENDED_METADATA
      osc_hpp << spacer << "static const FieldTrait *get_traits() { return _traits; };" << endl;
      osc_hpp << spacer << "static const unsigned get_fieldcnt() { return _fieldcnt; };" << endl;
#endif
		if (isHeader)
			osc_hpp << endl << _csMap.find(cs_header_preamble)->second << endl;
		else if (isTrailer)
			osc_hpp << endl << _csMap.find(cs_trailer_preamble)->second << endl;

// =============================== Repeating group nested classes ==============================

        generate_nested_group(pp.second, fspec, 1, osc_hpp, ctxt._fixns);
		generate_group_bodies(pp.second, fspec, 1, fsitr->second._name, osr_cpp, osc_hpp, globmap, ctxt._fixns);

		osc_hpp << "};" << endl << endl;
		osc_hpp << _csMap.find(cs_divider)->second << endl;
	}

// =============================== Message class instantiation ==============================

	osc_cpp << endl;

	osc_cpp << "const char *cn[] // Component names" << endl << '{' << endl;
	osc_cpp << spacer << "\"\"," << endl;
	for (Components::iterator citr(components.begin()); citr != components.end(); ++citr)
		osc_cpp << spacer << '"' << citr->first << "\", // " << (1 + distance(components.begin(), citr)) << endl;
	osc_cpp << "};" << endl;

	osc_cpp << endl << _csMap.find(cs_end_anon_namespace)->second << endl;

	osc_cpp << endl << _csMap.find(cs_divider)->second << endl;
	osc_cpp << "const " << ctxt._fixns << "::" << ctxt._clname << "_BaseMsgEntry::Pair "
		<< "msgpairs[] " << endl << '{' << endl;
	for (MessageSpecMap::const_iterator mitr(mspec.begin()); mitr != mspec.end(); ++mitr)
	{
		if (mitr != mspec.begin())
			osc_cpp << ',' << endl;
		osc_cpp << spacer << "{ \"" << mitr->first << "\", { ";
		if (mitr->second._name == "trailer" || mitr->second._name == "header")
         osc_cpp << "Type2Type<" << ctxt._fixns << "::" << mitr->second._name << ", bool>()";
      else
         osc_cpp << "Type2Type<" << ctxt._fixns << "::" << mitr->second._name << ">()";
		osc_cpp << ", \"" << mitr->second._name << '"';
		if (!mitr->second._comment.empty())
			osc_cpp << ',' << endl << spacer << spacer << '"' << mitr->second._comment << "\" }";
		else
			osc_cpp << " }";
		osc_cpp << " }";
	}
	osc_cpp << endl << "}; // " << mspec.size() << endl;

   size_t fields_generated(0);
	for (const auto& pp : fspec)
		if (gen_fields || pp.second._used)
         ++fields_generated;

	osc_cpp << endl << "extern const " << ctxt._clname << "_BaseEntry::Pair fldpairs[];" << endl << endl
      << "/// Compiler generated metadata object, accessed through this function." << endl
      << "const F8MetaCntx& ctx() // avoid SIOF" << endl << '{' << endl
      << spacer << "static const " << ctxt._clname << "_BaseMsgEntry "
         << "bme(msgpairs, " << mspec.size() << ");" << endl
      << spacer << "static const " << ctxt._clname << "_BaseEntry "
         << "be(fldpairs, " << fields_generated << ");" << endl
      << spacer << "static const F8MetaCntx _ctx(" << ctxt._version << ", bme, be, cn, \"" << ctxt._beginstr << "\");" << endl
      << spacer << "return _ctx;" << endl << '}' << endl;
	osc_cpp << endl << "} // namespace " << ctxt._fixns << endl;

// ==================================== Message router ==================================

	osu_hpp << "class " << ctxt._clname << "_Router : public Router" << endl
		<< '{' << endl << "public:" << endl;
	osu_hpp << spacer << ctxt._clname << "_Router() {}" << endl;
	osu_hpp << spacer << "virtual ~" << ctxt._clname << "_Router() {}" << endl << endl;
   osu_hpp << spacer << "virtual bool operator() (const class Message *msg) ";
   if (!nconst_router)
      osu_hpp << "const ";
   osu_hpp << "{ return false; }" << endl;
	for (const auto& pp : mspec)
	{
		if (pp.second._name == "trailer" || pp.second._name == "header")
			continue;
		osu_hpp << spacer << "virtual bool operator() (const class " << pp.second._name << " *msg)";
      if (no_default_routers)
         osu_hpp << ';' << endl;
      else
      {
         if (!nconst_router)
            osu_hpp << " const";
         osu_hpp << " { return " << (pp.second._is_admin ? "true" : "false") << "; }" << endl;
      }
	}
	osu_hpp << "};" << endl;

	// terminate files
	osc_hpp << endl << "} // namespace " << ctxt._fixns << endl;
	osc_hpp << _csMap.find(cs_end_namespace)->second << endl;
	osc_hpp << "#endif // " << bintoaschex(ctxt._out[Ctxt::classes_hpp].first.second) << endl;
	osu_hpp << endl << "} // namespace " << ctxt._fixns << endl;
	osu_hpp << _csMap.find(cs_end_namespace)->second << endl;
	osu_hpp << "#endif // " << bintoaschex(ctxt._out[Ctxt::router_hpp].first.second) << endl;
	osr_cpp << endl << "} // namespace " << ctxt._fixns << endl;
	osr_cpp << _csMap.find(cs_end_namespace)->second << endl;
	osc_cpp << endl << "// Compiler generated metadata object accessible outside namespace through this function." << endl;
	osc_cpp << "extern \"C\"" << endl << '{' << endl
      << spacer << "const F8MetaCntx& " << ctxt._fixns << "_ctx() { return " << ctxt._fixns << "::ctx(); }"
      << endl << '}' << endl << endl;
	osc_cpp << _csMap.find(cs_end_namespace)->second << endl;
	osc_cpp << endl;

// =============================== Generate optional user session and router ==============================

	if (!gen_classes.empty())
	{
		bool is_server(gen_classes == "server");
		generate_preamble(oss_hpp, ctxt._out[Ctxt::session_hpp].first.second, true, false);
		oss_hpp << "#ifndef " << bintoaschex(ctxt._out[Ctxt::session_hpp].first.second) << endl;
		oss_hpp << "#define " << bintoaschex(ctxt._out[Ctxt::session_hpp].first.second) << endl;
		oss_hpp << endl << _csMap.find(cs_divider)->second << endl;

		oss_hpp << "// " << gen_classes << " session and router classes" << endl;
		oss_hpp << _csMap.find(cs_divider)->second << endl;

		oss_hpp << "class " << ctxt._clname << "_session_" << gen_classes << ';' << endl << endl;
		oss_hpp << "class " << ctxt._clname << "_router_" << gen_classes
			<< " : public FIX8::" << ctxt._fixns << "::" << ctxt._clname << "_Router" << endl << '{' << endl;
		oss_hpp << spacer << ctxt._clname << "_session_" << gen_classes << "& _session; " << endl << endl;
		oss_hpp << "public:" << endl;
		oss_hpp << spacer << ctxt._clname << "_router_" << gen_classes
			<< '(' << ctxt._clname << "_session_" << gen_classes << "& session) : _session(session) {}" << endl;
		oss_hpp << spacer << "virtual ~" << ctxt._clname << "_router_" << gen_classes << "() {}" << endl << endl;
		oss_hpp << spacer << "// Override these methods to receive specific message callbacks." << endl;
		for (const auto& pp : mspec)
		{
			if (pp.second._name == "trailer" || pp.second._name == "header")
				continue;
			oss_hpp << spacer << "// bool operator() (const FIX8::"
				<< ctxt._fixns << "::" << pp.second._name << " *msg) const;" << endl;
		}
		oss_hpp << "};" << endl;

		oss_hpp << endl << _csMap.find(cs_divider)->second << endl;
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
				<< spacer << "// bool handle_admin(const unsigned seqnum, const FIX8::Message *msg);" << endl << endl
				<< spacer << "bool handle_application(const unsigned seqnum, const FIX8::Message *&msg);" << endl
				<< spacer << "/* In your compilation unit, this should be implemented with something like the following:" << endl
				<< spacer << "bool " << ctxt._clname << "_session_" << gen_classes << "::handle_application(const unsigned seqnum, const FIX8::Message *&msg)" << endl
				<< spacer << '{' << endl << spacer << spacer << "return enforce(seqnum, msg) || msg->process(_router);" << endl
				<< spacer << '}' << endl << spacer << "*/" << endl;

		oss_hpp << "};" << endl;

		oss_hpp << endl << "#endif // " << bintoaschex(ctxt._out[Ctxt::session_hpp].first.second) << endl;
	}

// ================================= Field processing =====================================

	// output file preambles
	generate_preamble(ost_hpp, ctxt._out[Ctxt::types_hpp].first.second, true);
	ost_hpp << "#ifndef " << bintoaschex(ctxt._out[Ctxt::types_hpp].first.second) << endl;
	ost_hpp << "#define " << bintoaschex(ctxt._out[Ctxt::types_hpp].first.second) << endl << endl;
	ost_hpp << _csMap.find(cs_start_namespace)->second << endl;
	ost_hpp << "namespace " << ctxt._fixns << " {" << endl;

	ost_hpp << endl << _csMap.find(cs_divider)->second << endl;
	generate_preamble(ost_cpp, ctxt._out[Ctxt::types_cpp].first.second, false);
	ost_cpp << _csMap.find(cs_generated_includes)->second << endl;
   generate_includes(ost_cpp);
	ost_cpp << "#include \"" << ctxt._out[Ctxt::types_hpp].first.second << '"' << endl;
	ost_cpp << _csMap.find(cs_divider)->second << endl;
	ost_cpp << _csMap.find(cs_start_namespace)->second << endl;
	ost_cpp << "namespace " << ctxt._fixns << " {" << endl << endl;

	ost_cpp << _csMap.find(cs_start_anon_namespace)->second << endl;
	ost_cpp << endl << _csMap.find(cs_divider)->second << endl;
	// generate field types
	for (FieldSpecMap::const_iterator fitr(fspec.begin()); fitr != fspec.end(); ++fitr)
	{
		if (!gen_fields && !fitr->second._used)
			continue;
		if (!fitr->second._comment.empty())
			ost_hpp << "// " << fitr->second._comment << endl;
      const auto tyitr(FieldSpec::_typeToCPP.find(fitr->second._ftype));
		ost_hpp << "using " << fitr->second._name << " = Field<"
         << (tyitr == FieldSpec::_typeToCPP.end() ? "unknown" : tyitr->second.first) << ", " << fitr->first << ">;" << endl;
		if (fitr->second._dvals)
			process_value_enums(fitr, ost_hpp, ost_cpp);
		ost_hpp << _csMap.find(cs_divider)->second << endl;
	}

	// generate realmbase objs
	ost_cpp << endl << _csMap.find(cs_divider)->second << endl;
	ost_cpp << "const RealmBase realmbases[] " << endl << '{' << endl;
	unsigned dcnt(0);
	for (auto& pp : fspec)
	{
		if ((!pp.second._used && !gen_fields) || !pp.second._dvals)
			continue;
      const auto tyitr(FieldSpec::_typeToCPP.find(pp.second._ftype));
		ost_cpp << spacer << "{ reinterpret_cast<const void *>(" << pp.second._name << "_realm), "
			<< "RealmBase::" << (pp.second._dtype == RealmBase::dt_set ? "dt_set" : "dt_range") << ", "
			<< "FieldTrait::" << tyitr->second.second << ", "
			<< pp.second._dvals->size() << ", " << pp.second._name << "_descriptions }," << endl;
		pp.second._doffset = dcnt++;
	}
	ost_cpp << "};" << endl;

	// generate field instantiators
	ost_cpp << endl << _csMap.find(cs_divider)->second << endl;
	ost_cpp << endl << _csMap.find(cs_end_anon_namespace)->second << endl;

	// generate field instantiator lookup
	ost_hpp << "using " << ctxt._clname << "_BaseEntry = FieldTable;" << endl;

	ost_cpp << endl << _csMap.find(cs_divider)->second << endl;
	ost_cpp << "extern const " << ctxt._clname << "_BaseEntry::Pair fldpairs[];" << endl;
	ost_cpp << "const " << ctxt._clname << "_BaseEntry::Pair fldpairs[] "
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
			ost_cpp << "Type2Type<" << ctxt._fixns << "::" << fitr->second._name << ", ";
         string ttype;
         if (!FieldTrait::get_type_string(fitr->second._ftype, ttype).empty())
				ost_cpp << ttype;
			else
			{
				ost_cpp << "unknown";
				cerr << shortName << ": error: unknown FieldTrait::type in realm '" << fitr->second._name << '\'' << endl;
				++glob_errors;
			}
      }
      else
			ost_cpp << "Type2Type<" << ctxt._fixns << "::" << fitr->second._name;
      ost_cpp << ">(), \"" << fitr->second._name << "\", " << fitr->first;
		if (fitr->second._dvals)
			ost_cpp << ", &" << ctxt._fixns << "::realmbases[" << fitr->second._doffset << ']';
		if (!fitr->second._comment.empty())
      {
			ost_cpp << ", ";
         if (!fitr->second._dvals)
            ost_cpp << "nullptr, ";
			ost_cpp << endl << spacer << spacer << '"' << fitr->second._comment << '"';
      }
		ost_cpp << " } }";
	}
	ost_cpp << endl << "}; // " << fields_generated << endl;

	// terminate files
	ost_cpp << "} // namespace " << ctxt._fixns << endl;
	ost_hpp << endl << "} // namespace " << ctxt._fixns << endl;
	ost_hpp << _csMap.find(cs_end_namespace)->second << endl;
	ost_hpp << "#endif // " << bintoaschex(ctxt._out[Ctxt::types_hpp].first.second) << endl;
	ost_cpp << endl << _csMap.find(cs_end_namespace)->second << endl;

	if (verbose)
	{
		unsigned cnt(0);
		for (const auto& pp : fspec)
			if (pp.second._used)
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
   if (no_shared_groups)   // hack so every group hash is unique
   {
      static uint32_t result(0);
      return ++result;
   }

   uint32_t result(0);

   for (const auto& pp : p1._fields.get_presence())
      result = rothash(result, pp._fnum);
   for (const auto& pp : p1._groups)
      result = rothash(result, group_hash(pp.second));

   return result;
}

//-------------------------------------------------------------------------------------------------
const MessageSpec *find_group(const CommonGroupMap& globmap, int& vers, unsigned tp, uint32_t key)
{
   CommonGroupMap::const_iterator tp_result(globmap.find(tp));
   if (tp_result == globmap.end())
      return nullptr;
   CommonGroups::const_iterator key_result(tp_result->second.find(key));
   if (key_result == tp_result->second.end())
      return nullptr;
   vers = 1 + distance(tp_result->second.begin(), key_result);
   return &key_result->second;
}

//-------------------------------------------------------------------------------------------------
void generate_preamble(ostream& to, const string& fname, bool isheader, bool donotedit)
{
	to << _csMap.find(cs_divider)->second << endl;
	string result;
	if (donotedit)
	{
		to << _csMap.find(cs_do_not_edit)->second << GetTimeAsStringMS(result, 0, 0) << " ***" << endl;
		to << _csMap.find(cs_divider)->second << endl;
	}
	to << _csMap.find(cs_copyright)->second << insert_year() << _csMap.find(cs_copyright2)->second << endl;
	to << _csMap.find(cs_divider)->second << endl;
	if (!precompHdr.empty() && !isheader)
	{
		to << "#include ";
		if (precompHdr[0] == '<')
			to << precompHdr;
		else
			to << '"' << precompHdr << '"';
		to << endl;
	}
	to << "#include " << (incpath ? "<fix8/" : "<") << "f8config.h" << '>' << endl;
	if (!nocheck)
	{
		to << "#if defined FIX8_MAGIC_NUM && FIX8_MAGIC_NUM > " << FIX8_MAGIC_NUM << 'L' << endl;
		to << "#error " << fname << " version " << FIX8_PACKAGE_VERSION << " is out of date. Please regenerate with f8c." << endl;
		to << "#endif" << endl;
	}
	to << _csMap.find(cs_divider)->second << endl;
	to << "// " << fname << endl;
	to << _csMap.find(cs_divider)->second << endl;
}
//-------------------------------------------------------------------------------------------------
void generate_export(ostream& to, const string& ns)
{
    to <<
        "#if defined(_MSC_VER) && defined(F8_" << ns << "_API_SHARED)\n"
        "    #if defined(BUILD_F8_" << ns << "_API)\n"
        "        #define F8_" << ns << "_API __declspec(dllexport)\n"
        "    #else\n"
        "        #define F8_" << ns << "_API __declspec(dllimport)\n"
        "    #endif\n"
        "#else\n"
        "    #define F8_" << ns << "_API\n"
        "#endif\n";
}

/* vim: set ts=3 sw=3 tw=0 et :*/
