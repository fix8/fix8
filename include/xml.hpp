//-----------------------------------------------------------------------------------------
#if 0

Orbweb is released under the New BSD License.

Copyright (c) 2007-2010, David L. Dight <www@orbweb.org>
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

$Id: xml.hpp 497 2010-01-10 01:57:03Z davidd $
$LastChangedDate: 2010-01-10 12:57:03 +1100 (Sun, 10 Jan 2010) $
$Rev: 497 $
$URL: svn://catfarm.electro.mine.nu/usr/local/repos/mongod/trunk/base/xml.hpp $

#endif
//-----------------------------------------------------------------------------------------
#ifndef _XMLDECODE_HPP_
#define _XMLDECODE_HPP_

//----------------------------------------------------------------------------------------
class XmlEntity;
typedef std::list<XmlEntity *> XmlList;

class XmlEntity
{
	static const int MaxDepth = 128;
	static int errors_, line_, maxdepth_, seq_;
	static FIX8::RegExp rCE_, rCX_;

	std::string tag_, *value_, *decl_;
	int depth_, sequence_, txtline_;

	typedef std::multimap<std::string, XmlEntity *> XmlSubEls;
	XmlSubEls *children_;   // simple n-ary tree

#ifdef HAS_TR1_UNORDERED_MAP
	typedef std::tr1::unordered_map<std::string, std::string> XmlAttrs;
#else
	typedef std::map<std::string, std::string> XmlAttrs;
#endif
	XmlAttrs *attrs_;

public:
	XmlEntity(std::istream& ifs, int txtline=0, int depth=0, const char *rootAttr=0);
	virtual ~XmlEntity();

	int ParseAttrs(const std::string& attlst);

	XmlEntity *find(const std::string& what, bool ignorecase=false,
			const std::string *atag=0, const std::string *aval=0, const char delim='/');
	int find(const std::string& what, XmlList& rlst, bool ignorecase=false,
			const std::string *atag=0, const std::string *aval=0, const char delim='/');
	template<typename T>
	int findSort(const std::string& what, XmlList& rlst, bool ignorecase=false,
			const std::string *atag=0, const std::string *aval=0, const char delim='/')
	{
		int result(find (what, rlst, ignorecase, atag, aval, delim));
		if (result)
			rlst.sort(T());
		return result;
	}

	bool GetAttr(const std::string& what, std::string& target) const;
	bool GetAttrValue(std::string& target) const
	{
		static const std::string valstr("value");
		return GetAttr(valstr, target);
	}

	bool FindAttrGetValue(const std::string& what, std::string& target)
	{
		 XmlEntity *inst(find(what));
		 return inst ? inst->GetAttrValue(target) : false;
	}

	bool findAttrByValue(const std::string& what, const std::string& target);
	template<typename T> T FindAttr(const std::string& what, const T defValue);

	const int GetDepth() const { return depth_; }
	const int GetErrorCnt() const { return errors_; }
	const int GetLineCnt() const { return line_; }
	const int GetLine() const { return txtline_; }
	const int GetSequence() const { return sequence_; }
	const int GetMaxDepth() const { return maxdepth_; }

	const std::string& GetTag() const { return tag_; }
	const std::string *GetVal() const { return value_; }
	const std::string *GetDecl() const { return decl_; }

	static void Reset() { errors_ = maxdepth_ = seq_ = 0; line_ = 1; }
	static XmlEntity *Factory(const std::string& fname);

	const std::string& InplaceXlate (std::string& what);

	friend std::ostream& operator<<(std::ostream& os, const XmlEntity& en);
};

struct EntityOrderComp
{
	bool operator()(const XmlEntity *a, const XmlEntity *b) const
		{ return a->GetSequence() < b->GetSequence(); }
};

#endif // _XMLDECODE_HPP_

