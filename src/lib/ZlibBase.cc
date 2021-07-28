#include "ZlibBase.hh"
#include "pv/Exception.hh"
#include "pv/Errors.hh"
#include <zlib.h>
#include <cstring>

namespace pv {
namespace utils {

ZlibBase::ZlibBase(int code)
	: zlibcompressor(code)
	, zlibdecompressor(code)
{
}

zlibdecompressor::zlibdecompressor(int code) {
	if (Z_OK != inflateInit2(&zstream, code))
		throw Exception(__FILE__, __LINE__, error::ZLIB);
	zstream.next_out  = reinterpret_cast<Bytef *>(buf);
	zstream.avail_out = buf_size;
}

zlibdecompressor::~zlibdecompressor() {
	inflateEnd(&zstream);
}

zlibcompressor::zlibcompressor(int code) {
	if (Z_OK != deflateInit2(&zstream, 9, Z_DEFLATED, code, 8,
				 Z_DEFAULT_STRATEGY))
		throw Exception(__FILE__, __LINE__, error::ZLIB);
	zstream.next_out  = reinterpret_cast<Bytef *>(buf);
	zstream.avail_out = buf_size;
}

zlibcompressor::~zlibcompressor() {
	deflateEnd(&zstream);
}

zlibcommon::zlibcommon()
	: buf(new char[buf_size])
{
	memset(&zstream, 0, sizeof(zstream));
}

zlibcommon::~zlibcommon() {
	delete [] buf;
}

}
}
