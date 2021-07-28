#include "CertParser.hh"
#include "Certs.hh"
#include "yajl_defs.h"
#include "pv/Exception.hh"
#include "pv/Errors.hh"

namespace pv {

int
CertParser::json_start_array(CertHandler *handler) {
	handler->on_start_array();
	return 1;
}

int
CertParser::json_end_array(CertHandler *handler) {
	handler->on_end_array();
	return 1;
}

int
CertParser::json_null(CertHandler *handler) {
	handler->on_null();
	return 1;
}

int
CertParser::json_bool(CertHandler *handler, int v) {
	handler->on_bool(0 != v);
	return 1;
}

int
CertParser::json_string(CertHandler *handler, const unsigned char *s,
			YAJL_UINT n) {
	handler->on_string(reinterpret_cast<const char *>(s), n);
	return 1;
}

int
CertParser::json_map_key(CertHandler *handler, const unsigned char *s,
			 YAJL_UINT n) {
	handler->on_map_key(reinterpret_cast<const char *>(s), n);
	return 1;
}

int
CertParser::json_int(CertHandler *handler, YAJL_LONG n) {
	handler->on_int(n);
	return 1;
}

int
CertParser::json_double(CertHandler *handler, double d) {
	handler->on_double(d);
	return 1;
}

int
CertParser::json_start_map(CertHandler *handler) {
	handler->on_start_map();
	return 1;
}

int
CertParser::json_end_map(CertHandler *handler) {
	handler->on_end_map();
	return 1;
}

CertParser::CertParser(Certs& certs)
	: handler(certs)
	, cb{
		reinterpret_cast<int (*)(void *)>(&json_null),
		reinterpret_cast<int (*)(void *, int)>(&json_bool),
		reinterpret_cast<int (*)(void *, YAJL_LONG)>(
			&json_int),
		reinterpret_cast<int (*)(void *, double)>(
			&json_double),
		nullptr,
		reinterpret_cast<int (*)(void *, const unsigned char *,
					 YAJL_UINT)>(&json_string),
		reinterpret_cast<int (*)(void *)>(&json_start_map),
		reinterpret_cast<int (*)(void *, const unsigned char *,
					 YAJL_UINT)>(
						 &json_map_key),
		reinterpret_cast<int (*)(void *)>(&json_end_map),
		reinterpret_cast<int (*)(void *)>(&json_start_array),
		reinterpret_cast<int (*)(void *)>(&json_end_array)
	},
#if YAJL_MAJOR < 2
	cnf{0, 0},
#endif
        handle(yajl_alloc(&cb,
#if YAJL_MAJOR < 2
			  &cnf,
#endif
			  nullptr, &handler))
{
	if (!handle)
		throw std::bad_alloc();
}

CertParser::~CertParser() noexcept {
	yajl_free(handle);
}

void
CertParser::parse(const char *d, std::size_t n) {
	if (yajl_status_ok !=
	    yajl_parse(handle,
		       reinterpret_cast<const unsigned char *>(d), n)) {
		unsigned char *msg = yajl_get_error(handle, 1, nullptr, 0);
		Exception e(__FILE__, __LINE__, error::INVALID_ARGS,
			    ": %s.", reinterpret_cast<const char *>(msg));
		yajl_free_error(handle, msg);
		throw e;
	}
}

void
CertParser::eos() {
	if (yajl_status_ok !=
#if YAJL_MAJOR >= 2
	    yajl_complete_parse(handle)
#else
	    yajl_parse_complete(handle)
#endif
		) {
		unsigned char *msg = yajl_get_error(handle, 1, nullptr, 0);
		Exception e(__FILE__, __LINE__, error::INVALID_ARGS,
		 	    ": %s.", reinterpret_cast<const char *>(msg));
		yajl_free_error(handle, msg);
		throw e;
	}
}

}

