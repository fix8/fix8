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

BECAUSE THE PROGRAM IS  LICENSED FREE OF  CHARGE, THERE IS NO  WARRANTY FOR THE PROGRAM, TO
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
#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <sys/stat.h>
#include <sys/types.h>
#include <algorithm>

#ifndef _MSC_VER
# include <strings.h>
# include <sys/time.h>
# include <unistd.h>
# include <netdb.h>
# include <syslog.h>
#endif

#include <string.h>
#include <time.h>
#include <errno.h>
#include <signal.h>
#include <fcntl.h>
#include <time.h>

#include <map>
#include <set>
#include <list>
#include <vector>

#include <fix8/f8includes.hpp>

//----------------------------------------------------------------------------------------
using namespace FIX8;
using namespace std;

//----------------------------------------------------------------------------------------
const XmlElement::XmlSet XmlElement::emptyset_;
const XmlElement::XmlAttrs XmlElement::emptyattrs_;
RegExp XmlElement::rCE_("&#(x[A-Fa-f0-9]+|[0-9]+);"), XmlElement::rCX_("&([a-z]{2,}[1-4]{0,});"),
	XmlElement::rIn_("href=\"([^\"]+)\""),
   XmlElement::rEn_("\\$\\{([^}]+)\\}"), XmlElement::rEv_("!\\{([^}]+)\\}");

//----------------------------------------------------------------------------------------
const Str2Chr::TypePair valueTable[] =
{
	Str2Chr::TypePair("amp", '&'),	Str2Chr::TypePair("lt", '<'),		Str2Chr::TypePair("gt", '>'),
	Str2Chr::TypePair("apos", '\''),	Str2Chr::TypePair("quot", '"'),	Str2Chr::TypePair("nbsp", 160),
	Str2Chr::TypePair("iexcl", 161),	Str2Chr::TypePair("cent", 162),	Str2Chr::TypePair("pound", 163),
	Str2Chr::TypePair("curren", 164),Str2Chr::TypePair("yen", 165),	Str2Chr::TypePair("brvbar", 166),
	Str2Chr::TypePair("sect", 167),	Str2Chr::TypePair("uml", 168),	Str2Chr::TypePair("copy", 169),
	Str2Chr::TypePair("ordf", 170),	Str2Chr::TypePair("laquo", 171),	Str2Chr::TypePair("not", 172),
	Str2Chr::TypePair("shy", 173),	Str2Chr::TypePair("reg", 174),	Str2Chr::TypePair("macr", 175),
	Str2Chr::TypePair("deg", 176),	Str2Chr::TypePair("plusmn", 177),Str2Chr::TypePair("sup2", 178),
	Str2Chr::TypePair("sup3", 179),	Str2Chr::TypePair("acute", 180),	Str2Chr::TypePair("micro", 181),
	Str2Chr::TypePair("para", 182),	Str2Chr::TypePair("middot", 183),Str2Chr::TypePair("cedil", 184),
	Str2Chr::TypePair("sup1", 185),	Str2Chr::TypePair("ordm", 186),	Str2Chr::TypePair("raquo", 187),
	Str2Chr::TypePair("frac14", 188),Str2Chr::TypePair("frac12", 189),Str2Chr::TypePair("frac34", 190),
	Str2Chr::TypePair("iquest", 191)
};
const Str2Chr XmlElement::stringtochar_(valueTable, sizeof(valueTable)/sizeof(Str2Chr::TypePair), '?');

//-----------------------------------------------------------------------------------------
ostream& operator<<(ostream& os, const XmlElement& en)
{
	const string spacer(en.depth_ * 3, ' ');
	if (en.decl_)
		os << *en.decl_ << endl;
	os << spacer << en.tag_;

	if (en.value_)
		os << "=\"" << *en.value_ << '\"';

	if (en.attrs_)
	{
		os << " [";
		bool first(true);
		for (XmlElement::XmlAttrs::iterator itr(en.attrs_->begin()); itr != en.attrs_->end(); ++itr)
		{
			if (first)
				first = false;
			else
				os << ' ';
			os << itr->first << "=\"" << itr->second << '"';
		}
		os << ']';
	}

	if (en.txtline_)
		os << " #" << en.txtline_;
	os	<< endl;

	if (en.children_)
	{
		os << spacer << '{' << endl;
		for (XmlElement::XmlSet::iterator itr(en.ordchildren_->begin()); itr != en.ordchildren_->end(); ++itr)
			os << **itr;
		os << spacer << '}' << endl;
	}

	return os;
}

