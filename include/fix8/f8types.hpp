//-------------------------------------------------------------------------------------------------
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
//-------------------------------------------------------------------------------------------------
#ifndef FIX8_TYPES_HPP_
#define FIX8_TYPES_HPP_

//-------------------------------------------------------------------------------------------------
#include <map>
#include <algorithm>

//-------------------------------------------------------------------------------------------------
namespace FIX8 {

//-------------------------------------------------------------------------------------------------
using f8String = std::string;
#if defined FIX8_USE_SINGLE_PRECISION
using fp_type = float;
#else
using fp_type = double;
#endif

//-------------------------------------------------------------------------------------------------
/// Supported session process models
enum ProcessModel { pm_thread, pm_pipeline, pm_coro, pm_count };

/// default FIX field separator (^A)
const unsigned char default_field_separator(0x1);

/// default FIX assignment separator (=)
const unsigned char default_assignment_separator('=');

//-------------------------------------------------------------------------------------------------
/// Pair abstraction for use with GeneratedTable
/*! \tparam Key the key
    \tparam Val the value */
template<typename Key, typename Val>
struct _pair
{
	Key _key;
	Val _value;

	const Key& first() const { return _key; }
	const Val& second() const { return _value; }

	/// Sort functor
	static bool Less(const _pair& p1, const _pair& p2) { return p1._key < p2._key; }
};

/// Partial specialisation of Pair abstraction for use with GeneratedTable
/*! \tparam Val the value */
template<typename Val>
struct _pair<const char *, Val>
{
	const char *_key;
	Val _value;

	const char *first() const { return _key; }
	const Val& second() const { return _value; }

	/// Sort functor
	static bool Less(const _pair& p1, const _pair& p2) { return ::strcmp(p1._key, p2._key) < 0; }
};

/// Fast map for statically generated data types. Assumes table is sorted. Complexity is O(logN).
/*! \tparam Key the key
    \tparam Val the value */
template<typename Key, typename Val>
class GeneratedTable
{
public:
	using Pair = _pair<Key, Val>;
	using iterator = Pair*;
	using const_iterator = const Pair*;

#ifndef _MSC_VER
private:
#endif
	/// The actual data set
	const_iterator _pairs;

	/// The number of elements in the data set
	const size_t _pairsz;

	/*! Find a key; complexity log2(N)+2
	  \param key the key to find
	  \return Pair * if found, 0 if not found */
   const_iterator _find(const Key& key) const
   {
		const_iterator res(std::lower_bound (begin(), end(), reinterpret_cast<const Pair&>(key), Pair::Less));
      /// res != end && key >= res
      return res != end() && !Pair::Less(reinterpret_cast<const Pair&>(key), *res) ? res : nullptr;
   }

public:
	///Ctor.
	GeneratedTable(const_iterator pairs, const size_t pairsz)
		: _pairs(pairs), _pairsz(pairsz) {}

	/*! Get iterator to start of Pairs
	  \return pointer to first pair */
	const_iterator begin() const { return _pairs; }

	/*! Get iterator to last + 1 of Pairs
	  \return pointer to last + 1 pair */
	const_iterator end() const { return _pairs + _pairsz; }

	/*! Find a key (reference). If not found, throw InvalidMetadata.
	  Ye Olde Binary Chop
	  \param key the key to find
	  \return value found (reference) */
	const Val& find_ref(const Key& key) const
	{
		const_iterator res(_find(key));
		if (res)
			return res->_value;
		throw InvalidMetadata<Key>(key);
	}

	/*! Find a key (pointer).
	  \param key the key to find
	  \return value found (pointer) or 0 if not found */
	const Val *find_ptr(const Key& key) const
	{
		const_iterator res(_find(key));
		return res ? &res->_value : nullptr;
	}

