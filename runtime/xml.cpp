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

#endif
//-----------------------------------------------------------------------------------------
#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <sys/stat.h>
#include <sys/types.h>
#include <algorithm>
#include <strings.h>
#include <string.h>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>
#include <errno.h>
#include <netdb.h>
#include <signal.h>
#include <syslog.h>
#include <fcntl.h>
#include <time.h>
#include <regex.h>

#include <map>
#include <set>
#include <list>
#include <vector>

#include <f8includes.hpp>

//----------------------------------------------------------------------------------------
using namespace FIX8;
using namespace std;

//----------------------------------------------------------------------------------------
int XmlElement::errors_(0), XmlElement::line_(1), XmlElement::incline_(1),
	 XmlElement::maxdepth_(0), XmlElement::seq_(0);
string XmlElement::inclusion_;
XmlElement::XmlSet XmlElement::emptyset_;
XmlElement::XmlAttrs XmlElement::emptyattrs_;
RegExp XmlElement::rCE_("&#(x[A-Fa-f0-9]+|[0-9]+);"), XmlElement::rCX_("&(amp|lt|gt|apos|quot);"),
	XmlElement::rIn_("href=\"([^\"]+)\""),
   XmlElement::rEn_("\\$\\{([^}]+)\\}"), XmlElement::rEv_("!\\{([^}]+)\\}");
XmlElement *XmlElement::root_;

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
string& exec_cmd(const string& cmd, string& result)
{
   FILE *apipe(popen(cmd.c_str(), "r"));
   if (apipe)
   {
      const size_t maxcmdlen(128);
      char buffer[maxcmdlen] = {};
      if (!feof(apipe) && fgets(buffer, maxcmdlen, apipe) && buffer[0])
      {
         result = buffer;
         result.resize(result.size() - 1);
      }
      pclose(apipe);
   }
   return result;
}

}

//-----------------------------------------------------------------------------------------
// finite state machine with simple recursive descent parser
//-----------------------------------------------------------------------------------------
XmlElement::XmlElement(istream& ifs, int subidx, int txtline, int depth, const char *rootAttr)
	: value_(), decl_(), depth_(depth), sequence_(++seq_), txtline_(txtline),
	chldcnt_(), subidx_(subidx), children_(), _was_include(), ordchildren_(), attrs_()
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

	if (maxdepth_ < depth)
		maxdepth_ = depth_;

	if (depth_ == 0)
		root_ = this;

	while (ifsptr->good() && state != finished)
	{
		char c;
		*ifsptr >> noskipws >> c;
		switch (c)
		{
		case '\n':
			if (!inclusion_.empty())
				incline_++;
			else
				line_++; // drop through
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
						++errors_;
						cerr << "Error (" << line_ << "): maximum depth exceeded (" << MaxDepth << ')' << endl;
						state = olb;
					}
					else
					{
						XmlElement *child(new XmlElement(*ifsptr, chldcnt_ + 1, line_, depth_ + 1));
						if (child->GetTag().empty()
							|| (child->_was_include && (!child->children_ || !child->children_->begin()->second->children_)))
						{
							delete child;
							--seq_;
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
								inclusion_.clear();
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
					++errors_;
					cerr << "Error (" << line_ << "): unmatched tag " << '\''
						 << tmpotag << '\'' << " does not close with " << '\'' << tmpctag << '\'';
					if (!inclusion_.empty())
						cerr << " in inclusion " << inclusion_ << " (" << incline_ << ')';
					cerr << endl;
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
								++errors_;
								cerr << "Error (" << line_ << "): could not process include " << '\'' << whatv << '\'' << endl;
							}
							else
							{
								ifsptr = ifs1;	// process xml elements from this stream now
								state = olb;	// reset
								inclusion_ = whatv;
								incline_ = 1;
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
							++errors_;
							cerr << "Error (" << line_ << "): invalid xml include specification " << '\'' << tmpattr << '\'' << endl;
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
	enum { ews, tag, oq, value } state(ews);
	string tmptag, tmpval;
	char comchar(0);

	while (istr.good())
	{
		char c;
		istr >> noskipws >> c;

		switch (state)
		{
		case ews:
			if (!isspace(c))
			{
				tmptag += c;
				state = tag;
			}
			break;
		case tag:
			if (c == '=')
				state = oq;
			else
				tmptag += c;
			break;
		case oq:
			if (c == '"' || c == '\'')
			{
				comchar = c;
				state = value;
			}
			break;
		case value:
			if (c != comchar)
				tmpval += c;
			else
			{
				if (tmptag != "docpath")
				{
					if (!attrs_)
						attrs_ = new XmlAttrs;
					if (!attrs_->insert(XmlAttrs::value_type(tmptag, InplaceXlate(tmpval))).second)
					{
						++errors_;
						cerr << "Error (" << line_ << ") attribute \'" << tmptag
							<< "\' already defined; ignoring";
						if (!inclusion_.empty())
							cerr << " in inclusion " << inclusion_ << " (" << incline_ << ')';
						cerr << endl;
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
	{
		const string rmsl(what, 2);
		return root_->find(rmsl, ignorecase, atag, aval, delim);
	}

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
	{
		const string rmsl(what, 2);
		return root_->find(rmsl, eset, ignorecase, atag, aval, delim);
	}

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
		string whatv, replv;
		rCX_.SubExpr(match, what, whatv, 1);
		if (whatv == "amp")
			replv = "&";
		else if (whatv == "lt")
			replv = "<";
		else if (whatv == "gt")
			replv = ">";
		else if (whatv == "apos")
			replv = "'";
		else if (whatv == "quot")
			replv = "\"";
		else
			break;	// cannot be reached

		rCX_.Replace(match, what, replv);
	}

	while (rCE_.SearchString(match, what, 2) == 2)	// translate Numeric character references &#x12d; or &#12;
	{
		string whatv;
		rCE_.SubExpr(match, what, whatv, 1);
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
      string result(getenv(whatv.c_str()));
      if (!result.empty())
         rEn_.Replace(match, what, result);
   }

   if (rEv_.SearchString(match, what, 2) == 2)  // evaluate shell command and replace with result !{XXX}
   {
      string whatv;
      rEv_.SubExpr(match, what, whatv, 0, 1);
      string result;
      exec_cmd(whatv, result);
      if (!result.empty())
         rEv_.Replace(match, what, result);
   }

	return what;
}

//-----------------------------------------------------------------------------------------
XmlElement *XmlElement::Factory(const string& fname)
{
	Reset();
	ifstream ifs(fname.c_str());
	return ifs ? new XmlElement(ifs, 0, 0, 0, fname.c_str()) : 0;
}

//-----------------------------------------------------------------------------------------
