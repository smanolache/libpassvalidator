#pragma once

#include "visibility.h"
#include <type_traits>
#include <utility>
#include <string>
#include <iterator>
#include "pv/Exception.hh"
#include "pv/Errors.hh"
#include "iterator_concepts.hh"
// #include "string_view.hh"

namespace pv {
namespace utils {

namespace base64_detail {

template<int n>
struct DSO_LOCAL ___tables;
template<int n>
struct ___tables {
	static const unsigned char b64tbl[64];
	static const unsigned char b64ndx[256];
	static const unsigned char ub64tbl[64];
	static const unsigned char ub64ndx[256];
};

typedef ___tables<0> tables;

template<const unsigned char table[64]>
struct DSO_LOCAL padder;

template<const unsigned char table[64]>
struct padder {
	template<typename OutputIterator>
	static OutputIterator pad1(OutputIterator&&);
	template<typename OutputIterator>
	static OutputIterator pad2(OutputIterator&&);
};

template<const unsigned char table[64]>
template<typename OutputIterator>
inline OutputIterator
padder<table>::pad1(OutputIterator&& o) {
	return std::forward<OutputIterator>(o);
}

template<const unsigned char table[64]>
template<typename OutputIterator>
inline OutputIterator
padder<table>::pad2(OutputIterator&& o) {
	return std::forward<OutputIterator>(o);
}

template<>
struct padder<tables::b64tbl> {
	template<typename OutputIterator>
	static OutputIterator pad1(OutputIterator&&);
	template<typename OutputIterator>
	static OutputIterator pad2(OutputIterator&&);
};

template<typename OutputIterator>
inline OutputIterator
padder<tables::b64tbl>::pad2(OutputIterator&& o) {
	*o = '=';
	++o;
	*o = '=';
	++o;
	return std::forward<OutputIterator>(o);
}

template<typename OutputIterator>
inline OutputIterator
padder<tables::b64tbl>::pad1(OutputIterator&& o) {
	*o = '=';
	++o;
	return std::forward<OutputIterator>(o);
}

struct DSO_LOCAL iterator_helper;
struct iterator_helper {
	template<typename Iterator>
	struct helper {
		typedef typename std::conditional<
			std::is_array<
				typename std::remove_reference<Iterator>::type
			>::value,
			typename std::decay<Iterator>::type,
			Iterator
		>::type type;
	};
};

template<const unsigned char ndx[256]>
class DSO_LOCAL is_b64_helper;
template<const unsigned char ndx[256]>
class is_b64_helper : public iterator_helper {
protected:
	template<typename II1, typename II2,
		 typename std::enable_if<
			 helpers::is_random_access<II1>::value, int
		 >::type = 0>
	static bool is_b64(const II1&, const II2&);

	template<typename II1, typename II2,
		 typename std::enable_if<
			 std::__not_<
				 typename helpers::is_random_access<II1>::type
			 >::value, int>::type = 0>
	static bool is_b64(II1&&, const II2&);
};

template<const unsigned char ndx[256]>
template<typename II1, typename II2,
	 typename std::enable_if<
		 helpers::is_random_access<II1>::value, int
	 >::type>
bool
is_b64_helper<ndx>::is_b64(const II1& i1, const II2& i2) {
	std::size_t n = i2 - i1;
	if (0 != (n % 4))
		return false;
	for (unsigned int i = 0; i < n; ++i) {
		unsigned char c = static_cast<unsigned char>(*(i1 + i));
		if ('=' == c)
			return i == n - 1 || (i == n - 2 && '=' == *(i1 + n - 1));
		if (ndx[c] >= 64)
			return false;
	}
	return true;
}

template<const unsigned char ndx[256]>
template<typename II1, typename II2,
	 typename std::enable_if<
		 std::__not_<
			 typename helpers::is_random_access<II1>::type
		 >::value, int>::type>
bool
is_b64_helper<ndx>::is_b64(II1&& i1, const II2& i2) {
	unsigned int n = 0;
	for (; i1 != i2; ++n, ++i1) {
		unsigned char c = static_cast<unsigned char>(*i1);
		if ('=' == c) {
			++i1;
			++n;
			if (i1 == i2)
				return 0 == (n % 4);
			if ('=' != static_cast<unsigned char>(*i1))
				return false;
			++i1;
			++n;
			return i1 == i2 && 0 == (n % 4);
		}
		if (ndx[c] >= 64)
			return false;
	}
	return 0 == (n % 4);
}

template<>
class is_b64_helper<tables::ub64ndx> : public iterator_helper {
protected:
	template<typename II1, typename II2,
		 typename std::enable_if<
			 helpers::is_random_access<II1>::value, int
		 >::type = 0>
	static bool is_b64(const II1&, const II2&);

