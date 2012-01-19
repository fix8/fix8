//-----------------------------------------------------------------------------------------
#if 0

fix8 is released under the New BSD License.

Copyright (c) 2007-2012, David L. Dight <fix@fix8.org>
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
#ifndef _F8_UTILS_HPP_
#define _F8_UTILS_HPP_

#include <tbb/atomic.h>
#include <tbb/mutex.h>

namespace FIX8 {

//----------------------------------------------------------------------------------------
std::string& InPlaceStrToUpper(std::string& src);
std::string& InPlaceStrToLower(std::string& src);
std::string& CheckAddTrailingSlash(std::string& source);
std::string Str_error(const int err, const char *str=0);
const std::string& GetTimeAsStringMS(std::string& result, class Tickval *tv=0, const unsigned dplaces=6);

//----------------------------------------------------------------------------------------
/*! case insensitive std::string == std::string operator
  \tparam _CharT char type
  \tparam _Traits char traits
  \tparam _Alloc allocator
  \param __lhs left hand value
  \param __rhs right hand value
  \return true if strings are equivalent */
template<typename _CharT, typename _Traits, typename _Alloc>
	inline bool operator% (const std::basic_string<_CharT, _Traits, _Alloc>& __lhs,
		const std::basic_string<_CharT, _Traits, _Alloc>& __rhs)
			{ return strcasecmp(__lhs.c_str(), __rhs.c_str()) == 0; }

/*! case insensitive char* == std::string operator
  \tparam _CharT char type
  \tparam _Traits char traits
  \tparam _Alloc allocator
  \param __lhs left hand value
  \param __rhs right hand value
  \return true if strings are equivalent */
template<typename _CharT, typename _Traits, typename _Alloc>
	inline bool operator% (const _CharT* __lhs, const std::basic_string<_CharT, _Traits, _Alloc>& __rhs)
		{ return strcasecmp(__lhs, __rhs.c_str()) == 0; }

/*! case insensitive std::string == char* operator
  \tparam _CharT char type
  \tparam _Traits char traits
  \tparam _Alloc allocator
  \param __lhs left hand value
  \param __rhs right hand value
  \return true if strings are equivalent */
template<typename _CharT, typename _Traits, typename _Alloc>
	inline bool operator% (const std::basic_string<_CharT, _Traits, _Alloc>& __lhs, const _CharT* __rhs)
		{ return strcasecmp(__lhs.c_str(), __rhs) == 0; }

/*! case insensitive std::string < std::string operator
  \tparam _CharT char type
  \tparam _Traits char traits
  \tparam _Alloc allocator
  \param __lhs left hand value
  \param __rhs right hand value
  \return true if lhs < rhs */
template<typename _CharT, typename _Traits, typename _Alloc>
	inline bool operator^ (const std::basic_string<_CharT, _Traits, _Alloc>& __lhs,
		const std::basic_string<_CharT, _Traits, _Alloc>& __rhs)
			{ return strcasecmp(__lhs.c_str(), __rhs.c_str()) < 0; }

//----------------------------------------------------------------------------------------
/*! C++11 inspired scoped pointer.
  \tparam T typename */
template <typename T>
class scoped_ptr
{
	T *ptr_;

	/// Copy Ctor. Non-copyable.
	scoped_ptr(const scoped_ptr&);

	/// Assignment operator. Non-copyable.
	void operator=(const scoped_ptr&);

public:
	/*! Ctor.
	  \param p pointer to T */
	explicit scoped_ptr(T *p=0) : ptr_(p) {}

	/// Dtor. Destroys object.
	~scoped_ptr() { delete ptr_; }

	/*! Equivalence operator (other is scoped_ptr)
	  \tparam U type of that object
	  \return true if objects are equivalent */
	template <typename U>
	bool operator==(const scoped_ptr<U>& that) const { return ptr_ == that.get(); }

	/*! Equivalence operator (other is ptr)
	  \tparam U type of that object
	  \return true if objects are equivalent */
	template <typename U>
	bool operator==(const scoped_ptr<U> *that) const { return ptr_ == that; }

	/*! Non-equivalence operator (other is scoped_ptr)
	  \tparam U type of that object
	  \return true if objects are not equal */
	template <typename U>
	bool operator!=(const scoped_ptr<U>& that) const { return ptr_ != that.get(); }

	/*! Non-equivalence operator (other is scoped_ptr)
	  \tparam U type of that object
	  \return true if objects are not equal */
	template <typename U>
	bool operator!=(const scoped_ptr<U> *that) const { return ptr_ != that; }

