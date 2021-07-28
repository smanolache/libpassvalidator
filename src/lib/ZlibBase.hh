#pragma once

#include "visibility.h"
#include <type_traits>
#include <utility>
#include <string>
#include <zlib.h>
#include "is_contiguous.hh"
#include <iterator>
#include <algorithm>
#include "pv/Exception.hh"
#include "pv/Errors.hh"

namespace pv {
namespace utils {

class DSO_LOCAL zlibcommon;
class DSO_LOCAL zlibdecompressor;
class DSO_LOCAL zlibcompressor;
class DSO_LOCAL ZlibBase;

class zlibcommon {
protected:
	zlibcommon();
	~zlibcommon();

	z_stream zstream;
	char *buf;

	static const std::size_t buf_size = 32768;
};

class zlibdecompressor : protected zlibcommon {
protected:
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
	zlibdecompressor(int);
	~zlibdecompressor();

	template<typename II1, typename II2, typename OI,
		 typename std::enable_if<
			 std::is_same<
				 typename std::decay<II1>::type,
				 typename std::decay<II2>::type
			 >::value,
		 int>::type = 0>
	auto chunk_decompress(II1&&, const II2&, OI&&)
		-> typename helper<OI>::type;

	template<typename OI>
	auto end_decompress(OI&&) -> typename helper<OI>::type;

protected:
	template<typename II1, typename II2, typename OI,
		 typename std::enable_if<
			 std::is_same<
				 typename std::decay<II1>::type,
				 typename std::decay<II2>::type
			 >::value,
		 int>::type = 0>
	static auto decompress(II1&&, const II2&, OI&&, int)
		-> typename helper<OI>::type;

private:
	template<typename II1, typename II2, typename OI,
		 typename std::enable_if<
			 helpers::is_contiguous<II1>::value,
		 int>::type = 0>
	OI chunk_decomp(II1&&, const II2&, OI&&, int);

	template<typename II1, typename II2, typename OI,
		 typename std::enable_if<
			 !helpers::is_contiguous<II1>::value,
		 int>::type = 0>
	OI chunk_decomp(II1&&, const II2&, OI&&, int);