	template<typename II1, typename II2,
		 typename std::enable_if<
			 std::__not_<
				 typename helpers::is_random_access<II1>::type
			 >::value, int>::type = 0>
	static bool is_b64(II1&&, const II2&);
};

template<typename II1, typename II2,
	 typename std::enable_if<
		 helpers::is_random_access<II1>::value, int
	 >::type>
bool
is_b64_helper<tables::ub64ndx>::is_b64(const II1& i1, const II2& i2) {
	std::size_t n = i2 - i1;
	if (1 == (n % 4))
		return false;
	for (unsigned int i = 0; i < n; ++i) {
		unsigned char c = static_cast<unsigned char>(*(i1 + i));
		if ('=' == c)
			return (2 == (i % 4) && (i == n - 1 ||
						 (i == n - 2 &&
						  '=' == *(i1 + n - 1)))) ||
				(3 == (i % 4) && i == n - 1);
		if (tables::ub64ndx[c] >= 64)
			return false;
	}
	return true;
}

template<typename II1, typename II2,
	 typename std::enable_if<
		 std::__not_<
			 typename helpers::is_random_access<II1>::type
		 >::value, int>::type>
bool
is_b64_helper<tables::ub64ndx>::is_b64(II1&& i1, const II2& i2) {
	unsigned int n = 0;
	for (; i1 != i2; ++n, ++i1) {
		unsigned char c = static_cast<unsigned char>(*i1);
		if ('=' == c) {
			if (1 >= (n % 4))
				return false;
			++i1;
			if (i1 == i2)
				return true;
			if ('=' != static_cast<unsigned char>(*i1))
				return false;
			++i1;
			return i1 == i2;
		}
		if (tables::ub64ndx[c] >= 64)
			return false;
	}
	return 1 != (n % 4);
}

template<const unsigned char ndx[256]>
class DSO_LOCAL flusher;
template<const unsigned char ndx[256]>
class flusher {
public:
	flusher() : sl(0) {}

	template<typename OutputIterator>
	OutputIterator flush(OutputIterator&&);

	template<typename OutputIterator>
	static OutputIterator error(OutputIterator&&);

protected:
	unsigned int sl;
	unsigned char state[4];
};


template<const unsigned char ndx[256]>
template<typename OutputIterator>
inline OutputIterator
flusher<ndx>::flush(OutputIterator&& o) {
	// error: If we had blocks of 4k size then 'flush' would not be called
	// Thus, if it is called it follows that we have incomplete blocks.
	// Cannot detect.
	sl = 0; // reset for reuse
	throw Exception(__FILE__, __LINE__, error::INVALID_ARGS, ": The "
			"length of the base64 data is not a multiple of four");
}

template<const unsigned char ndx[256]>
template<typename OutputIterator>
inline OutputIterator
flusher<ndx>::error(OutputIterator&&) {
	throw Exception(__FILE__, __LINE__, error::INVALID_ARGS, ": The "
			"length of the base64 data is not a multiple of four");
}

template<>
class flusher<tables::ub64ndx> {
public:
	flusher() : sl(0) {}

	template<typename OutputIterator>
	OutputIterator flush(OutputIterator&&);