	/*! Equivalence operator (other is scoped_ptr)
	  \return true if objects are equivalent */
	bool operator==(const scoped_ptr<T>& that) const { return ptr_ == that.get(); }

	/*! Equivalence operator (other is ptr)
	  \return true if objects are equivalent */
	bool operator==(const T *that) const { return (ptr_ == that); }

	/*! Non-equivalence operator (other is scoped_ptr)
	  \return true if objects are not equal */
	bool operator!=(const scoped_ptr<T>& that) const { return ptr_ != that.get(); }

	/*! Non-equivalence operator (other is ptr)
	  \return true if objects are not equal */
	bool operator!=(const T *that) const { return ptr_ != that; }

	/*! Member selection operator.
	  \return pointer to object */
	T *operator->() const { return ptr_; }

	/*! Member dereference operator.
	  \return object */
	T& operator*() const { return *ptr_; }

	/*! Member dereference operator.
	  \return object */
	T *release() { T *tmp(ptr_); ptr_ = 0; return tmp; }

	/*! Replace the pointer with the supplied pointer.
	  \return the original pointer */
	T *Reset(T *p=0) { delete ptr_; return ptr_ = p; }

	/*! Get the object pointer.
	  \return object */
	T *get() const { return ptr_; }
};

//----------------------------------------------------------------------------------------
/// A class to contain regex matches using RegExp.
class RegMatch
{
	/// Maximum number of sub-expressions.
	static const int SubLimit_ = 32;

	regmatch_t subexprs_[SubLimit_];
	int subCnt_;

public:
	/// Ctor.
	RegMatch() : subexprs_(), subCnt_() {}

	/// Ctor.
	virtual ~RegMatch() {}

	/*! Get the number of sub-expressions found.
	  \return number of sub-expression */
	const unsigned SubCnt() const { return subCnt_; }

	/*! Get the size (length) of the specified sub-expression.
	  \param which sub-expression index (0 based)
	  \return size of sub-expression, -1 if not found */
	const size_t SubSize(const int which=0) const
		{ return which < subCnt_ ? subexprs_[which].rm_eo - subexprs_[which].rm_so : -1; }

	/*! Get the starting offset of the specified sub-expression.
	  \param which sub-expression index (0 based)
	  \return offset of the sub-expression, -1 if not found */
	const unsigned SubPos(const int which=0) const
		{ return which < subCnt_ ? subexprs_[which].rm_so : -1; }

	friend class RegExp;
};

//----------------------------------------------------------------------------------------
/// POSIX regex wrapper class.
class RegExp
{
	/// Maximum length of an error message.
	static const int MaxErrLen_ = 256;

	regex_t reg_;
	const std::string pattern_;
	std::string errString;
	int errCode_;

public:
	/*! Ctor.
	  \param pattern regular expression to compile. errCode and errString set on error.
	  \param flags POSIX regcomp flags */
	RegExp(const char *pattern, const int flags=0) : pattern_(pattern)
	{
		if ((errCode_ = regcomp(&reg_, pattern_.c_str(), REG_EXTENDED|flags)) != 0)
		{
			char rbuf[MaxErrLen_];
			regerror(errCode_, &reg_, rbuf, MaxErrLen_);
			errString = rbuf;
		}
	}

	/// Dtor. Destroys internal compiled expression.
	virtual ~RegExp() { if (errCode_ == 0) regfree(&reg_); }

	/*! Search a string.
	  \param match reference to a RegMatch object
	  \param source string to search
	  \param subExpr number of sub-expression
	  \param offset to start searching
	  \return number of sub-expressions found (0=none) */
	int SearchString(RegMatch& match, const std::string& source, const int subExpr, const int offset=0) const
	{
		match.subCnt_ = 0;
		if (regexec(&reg_, source.c_str() + offset, subExpr <= RegMatch::SubLimit_ ? subExpr : RegMatch::SubLimit_, match.subexprs_, 0) == 0)
			while (match.subCnt_ < subExpr && match.subexprs_[match.subCnt_].rm_so != -1)
				++match.subCnt_;
		return match.subCnt_;
	}

