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
#ifndef _XML_ELEMENT_HPP_
# define _XML_ELEMENT_HPP_

//-----------------------------------------------------------------------------------------
#include <set>

//----------------------------------------------------------------------------------------
using Str2Chr = std::map<std::string, unsigned char>;

//----------------------------------------------------------------------------------------
/// A simple xml parser with Xpath style lookup.
class XmlElement
{
	/// XML entity char lookup
	static const Str2Chr stringtochar_;

	/// Maximum depth levels supported.
	enum { MaxDepth = 128 };
	static FIX8::RegExp rCE_, rCX_, rIn_, rEn_, rEv_;

	XmlElement *parent_, *root_;

	int errors_, line_, incline_, maxdepth_, seq_;
	std::string inclusion_;

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
	using XmlSet = std::set<const XmlElement *, EntityOrderComp>;
	using XmlAttrs = std::map<std::string, std::string>;
	XmlAttrs *attrs_;
	F8API static const XmlAttrs emptyattrs_;

	enum Flags { noextensions, nocase };
	using XmlFlags = FIX8::ebitset<Flags>;
	static XmlFlags flags_;
	static void set_flags(XmlFlags flags) { flags_ = flags; }

private:
	using XmlSubEls = std::multimap<std::string, XmlElement *>;
	/// simple n-ary tree
	XmlSubEls *children_;
	bool _was_include;

	/// Set of all child elements in file order
	XmlSet *ordchildren_;
	F8API static const XmlSet emptyset_;

public:
	/*! Ctor.
	  \param ifs input file stream
	  \param subidx the subindex for this child
	  \param parent parent XmlElement element
	  \param txtline xml sourcefile line number
	  \param depth depth nesting level
	  \param rootAttr root attribute string */
	F8API XmlElement(std::istream& ifs, int subidx, XmlElement *parent=nullptr, int txtline=0, int depth=0, const char *rootAttr=nullptr);

	/// Dtor.
	F8API virtual ~XmlElement();

	/// Copy Ctor. Non-copyable.
	XmlElement(const XmlElement&) = delete;

	/// Assignment operator. Non-copyable.
	XmlElement& operator=(const XmlElement&) = delete;

	/*! Parse the xml attributes from an element.
	  \param attlst string of attributes
	  \return number of attributes extracted */
	F8API int ParseAttrs(const std::string& attlst);

	/*! Find an element with a given name, attribute name and attribute value.
	  \param what the name to search for
	  \param atag the attribute name
	  \param aval the attribute value
	  \param delim the Xpath delimiter
	  \return the found or 0 if not found */
	F8API const XmlElement *find(const std::string& what,
		const std::string *atag=nullptr, const std::string *aval=nullptr, const char delim='/') const;

	/*! Recursively find all elements with a given name, attribute name and attribute value.
	  \param what the name to search for
	  \param eset target XmlSet to place results
	  \param atag the attribute name
	  \param aval the attribute value
	  \param delim the Xpath delimiter
	  \return the number of found elements */
	F8API int find(const std::string& what, XmlSet& eset,
		const std::string *atag=nullptr, const std::string *aval=nullptr, const char delim='/') const;

	/*! Find a child element with a given name, attribute name and attribute value. This version assumes the base of the
	  search term already contains the current xpath tag and delimiter.
	  \param what the name to search for
	  \param atag the attribute name
	  \param aval the attribute value
	  \param delim the Xpath delimiter
	  \return the found or 0 if not found */
	F8API const XmlElement *find_child(const std::string& what,
		const std::string *atag=nullptr, const std::string *aval=nullptr, const char delim='/') const
	{
		const std::string sterm(tag_ + '/' + what);
		return find(sterm, atag, aval, delim);
	}

	/*! Recursively find all child elements with a given name, attribute name and attribute value.
		This version assumes the base of the search term already contains the current xpath tag and delimiter.
	  \param what the name to search for
	  \param eset target XmlSet to place results
	  \param atag the attribute name
	  \param aval the attribute value
	  \param delim the Xpath delimiter
	  \return the number of found elements */
	F8API int find_child(const std::string& what, XmlSet& eset,
		const std::string *atag=nullptr, const std::string *aval=nullptr, const char delim='/') const
	{
		const std::string sterm(tag_ + '/' + what);
		return find(sterm, eset, atag, aval, delim);
	}