	template<typename OI>
	OI end_decomp(OI&&);
};

template<typename II1, typename II2, typename OI,
	 typename std::enable_if<
		 std::is_same<
			 typename std::decay<II1>::type,
			 typename std::decay<II2>::type
		 >::value,
	 int>::type>
auto
zlibdecompressor::decompress(II1&& i1, const II2& i2, OI&& o, int code)
	-> typename helper<OI>::type {
	zlibdecompressor z(code);
	return z.end_decompress(
		z.chunk_decompress(std::forward<II1>(i1), i2,
				   std::forward<OI>(o)));
}

template<typename II1, typename II2, typename OI,
	 typename std::enable_if<
		 std::is_same<
			 typename std::decay<II1>::type,
			 typename std::decay<II2>::type
		 >::value,
	 int>::type>
auto
zlibdecompressor::chunk_decompress(II1&& s, const II2& e, OI&& o)
	-> typename helper<OI>::type {
	return chunk_decomp(std::forward<typename helper<II1>::type>(s), e,
			    std::forward<typename helper<OI>::type>(o), 0);
}

template<typename OI>
auto
zlibdecompressor::end_decompress(OI&& o)
	-> typename helper<OI>::type {
	return end_decomp(std::forward<typename helper<OI>::type>(o));
}

template<typename II1, typename II2, typename OI,
	 typename std::enable_if<helpers::is_contiguous<II1>::value,
				 int>::type>
inline OI
zlibdecompressor::chunk_decomp(II1&& i1, const II2& i2, OI&& o, int flag) {
	zstream.next_in =
		const_cast<Bytef *>(reinterpret_cast<const Bytef *>(&*i1));
	zstream.avail_in = static_cast<int>(i2 - i1);
	int r;
	do {
		r = ::inflate(&zstream, flag);
		if (Z_OK != r && Z_STREAM_END != r)
			throw Exception(__FILE__, __LINE__, error::INVALID_ARGS,
					": Invalid compressed data");
		// copy
		o = std::copy(buf, buf + buf_size - zstream.avail_out,
			      std::forward<OI>(o));
		// adjust z.out
		zstream.next_out  = reinterpret_cast<Bytef *>(buf);
		zstream.avail_out = buf_size;
	} while (Z_OK == r && (Z_FINISH == flag || zstream.avail_in > 0));

	return std::forward<OI>(o);
}

template<typename II1, typename II2, typename OI,
	 typename std::enable_if<!helpers::is_contiguous<II1>::value,
				 int>::type>
inline OI
zlibdecompressor::chunk_decomp(II1&& i1, const II2& i2, OI&& o, int flag) {
	Bytef b;
	if (i1 == i2) {
		if (Z_FINISH != flag)
			return std::forward<OI>(o);
		zstream.next_in = nullptr;
		zstream.avail_in = 0;
	} else {
		b = *i1;
		++i1;
		zstream.next_in  = &b;
		zstream.avail_in = 1;
	}
	do {
		int r;
		do {
			r = ::inflate(&zstream, flag);
			if (Z_OK != r && Z_STREAM_END != r)
				throw Exception(__FILE__, __LINE__,
						error::INVALID_ARGS,
						": Invalid compressed data");
			// r == Z_OK || r == Z_STREAM_END
			// copy
			o = std::copy(buf, buf + buf_size - zstream.avail_out,
				      std::forward<OI>(o));
			// adjust z.out
			zstream.next_out  = reinterpret_cast<Bytef *>(buf);
			zstream.avail_out = buf_size;
		} while (Z_OK == r &&
			 (Z_FINISH == flag || zstream.avail_in > 0));
		// r == Z_STREAM_END ||
		// (r == Z_OK && Z_FINISH != flag && zstream.avail_in == 0)
		if (i1 == i2)
			return std::forward<OI>(o);
		b = *i1;
		++i1;
		zstream.next_in  = &b;
		zstream.avail_in = 1;
	} while (true);
}

template<typename OI>
inline OI
zlibdecompressor::end_decomp(OI&& o) {
	return chunk_decomp(static_cast<char *>(0), static_cast<char *>(0),
			    std::forward<OI>(o), Z_FINISH);
}

class zlibcompressor : protected zlibcommon {
protected:
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
	zlibcompressor(int);
	~zlibcompressor();

	template<typename II1, typename II2, typename OI,
		 typename std::enable_if<
			 std::is_same<
				 typename std::decay<II1>::type,
				 typename std::decay<II2>::type
			 >::value,
		 int>::type = 0>
	auto chunk_compress(II1&&, const II2&, OI&&)
		-> typename helper<OI>::type;

	template<typename OI>
	auto end_compress(OI&&) -> typename helper<OI>::type;

protected:
	template<typename II1, typename II2, typename OI,
		 typename std::enable_if<
			 std::is_same<
				 typename std::decay<II1>::type,
				 typename std::decay<II2>::type
			 >::value,
		 int>::type = 0>
	static auto compress(II1&&, const II2&, OI&&, int)
		-> typename helper<OI>::type;

private:
	template<typename II1, typename II2, typename OI,
		 typename std::enable_if<
			 helpers::is_contiguous<II1>::value,
			 int>::type = 0>
	OI chunk_comp(II1&&, const II2&, OI&&, int);

	template<typename II1, typename II2, typename OI,
		 typename std::enable_if<
			 !helpers::is_contiguous<II1>::value,
			 int>::type = 0>
	OI chunk_comp(II1&&, const II2&, OI&&, int);

