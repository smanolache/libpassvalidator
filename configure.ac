AC_INIT([check], [1.0.0], [bug-automake@gnu.org])
AM_INIT_AUTOMAKE([-Wall -Werror foreign -Wno-portability -Wno-unsupported])

cxxflags_save="$CXXFLAGS"
AC_PROG_CXX
CXXFLAGS="$cxxflags_save"

AM_PROG_AR

AC_CONFIG_MACRO_DIR([m4])

AC_ENABLE_STATIC(no)
AC_PROG_LIBTOOL

AC_SUBST([EXTRA_CXXFLAGS], ["-Wall -Wnon-virtual-dtor -O3"])

AC_ARG_ENABLE(debug,
	[AS_HELP_STRING([--enable-debug], [enable debug @<:@default=no@:>@])],
	[ if test "${enable_debug:-no}" != no; then EXTRA_CXXFLAGS="-Wall -Wnon-virtual-dtor -g -O0 -fno-inline"; fi ],
	[])

AC_SUBST([CXXSTD], [])
AC_ARG_WITH(cppstd,
	[AS_HELP_STRING([--with-cppstd], [specifies the C++ standard to use @<:@default=none@:>@])],
	[ if test "x${withval}" != x; then CXXSTD="-std=${withval}"; fi ],
	[]
)

AC_PREFIX_DEFAULT([/usr])
AC_CONFIG_COMMANDS_PRE([
if test "$sysconfdir" = '${prefix}/etc'; then sysconfdir=/etc; fi;
if test "$localstatedir" = '${prefix}/var'; then localstatedir=/var; fi;
if test "$includedir" = '${prefix}/include'; then includedir=${prefix}/include; fi
])

#LIBCURL_CHECK_CONFIG

dnl
snippet='
ifeq ($(strip $(foreach d,$(SUBDIRS),$(wildcard $(d)/*.gcda))),)
ifeq ($(strip $(foreach d,$(SUBDIRS),$(wildcard $(d)/$(d).info))),)
coverage-am:
else
coverage-am: $(notdir $(CURDIR)).info
$(notdir $(CURDIR)).info: $(foreach d,$(SUBDIRS),$(wildcard $(d)/$(d).info))
	if ! test -r $@; then \
		lcovargs=""; \
	else \
		lcovargs=" --add-tracefile $@"; \
	fi; \
	for p in $?; do \
		if test -s $${p}; \
		then \
			lcovargs+=" --add-tracefile $${p}"; \
		fi; \
	done; \
	if ! test -z "$${lcovargs}"; \
	then \
		lcov -q --no-external --base-directory $(top_srcdir) --compat libtool=on,split_crc=on --test-name $(notdir $(CURDIR)) --output-file $@.tmp $${lcovargs}; \
	fi; \
	mv $@.tmp $@
endif
else
coverage-am: $(notdir $(CURDIR)).info
ifeq ($(strip $(foreach d,$(SUBDIRS),$(wildcard $(d)/$(d).info))),)
$(notdir $(CURDIR)).info: $(foreach d,$(SUBDIRS) .libs,$(wildcard $(d)/*.gcda))
	dirs=""; \
	for p in $(sort $(dir $(filter %.gcda,$?))); do \
		dirs+=" --directory $${p}"; \
	done; \
	lcov -q --no-external --base-directory $(top_srcdir) --compat libtool=on,split_crc=on --output-file $@.full --capture $$dirs --test-name $(notdir $(CURDIR)); \
	lcov -q --no-external --base-directory $(top_srcdir) --compat libtool=on,split_crc=on --output-file $@.tmp --extract $@.full "*/$(notdir $(top_srcdir))/*"; \
	lcov -q --no-external --base-directory $(top_srcdir) --compat libtool=on,split_crc=on --output-file $@ --remove $@.tmp "*/unit/*" "*/TestHelpers/*" "*/Cassandra/*"; \
	rm $@.full $@.tmp
else
$(notdir $(CURDIR)).info: $(foreach d,$(SUBDIRS) .libs,$(wildcard $(d)/*.gcda)) $(foreach d,$(SUBDIRS),$(wildcard $(d)/$(d).info))
	dirs=""; \
	for p in $(sort $(dir $(filter %.gcda,$?))); do \
		dirs+=" --directory $${p}"; \
	done; \
	lcov -q --no-external --base-directory $(top_srcdir) --compat libtool=on,split_crc=on --output-file $@.full --capture $$dirs --test-name $(notdir $(CURDIR)); \
	lcov -q --no-external --base-directory $(top_srcdir) --compat libtool=on,split_crc=on --output-file $@.tmp --extract $@.full "*/$(notdir $(top_srcdir))/*" ; \
	lcov -q --no-external --base-directory $(top_srcdir) --compat libtool=on,split_crc=on --output-file $@.tmpinfo --remove $@.tmp "*/unit/*" "*/TestHelpers/*" "*/Cassandra/*"; \
	rm $@.full $@.tmp; \
	lcovargs=" --add-tracefile $@.tmpinfo"; \
	if test -r $@; then \
		lcovargs=" --add-tracefile $@"; \
	fi; \
	for p in $(filter %.info,$?); do \
		lcovargs+=" --add-tracefile $${p}"; \
	done; \
	lcov -q --no-external --base-directory $(top_srcdir) --compat libtool=on,split_crc=on --test-name $(notdir $(CURDIR)) --output-file $@.tmp $$lcovargs; \
	rm $@.tmpinfo; \
	mv $@.tmp $@
endif
endif
'

AC_SUBST([snippet])
AM_SUBST_NOTMAKE([snippet])

AC_CONFIG_HEADERS([config.h])
AC_CONFIG_FILES([
	Makefile
	include/Makefile
	src/lib/Makefile
	src/lib/unit/Makefile
	src/Makefile
])

AC_OUTPUT
