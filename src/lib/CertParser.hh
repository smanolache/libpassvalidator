#pragma once

#include "visibility.h"
#include "CertHandler.hh"
#include <yajl/yajl_parse.h>
#include "Certs.hh"
#include "yajl_defs.h"

namespace pv {

class DSO_LOCAL CertParser;
class CertParser {
private:
	CertHandler handler;

	yajl_callbacks cb;
#if YAJL_MAJOR < 2
	yajl_parser_config cnf;
#endif
	yajl_handle handle;

public:
	CertParser(Certs&);
	~CertParser();

	void parse(const char *, std::size_t);
	void eos();

private:
	static int json_null(CertHandler *);
	static int json_start_map(CertHandler *);
	static int json_end_map(CertHandler *);
	static int json_start_array(CertHandler *);
	static int json_end_array(CertHandler *);
	static int json_bool(CertHandler *, int);
	static int json_int(CertHandler *, YAJL_LONG);
	static int json_double(CertHandler *, double);
	static int json_string(CertHandler *, const unsigned char *, YAJL_UINT);
	static int json_map_key(CertHandler *, const unsigned char *,
				YAJL_UINT);
};

}
