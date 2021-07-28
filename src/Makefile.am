include $(top_srcdir)/coverage.mk

SUBDIRS = lib

bin_PROGRAMS = pv

AM_CXXFLAGS = @EXTRA_CXXFLAGS@ @CXXSTD@

pv_SOURCES = main.cc

pv_CXXFLAGS = $(AM_CXXFLAGS) -I$(top_srcdir)/include

pv_LDFLAGS = $(top_builddir)/src/lib/libpv.la