//-----------------------------------------------------------------------------------------
namespace {

// execute command and pipe output to string; only 1 line is captured.
bool exec_cmd(const string& cmd, string& result)
{
#ifdef _MSC_VER
   FILE *apipe(_popen(cmd.c_str(), "r"));
#else
   FILE *apipe(popen(cmd.c_str(), "r"));
#endif
   if (apipe)
   {
      const size_t maxcmdresultlen(1024);
      char buffer[maxcmdresultlen] = {};
      if (!feof(apipe) && fgets(buffer, maxcmdresultlen, apipe) && buffer[0])
      {
         result = buffer;
         result.resize(result.size() - 1); // remove lf
      }
#ifdef _MSC_VER
	  _pclose(apipe);
#else
      pclose(apipe);
#endif
   }
   return !result.empty();
}

}

//-----------------------------------------------------------------------------------------
// finite state machine with simple recursive descent parser
//-----------------------------------------------------------------------------------------
XmlElement::XmlElement(istream& ifs, int subidx, XmlElement *parent, int txtline, int depth, const char *rootAttr)
	: parent_(parent), root_(parent_ ? parent_->root_ : this), errors_(), line_(1), incline_(1), maxdepth_(),
	seq_(), value_(), decl_(), depth_(depth), sequence_(++root_->seq_), txtline_(txtline),
	chldcnt_(), subidx_(subidx), attrs_(), children_(), _was_include(), ordchildren_()
{
	istream *ifsptr(&ifs);

	enum
  	{
		olb, otag, ocom0, ocom1, comment, ccom0, ccom1, ccomment,
		oattr, odec, cdec, value, cls, ctag, finished
	}
  	state(olb);

	string tmpotag, tmpctag, tmpval, tmpattr, tmpdec;

	if (rootAttr)
	{
		attrs_ = new XmlAttrs;
		attrs_->insert(XmlAttrs::value_type("docpath", rootAttr));
	}

	if (root_->maxdepth_ < depth)
		root_->maxdepth_ = depth_;

	while (ifsptr->good() && state != finished)
	{
		char c;
		*ifsptr >> noskipws >> c;
		switch (c)
		{
		case '\n':
			if (!root_->inclusion_.empty())
				++root_->incline_;
			else
				++root_->line_; // drop through
		case '\r':
			continue;
		default:
			break;
		}

		switch (state)
		{
		case olb:
			if (c == '<')
				state = otag;
			break;
		case otag:
			if (c == '>')
				state = value;
			else if (c == '/' && ifsptr->peek() == '>')	//empty
			{
				state = ctag;
				tmpctag = tmpotag;
			}
			else if (isspace(c) && !tmpotag.empty())
				state = oattr;
			else if (c == '?' && tmpotag.empty())
				state = odec;
			else if (c == '!' && tmpotag.empty())
				state = ocom0;
			else if (c == '=' || c == '\\' || c == '"' || c == '\'')
				goto illegal_tag;
			else if (!isspace(c))
				tmpotag += c;
			break;
		case ocom0:
			if (c == '-')
				state = ocom1;
			else
			{
				tmpotag += "!";
				state = otag;
			}
			break;
		case ocom1:
			if (c == '-')
				state = comment;
			else
			{
				tmpotag += "!-";
				state = otag;
			}
			break;
		case odec:
			if (c == '?')	//declaration
			{
				state = cdec;
				if (!tmpdec.empty())
					decl_ = new string(tmpdec);
			}
			else
				tmpdec += c;
			break;
		case oattr:
			if (c == '/' && ifsptr->peek() == '>')	//empty
			{
				state = ctag;
				tmpctag = tmpotag;
			}
			else if (c == '>')
				state = value;
			else
				tmpattr += c;
			break;
		case comment:
			if (c == '-')
				state = ccom0;
			break;
		case ccom0:
			if (c == '-')
				state = ccomment;
			else
				state = comment;
			break;
		case ccomment:
			if (c == '>')
				state = depth_ ? finished : olb;
			else
				state = comment;
			break;
		case cdec:
			if (c == '>')
				state = depth_ ? finished : olb;
			break;
		case value:
			if (c == '<')
			{
				if (ifsptr->peek() != '/')
				{
					ifsptr->putback(c);
					if (depth_ + 1 > MaxDepth)
					{
						++root_->errors_;
						ostringstream ostr;
						ostr << "Error (" << root_->line_ << "): maximum depth exceeded (" << MaxDepth << ')';
						state = olb;
						throw XMLError(ostr.str());
					}
					else
					{
						XmlElement *child(new XmlElement(*ifsptr, chldcnt_ + 1, this, root_->line_, depth_ + 1));
						if (child->GetTag().empty()
							|| (child->_was_include && (!child->children_ || !child->children_->begin()->second->children_)))
						{
							delete child;
							--root_->seq_;
						}
						else
						{
							if (!children_)
							{
								children_ = new XmlSubEls;
								ordchildren_ = new XmlSet;
							}

							if (child->_was_include) // move great-grandchildren to children
							{
								for (XmlSubEls::const_iterator itr(child->children_->begin()->second->children_->begin());
									itr != child->children_->begin()->second->children_->end(); ++itr)
								{
									--itr->second->depth_;
									children_->insert(XmlSubEls::value_type(itr->first, itr->second));
									ordchildren_->insert(itr->second);
								}

								delete child;
								root_->inclusion_.clear();
							}
							else
							{
								++chldcnt_;
								children_->insert(XmlSubEls::value_type(child->GetTag(), child));
								ordchildren_->insert(child);
							}
						}
					}
				}
				else
					state = cls;
			}
			else
				tmpval += c;
			break;
		case cls:
			if (c == '/')
				state = ctag;
			break;
		case ctag:
			if (c == '>')
			{
				state = finished;
				if (tmpotag != tmpctag)
				{
illegal_tag:
					++root_->errors_;
					ostringstream ostr;
					ostr << "Error (" << root_->line_ << "): unmatched tag " << '\''
						 << tmpotag << '\'' << " does not close with " << '\'' << tmpctag << '\'';
					if (!root_->inclusion_.empty())
						ostr << " in inclusion " << root_->inclusion_ << " (" << root_->incline_ << ')';
					throw XMLError(ostr.str());
				}
				else
				{
					if ((tag_ = tmpotag) == "xi:include")	// handle inclusion
					{
						RegMatch match;
						if (rIn_.SearchString(match, tmpattr, 2) == 2)
						{
							string whatv;
							rIn_.SubExpr(match, tmpattr, whatv, 0, 1);
							ifstream *ifs1(new ifstream(InplaceXlate(whatv).c_str()));
							if (!*ifs1)
							{
								++root_->errors_;
								ostringstream ostr;
								ostr << "Error (" << root_->line_ << "): could not process include " << '\'' << whatv << '\'';
								throw XMLError(ostr.str());
							}
							else
							{
								ifsptr = ifs1;	// process xml elements from this stream now
								state = olb;	// reset
								root_->inclusion_ = whatv;
								root_->incline_ = 1;
								tmpctag.clear();
								tmpval.clear();
								tmpattr.clear();
								tmpdec.clear();
								_was_include = true;
								break;
							}
						}
						else
						{
							ostringstream ostr;
							++root_->errors_;
							ostr << "Error (" << root_->line_ << "): invalid xml include specification " << '\'' << tmpattr << '\'';
							throw XMLError(ostr.str());
						}
					}

					tag_ = tmpotag;
					if (!tmpval.empty() && tmpval.find_first_not_of(" \t\n\r") != string::npos)
						value_ = new string(InplaceXlate(tmpval));
				}
			}
			else if (!isspace(c))
				tmpctag += c;
			break;
		default:
			break;
		}
	}

	if (!tmpattr.empty())
		ParseAttrs(tmpattr);

	if (_was_include)
		delete ifsptr;
}

