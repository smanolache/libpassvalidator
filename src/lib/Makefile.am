include $(top_srcdir)/coverage.mk

SUBDIRS = . unit

lib_LTLIBRARIES = libpv.la

AM_CXXFLAGS = @EXTRA_CXXFLAGS@ @CXXSTD@

libpv_la_SOURCES = \
	Base.hh \
	CertHandler.hh \
	CertHandler.cc \
	CertMgr.hh \
	CertMgr.cc \
	CertParser.hh \
	CertParser.cc \
	Certs.hh \
	Data.hh \
	DataFwd.hh \
	DataTmpl.hh \
	Errors.cc \
	Exception.cc \
	QuoteJSON.hh \
	QuoteJSON.cc \
	Zlib.hh \
	Zlib.cc \
	ZlibBase.hh \
	ZlibBase.cc \
	base45.hh \
	base45.cc \
	base64.hh \
	base64.cc \
	impl.cc \
	iso8601.hh \
	iso8601.cc \
	is_contiguous.hh \
	iterator_concepts.hh \
	unique.hh \
	ustring.hh \
	visibility.h \
	yajl_defs.h \
	#

libpv_la_CXXFLAGS = $(AM_CXXFLAGS) -I$(top_srcdir)/include

libpv_la_LDFLAGS = \
	-lstdc++ \
	-lcurl \
	-lyajl \
	-lz \
	-lcbor \
	-lcrypto \
	-pthread \
	#

