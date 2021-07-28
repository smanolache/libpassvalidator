#pragma once

#include "visibility.h"
#include "ZlibBase.hh"
#include <type_traits>
#include <utility>
#include <string>
#include "iterator_concepts.hh"
#include <stdint.h>
#include <endian.h>
// #include "string_view.hh"

namespace pv {
namespace utils {

class DSO_LOCAL  Zlib;

class Zlib : public ZlibBase {
public:
	Zlib();

	template<typename II1, typename II2, typename OI,
		 typename std::enable_if<
			 std::is_same<
				 typename std::decay<II1>::type,
				 typename std::decay<II2>::type
			 >::value,
		 int>::type = 0>
	static auto decompress(II1&&, const II2&, OI&&)
		-> typename zlibdecompressor::helper<OI>::type;

	template<typename II1, typename II2, typename OI,
		 typename std::enable_if<
			 std::is_same<
				 typename std::decay<II1>::type,
				 typename std::decay<II2>::type
			 >::value,
		 int>::type = 0>
	static auto compress(II1&&, const II2&, OI&&)
		-> typename zlibcompressor::helper<OI>::type;

	template<typename II1, typename II2,
		 typename std::enable_if<
			 std::is_same<
				 typename std::decay<II1>::type,
				 typename std::decay<II2>::type
				 >::value,
			 int>::type = 0>
	static std::string decompress(II1&& i1, const II2& i2);

	template<typename Sequence>
	static std::string decompress(Sequence&& s);

	template<typename II1, typename II2,
		 typename std::enable_if<
			 std::is_same<
				 typename std::decay<II1>::type,
				 typename std::decay<II2>::type
				 >::value,
			 int>::type = 0>
	static std::string compress(II1&& i1, const II2& i2);

	template<typename Sequence>
	static std::string compress(Sequence&& s);

	template<typename II1, typename II2,
		 typename std::enable_if<
			 std::is_same<
				 typename std::decay<II1>::type,
				 typename std::decay<II2>::type
			 >::value, int>::type = 0>
	static bool is_zlib(II1&&, const II2&);

	template<typename Sequence>
	static bool is_zlib(Sequence&&);
private:
	template<typename II1, typename II2,
		 typename std::enable_if<
			 helpers::is_random_access<II1>::value,
			 int>::type = 0>
	static bool is_z(const II1&, const II2&);

