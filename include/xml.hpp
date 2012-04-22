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
#ifndef _XML_ENTITY_HPP_
#define _XML_ENTITY_HPP_

//----------------------------------------------------------------------------------------
/// A simple xml parser with Xpath style lookup.
class XmlElement
{
	/// Maximum depth levels supported.
	enum { MaxDepth = 128 };
	static int errors_, line_, maxdepth_, seq_;
	static FIX8::RegExp rCE_, rCX_;

	std::string tag_, *value_, *decl_;
	int depth_, sequence_, txtline_, chldcnt_, subidx_;

	/// Comparitor.
	struct EntityOrderComp
	{
		bool operator()(const XmlElement *a, const XmlElement *b) const
			{ return a->GetSequence() < b->GetSequence(); }
	};

public:
	/*! XmlSet ordering preserved from source file */
	typedef std::set<XmlElement *, EntityOrderComp> XmlSet;

private:
	typedef std::multimap<std::string, XmlElement *> XmlSubEls;
	/// simple n-ary tree
	XmlSubEls *children_;

	/// Set of all child entities in file order
	XmlSet *ordchildren_;
	static XmlSet emptyset_;

public:
	typedef std::map<std::string, std::string> XmlAttrs;
	XmlAttrs *attrs_;
	static XmlAttrs emptyattrs_;

	/// Copy Ctor. Non-copyable.
	XmlElement(const XmlElement&);

	/// Assignment operator. Non-copyable.
	XmlElement& operator=(const XmlElement&);

public:
	/*! Ctor.
	  \param ifs input file stream
	  \param subidx the subindex for this child
	  \param txtline xml sourcefile line number
	  \param depth depth nesting level
	  \param rootAttr root attribute string */
	XmlElement(std::istream& ifs, int subidx, int txtline=0, int depth=0, const char *rootAttr=0);

	/// Dtor.
	virtual ~XmlElement();

	/*! Parse the xml attributes from an element.
	  \param attlst string of attributes
	  \return number of attributes extracted */
	int ParseAttrs(const std::string& attlst);

	/*! Find an element with a given entity name, attribute name and attribute value.
	  \param what the entity name to search for
	  \param ignorecase if true ignore case of entity name
	  \param atag the attribute name
	  \param aval the attribute value
	  \param delim the Xpath delimiter
	  \return the found entity or 0 if not found */
	XmlElement *find(const std::string& what, bool ignorecase=false,
		const std::string *atag=0, const std::string *aval=0, const char delim='/');

	/*! Recursively find all elements with a given entity name, attribute name and attribute value.
	  \param what the entity name to search for
	  \param eset target XmlSet to place results
	  \param ignorecase if true ignore case of entity name
	  \param atag the attribute name
	  \param aval the attribute value
	  \param delim the Xpath delimiter
	  \return the number of found entities */
	int find(const std::string& what, XmlSet& eset, bool ignorecase=false,
		const std::string *atag=0, const std::string *aval=0, const char delim='/');

	/*! Find an attribute's with the given name.
	  \param what attribute to find
	  \param target where to place value
	  \return true if found */
	bool GetAttr(const std::string& what, std::string& target) const;

	/*! Find an attribute's value with the name "value".
	  \param target where to place value
	  \return true if found */
	bool GetAttrValue(std::string& target) const
	{
		static const std::string valstr("value");
		return GetAttr(valstr, target);
	}

	/*! Find an element and obtain the attribute's value with the name "value".
	  \param what entity name to find
	  \param target where to place value
	  \return true if found */
	bool FindAttrGetValue(const std::string& what, std::string& target)
	{
		 XmlElement *inst(find(what));
		 return inst ? inst->GetAttrValue(target) : false;
	}

	/*! Find an attribute with the given name and value.
	  \param what attribute to find
	  \param value attribute value
	  \return true if found */
	bool findAttrByValue(const std::string& what, const std::string& value);

	/*! Find an attribute with the given name and return its typed value.
	  \tparam type of target attribute
	  \param what attribute to find
	  \param defValue value to return if attribute was not found
	  \return the value of the found attribute */
	template<typename T>
	T FindAttr(const std::string& what, const T defValue) const
	{
		if (attrs_)
		{
			XmlAttrs::iterator itr(attrs_->find(what));
			if (itr != attrs_->end())
				return FIX8::GetValue<T>(itr->second);
		}

		return defValue;
	}

	/*! Find an attribute with the given name and populate supplied target with its typed value.
	  \tparam type of target attribute
	  \param what attribute to find
	  \param target location to return value
	  \return reference to value of the found attribute */
	template<typename T>
	T& FindAttrRef(const std::string& what, T& target) const
	{
		if (attrs_)
		{
			XmlAttrs::iterator itr(attrs_->find(what));
			if (itr != attrs_->end())
				target = FIX8::GetValue<T>(itr->second);
		}

		return target;
	}

	/*! Perform xml translation on the supplied string inplace.
	  Translate predefined entities and numeric character references.
	  \param what source string to translate
	  \return the translated string */
	const std::string& InplaceXlate (std::string& what);

	/*! Get the depth of this element
	  \return the depth */
	int GetDepth() const { return depth_; }

	/*! Get the global error count.
	  \return the error count */
	int GetErrorCnt() const { return errors_; }

	/*! Get the actual current source line.
	  \return the source line */
	int GetLineCnt() const { return line_; }

	/*! Get the count of children this element has.
	  \return the number of children */
	int GetChildCnt() const { return chldcnt_; }

	/*! Get the source line of this element (element order).
	  \return the line */
	int GetLine() const { return txtline_; }

	/*! Get the subindex.
	  \return the subindex */
	int GetSubIdx() const { return subidx_; }

	/*! Get the sequence number for this element (incremented for each new element).
	  \return the sequence number */
	int GetSequence() const { return sequence_; }

	/*! Get the maximum depth supported.
	  \return the maximum depth */
	int GetMaxDepth() const { return maxdepth_; }


	/*! Get the element tag name.
	  \return the tag */
	const std::string& GetTag() const { return tag_; }

	/*! Get the element value.
	  \return the value */
	const std::string *GetVal() const { return value_; }

	/*! Get the declaration if available.
	  \return the depth */
	const std::string *GetDecl() const { return decl_; }

	/// Reset all counters, errors and sequences.
	static void Reset() { errors_ = maxdepth_ = seq_ = 0; line_ = 1; }

	/*! Create a new root element (and recursively parse) from a given xml filename.
	  \param fname the xml filename
	  \return the depth */
	static XmlElement *Factory(const std::string& fname);

	/*! Get an iterator to the first child attribute.
	  \return const_iterator to first attribute */
	XmlAttrs::const_iterator abegin() const { return attrs_ ? attrs_->begin() : emptyattrs_.end(); }

	/*! Get an iterator to the last+1 child attribute.
	  \return const_iterator to last+1 attribute */
	XmlAttrs::const_iterator aend() const { return attrs_ ? attrs_->end() : emptyattrs_.end(); }

	/*! Get an iterator to the first child element.
	  \return const_iterator to first element */
	XmlSet::const_iterator begin() const { return ordchildren_ ? ordchildren_->begin() : emptyset_.end(); }

	/*! Get an iterator to the last+1 child element.
	  \return const_iterator to last+1 element */
	XmlSet::const_iterator end() const { return ordchildren_ ? ordchildren_->end() : emptyset_.end(); }

	/*! Inserter friend.
	    \param os stream to send to
	    \param en XmlElement REFErence
	    \return stream */
	friend std::ostream& operator<<(std::ostream& os, const XmlElement& en);
};

#endif // _XML_ENTITY_HPP_