//-----------------------------------------------------------------------------------------
int XmlElement::ParseAttrs(const string& attlst)
{
	istringstream istr(attlst);
	enum { ews, tag, es, oq, value, oc0, comment, cc0 } state(ews);
	string tmptag, tmpval;
	char comchar(0);

	while (istr.good())
	{
		char c;
		istr >> noskipws >> c;

		switch (state)
		{
		case comment:
			if (c == '*')
				state = cc0;
			break;
		case cc0:
			if (c == '/')
				state = ews;
			else
				state = comment;
			break;
		case ews:
			if (c == '/')
				state = oc0;
			else if (!isspace(c))
			{
				tmptag += c;
				state = tag;
			}
			break;
		case oc0:
			if (c == '*')
				state = comment;
			else
			{
				tmptag += '/';
				tmptag += c;
				state = tag;
			}
		case tag:
			if (isspace(c))
				state = es;
			else if (c == '=')
				state = oq;
			else if (c == '"' || c == '\'')
			{
illegal_char:
				ostringstream ostr;
				ostr << "Error (" << root_->line_ << ") attribute \'" << tmptag << "\' illegal character defined";
				if (!root_->inclusion_.empty())
					ostr << " in inclusion " << root_->inclusion_ << " (" << root_->incline_ << ')';
				throw XMLError(ostr.str());
			}
			else
				tmptag += c;
			break;
		case es:
			if (c == '=')
				state = oq;
			else if (c == '"' || c == '\'')
				goto illegal_char;
			break;
		case oq:
			if (c == '"' || c == '\'')
			{
				comchar = c;
				state = value;
			}
			else if (!isspace(c))
				goto illegal_char;
			break;
		case value:
			if (c != comchar)
				tmpval += c;
			else
			{
				if (tmptag.find_first_of("\\\'\"=") != string::npos)
					goto illegal_char;
				if (tmptag != "docpath")
				{
					if (!attrs_)
						attrs_ = new XmlAttrs;
					if (!attrs_->insert(XmlAttrs::value_type(tmptag, InplaceXlate(tmpval))).second)
					{
						++root_->errors_;
						ostringstream ostr;
						ostr << "Error (" << root_->line_ << ") attribute \'" << tmptag << "\' already defined";
						if (!root_->inclusion_.empty())
							ostr << " in inclusion " << root_->inclusion_ << " (" << root_->incline_ << ')';
						throw XMLError(ostr.str());
					}
				}
				tmptag.clear();
				tmpval.clear();
				state = ews;
				comchar = 0;
			}
			break;
		default:
			break;
		}
	}

	return attrs_ ? attrs_->size() : 0;
}

