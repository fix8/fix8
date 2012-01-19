//-------------------------------------------------------------------------------------------------
#if 0

fix8 is released under the New BSD License.

Copyright (c) 2010-12, David L. Dight <fix@fix8.org>
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
//-------------------------------------------------------------------------------------------------
#ifndef _F8_TYPES_HPP_
#define _F8_TYPES_HPP_

//-------------------------------------------------------------------------------------------------
namespace FIX8 {

typedef std::string f8String;
typedef std::ostringstream f8ostrstream;

const unsigned char default_field_separator(0x1);

//-------------------------------------------------------------------------------------------------
/*! Fast map for statically generated data types. Assumes table is sorted. O(logN).
  \tparam Key the key
  \tparam Val the value */
template<typename Key, typename Val>
class GeneratedTable
{
	/// A pair structure to statically declare the data set
	struct Pair
	{
		Key _key;
		Val _value;

		/// Sort functor
		struct Less
		{
			bool operator()(const Pair &p1, const Pair &p2) const { return p1._key < p2._key; }
		};
	};

	/// The actual data set
	static const Pair _pairs[];

	/// The number of elements in the data set
	static const size_t _pairsz;

	typedef Val NotFoundType;
	/// The value to return when the key is not found
	static const NotFoundType _noval;

	typedef typename std::pair<const Pair *, const Pair *> PResult;

public:
	/*! Find a key (reference). If not found, throw InvalidMetadata.
	  Ye Olde Binary Chop
	  \param key the key to find
	  \return value found (reference) */
	static const Val& find_ref(const Key& key)
	{
		const Pair what = { key };
		PResult res(std::equal_range (_pairs, _pairs + _pairsz, what, typename Pair::Less()));
		if (res.first != res.second)
			return res.first->_value;
		static const std::string error_str("Invalid metadata or entry not found");
		throw InvalidMetadata(error_str);
	}

	/*! Find a key (value).
	  \param key the key to find
	  \return value found (value) or _noval if not found */
	static const Val find_val(const Key& key)
	{
		const Pair what = { key };
		PResult res(std::equal_range (_pairs, _pairs + _pairsz, what, typename Pair::Less()));
		return res.first != res.second ? res.first->_value : _noval;
	}

	/*! Find a key (pointer).
	  \param key the key to find
	  \return value found (pointer) or 0 if not found */
	static const Val *find_ptr(const Key& key)
	{
		const Pair what = { key };
		PResult res(std::equal_range (_pairs, _pairs + _pairsz, what, typename Pair::Less()));
		return res.first != res.second ? &res.first->_value : 0;
	}

	///Ctor.
	GeneratedTable() {}

	/*! Get the number of elements in the data set.
	  \return number of elements */
	static const size_t size() { return _pairsz; }
};

//-------------------------------------------------------------------------------------------------
/*! A specialised map to enable native static initalisation.
  \tparam Key the key
  \tparam Val the value
  \tparam Compare the comparitor */
template<typename Key, typename Val, typename Compare=std::less<Key> >
struct StaticTable
{
	typedef typename std::map<Key, Val, Compare> TypeMap;
	typedef typename TypeMap::value_type TypePair;
	typedef Val NotFoundType;

	/// The actual data set
	static const TypePair _valueTable[];

	/// The container
	static const TypeMap _valuemap;

	/// The value to return when the key is not found
	static const NotFoundType _noval;

	/// Ctor.
	StaticTable() {}

	/*! Find a key (value).
	  \param key the key to find
	  \return value found (value) or _noval if not found */
	static const Val find_value(const Key& key)
	{
		typename TypeMap::const_iterator itr(_valuemap.find(key));
		return itr != _valuemap.end() ? itr->second : _noval;
	}

	/*! Find a key (reference).
	  \param key the key to find
	  \return value found (reference) or _noval if not found */
	static const Val& find_ref(const Key& key)
	{
		typename TypeMap::const_iterator itr(_valuemap.find(key));
		return itr != _valuemap.end() ? itr->second : _noval;
	}

	/*! Find a key (pointer).
	  \param key the key to find
	  \return value found (pointer) or 0 if not found */
	static const Val *find_ptr(const Key& key)
	{
		typename TypeMap::const_iterator itr(_valuemap.find(key));
		return itr != _valuemap.end() ? &itr->second : 0;
	}

	/*! Get the number of elements in the data set.
	  \return number of elements */
	static const size_t get_count() { return _valuemap.size(); }

	/*! Get a pointer to the end of the data set. Used as an end inputiterator for initialisation.
	  \return pointer to end of table */
	static const TypePair *get_table_end() { return _valueTable + sizeof(_valueTable)/sizeof(TypePair); }
};

} // FIX8

#endif // _F8_TYPES_HPP_