	/*! Extract a sub-expression.
	  \param match reference to a RegMatch object
	  \param source source string
	  \param target location to place sub-experssion
	  \param offset to start searching
	  \param num desired sub-expression
	  \return the target string */
	std::string& SubExpr(RegMatch& match, const std::string& source, std::string& target, const int offset=0, const int num=0) const
	{
		if (num < match.subCnt_)
			target = source.substr(offset + match.subexprs_[num].rm_so, match.subexprs_[num].rm_eo - match.subexprs_[num].rm_so);
		else
			target.empty();
		return target;
	}

	/*! Erase a sub-expression from a string.
	  \param match reference to a RegMatch object
	  \param source source string
	  \param num desired sub-expression
	  \return the source string */
	std::string& Erase(RegMatch& match, std::string& source, const int num=0) const
	{
		if (num < match.subCnt_)
			source.erase(match.subexprs_[num].rm_so, match.subexprs_[num].rm_eo - match.subexprs_[num].rm_so);
		return source;
	}

	/*! Replace a sub-expression with a string.
	  \param match reference to a RegMatch object
	  \param source source string
	  \param with replacement string
	  \param num desired sub-expression
	  \return the source string */
	std::string& Replace(RegMatch& match, std::string& source, const std::string& with, const int num=0) const
	{
		if (num < match.subCnt_)
			source.replace(match.subexprs_[num].rm_so, match.subexprs_[num].rm_eo - match.subexprs_[num].rm_so, with);
		return source;
	}

	/*! Get the regular expression pattern.
	  \return the pattern string */
	const std::string& GetPattern() const { return pattern_; }

	/*! Get the error string (set when Ctor fails to compile).
	  \return the error string */
	const std::string& ErrString() const { return errString; }

	/*! Check if a pattern did not compile ok.
	  \return true on failure */
	bool operator!() const { return errCode_; }

	/*! Check if a pattern compiled ok.
	  \return true onsuccess */
	operator void*() { return errCode_ ? 0 : this; }
};

//----------------------------------------------------------------------------------------
/// Case-insensitive string comparison, pointer version.
struct StringPtrLessThanNoCase
{
	bool operator()(const std::string *a, const std::string *b) const
		{ return *a ^ *b; }
};

/// Case-sensitive string comparison, pointer version.
struct StringPtrLessThan
{
	bool operator()(const std::string *a, const std::string *b) const
		{ return *a < *b; }
};

/// Case-insensitive string comparison.
struct StringLessThanNoCase
{
	bool operator()(const std::string& a, const std::string& b) const
		{ return a ^ b; }
};

//----------------------------------------------------------------------------------------
/*! Extract a typed value from a string.
  \tparam typename
  \param source source string
  \param defval value to return if source string is empty
  \return the extracted value. */
template<typename T>
inline T GetValue(const std::string& source, T defval)
{
	if (source.empty())
		return defval;
	std::istringstream istr(source);
	T result(defval);
	istr >> result;
	return result;
}

/*! Extract a typed value from a string.
  \tparam typename
  \param source source string
  \return the extracted value. */
template<typename T>
inline T GetValue(const std::string& source)
{
	std::istringstream istr(source);
	T result;
	istr >> result;
	return result;
}

/*! Extract a bool value from a string.
  \tparam typename
  \param source source string
  \return the extracted value. */
template<>
inline bool GetValue(const std::string& source)
{
	if (source.empty())
		return false;
#if !defined XMLENTITY_STRICT_BOOL
	return source % "true" || source % "yes" || source % "y" || source == "1";
#else
	bool result(false);
	std::istringstream istr(source);
	istr >> std::boolalpha >> result;
	return result;
#endif
}

/*! Insert a typed value into a string.
  \tparam typename
  \param a source value
  \param target string to place result
  \return the inserted string */
template<typename T>
inline const std::string& PutValue(const T& a, std::string& target)
{
	std::ostringstream ostr;
	ostr << a;
	return target = ostr.str();
}

//----------------------------------------------------------------------------------------
/*! Bitset for enums.
    \tparam T the enum type
    \tparam B the integral type of the enumeration */
template<typename T, typename B=unsigned int>
class ebitset
{
	typedef B integral_type;
	integral_type a_;

public:
	/// Ctor.
	ebitset() : a_() {}

	/*! Ctor.
	    \param from ebitset_r to copy */
	ebitset(const ebitset<T, B>& from) : a_(from.a_) {}

	/*! Ctor.
	    \param a integral type to construct from */
	explicit ebitset(const integral_type a) : a_(a) {}

	/*! Ctor.
	    \param sbit enum to construct from */
	explicit ebitset(const T sbit) : a_((1 << sbit) - 1) {}