//-----------------------------------------------------------------------------------------
XmlElement::~XmlElement()
{
	delete value_;
	delete attrs_;
	delete decl_;

	if (children_ && !_was_include)
		for_each (children_->begin(), children_->end(), free_ptr<Delete2ndPairObject<> >());
	delete children_;
	delete ordchildren_;
}

//-----------------------------------------------------------------------------------------
const XmlElement *XmlElement::find(const string& what, bool ignorecase, const string *atag,
	const string *aval, const char delim)	const// find 1st matching entity
{
	if (what.compare(0, 2, "//") == 0) 	// root based
		return root_->find(what.substr(2), ignorecase, atag, aval, delim);

	if (ignorecase ? what % tag_ : what == tag_)
		return atag && aval && !findAttrByValue(*atag, *aval) ? 0 : this;

	if (children_)
	{
		string lwhat(what);
		string::size_type fpos(lwhat.find_first_of(delim));

		if (fpos != string::npos && lwhat.substr(0, fpos) == tag_)
		{
			lwhat.erase(0, fpos + 1);
			fpos = lwhat.find_first_of(delim);
			string nwhat(fpos == string::npos ? lwhat : lwhat.substr(0, fpos));
			XmlSubEls::iterator itr(children_->find(nwhat));
			if (itr != children_->end())
				return itr->second->find(lwhat, ignorecase, atag, aval);
		}
	}

	return 0;
}