	template<typename OI>
	OI end_comp(OI&&);
};


template<typename II1, typename II2, typename OI,
	 typename std::enable_if<
		 std::is_same<
			 typename std::decay<II1>::type,
			 typename std::decay<II2>::type
		 >::value,
	 int>::type>
auto
zlibcompressor::compress(II1&& i1, const II2& i2, OI&& o, int code)
	-> typename helper<OI>::type {
	zlibcompressor z(code);
	return z.end_compress(
		z.chunk_compress(std::forward<II1>(i1), i2,
				 std::forward<OI>(o)));
}

template<typename II1, typename II2, typename OI,
	 typename std::enable_if<
		 std::is_same<
			 typename std::decay<II1>::type,
			 typename std::decay<II2>::type
		 >::value,
	 int>::type>
auto
zlibcompressor::chunk_compress(II1&& s, const II2& e, OI&& o)
	-> typename helper<OI>::type {
	return chunk_comp(std::forward<typename helper<II1>::type>(s), e,
			  std::forward<typename helper<OI>::type>(o), 0);
}

template<typename OI>
auto
zlibcompressor::end_compress(OI&& o)
	-> typename helper<OI>::type {
	return end_comp(std::forward<typename helper<OI>::type>(o));
}

template<typename II1, typename II2, typename OI,
	 typename std::enable_if<helpers::is_contiguous<II1>::value,
				 int>::type>
inline OI
zlibcompressor::chunk_comp(II1&& i1, const II2& i2, OI&& o, int flag) {
	if (i1 == i2 && Z_FINISH != flag)
		return std::forward<OI>(o);
	zstream.next_in =
		const_cast<Bytef *>(reinterpret_cast<const Bytef *>(&*i1));
	zstream.avail_in = static_cast<int>(i2 - i1);
	int r;
	do {
		r = ::deflate(&zstream, flag);
		if (Z_OK != r && Z_STREAM_END != r)
			throw Exception(__FILE__, __LINE__, error::ZLIB);
		// copy
		o = std::copy(buf, buf + buf_size - zstream.avail_out,
			      std::forward<OI>(o));
		// adjust z.out
		zstream.next_out  = reinterpret_cast<Bytef *>(buf);
		zstream.avail_out = buf_size;
	} while (Z_OK == r && (Z_FINISH == flag || zstream.avail_in > 0));

	return std::forward<OI>(o);
}

template<typename II1, typename II2, typename OI,
	 typename std::enable_if<!helpers::is_contiguous<II1>::value,
				 int>::type>
inline OI
zlibcompressor::chunk_comp(II1&& i1, const II2& i2, OI&& o, int flag) {
	Bytef b;
	if (i1 == i2) {
		if (Z_FINISH != flag)
			return std::forward<OI>(o);
		zstream.next_in = nullptr;
		zstream.avail_in = 0;
	} else {
		b = *i1;
		++i1;
		zstream.next_in  = &b;
		zstream.avail_in = 1;
	}
	do {
		int r;
		do {
			r = ::deflate(&zstream, flag);
			if (Z_OK != r && Z_STREAM_END != r)
				throw Exception(__FILE__, __LINE__,
						error::ZLIB);
			// r == Z_OK || r == Z_STREAM_END
			// copy
			o = std::copy(buf, buf + buf_size - zstream.avail_out,
				      std::forward<OI>(o));
			// adjust z.out
			zstream.next_out  = reinterpret_cast<Bytef *>(buf);
			zstream.avail_out = buf_size;
		} while (Z_OK == r &&
			 (Z_FINISH == flag || zstream.avail_in > 0));
		// r == Z_STREAM_END ||
		// (r == Z_OK && Z_FINISH != flag && zstream.avail_in == 0)
		if (i1 == i2)
			return std::forward<OI>(o);
		b = *i1;
		++i1;
		zstream.next_in  = &b;
		zstream.avail_in = 1;
	} while (true);

	return std::forward<OI>(o);
}

template<typename OI>
OI
zlibcompressor::end_comp(OI&& o) {
	return chunk_comp(
		static_cast<const char *>(0), static_cast<const char *>(0),
		std::forward<OI>(o), Z_FINISH);
}

class ZlibBase : public zlibcompressor, public zlibdecompressor {
public:
	ZlibBase(int);
};

}
}