	/*! Find a key pair record (pointer).
	  \param key the key to find
	  \return key/value pair (pointer) or 0 if not found */
	const_iterator find_pair_ptr(const Key& key) const
	{
		const_iterator res(_find(key));
		return res ? res : nullptr;
	}

	/*! Get the pair at index location
	  \param idx of the pair to retrieve
	  \return pointer to pair or nullptr if not found */
	const_iterator at(const size_t idx) const { return idx < _pairsz ? _pairs + idx : nullptr; }

	/*! Get the number of elements in the data set.
	  \return number of elements */
	size_t size() const { return _pairsz; }
};

//-------------------------------------------------------------------------------------------------
/// Presorted set designed for infrequent insertions but super fast initialisation from a sorted static array.
/// Search complexity is O(logN), ctor complexity approaches O(1), insert is O(N).
/*! \tparam K the key
    \tparam T the value type
    \tparam Comp the comparitor */
template<typename K, typename T, typename Comp>
class presorted_set
{
public:
	using iterator = T*;
	using const_iterator = const T*;
	using result = std::pair<iterator, bool>;

private:
	const size_t _reserve;
	size_t _sz, _rsz;
	T *_arr;

	using internal_result = std::pair<iterator, iterator>;
	using const_internal_result = std::pair<const_iterator, const_iterator>;

	/*! Calculate the amount of space to reserve in set
	  \param sz number of elements currently in set; if 0 retun reserve elements as size to reserve
	  \param res percentage of sz to keep in reserve
	  \return number of elements to additionally reserve (at least 1) */
	static size_t calc_reserve(size_t sz, size_t res)
	{
		if (!sz)  // special case - reserve means number to reserve, not %
			return res;
		const size_t val(sz * res / 100);
		return val ? val : 1;
	}

public:
	/*! ctor - initialise from static sorted set
	  \param arr_start pointer to start of static array to copy elements from
	  \param sz number of elements in set to copy
	  \param reserve percentage of sz to keep in reserve */
	presorted_set(const_iterator arr_start, const size_t sz, const size_t reserve=FIX8_RESERVE_PERCENT) : _reserve(reserve),
		_sz(sz), _rsz(_sz + calc_reserve(_sz, _reserve)), _arr(new T[_rsz])
			{ memcpy(_arr, arr_start, _sz * sizeof(T)); }

	/*! copy ctor
	  \param from presorted_set object to copy */
	presorted_set(const presorted_set& from) : _reserve(from._reserve),
		_sz(from._sz), _rsz(from._rsz), _arr(from._arr ? new T[_rsz] : 0)
			{ if (_arr) memcpy(_arr, from._arr, _sz * sizeof(T)); }

	/*! ctor - initialise an empty set; defer memory allocation;
	  \param sz number of elements to initially allocate
	  \param reserve percentage of sz to keep in reserve */
	explicit presorted_set(const size_t sz=0, const size_t reserve=FIX8_RESERVE_PERCENT) : _reserve(reserve),
		_sz(sz), _rsz(_sz + calc_reserve(_sz, _reserve)), _arr() {}

	/// dtor
	~presorted_set() { delete[] _arr; }

	/*! Find an element with the given value
	  \param what element to find
	  \param answer true if element is found
	  \return pointer to found element or pointer to location where element would be inserted */
	iterator find(const T what, bool& answer)
	{
		const internal_result res(std::equal_range (_arr, _arr + _sz, what, Comp()));
		answer = res.first != res.second;
		return res.first;
	}

	/*! Find an element with the given key
	  \param key to find
	  \param answer true if element is found
	  \return pointer to found element or pointer to location where element would be inserted */
	iterator find(const K key, bool& answer) { return find(T(key), answer); }

	/*! Find an element with the given key (const version)
	  \param key to find
	  \return pointer to found element or end() */
	const_iterator find(const K key) const { return find(T(key)); }

	/*! Find an element with the given value (const version)
	  \param what element to find
	  \return pointer to found element or end() */
	const_iterator find(const T what) const
	{
		const const_internal_result res(std::equal_range (_arr, _arr + _sz, what, Comp()));
		return res.first != res.second ? res.first : end();
	}

