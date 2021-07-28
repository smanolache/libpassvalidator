include $(top_srcdir)/coverage.mk

check_PROGRAMS = testLeak

check:
	./testLeak

AM_CXXFLAGS = @EXTRA_CXXFLAGS@ @CXXSTD@

testLeak_SOURCES = testLeak.hh testLeak.cc main.cc

testLeak_CXXFLAGS = \
	$(AM_CXXFLAGS) \
	-I$(top_srcdir)/include \
	#

testLeak_LDFLAGS = \
	$(top_builddir)/src/lib/libpv.la \
	-lcppunit \
	-pthread \
	#