	template<typename OutputIterator>
	static OutputIterator error(OutputIterator&&);
protected:
	unsigned int sl;
	unsigned char state[4];
};

template<typename OutputIterator>
OutputIterator
flusher<tables::ub64ndx>::flush(OutputIterator&& o) {
	if (1 == sl) {
		// error: 4k + 1 characters
		// It II were a random iterator then the error would be
		// detectable because we could check if i2 - i1 is 4k+1
		sl = 0;
		throw Exception(__FILE__, __LINE__, error::INVALID_ARGS, "The "
				"length of the base64 data is not a multiple "
				"of four");
	}
	*o = static_cast<unsigned char>((state[0] << 2) | (state[1] >> 4));
	++o;

	if (2 == sl) { // OK
		sl = 0;
		return std::forward<OutputIterator>(o);
	}

	// sl must be 3 because 'flush' is not invoked for 0 or 4
	sl = 0; // reset for reuse
	if ('=' == state[2])
		// that's a bit strange: the last block consists of three
		// characters and the third on is '='. Normally either
		// there are no '=' characters and then this block has
		// two or three characters, or it does have '=' characters
		// but then we have a full block of 4 characters and then
		// 'flush' is not called.
		// But let us say that we tolerate this strange block.
		return std::forward<OutputIterator>(o);
	*o = static_cast<unsigned char>((state[1] << 4) | (state[2] >> 2));
	++o;

	return std::forward<OutputIterator>(o);
}

template<typename OutputIterator>
inline OutputIterator
flusher<tables::ub64ndx>::error(OutputIterator&& o) {
	return std::forward<OutputIterator>(o);
}

}

template<const unsigned char ndx[256]>
class DSO_LOCAL base64dectmpl;
template<const unsigned char ndx[256]>
class base64dectmpl : public base64_detail::flusher<ndx>,
		      public base64_detail::is_b64_helper<ndx> {
private:
	template<typename Iterator>	
	using hlp = typename base64_detail::is_b64_helper<ndx>
		::template helper<Iterator>;
public:
	template<typename II1, typename II2, typename OI,
		 typename std::enable_if<
			 std::is_same<
				 typename std::decay<II1>::type,
				 typename std::decay<II2>::type
			 >::value,
		 int>::type = 0>
	static auto dec(II1&&, const II2&, OI&&) -> typename hlp<OI>::type;

	template<typename II1, typename II2,
		 typename std::enable_if<
			 std::is_same<
				 typename std::decay<II1>::type,
				 typename std::decay<II2>::type
			 >::value,
		 int>::type = 0>
	static std::string dec(II1&&, const II2&);

	template<typename Sequence>
	static std::string dec(Sequence&&);

	template<typename II1, typename II2, typename OI,
		 typename std::enable_if<
			 std::is_same<
				 typename std::decay<II1>::type,
				 typename std::decay<II2>::type
			 >::value,
		 int>::type = 0>
	auto chunk_dec(II1&&, const II2&, OI&&) -> typename hlp<OI>::type;

	template<typename OI>
	auto end_dec(OI&&) -> typename hlp<OI>::type;

	template<typename Sequence>
	static bool is_base64(Sequence&&);

	template<typename II1, typename II2,
		 typename std::enable_if<
			 std::is_same<typename std::decay<II1>::type,
				      typename std::decay<II2>::type>
			 ::value, int>::type = 0>
	static bool is_base64(II1&&, const II2&);
private:
	template<typename II1, typename II2, typename OI>
	static OI decode(II1&&, const II2&, OI&&);

	template<typename II1, typename II2, typename OI>
	OI chunk_decode(II1&&, const II2&, OI&&);

	template<typename OI>
	OI end_decode(OI&&);

	template<typename OI>
	OI dec4(OI&&);
};

template<const unsigned char ndx[256]>
template<typename Sequence>
bool
base64dectmpl<ndx>::is_base64(Sequence&& s) {
	return is_base64(s.begin(), s.end());
}

template<const unsigned char ndx[256]>
template<typename II1, typename II2,
	 typename std::enable_if<
		 std::is_same<typename std::decay<II1>::type,
			      typename std::decay<II2>::type>
		 ::value, int>::type>
bool
base64dectmpl<ndx>::is_base64(II1&& i1, const II2& i2) {
	return base64_detail::is_b64_helper<ndx>::is_b64(
		std::forward<typename hlp<II1>::type>(i1), i2);
}

template<const unsigned char ndx[256]>
template<typename II1, typename II2, typename OI,
	 typename std::enable_if<
		 std::is_same<
			 typename std::decay<II1>::type,
			 typename std::decay<II2>::type
		 >::value,
	 int>::type>
auto
base64dectmpl<ndx>::dec(II1&& s, const II2& e, OI&& o)
	-> typename hlp<OI>::type {
	return decode(std::forward<typename hlp<II1>::type>(s), e,
		      std::forward<typename hlp<OI>::type>(o));
}

template<const unsigned char ndx[256]>
template<typename II1, typename II2,
	 typename std::enable_if<
		 std::is_same<
			 typename std::decay<II1>::type,
			 typename std::decay<II2>::type
		 >::value, int>::type>
std::string
base64dectmpl<ndx>::dec(II1&& i1, const II2& i2) {
	std::string r;
	dec(std::forward<II1>(i1), i2, std::back_inserter(r));
	return r;
}

template<const unsigned char ndx[256]>
template<typename Sequence>
std::string
base64dectmpl<ndx>::dec(Sequence&& s) {
	return dec(s.begin(), s.end());
}

template<const unsigned char ndx[256]>
template<typename II1, typename II2, typename OI,
	 typename std::enable_if<
		 std::is_same<
			 typename std::decay<II1>::type,
			 typename std::decay<II2>::type
		 >::value,
	 int>::type>
auto
base64dectmpl<ndx>::chunk_dec(II1&& s, const II2& e, OI&& o)
	-> typename hlp<OI>::type {
	return chunk_decode(std::forward<typename hlp<II1>::type>(s), e,
			    std::forward<typename hlp<OI>::type>(o));
}

template<const unsigned char ndx[256]>
template<typename OI>
auto
base64dectmpl<ndx>::end_dec(OI&& o) -> typename hlp<OI>::type {
	return end_decode(std::forward<typename hlp<OI>::type>(o));
}

template<const unsigned char ndx[256]>
template<typename II1, typename II2, typename OI>
inline OI
base64dectmpl<ndx>::decode(II1&& i1, const II2& i2, OI&& o) {
	while (i1 != i2) {
		unsigned char c0 = ndx[static_cast<unsigned char>(*i1)];
		if (c0 >= 64)
			// error: non-base64 char
			// detectable because i1 != i2
			throw Exception(__FILE__, __LINE__, error::INVALID_ARGS,
					": %02x is not a base64 character",
					static_cast<unsigned int>(*i1));
		++i1;
		if (i1 == i2)
			// error: 4k + 1 characters
			// Undetectable for ForwardIterators or weaker (e.g.
			// istream_iterator).
			// if I1 were a random iterator then the error would be
			// detectable because we could check if i2 - i1 is 4k+1
			// If II were a bidirectional iterator, then we could
			// rewind.
			throw Exception(__FILE__, __LINE__, error::INVALID_ARGS,
					": The length of the base64 data is "
					"not a multiple of four");

		unsigned char c1 = ndx[static_cast<unsigned char>(*i1)];
		if (c1 >= 64)
			// error: non-base64 char
			// detectable because i1 != i2
			throw Exception(__FILE__, __LINE__, error::INVALID_ARGS,
					": %02x is not a base64 character",
					static_cast<unsigned int>(*i1));
		++i1;
		*o = static_cast<unsigned char>((c0 << 2) | (c1 >> 4));
		++o;
		if (i1 == i2)
			// 4k + 2 characters
			// ubase64: it is not an error.
			// base64: error.
			// Undetectable for ForwardIterators or weaker.
			// If I1 were a random iterator then the error would be
			// detectable because we could check if i2 - i1 is 4k+2
			// If I1 were a bidirectional iterator then we could
			// rewind.
			return base64_detail::flusher<ndx>::
				error(std::forward<OI>(o));

		unsigned char r2 = *i1;
		if ('=' == r2) {
			++i1;
			if (i1 == i2)
				// 4k + 3 characters
				// base64: error: encoding is truncated
				// Undetectable for ForwardIterators or weaker.
				// If I1 were a random iterator then the error
				// would be detectable becase we could check if
				// i2 - i1 is 4k+3.
				// If I1 were a bidirectional iterator then we
				// could rewind (3).
				// ubase64: it is not an error but it is
				// strange: we get a group of 3 characters of
				// which the third is '='. Normally either there
				// are no '=' characters or they pad to the full
				// block border, i.e. the group has 4 characters
				return base64_detail::flusher<ndx>::
					error(std::forward<OI>(o));
			if ('=' != *i1)
				// error, non-'=' after '='
				// detectable because i1 != i2
				throw Exception(__FILE__, __LINE__, error::INVALID_ARGS,
						": Expected '='");
			++i1;
			if (i1 != i2)
				throw Exception(__FILE__, __LINE__, error::INVALID_ARGS,
						": Extra data");
			return std::forward<OI>(o);
		}

		unsigned char c2 = ndx[r2];
		if (c2 >= 64)
			// error: non-base64 char
			// detectable because i1 != i2
			throw Exception(__FILE__, __LINE__, error::INVALID_ARGS,
					": %02x is not a base64 character",
					static_cast<unsigned int>(r2));
		*o = static_cast<unsigned char>((c1 << 4) | (c2 >> 2));
		++o;
		++i1;
		if (i1 == i2)
			// 4k + 3 characters
			// ubase64: not an error
			// base64: error: encoding is truncated
			// Undetectable for ForwardIterators or weaker.
			// if II were a random iterator then the error would be
			// detectable because we could check if i2 - i1 is 4k+3
			return base64_detail::flusher<ndx>::
				error(std::forward<OI>(o));

		unsigned char r3 = *i1;
		if ('=' == r3) {
			++i1;
			if (i1 != i2)
				throw Exception(__FILE__, __LINE__, error::INVALID_ARGS,
						": Expected '='");
			return std::forward<OI>(o);
		}
		unsigned char c3 = ndx[r3];
		if (c3 >= 64)
			// error: non-base64 char
			// detectable because i1 != i2
			throw Exception(__FILE__, __LINE__, error::INVALID_ARGS,
					": %02x is not a base64 character",
					static_cast<unsigned int>(r3));
		*o = static_cast<unsigned char>((c2 << 6) | c3);
		++o;
		++i1;
	}
	return std::forward<OI>(o);
}

template<const unsigned char ndx[256]>
template<typename II1, typename II2, typename OI>
inline OI
base64dectmpl<ndx>::chunk_decode(II1&& i1, const II2& i2, OI&& o) {
	if (4 == this->sl) {
		// error: we already had padding
		this->sl = 0; // reset for reuse
		throw Exception(__FILE__, __LINE__, error::INVALID_ARGS,
				": Not a base64 character");
	}
	while (i1 != i2) {
		unsigned char c0 = static_cast<unsigned char>(*i1);
		if ('=' != c0) {
			if (3 == this->sl && '=' == this->state[2]) {
				// error: non-'=' after '='
				// detectable because i2 != i1
				this->sl = 0; // reset for reuse
				throw Exception(__FILE__, __LINE__,
						error::INVALID_ARGS,
						": Character after padding");
			}
			this->state[this->sl] = ndx[c0];
			if (this->state[this->sl] >= 64) {
				// error: non-base64 character
				// detectable because i2 != i1
				this->sl = 0; // reset for reuse
				throw Exception(__FILE__, __LINE__,
						error::INVALID_ARGS,
						": %02x is not a base64 character",
						static_cast<unsigned int>(c0));
			}
		} else {
			if (this->sl < 2) {
				// error: non-base64 character
				// detectable because i2 != i1
				this->sl = 0; // reset for reuse
				throw Exception(__FILE__, __LINE__,
						error::INVALID_ARGS,
						": Not a base64 character");
			}
			this->state[this->sl] = '=';
			if (3 == this->sl) {
				++i1;
				if (i1 != i2) {
					this->sl = 0; // reset for reuse
					throw Exception(__FILE__, __LINE__,
							error::INVALID_ARGS,
							": Not a base64 character");
				}
				this->sl = 4;
				return dec4(std::forward<OI>(o));
			}
			// sl = 2
		}
		++i1;
		++this->sl;

		if (4 == this->sl) {
			this->sl = 0;
			o = dec4(std::forward<OI>(o));
		}
	}
	return std::forward<OI>(o);
}

template<const unsigned char ndx[256]>
template<typename OI>
inline OI
base64dectmpl<ndx>::end_decode(OI&& o) {
	if (0 == this->sl)
		return std::forward<OI>(o);
	if (4 == this->sl) {
		this->sl = 0; // reset for reuse
		return std::forward<OI>(o);
	}
	// base64: error: input size is not a multiple of 4
	// Undetectable at chunk level.
	// ubase64: not necessarily an error
	return this->flush(std::forward<OI>(o));
}

template<const unsigned char ndx[256]>
template<typename OI>
inline OI
base64dectmpl<ndx>::dec4(OI&& o) {
	*o = static_cast<unsigned char>((this->state[0] << 2) |
					(this->state[1] >> 4));
	++o;

	if ('=' == this->state[2])
		return std::forward<OI>(o);
	*o = static_cast<unsigned char>((this->state[1] << 4) |
					(this->state[2] >> 2));
	++o;

	if ('=' == this->state[3])
		return std::forward<OI>(o);
	*o = static_cast<unsigned char>((this->state[2] << 6) |
					this->state[3]);
	++o;
	return std::forward<OI>(o);
}


template<const unsigned char table[64]>
class DSO_LOCAL base64enctmpl;
template<const unsigned char table[64]>
class base64enctmpl : public base64_detail::padder<table> {
public:
private:
	template<typename Iterator>
	struct helper {
		typedef typename std::conditional<
			std::is_array<
				typename std::remove_reference<Iterator>::type
			>::value,
			typename std::decay<Iterator>::type,
			Iterator
		>::type type;
	};
public:
	base64enctmpl() : sl(0) {}