	/*! Assignment operator.
	    \param that ebitset_r to assign from
	    \return  this */
	ebitset<T, B>& operator=(const ebitset<T, B>& that)
	{
		if (this != &that)
			a_ = that.a_;
		return *this;
	}

	/*! Check if an enum is in the set.
	    \param sbit enum to check
	    \return integral_type of bits if found */
	integral_type has(const T sbit) { return a_ & 1 << sbit; }

	/*! Check if an enum is in the set.
	    \param sbit enum to check
	    \return integral_type of bits if found */
	integral_type operator&(const T sbit) { return a_ & 1 << sbit; }

	/*! Set a bit on or off.
	    \param sbit enum to set
	    \param on set on or off */
	void set(const T sbit, bool on=true) { if (on) a_ |= 1 << sbit; else a_ &= ~(1 << sbit); }

	/*! Set a bit on or off.
	    \param bset integral_type to set */
	void set(const integral_type bset) { a_ = bset; }

	/*! Clear a bit on or off.
	    \param sbit enum to set */
	void clear(const T sbit) { a_ &= ~(1 << sbit); }

	/// Clear all bits.
	void clearall() { a_ = 0; }

	/*! Set all bits to a value.
	    \param sbit value to set to */
	void setall(const T sbit) { a_ = (1 << sbit) - 1; }

	/*! Get the enum integral_type.
	    \return integral_type of enum */
	integral_type get() const { return a_; }

	/*! Or a bit value with the current set.
	    \param sbit to set */
	void operator|=(const T sbit) { a_ |= 1 << sbit; }

	/*! Or a bit value with the current set.
	    \param sbit to set
	    \return ebitset_r */
	ebitset& operator<<(const T sbit) { a_ |= 1 << sbit; return *this; }
	//friend ebitset operator|(const T lbit, const T rbit) { return ebitset(lbit) |= 1 << rbit; }
};

/*! Atomic bitset for enums.
    \tparam T the enum type
    \tparam B the integral type of the enumeration */
template<typename T, typename B=unsigned int>
class ebitset_r
{
	typedef B integral_type;
	tbb::atomic<integral_type> a_;

public:
	/// Ctor.
	ebitset_r() { a_ = 0; }

	/*! Ctor.
	    \param from ebitset_r to copy */
	ebitset_r(const ebitset_r<T, B>& from) { a_ = from.a_; }

	/*! Ctor.
	    \param a integral type to construct from */
	explicit ebitset_r(const integral_type a) { a_ = a; }

	/*! Ctor.
	    \param sbit enum to construct from */
	explicit ebitset_r(const T sbit) { a_ = (1 << sbit) - 1; }

	/*! Assignment operator.
	    \param that ebitset_r to assign from
	    \return  this */
	ebitset_r<T, B>& operator=(const ebitset_r<T, B>& that)
	{
		if (this != &that)
			a_ = that.a_;
		return *this;
	}

	/*! Check if an enum is in the set.
	    \param sbit enum to check
	    \return integral_type of bits if found */
	integral_type has(const T sbit) { return a_ & 1 << sbit; }

	/*! Check if an enum is in the set.
	    \param sbit enum to check
	    \return integral_type of bits if found */
	integral_type operator&(const T sbit) { return a_ & 1 << sbit; }

	/*! Set a bit on or off.
	    \param sbit enum to set
	    \param on set on or off */
	void set(const T sbit, bool on=true) { if (on) a_ |= 1 << sbit; else a_ &= ~(1 << sbit); }

	/*! Set a bit on or off.
	    \param bset integral_type to set */
	void set(const integral_type bset) { a_ = bset; }

	/*! Clear a bit on or off.
	    \param sbit enum to set */
	void clear(const T sbit) { integral_type a = a_; a &= ~(1 << sbit); a_ = a; }

	/// Clear all bits.
	void clearall() { a_ = 0; }

	/*! Set all bits to a value.
	    \param sbit value to set to */
	void setall(const T sbit) { a_ = (1 << sbit) - 1; }

	/*! Get the enum integral_type.
	    \return integral_type of enum */
	integral_type get() const { return a_; }

	/*! Or a bit value with the current set.
	    \param sbit to set */
	void operator|=(const T sbit) { integral_type a = a_; a |= 1 << sbit; a_ = a; }

	/*! And a bit value with the current set.
	    \param sbit to set */
	void operator&=(const T sbit) { integral_type a = a_; a &= 1 << sbit; a_ = a; }

