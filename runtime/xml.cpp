//-----------------------------------------------------------------------------------------
#if 0

Orbweb is released under the New BSD License.

Copyright (c) 2010-12, David L. Dight <www@orbweb.org>
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
int XmlElement::errors_(0), XmlElement::line_(1), XmlElement::maxdepth_(0), XmlElement::seq_(0);
XmlElement::XmlSet XmlElement::emptyset_;
XmlElement::XmlAttrs XmlElement::emptyattrs_;
RegExp XmlElement::rCE_("&#(x[A-Fa-f0-9]+|[0-9]+);"), XmlElement::rCX_("&(amp|lt|gt|apos|quot);");

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
// finite state machine with simple recursive descent parser
//-----------------------------------------------------------------------------------------
XmlElement::XmlElement(istream& ifs, int subidx, int txtline, int depth, const char *rootAttr)
	: value_(), decl_(), depth_(depth), sequence_(++seq_), txtline_(txtline),
	chldcnt_(), subidx_(subidx), children_(), ordchildren_(), attrs_()
{
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

	while (ifs.good() && state != finished)
	{
		char c;
		ifs >> noskipws >> c;
		switch (c)
		{
		case '\n':	// lame but the only way to track it
			line_++;	// drop through
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
			else if (c == '/' && ifs.peek() == '>')	//empty
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
			if (c == '/' && ifs.peek() == '>')	//empty
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
				if (ifs.peek() != '/')
				{
					ifs.putback(c);
					if (depth_ + 1 > MaxDepth)
					{
						++errors_;
						cerr << "Error (" << line_ << "): maximum depth exceeded (" << MaxDepth << ')' << endl;
						state = olb;
					}
					else
					{
						scoped_ptr<XmlElement> child(new XmlElement(ifs, chldcnt_ + 1, line_, depth_ + 1));
						if (child->GetTag().empty())
							--seq_;
						else
						{
							if (!children_)
							{
								children_ = new XmlSubEls;
								ordchildren_ = new XmlSet;
							}
							XmlElement *chld(child.release());
							++chldcnt_;
							children_->insert(XmlSubEls::value_type(chld->GetTag(), chld));
							ordchildren_->insert(chld);
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
						 << tmpotag << '\'' << " against " << '\'' << tmpctag << '\'' << endl;
				}
				else
				{
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
							<< "\' already defined; ignoring" << endl;
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

	if (children_)
		for_each (children_->begin(), children_->end(), free_ptr<Delete2ndPairObject<> >());
	delete children_;
	delete ordchildren_;
}

//-----------------------------------------------------------------------------------------
XmlElement *XmlElement::find(const string& what, bool ignorecase, const string *atag,
	const string *aval, const char delim)	// find 1st matching entity
{
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
	const string *atag, const string *aval, const char delim) 	// find all matching entities
{
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
bool XmlElement::findAttrByValue(const string& what, const string& val)
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