	template<typename II1, typename II2, typename OI,
		 typename std::enable_if<
			 std::is_same<
				 typename std::decay<II1>::type,
				 typename std::decay<II2>::type
			 >::value,
		 int>::type = 0>
	static auto enc(II1&&, const II2&, OI&&) -> typename helper<OI>::type;

	template<typename II1, typename II2,
		 typename std::enable_if<
			 std::is_same<
				 typename std::decay<II1>::type,
				 typename std::decay<II2>::type
			 >::value,
		 int>::type = 0>
	static std::string enc(II1&&, const II2&);

	template<typename Sequence>
	static std::string enc(Sequence&&);

	template<typename II1, typename II2, typename OI,
		 typename std::enable_if<
			 std::is_same<
				 typename std::decay<II1>::type,
				 typename std::decay<II2>::type
			 >::value,
		 int>::type = 0>
	auto chunk_enc(II1&&, const II2&, OI&&)
		-> typename helper<OI>::type;

	template<typename OI>
	auto end_enc(OI&&) -> typename helper<OI>::type;
private:
	template<typename II1, typename II2, typename OI>
	static OI encode(II1&&, const II2&, OI&&);

	template<typename II1, typename II2, typename OI>
	OI chunk_encode(II1&&, const II2&, OI&&);

	template<typename OI>
	OI end_encode(OI&&);