	template<typename II1, typename II2,
		 typename std::enable_if<
			 std::__not_<
				 typename helpers::is_random_access<II1>::type
			 >::value, int>::type = 0>
	static bool is_z(II1&&, const II2&);
};

inline
Zlib::Zlib()
	: ZlibBase(15)
{
}

template<typename II1, typename II2, typename OI,
	 typename std::enable_if<
		 std::is_same<
			 typename std::decay<II1>::type,
			 typename std::decay<II2>::type
		 >::value,
	 int>::type>
auto
Zlib::decompress(II1&& i1, const II2& i2, OI&& o)
	-> typename zlibdecompressor::helper<OI>::type {
	return ZlibBase::decompress(std::forward<II1>(i1), i2,
				    std::forward<OI>(o), 15);
}

template<typename II1, typename II2, typename OI,
	 typename std::enable_if<
		 std::is_same<
			 typename std::decay<II1>::type,
			 typename std::decay<II2>::type
		 >::value,
	 int>::type>
auto
Zlib::compress(II1&& i1, const II2& i2, OI&& o)
	-> typename zlibcompressor::helper<OI>::type {
	return ZlibBase::compress(std::forward<II1>(i1), i2,
				  std::forward<OI>(o), 15);
}

template<typename II1, typename II2,
	 typename std::enable_if<
		 std::is_same<
			 typename std::decay<II1>::type,
			 typename std::decay<II2>::type
		 >::value,
	 int>::type>
std::string
Zlib::decompress(II1&& i1, const II2& i2) {
	std::string r;
	decompress(std::forward<II1>(i1), i2, std::back_inserter(r));
	return r;
}

template<typename Sequence>
std::string
Zlib::decompress(Sequence&& s) {
	return decompress(s.begin(), s.end());
}

template<typename II1, typename II2,
	 typename std::enable_if<
		 std::is_same<
			 typename std::decay<II1>::type,
			 typename std::decay<II2>::type
		 >::value,
	 int>::type>
std::string
Zlib::compress(II1&& i1, const II2& i2) {
	std::string r;
	compress(std::forward<II1>(i1), i2, std::back_inserter(r));
	return r;
}

template<typename Sequence>
std::string
Zlib::compress(Sequence&& s) {
	return compress(s.begin(), s.end());
}

template<typename Sequence>
inline bool
Zlib::is_zlib(Sequence&& s) {
	return is_zlib(s.begin(), s.end());
}

template<typename II1, typename II2,
	 typename std::enable_if<
		 std::is_same<
			 typename std::decay<II1>::type,
			 typename std::decay<II2>::type
		 >::value, int>::type>
bool
Zlib::is_zlib(II1&& i1, const II2& i2) {
	return is_z(
		std::forward<typename zlibdecompressor::helper<II1>::type>(i1),
		i2);
}

template<typename II1, typename II2,
	 typename std::enable_if<
		 std::__not_<
			 typename helpers::is_random_access<II1>::type
		 >::value, int>::type>
bool
Zlib::is_z(II1&& i1, const II2& i2) {
	if (i1 == i2)
		return false;
	char c0 = *i1;
	if (8 != (0x0f & c0) || 7 < ((0xf0 & c0) >> 4))
		return false;
	++i1;
	if (i1 == i2)
		return false;
	union {
		uint16_t u;
		char c[sizeof(uint16_t)];
	} q;
	q.c[0] = c0;
	q.c[1] = *i1;
	return 0 == (be16toh(q.u) % 31);
}

template<typename II1, typename II2,
	 typename std::enable_if<
		 helpers::is_random_access<II1>::value,
		 int>::type>
bool
Zlib::is_z(const II1& i1, const II2& i2) {
	if (i2 - i1 < 2 || 8 != (0x0f & *i1) || 7 < ((0xf0 & *i1) >> 4))
		return false;
	union {
		uint16_t u;
		char c[sizeof(uint16_t)];
	} q;
	if (i2 - i1 < static_cast<int>(sizeof(q.u)))
		return false;
	q.c[0] = *i1;
	q.c[1] = *(i1 + 1);
	return 0 == (be16toh(q.u) % 31);
}

// common instantiations
// is_zlib
extern template bool
Zlib::is_zlib<std::string>(std::string&&) noexcept;
extern template bool
Zlib::is_zlib<std::string&>(std::string&) noexcept;
extern template bool
Zlib::is_zlib<const std::string&>(const std::string&) noexcept;

extern template bool
Zlib::is_zlib<std::string::iterator,
	      std::string::iterator, 0>(
		      std::string::iterator&&,
		      const std::string::iterator&) noexcept;
extern template bool
Zlib::is_zlib<std::string::iterator&,
	      std::string::iterator, 0>(
		      std::string::iterator&,
		      const std::string::iterator&) noexcept;
extern template bool
Zlib::is_zlib<std::string::const_iterator,
	      std::string::const_iterator, 0>(
		      std::string::const_iterator&&,
		      const std::string::const_iterator&) noexcept;
extern template bool
Zlib::is_zlib<std::string::const_iterator&,
	      std::string::const_iterator, 0>(
		      std::string::const_iterator&,
		      const std::string::const_iterator&) noexcept;

// extern template bool
// Zlib::is_zlib<std::string_view>(std::string_view&&) noexcept;
// extern template bool
// Zlib::is_zlib<std::string_view&>(std::string_view&) noexcept;
// extern template bool
// Zlib::is_zlib<const std::string_view&>(const std::string_view&) noexcept;

// extern template bool
// Zlib::is_zlib<std::string_view::const_iterator,
// 	      std::string_view::const_iterator, 0>(
// 		      std::string_view::const_iterator&&,
// 		      const std::string_view::const_iterator&) noexcept;
// extern template bool
// Zlib::is_zlib<std::string_view::const_iterator&,
// 	      std::string_view::const_iterator, 0>(
// 		      std::string_view::const_iterator&,
// 		      const std::string_view::const_iterator&) noexcept;

extern template bool
Zlib::is_zlib<char const *, char const *, 0>(
	char const * &&, char const * const &) noexcept;
extern template bool
Zlib::is_zlib<char *, char *, 0>(char * &&, char * const &) noexcept;
extern template bool
Zlib::is_zlib<char const * &, char const *, 0>(
	char const * &, char const * const &) noexcept;
extern template bool
Zlib::is_zlib<char * &, char *, 0>(char * &, char * const &) noexcept;

// decompress
extern template std::string
Zlib::decompress<std::string>(std::string&&);
extern template std::string
Zlib::decompress<std::string&>(std::string&);
extern template std::string
Zlib::decompress<const std::string&>(const std::string&);

extern template std::string
Zlib::decompress<std::string::iterator,
		 std::string::iterator, 0>(
			 std::string::iterator&&,
			 const std::string::iterator&);
extern template std::string
Zlib::decompress<std::string::iterator&,
		 std::string::iterator, 0>(
			 std::string::iterator&,
			 const std::string::iterator&);
extern template std::string
Zlib::decompress<std::string::const_iterator,
		 std::string::const_iterator, 0>(
			 std::string::const_iterator&&,
			 const std::string::const_iterator&);
extern template std::string
Zlib::decompress<std::string::const_iterator&,
		 std::string::const_iterator, 0>(
			 std::string::const_iterator&,
			 const std::string::const_iterator&);

// extern template std::string
// Zlib::decompress<std::string_view>(std::string_view&&);
// extern template std::string
// Zlib::decompress<std::string_view&>(std::string_view&);
// extern template std::string
// Zlib::decompress<const std::string_view&>(const std::string_view&);

// extern template std::string
// Zlib::decompress<std::string_view::const_iterator,
// 		 std::string_view::const_iterator, 0>(
// 			 std::string_view::const_iterator&&,
// 			 const std::string_view::const_iterator&);
// extern template std::string
// Zlib::decompress<std::string_view::const_iterator&,
// 		 std::string_view::const_iterator, 0>(
// 			 std::string_view::const_iterator&,
// 			 const std::string_view::const_iterator&);

extern template std::string
Zlib::decompress<char const *, char const *, 0>(
	char const * &&, char const * const &);
extern template std::string
Zlib::decompress<char *, char *, 0>(char * &&, char * const &);
extern template std::string
Zlib::decompress<char const * &, char const *, 0>(
	char const * &, char const * const &);
extern template std::string
Zlib::decompress<char * &, char *, 0>(char * &, char * const &);

// compress
extern template std::string
Zlib::compress<std::string>(std::string&&);
extern template std::string
Zlib::compress<std::string&>(std::string&);
extern template std::string
Zlib::compress<const std::string&>(const std::string&);

extern template std::string
Zlib::compress<std::string::iterator,
	       std::string::iterator, 0>(std::string::iterator&&,
					 const std::string::iterator&);
extern template std::string
Zlib::compress<std::string::iterator&,
	       std::string::iterator, 0>(std::string::iterator&,
					 const std::string::iterator&);
extern template std::string
Zlib::compress<std::string::const_iterator,
	       std::string::const_iterator, 0>(
		       std::string::const_iterator&&,
		       const std::string::const_iterator&);
extern template std::string
Zlib::compress<std::string::const_iterator&,
	       std::string::const_iterator, 0>(
		       std::string::const_iterator&,
		       const std::string::const_iterator&);

// extern template std::string
// Zlib::compress<std::string_view>(std::string_view&&);
// extern template std::string
// Zlib::compress<std::string_view&>(std::string_view&);
// extern template std::string
// Zlib::compress<const std::string_view&>(const std::string_view&);

// extern template std::string
// Zlib::compress<std::string_view::const_iterator,
// 	       std::string_view::const_iterator, 0>(
// 		       std::string_view::const_iterator&&,
// 		       const std::string_view::const_iterator&);
// extern template std::string
// Zlib::compress<std::string_view::const_iterator&,
// 	       std::string_view::const_iterator, 0>(
// 		       std::string_view::const_iterator&,
// 		       const std::string_view::const_iterator&);

extern template std::string
Zlib::compress<char const *, char const *, 0>(
	char const * &&, char const * const &);
extern template std::string
Zlib::compress<char *, char *, 0>(char * &&, char * const &);
extern template std::string
Zlib::compress<char const * &, char const *, 0>(
	char const * &, char const * const &);
extern template std::string
Zlib::compress<char * &, char *, 0>(char * &, char * const &);

}
}