	/*! Find an element with the given key
	  \param key to find
	  \return pointer to found element or end() */
	iterator find(const K key) { return find(T(key)); }

	/*! Find an element with the given value
	  \param what value to find
	  \return pointer to found element or end() */
	iterator find(const T what)
	{
		internal_result res(std::equal_range (_arr, _arr + _sz, what, Comp()));
		return res.first != res.second ? res.first : end();
	}

	/*! Get the element at index location
	  \param idx of the pair to retrieve
	  \return const_iterator to element or end() if not found */
	const_iterator at(const size_t idx) const { return idx < _sz ? _arr + idx : end(); }

	/*! Insert an element into the set
	  \param what pointer to element to insert
	  \return result with iterator to insert location and true or end() and false */
	result insert(const_iterator what)
	{
		if (!_sz)
		{
			_arr = new T[_rsz];
			memcpy(_arr, what, sizeof(T));
			++_sz;
			return result(_arr, true);
		}

		bool answer;
		iterator where(find(*what, answer));
		if (answer)
			return result(end(), false);

		if (_sz < _rsz)
		{
			memmove(where + 1, where, (end() - where) * sizeof(T));
			memcpy(where, what, sizeof(T));
		}
		else
		{
			iterator new_arr(new T[_rsz = _sz + calc_reserve(_sz, _reserve)]);
			const size_t wptr(where - _arr);
			if (wptr > 0)
				memcpy(new_arr, _arr, sizeof(T) * wptr);
			memcpy(new_arr + wptr, what, sizeof(T));
			memcpy(new_arr + wptr + 1, where, (end() - where) * sizeof(T));
			delete[] _arr;
			_arr = new_arr;
		}
		++_sz;
		return result(where, true);
	}

	/*! Find the distance between two iterators
	  \param what_begin start iterator
	  \param what_end end iterator (must be >= to what_begin
	  \return distance in elements */
	static size_t distance(const_iterator what_begin, const_iterator what_end)
		{ return what_end - what_begin; }

	/*! Insert a range of elements into the set
	  \param what_begin pointer to 1st element to insert
	  \param what_end pointer to nth element + 1 to insert */
	void insert(const_iterator what_begin, const_iterator what_end)
	{
		for (const_iterator ptr(what_begin); ptr < what_end; ++ptr)
			if (!insert(ptr).second)
				break;
	}

	/*! Clear the set (does not delete) */
	void clear() { _sz = 0; }

	/*! Obtain the number of elements in the set
	  \return the number of elements */
	size_t size() const { return _sz; }

	/*! Check if the set is empty
	  \return true if empty */
	bool empty() const { return _sz == 0; }

	/*! Obtain the number of elements that can be inserted before reallocating
	  \return reserved + sz */
	size_t rsize() const { return _rsz; }

	/*! Get a pointer to the first element
	  \return the first element */
	iterator begin() { return _arr; }

	/*! Get a pointer to the last element + 1
	  \return the last element + 1 */
	iterator end() { return _arr + _sz; }

	/*! Get a const pointer to the first element
	  \return the first element */
	const_iterator begin() const { return _arr; }

	/*! Get a const pointer to the last element + 1
	  \return the last element + 1 */
	const_iterator end() const { return _arr + _sz; }
};

//-------------------------------------------------------------------------------------------------
/// Type2Type idiom. Variadic template version. Kudos to Andrei Alexandrescu
/*! \tparam T type to convey
	 \tparam args further types to convey */
template<typename T, typename... args> struct Type2Type {};

//-------------------------------------------------------------------------------------------------
/// Template provides insert operator that inserts nothing
struct null_insert { template <typename T> null_insert& operator<<(const T&) { return *this; } };

} // FIX8

#endif // F8_TYPES_HPP_