//-----------------------------------------------------------------------------------------
int XmlElement::find(const string& what, XmlSet& eset, bool ignorecase,
	const string *atag, const string *aval, const char delim) const	// find all matching entities
{
	if (what.compare(0, 2, "//") == 0) 	// root based
		return root_->find(what.substr(2), eset, ignorecase, atag, aval, delim);

	if (ignorecase ? what % tag_ : what == tag_)
	{
		if (atag && aval && !findAttrByValue(*atag, *aval))
			return 0;
		eset.insert(this);
		return eset.size();
	}

	if (children_)
	{
		string lwhat(what);
		string::size_type fpos(lwhat.find_first_of(delim));

		if (fpos != string::npos && lwhat.substr(0, fpos) == tag_)
		{
			lwhat.erase(0, fpos + 1);
			fpos = lwhat.find_first_of(delim);
			string nwhat(fpos == string::npos ? lwhat : lwhat.substr(0, fpos));
			pair<XmlSubEls::iterator, XmlSubEls::iterator> result(children_->equal_range(nwhat));
			while (result.first != result.second)
				(*result.first++).second->find(lwhat, eset, ignorecase, atag, aval);
			return eset.size();
		}
	}

	return 0;
}

//-----------------------------------------------------------------------------------------
bool XmlElement::findAttrByValue(const string& what, const string& val) const
{
	if (attrs_)
	{
		XmlAttrs::iterator itr(attrs_->find(what));
		return itr != attrs_->end() && itr->second == val;
	}

	return false;
}

//-----------------------------------------------------------------------------------------
bool XmlElement::GetAttr(const string& what, string& target) const
{
	if (attrs_)
	{
		XmlAttrs::iterator itr(attrs_->find(what));
		if (itr != attrs_->end())
		{
			target = itr->second;
			return true;
		}
	}

	return false;
}

//-----------------------------------------------------------------------------------------
const string& XmlElement::InplaceXlate (string& what)
{
	RegMatch match;
	while (rCX_.SearchString(match, what, 2) == 2)
	{
		string whatv;
		rCX_.SubExpr(match, what, whatv, 0, 1);
		rCX_.Replace(match, what, stringtochar_.find_value(whatv)); // not found character entity replaces string with '?'
	}

	while (rCE_.SearchString(match, what, 2) == 2)	// translate Numeric character references &#x12d; or &#12;
	{
		string whatv;
		rCE_.SubExpr(match, what, whatv, 0, 1);
		istringstream istr(whatv);
		int value;
		if (whatv[0] == 'x')
		{
			istr.ignore();
			istr >> hex >> value;
		}
		else
			istr >> dec >> value;
		string oval;
		if (value & 0xff00)	// handle hi byte
			oval += static_cast<char>(value >> 8 & 0xff);
		oval += static_cast<char>(value & 0xff);
		rCE_.Replace(match, what, oval);
	}

   if (rEn_.SearchString(match, what, 2) == 2)  // environment var replacement ${XXX}
   {
      string whatv;
      rEn_.SubExpr(match, what, whatv, 0, 1);
      const string result(getenv(whatv.c_str()));
      if (!result.empty())
         rEn_.Replace(match, what, result);
   }

   if (rEv_.SearchString(match, what, 2) == 2)  // evaluate shell command and replace with result !{XXX}
   {
      string whatv;
      rEv_.SubExpr(match, what, whatv, 0, 1);
      string result;
      if (exec_cmd(whatv, result))
         rEv_.Replace(match, what, result);
   }

	return what;
}

//-----------------------------------------------------------------------------------------
XmlElement *XmlElement::Factory(const string& fname)
{
	ifstream ifs(fname.c_str());

#ifdef _MSC_VER
	stringstream buffer;
	buffer << ifs.rdbuf();
   return ifs ? new XmlElement(buffer, 0, 0, 0, 0, fname.c_str()) : 0;
#else
	return ifs ? new XmlElement(ifs, 0, 0, 0, 0, fname.c_str()) : 0;
#endif
}

//-----------------------------------------------------------------------------------------