	/*! Find an attribute's with the given name.
	  \param what attribute to find
	  \param target where to place value
	  \return true if found */
	F8API bool GetAttr(const std::string& what, std::string& target) const;

	/*! Find an attribute's value with the name "value".
	  \param target where to place value
	  \return true if found */
	bool GetAttrValue(std::string& target) const
	{
		static const std::string valstr("value");
		return GetAttr(valstr, target);
	}

	/*! Check if an attribute with the given name is present.
	  \param what attribute to find
	  \return true if found */
	bool HasAttr(const std::string& what) const
		{ return attrs_ ? attrs_->find(what) != attrs_->end() : false; }

	/*! Find an element and obtain the attribute's value with the name "value".
	  \param what name to find
	  \param target where to place value
	  \return true if found */
	bool FindAttrGetValue(const std::string& what, std::string& target)
	{
		 const XmlElement *inst(find(what));
		 return inst ? inst->GetAttrValue(target) : false;
	}

	/*! Find an attribute with the given name and value.
	  \param what attribute to find
	  \param value attribute value
	  \return true if found */
	F8API bool findAttrByValue(const std::string& what, const std::string& value) const;

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
				return FIX8::get_value<T>(itr->second);
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
				target = FIX8::get_value<T>(itr->second);
		}

		return target;
	}

	/*! Insert an element as a child of this element
	  \param what elekent to insert
	  \return true if successful */
	F8API bool Insert( XmlElement *what );

	/*! Perform xml translation on the supplied string inplace.
	  Translate predefined entities and numeric character references.
	  \param what source string to translate
	  \return the translated string */
	F8API const std::string& InplaceXlate( std::string& what );

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
	int GetMaxDepthPermitted() const { return MaxDepth; }

	/*! Get the maximum depth from this node
	  \return the maximum depth */
	int GetMaxDepth() const { return maxdepth_; }

	/*! Get the full xpath for the element; recursive
	  \return the path */
	std::string GetPath() const { return parent_ ? parent_->GetPath() + '/' + tag_ : "//" + tag_; }

	/*! Get the element tag name.
	  \return the tag */
	const std::string& GetTag() const { return tag_; }

	/*! Generate a formatted string for error reporting
	  \return formatted string */
	std::string GetLocString() const
	{
		std::ostringstream ostr;
		ostr << '"' << tag_ << "\" (" << GetPath() << ':' << txtline_ << ')';
		return ostr.str();
	}

	/*! Get the element value.
	  \return the value */
	const std::string *GetVal() const { return value_; }

	/*! Get the declaration if available.
	  \return the declaration */
	const std::string *GetDecl() const { return decl_; }

	/*! Create a new root element (and recursively parse) from a given xml filename.
	  \param istr an open std::istream of an xml document
	  \param docpath string providing info about the document or path
	  \return the new element */
	F8API static XmlElement *Factory(std::istream& istr, const char *docpath=nullptr);

	/*! Create a new root element (and recursively parse) from a given xml istream.
	  \param fname the xml filename
	  \return the new element */
	F8API static XmlElement *Factory(const std::string& fname);

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

	/*! Get the root element of this tree, const version
	  \return the root element */
	const XmlElement *GetRoot() const { return parent_ ? parent_->root_ : this; }

	/*! Get the parent element of this tree, const version
	  \return the parent element, 0 if no parent */
	const XmlElement *GetParent() const { return parent_; }

	/*! Inserter friend.
	    \param os stream to send to
	    \param en XmlElement REFErence
	    \return stream */
	friend F8API std::ostream& operator<<(std::ostream& os, const XmlElement& en);
};

F8API std::ostream& operator<<( std::ostream& os, const XmlElement& en );

#endif // _XML_ELEMENT_HPP_

