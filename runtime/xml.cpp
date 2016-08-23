//-----------------------------------------------------------------------------------------
/*

Fix8 is released under the GNU LESSER GENERAL PUBLIC LICENSE Version 3.

Fix8 Open Source FIX Engine.
Copyright (C) 2010-16 David L. Dight <fix@fix8.org>

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
#include "precomp.hpp"
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
XmlElement::XmlFlags XmlElement::flags_;

//----------------------------------------------------------------------------------------
const Str2Chr XmlElement::stringtochar_
{
	{"amp", '&'},		{"lt", '<'},		{"gt", '>'},
	{"apos", '\''},	{"quot", '"'},		{"nbsp", 160},
	{"iexcl", 161},	{"cent", 162},		{"pound", 163},
	{"curren", 164},	{"yen", 165},		{"brvbar", 166},
	{"sect", 167},		{"uml", 168},		{"copy", 169},
	{"ordf", 170},		{"laquo", 171},	{"not", 172},
	{"shy", 173},		{"reg", 174},		{"macr", 175},
	{"deg", 176},		{"plusmn", 177},	{"sup2", 178},
	{"sup3", 179},		{"acute", 180},	{"micro", 181},
	{"para", 182},		{"middot", 183},	{"cedil", 184},
	{"sup1", 185},		{"ordm", 186},		{"raquo", 187},
	{"frac14", 188},	{"frac12", 189},	{"frac34", 190},
	{"iquest", 191}
};

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
      char buffer[maxcmdresultlen] {};
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
		oattr, odec, cdec, value, cls, ctag,
		cdata0, cdata1, cdata2, cdata3, cdata4, cdata5, cdata6, cdata7,	// ![CDATA[
		vcdata, ecdata0, ecdata1, // ]]
		finished
	}
  	state(olb);

	string tmpotag, tmpctag, tmpval, tmpattr, tmpdec;
	int starttmpotag(0);

	if (rootAttr)
	{
		attrs_ = new XmlAttrs;
		attrs_->insert({"docpath", rootAttr});
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
			{
				if (!starttmpotag)
					starttmpotag = root_->line_;
				tmpotag += c;
			}
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
		case cdata0:
			state = cdata1;
			break;
		case cdata1:
			if (c == '[')
				state = cdata2;
			else
			{
				state = value;
				tmpval += "![";
				tmpval += c;
			}
			break;
		case cdata2:
			if (c == 'C')
				state = cdata3;
			else
			{
				state = value;
				tmpval += "![";
				tmpval += c;
			}
			break;
		case cdata3:
			if (c == 'D')
				state = cdata4;
			else
			{
				state = value;
				tmpval += "![C";
				tmpval += c;
			}
			break;
		case cdata4:
			if (c == 'A')
				state = cdata5;
			else
			{
				state = value;
				tmpval += "![CD";
				tmpval += c;
			}
			break;
		case cdata5:
			if (c == 'T')
				state = cdata6;
			else
			{
				state = value;
				tmpval += "![CDA";
				tmpval += c;
			}
			break;
		case cdata6:
			if (c == 'A')
				state = cdata7;
			else
			{
				state = value;
				tmpval += "![CDAT";
				tmpval += c;
			}
			break;
		case cdata7:
			if (c == '[')
				state = vcdata;
			else
			{
				state = value;
				tmpval += "![CDATA";
				tmpval += c;
			}
			break;
		case vcdata:
			if (c == ']')
				state = ecdata0;
			else
				tmpval += c;
			break;
		case ecdata0:
			if (c == ']')
				state = ecdata1;
			else
			{
				tmpval += ']';
				tmpval += c;
				state = vcdata;
			}
			break;
		case ecdata1:
			if (c == '>')
				state = value;
			else
			{
				tmpval += "]]";
				state = vcdata;
			}
			break;
		case value:
			if (c == '<')
			{
				/*	// FIXME - breaks comments
				if (ifsptr->peek() == '!')
				{
					state = cdata0;
					break;
				}
				*/
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
									children_->insert({itr->first, itr->second});
									ordchildren_->insert(itr->second);
								}

								delete child;
								root_->inclusion_.clear();
							}
							else
							{
								++chldcnt_;
								children_->insert({child->GetTag(), child});
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
						 << tmpotag << "' (" << starttmpotag << ") does not close with " << '\'' << tmpctag << '\'';
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
bool XmlElement::Insert(XmlElement *what)
{
	if (!what)
		return false;

	if (!children_)
	{
		children_ = new XmlSubEls;
		ordchildren_ = new XmlSet;
	}

	++chldcnt_;
	children_->insert({what->GetTag(), what});
	ordchildren_->insert(what);

	return true;
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
			if (c == '*' && !(flags_ & noextensions))
			{
				state = comment;
				break;
			}
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
					if (!attrs_->insert({tmptag, InplaceXlate(tmpval)}).second)
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

	return attrs_ ? static_cast<int>(attrs_->size()) : 0;
}

//-----------------------------------------------------------------------------------------
XmlElement::~XmlElement()
{
	delete value_;
	delete attrs_;
	delete decl_;

	if (children_ && !_was_include)
		for_each (children_->begin(), children_->end(), [](XmlSubEls::value_type& pp) { delete pp.second; });
	delete children_;
	delete ordchildren_;
}

//-----------------------------------------------------------------------------------------
const XmlElement *XmlElement::find(const string& what, const string *atag,
	const string *aval, const char delim)	const // find 1st matching entity
{
	if (what.compare(0, 2, "//") == 0) 	// root based
		return root_->find(what.substr(2), atag, aval, delim);

	if (flags_ & nocase ? what % tag_ : what == tag_)
		return atag && aval && !findAttrByValue(*atag, *aval) ? nullptr : this;

	if (children_)
	{
		string lwhat(what);
		string::size_type fpos(lwhat.find_first_of(delim));

		if (fpos != string::npos && lwhat.substr(0, fpos) == tag_)
		{
			lwhat.erase(0, fpos + 1);
			fpos = lwhat.find_first_of(delim);
			const string nwhat(fpos == string::npos ? lwhat : lwhat.substr(0, fpos));
			pair<XmlSubEls::iterator, XmlSubEls::iterator> result(children_->equal_range(nwhat));
			while (result.first != result.second)
			{
				const XmlElement *ptr((*result.first++).second->find(lwhat, atag, aval, delim));
				if (ptr)
					return ptr;
			}
		}
	}

	return nullptr;
}

//-----------------------------------------------------------------------------------------
int XmlElement::find(const string& what, XmlSet& eset, const string *atag, const string *aval,
		const char delim) const	// find all matching entities
{
	if (what.compare(0, 2, "//") == 0) 	// root based
		return root_->find(what.substr(2), eset, atag, aval, delim);

	if (flags_ & nocase ? what % tag_ : what == tag_)
	{
		if (atag && aval && !findAttrByValue(*atag, *aval))
			return 0;
		eset.insert(this);
		return static_cast<int>(eset.size());
	}

	if (children_)
	{
		string lwhat(what);
		string::size_type fpos(lwhat.find_first_of(delim));

		if (fpos != string::npos && lwhat.substr(0, fpos) == tag_)
		{
			lwhat.erase(0, fpos + 1);
			fpos = lwhat.find_first_of(delim);
			const string nwhat(fpos == string::npos ? lwhat : lwhat.substr(0, fpos));
			pair<XmlSubEls::iterator, XmlSubEls::iterator> result(children_->equal_range(nwhat));
			while (result.first != result.second)
				(*result.first++).second->find(lwhat, eset, atag, aval, delim);
			return static_cast<int>(eset.size());
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
		const auto sitr(stringtochar_.find(whatv));
		rCX_.Replace(match, what, sitr == stringtochar_.cend() ? '?' : sitr->second); // not found character entity replaces string with '?'
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

	if (!(flags_ & noextensions))
	{
		if (rEn_.SearchString(match, what, 2) == 2)  // environment var replacement ${XXX}
		{
			string whatv;
			rEn_.SubExpr(match, what, whatv, 0, 1);
			const char *gresult(getenv(whatv.c_str()));
			if (gresult)
			{
				const string result(gresult);
				if (!result.empty())
					rEn_.Replace(match, what, result);
			}
		}

		if (rEv_.SearchString(match, what, 2) == 2)  // evaluate shell command and replace with result !{XXX}
		{
			string whatv;
			rEv_.SubExpr(match, what, whatv, 0, 1);
			string result;
			if (exec_cmd(whatv, result))
				rEv_.Replace(match, what, result);
		}
   }

	return what;
}

//-----------------------------------------------------------------------------------------
XmlElement *XmlElement::Factory(istream& ifs, const char *docpath)
{
#ifdef _MSC_VER
	stringstream buffer;
	buffer << ifs.rdbuf();
   return ifs ? new XmlElement(buffer, 0, nullptr, 0, 0, docpath) : nullptr;
#else
	return ifs ? new XmlElement(ifs, 0, nullptr, 0, 0, docpath) : nullptr;
#endif
}

//-----------------------------------------------------------------------------------------
XmlElement *XmlElement::Factory(const string& fname)
{
	ifstream ifs(fname.c_str());
	return Factory(ifs, fname.c_str());
}

//-----------------------------------------------------------------------------------------