	/*! Or a bit value with the current set.
	    \param sbit to set
	    \return ebitset_r */
	ebitset_r& operator<<(const T sbit) { a_ |= 1 << sbit; return *this; }
};

//----------------------------------------------------------------------------------------
/*! Check for file existance.
    \param fname filename to check
    \return true if file exists */
inline bool exist(const std::string& fname)
{
	return access(fname.c_str(), F_OK) == 0;
}

//----------------------------------------------------------------------------------------
/*! Copy a string safely to a target.
    \param src source string
    \param target target location
    \param limit maximum bytes to copy, 0 for no limit
    \return pointer to target */
inline char *CopyString(const std::string& src, char *target, unsigned limit=0)
{
   if (!target)
      return 0;
   unsigned sz(limit && src.size() > limit ? limit : src.size() + 1);
   src.copy(target, sz - 1);
   target[sz - 1] = 0;
   return target;
}


//----------------------------------------------------------------------------------------
/*! Sleep the specified number of milliseconds.
    \param ms time to sleep in milliseconds
    \return 0 on success */
inline int millisleep (const int ms)
{
	struct timespec tspec = { ms / 1000, 1000 * 1000 * (ms % 1000) };
	return nanosleep(&tspec, 0);
}

//----------------------------------------------------------------------------------------
/*! Sleep the specified number of microseconds.
    \param us time to sleep in microseconds
    \return 0 on success */
inline int microsleep (const int us)
{
	struct timespec tspec = { us / (1000 * 1000), 1000 * (us % (1000 * 1000)) };
	return nanosleep(&tspec, 0);
}

//----------------------------------------------------------------------------------------
/// Delete ptr.
struct DeleteObject
{
	/*! delete ptr operator
		 \tparam T typename
		 \param m object to delete */
	template<typename T>
	void operator()(const T& m) const { delete m; }
};

/// Delete array.
struct DeleteArrayObject
{
	/*! delete array operator
		 \tparam T typename
		 \param m object to delete */
	template<typename T>
	void operator()(const T& m) const { delete[] m; }
};

/// Delete 1st of pair.
template <typename Deleter = DeleteObject>
struct Delete1stPairObject
{
	/*! delete 1st of a std::pair operator
		 \tparam T typename
		 \param m object to delete */
	template<typename A, typename B>
	void operator()(const std::pair<A, B>& m) const { Deleter()(m.first); }
};

/// Delete 2nd of pair.
template <typename Deleter = DeleteObject>
struct Delete2ndPairObject
{
	/*! delete 2nd of a std::pair operator
		 \tparam T typename
		 \param m object to delete */
	template<typename A, typename B>
	void operator()(const std::pair<A, B>& m) const { Deleter()(m.second); }
};

/*! Parameterisable deleter functor.
    \tparam Deleter the desired deleter (see above) */
template <typename Deleter = DeleteObject>
struct free_ptr
{
	/*! function operator
		 \tparam T typename
		 \param ptr object to delete */
	template<typename T>
	void operator()(const T& ptr) const { Deleter()(ptr); }
};

//----------------------------------------------------------------------------------------
/*! A lockfree Singleton.
    \tparam T the instance object type */
template <typename T>
class Singleton
{
	static tbb::atomic<T*> _instance;
	static tbb::mutex _mutex;

	Singleton(const Singleton&);
	Singleton& operator=(const Singleton&);

public:
	/// Ctor.
	Singleton() {}

	/// Dtor.
	virtual ~Singleton() { delete _instance.fetch_and_store(0); }

	/*! Get the instance of the underlying object. If not created, create.
	    \return the instance */
	static T *instance()
	{
		if (_instance) // cast operator performs atomic load with acquire
			return _instance;

		tbb::mutex::scoped_lock guard(_mutex);
		if (_instance == 0)
			_instance = new T;
		return _instance;
	}


	/*! Get the instance of the underlying object. If not created, create.
	    \return the instance */
	T *operator->() const { return instance(); }

	/*! Replace the instance object with a new instance.
	    \param what the new instance
	    \return the original instance */
	static T *reset(T *what) { return _instance.fetch_and_store(what); }

	/*! Get the instance of the underlying object removing it from the singleton.
	    \return the instance */
	static T *release() { return _instance.fetch_and_store(0); }
};

//----------------------------------------------------------------------------------------
} // namespace FIX8

#endif // _F8_UTILS_HPP_