	template<typename OI>
	OI enc3(OI&&);

	unsigned int sl;
	unsigned char state[3];
};


template<const unsigned char table[64]>
template<typename II1, typename II2, typename OI,
	 typename std::enable_if<
		 std::is_same<
			 typename std::decay<II1>::type,
			 typename std::decay<II2>::type
		 >::value,
	 int>::type>
auto
base64enctmpl<table>::enc(II1&& s, const II2& e, OI&& o)
	-> typename base64enctmpl<table>::template helper<OI>::type {
	return encode(std::forward<typename helper<II1>::type>(s), e,
		      std::forward<typename helper<OI>::type>(o));
}

template<const unsigned char table[64]>
template<typename II1, typename II2,
	 typename std::enable_if<
		 std::is_same<
			 typename std::decay<II1>::type,
			 typename std::decay<II2>::type
		 >::value, int>::type>
std::string
base64enctmpl<table>::enc(II1&& i1, const II2& i2) {
	std::string r;
	enc(std::forward<II1>(i1), i2, std::back_inserter(r));
	return r;
}

template<const unsigned char table[64]>
template<typename Sequence>
std::string
base64enctmpl<table>::enc(Sequence&& s) {
	return enc(s.begin(), s.end());
}

template<const unsigned char table[64]>
template<typename II1, typename II2, typename OI,
	 typename std::enable_if<
		 std::is_same<
			 typename std::decay<II1>::type,
			 typename std::decay<II2>::type
		 >::value,
	 int>::type>
auto
base64enctmpl<table>::chunk_enc(II1&& s, const II2& e, OI&& o)
	-> typename base64enctmpl<table>::template helper<OI>::type {
	return chunk_encode(std::forward<typename helper<II1>::type>(s), e,
			    std::forward<typename helper<OI>::type>(o));
}

template<const unsigned char table[64]>
template<typename OI>
auto
base64enctmpl<table>::end_enc(OI&& o)
	-> typename base64enctmpl<table>::template helper<OI>::type {
	return end_encode(std::forward<typename helper<OI>::type>(o));
}

template<const unsigned char table[64]>
template<typename II1, typename II2, typename OI>
inline OI
base64enctmpl<table>::encode(II1&& i1, const II2& i2, OI&& o) {
	while (i1 != i2) {
		unsigned char c0 = *i1;
		*o = table[c0 >> 2];
		++o;
		++i1;
		if (i1 == i2) {
			*o = table[(c0 << 4) & 0x3f];
			++o;
			return base64_detail::padder<table>::pad2(
				std::forward<OI>(o));
		}

		unsigned char c1 = *i1;
		*o = table[((c0 << 4) | (c1 >> 4)) & 0x3f];
		++o;
		++i1;
		if (i1 == i2) {
			*o = table[(c1 << 2) & 0x3f];
			++o;
			return base64_detail::padder<table>::pad1(
				std::forward<OI>(o));
		}

		unsigned char c2 = *i1;
		*o = table[((c1 << 2) | (c2 >> 6)) & 0x3f];
		++o;
		*o = table[c2 & 0x3f];
		++o;
		++i1;
	}
	return std::forward<OI>(o);
}

template<const unsigned char table[64]>
template<typename II1, typename II2, typename OI>
inline OI
base64enctmpl<table>::chunk_encode(II1&& i1, const II2& i2, OI&& o) {
	while (i1 != i2) {
		state[sl] = *i1;
		++i1;
		++sl;

		if (3 == sl) {
			sl = 0;
			o = enc3(std::forward<OI>(o));
		}
	}
	return std::forward<OI>(o);
}

template<const unsigned char table[64]>
template<typename OI>
inline OI
base64enctmpl<table>::end_encode(OI&& o) {
	if (0 == sl)
		return std::forward<OI>(o);
	*o = table[state[0] >> 2];
	++o;
	if (1 == sl) {
		*o = table[(state[0] << 4) & 0x3f];
		++o;
		sl = 0; // reset for reuse
		return base64_detail::padder<table>::pad2(std::forward<OI>(o));
	}

	*o = table[((state[0] << 4) | (state[1] >> 4)) & 0x3f];
	++o;

	*o = table[(state[1] << 2) & 0x3f];
	++o;

	sl = 0; // reset for reuse
	return base64_detail::padder<table>::pad1(std::forward<OI>(o));
}

template<const unsigned char table[64]>
template<typename OI>
inline OI
base64enctmpl<table>::enc3(OI&& o) {
	*o = table[state[0] >> 2];
	++o;
	
	*o = table[((state[0] << 4) | (state[1] >> 4)) & 0x3f];
	++o;
	
	*o = table[((state[1] << 2) | (state[2] >> 6)) & 0x3f];
	++o;

	*o = table[state[2] & 0x3f];
	++o;

	return std::forward<OI>(o);
}

template<const unsigned char table[64], const unsigned char ndx[256]>
class DSO_LOCAL base64tmpl;
template<const unsigned char table[64], const unsigned char ndx[256]>
class base64tmpl : public base64enctmpl<table>,
		   public base64dectmpl<ndx> {
};

namespace base64_detail {

template<int n>
const unsigned char ___tables<n>::b64tbl[64] = {
	'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J',
	'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T',
	'U', 'V', 'W', 'X', 'Y', 'Z',
	'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j',
	'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't',
	'u', 'v', 'w', 'x', 'y', 'z',
	'0', '1', '2', '3', '4', '5', '6', '7', '8', '9',
	'+', '/'
};
template<int n>
const unsigned char ___tables<n>::ub64tbl[64] = {
	'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J',
	'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T',
	'U', 'V', 'W', 'X', 'Y', 'Z',
	'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j',
	'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't',
	'u', 'v', 'w', 'x', 'y', 'z',
	'0', '1', '2', '3', '4', '5', '6', '7', '8', '9',
	'-', '_'
};
template<int n>
const unsigned char ___tables<n>::b64ndx[256] = {
/*  0*/	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
/* 10*/	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
/* 20*/	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
/* 30*/	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
/* 40*/	0xff, 0xff, 0xff,   62, 0xff, 0xff, 0xff,   63,   52,   53,
/* 50*/	  54,   55,   56,   57,   58,   59,   60,   61, 0xff, 0xff,
/* 60*/	0xff, 0xff, 0xff, 0xff, 0xff,    0,    1,    2,    3,    4,
/* 70*/	   5,    6,    7,    8,    9,   10,   11,   12,   13,   14,
/* 80*/	  15,   16,   17,   18,   19,   20,   21,   22,   23,   24,
/* 90*/	  25, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,   26,   27,   28,
/*100*/	  29,   30,   31,   32,   33,   34,   35,   36,   37,   38,
/*110*/	  39,   40,   41,   42,   43,   44,   45,   46,   47,   48,
/*120*/	  49,   50,   51, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
/*130*/	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
/*140*/	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
/*150*/	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
/*160*/	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
/*170*/	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
/*180*/	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
/*190*/	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
/*200*/	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
/*210*/	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
/*220*/	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
/*230*/	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
/*240*/	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
/*250*/	0xff, 0xff, 0xff, 0xff, 0xff, 0xff
};
template<int n>
const unsigned char ___tables<n>::ub64ndx[256] = {
/*  0*/	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
/* 10*/	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
/* 20*/	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
/* 30*/	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
/* 40*/	0xff, 0xff, 0xff, 0xff, 0xff,   62, 0xff, 0xff,   52,   53,
/* 50*/	  54,   55,   56,   57,   58,   59,   60,   61, 0xff, 0xff,
/* 60*/	0xff, 0xff, 0xff, 0xff, 0xff,    0,    1,    2,    3,    4,
/* 70*/	   5,    6,    7,    8,    9,   10,   11,   12,   13,   14,
/* 80*/	  15,   16,   17,   18,   19,   20,   21,   22,   23,   24,
/* 90*/	  25, 0xff, 0xff, 0xff, 0xff,   63, 0xff,   26,   27,   28,
/*100*/	  29,   30,   31,   32,   33,   34,   35,   36,   37,   38,
/*110*/	  39,   40,   41,   42,   43,   44,   45,   46,   47,   48,
/*120*/	  49,   50,   51, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
/*130*/	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
/*140*/	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
/*150*/	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
/*160*/	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
/*170*/	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
/*180*/	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
/*190*/	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
/*200*/	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
/*210*/	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
/*220*/	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
/*230*/	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
/*240*/	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
/*250*/	0xff, 0xff, 0xff, 0xff, 0xff, 0xff
};

}

typedef base64tmpl<base64_detail::tables::b64tbl,
		   base64_detail::tables::b64ndx> base64;
typedef base64tmpl<base64_detail::tables::ub64tbl,
		   base64_detail::tables::ub64ndx> ubase64;


// common instantiations
// is_base64
extern template bool
base64::is_base64<std::string>(std::string&&);
extern template bool
base64::is_base64<std::string&>(std::string&);
extern template bool
base64::is_base64<const std::string&>(const std::string&);

extern template bool
base64::is_base64<std::string::iterator,
		  std::string::iterator, 0>(
			  std::string::iterator&&,
			  const std::string::iterator&);
extern template bool
base64::is_base64<std::string::iterator&,
		  std::string::iterator, 0>(
			  std::string::iterator&,
			  const std::string::iterator&);
extern template bool
base64::is_base64<std::string::const_iterator,
		  std::string::const_iterator, 0>(
			  std::string::const_iterator&&,
			  const std::string::const_iterator&);
extern template bool
base64::is_base64<std::string::const_iterator&,
		  std::string::const_iterator, 0>(
			  std::string::const_iterator&,
			  const std::string::const_iterator&);

// extern template bool
// base64::is_base64<std::string_view>(std::string_view&&);
// extern template bool
// base64::is_base64<std::string_view&>(std::string_view&);
// extern template bool
// base64::is_base64<const std::string_view&>(const std::string_view&);

// extern template bool
// base64::is_base64<std::string_view::const_iterator,
// 		  std::string_view::const_iterator, 0>(
// 			  std::string_view::const_iterator&&,
// 			  const std::string_view::const_iterator&);
// extern template bool
// base64::is_base64<std::string_view::const_iterator&,
// 		  std::string_view::const_iterator, 0>(
// 			  std::string_view::const_iterator&,
// 			  const std::string_view::const_iterator&);

extern template bool
base64::is_base64<char const *, char const *, 0>(
	char const * &&, char const * const &);
extern template bool
base64::is_base64<char *, char *, 0>(char * &&, char * const &);
extern template bool
base64::is_base64<char const * &, char const *, 0>(
	char const * &, char const * const &);
extern template bool
base64::is_base64<char * &, char *, 0>(
	char * &, char * const &);

// dec
extern template std::string
base64::dec<std::string>(std::string&&);
extern template std::string
base64::dec<std::string&>(std::string&);
extern template std::string
base64::dec<const std::string&>(const std::string&);

extern template std::string
base64::dec<std::string::iterator,
	    std::string::iterator, 0>(
		    std::string::iterator&&,
		    const std::string::iterator&);
extern template std::string
base64::dec<std::string::iterator&,
	    std::string::iterator, 0>(
		    std::string::iterator&,
		    const std::string::iterator&);
extern template std::string
base64::dec<std::string::const_iterator,
	    std::string::const_iterator, 0>(
		    std::string::const_iterator&&,
		    const std::string::const_iterator&);
extern template std::string
base64::dec<std::string::const_iterator&,
	    std::string::const_iterator, 0>(
		    std::string::const_iterator&,
		    const std::string::const_iterator&);

// extern template std::string
// base64::dec<std::string_view>(std::string_view&&);
// extern template std::string
// base64::dec<std::string_view&>(std::string_view&);
// extern template std::string
// base64::dec<const std::string_view&>(const std::string_view&);

// extern template std::string
// base64::dec<std::string_view::const_iterator,
// 	    std::string_view::const_iterator, 0>(
// 		    std::string_view::const_iterator&&,
// 		    const std::string_view::const_iterator&);
// extern template std::string
// base64::dec<std::string_view::const_iterator&,
// 	    std::string_view::const_iterator, 0>(
// 		    std::string_view::const_iterator&,
// 		    const std::string_view::const_iterator&);

extern template std::string
base64::dec<char const *, char const *, 0>(
	char const * &&, char const * const &);
extern template std::string
base64::dec<char *, char *, 0>(char * &&, char * const &);
extern template std::string
base64::dec<char const * &, char const *, 0>(
	char const * &, char const * const &);
extern template std::string
base64::dec<char * &, char *, 0>(char * &, char * const &);

// enc
extern template std::string
base64::enc<std::string>(std::string&&);
extern template std::string
base64::enc<std::string&>(std::string&);
extern template std::string
base64::enc<const std::string&>(const std::string&);

extern template std::string
base64::enc<std::string::iterator,
	    std::string::iterator, 0>(std::string::iterator&&,
				      const std::string::iterator&);
extern template std::string
base64::enc<std::string::iterator&,
	    std::string::iterator, 0>(std::string::iterator&,
				      const std::string::iterator&);
extern template std::string
base64::enc<std::string::const_iterator,
	    std::string::const_iterator, 0>(
		    std::string::const_iterator&&,
		    const std::string::const_iterator&);
extern template std::string
base64::enc<std::string::const_iterator&,
	    std::string::const_iterator, 0>(
		    std::string::const_iterator&,
		    const std::string::const_iterator&);

// extern template std::string
// base64::enc<std::string_view>(std::string_view&&);
// extern template std::string
// base64::enc<std::string_view&>(std::string_view&);
// extern template std::string
// base64::enc<const std::string_view&>(const std::string_view&);

// extern template std::string
// base64::enc<std::string_view::const_iterator,
// 	    std::string_view::const_iterator, 0>(
// 		    std::string_view::const_iterator&&,
// 		    const std::string_view::const_iterator&);
// extern template std::string
// base64::enc<std::string_view::const_iterator&,
// 	    std::string_view::const_iterator, 0>(
// 		    std::string_view::const_iterator&,
// 		    const std::string_view::const_iterator&);

extern template std::string
base64::enc<char const *, char const *, 0>(
	char const * &&, char const * const &);
extern template std::string
base64::enc<char *, char *, 0>(char * &&, char * const &);
extern template std::string
base64::enc<char const * &, char const *, 0>(
	char const * &, char const * const &);
extern template std::string
base64::enc<char * &, char *, 0>(char * &, char * const &);

// common instantiations
// is_base64
extern template bool
ubase64::is_base64<std::string>(std::string&&);
extern template bool
ubase64::is_base64<std::string&>(std::string&);
extern template bool
ubase64::is_base64<const std::string&>(const std::string&);

extern template bool
ubase64::is_base64<std::string::iterator,
		   std::string::iterator, 0>(
			   std::string::iterator&&,
			   const std::string::iterator&);
extern template bool
ubase64::is_base64<std::string::iterator&,
		   std::string::iterator, 0>(
			   std::string::iterator&,
			   const std::string::iterator&);
extern template bool
ubase64::is_base64<std::string::const_iterator,
		   std::string::const_iterator, 0>(
			   std::string::const_iterator&&,
			   const std::string::const_iterator&);
extern template bool
ubase64::is_base64<std::string::const_iterator&,
		   std::string::const_iterator, 0>(
			   std::string::const_iterator&,
			   const std::string::const_iterator&);

// extern template bool
// ubase64::is_base64<std::string_view>(std::string_view&&);
// extern template bool
// ubase64::is_base64<std::string_view&>(std::string_view&);
// extern template bool
// ubase64::is_base64<const std::string_view&>(const std::string_view&);

// extern template bool
// ubase64::is_base64<std::string_view::const_iterator,
// 		   std::string_view::const_iterator, 0>(
// 			   std::string_view::const_iterator&&,
// 			   const std::string_view::const_iterator&);
// extern template bool
// ubase64::is_base64<std::string_view::const_iterator&,
// 		   std::string_view::const_iterator, 0>(
// 			   std::string_view::const_iterator&,
// 			   const std::string_view::const_iterator&);

extern template bool
ubase64::is_base64<char const *, char const *, 0>(
	char const * &&, char const * const &);
extern template bool
ubase64::is_base64<char *, char *, 0>(char * &&, char * const &);
extern template bool
ubase64::is_base64<char const * &, char const *, 0>(
	char const * &, char const * const &);
extern template bool
ubase64::is_base64<char * &, char *, 0>(
	char * &, char * const &);

// dec
extern template std::string
ubase64::dec<std::string>(std::string&&);
extern template std::string
ubase64::dec<std::string&>(std::string&);
extern template std::string
ubase64::dec<const std::string&>(const std::string&);

extern template std::string
ubase64::dec<std::string::iterator,
	     std::string::iterator, 0>(
		     std::string::iterator&&,
		     const std::string::iterator&);
extern template std::string
ubase64::dec<std::string::iterator&,
	     std::string::iterator, 0>(
		     std::string::iterator&,
		     const std::string::iterator&);
extern template std::string
ubase64::dec<std::string::const_iterator,
	     std::string::const_iterator, 0>(
		     std::string::const_iterator&&,
		     const std::string::const_iterator&);
extern template std::string
ubase64::dec<std::string::const_iterator&,
	     std::string::const_iterator, 0>(
		     std::string::const_iterator&,
		     const std::string::const_iterator&);

// extern template std::string
// ubase64::dec<std::string_view>(std::string_view&&);
// extern template std::string
// ubase64::dec<std::string_view&>(std::string_view&);
// extern template std::string
// ubase64::dec<const std::string_view&>(const std::string_view&);

// extern template std::string
// ubase64::dec<std::string_view::const_iterator,
// 	     std::string_view::const_iterator, 0>(
// 		     std::string_view::const_iterator&&,
// 		     const std::string_view::const_iterator&);
// extern template std::string
// ubase64::dec<std::string_view::const_iterator&,
// 	     std::string_view::const_iterator, 0>(
// 		     std::string_view::const_iterator&,
// 		     const std::string_view::const_iterator&);

extern template std::string
ubase64::dec<char const *, char const *, 0>(
	char const * &&, char const * const &);
extern template std::string
ubase64::dec<char *, char *, 0>(char * &&, char * const &);
extern template std::string
ubase64::dec<char const * &, char const *, 0>(
	char const * &, char const * const &);
extern template std::string
ubase64::dec<char * &, char *, 0>(char * &, char * const &);

// enc
extern template std::string
ubase64::enc<std::string>(std::string&&);
extern template std::string
ubase64::enc<std::string&>(std::string&);
extern template std::string
ubase64::enc<const std::string&>(const std::string&);

extern template std::string
ubase64::enc<std::string::iterator,
	     std::string::iterator, 0>(std::string::iterator&&,
				       const std::string::iterator&);
extern template std::string
ubase64::enc<std::string::iterator&,
	     std::string::iterator, 0>(std::string::iterator&,
				       const std::string::iterator&);
extern template std::string
ubase64::enc<std::string::const_iterator,
	     std::string::const_iterator, 0>(
		     std::string::const_iterator&&,
		     const std::string::const_iterator&);
extern template std::string
ubase64::enc<std::string::const_iterator&,
	     std::string::const_iterator, 0>(
		     std::string::const_iterator&,
		     const std::string::const_iterator&);

// extern template std::string
// ubase64::enc<std::string_view>(std::string_view&&);
// extern template std::string
// ubase64::enc<std::string_view&>(std::string_view&);
// extern template std::string
// ubase64::enc<const std::string_view&>(const std::string_view&);

// extern template std::string
// ubase64::enc<std::string_view::const_iterator,
// 	     std::string_view::const_iterator, 0>(
// 		     std::string_view::const_iterator&&,
// 		     const std::string_view::const_iterator&);
// extern template std::string
// ubase64::enc<std::string_view::const_iterator&,
// 	     std::string_view::const_iterator, 0>(
// 		     std::string_view::const_iterator&,
// 		     const std::string_view::const_iterator&);

extern template std::string
ubase64::enc<char const *, char const *, 0>(
	char const * &&, char const * const &);
extern template std::string
ubase64::enc<char *, char *, 0>(char * &&, char * const &);
extern template std::string
ubase64::enc<char const * &, char const *, 0>(
	char const * &, char const * const &);
extern template std::string
ubase64::enc<char * &, char *, 0>(char * &, char * const &);

}
}
