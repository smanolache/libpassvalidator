#include "iso8601.hh"
#include <chrono>
#include <cstddef>
#include <string>
#include <ctime>
#include <cctype>
#include <cstring>
#include "pv/Exception.hh"
#include "pv/Errors.hh"

using clock_type = std::chrono::system_clock;
using std::chrono::milliseconds;
using std::chrono::minutes;
using std::chrono::hours;
using std::chrono::seconds;

namespace pv {
namespace utils {

static clock_type::time_point iso8601_date(const char *d, std::size_t n,
					   clock_type::duration&);
static clock_type::time_point do_iso8601(const char *d, std::size_t n,
					 clock_type::duration&);

static clock_type::time_point
iso8601(struct tm&, const char *d, std::size_t n, bool extended,
	clock_type::duration&);

static clock_type::time_point
do_iso8601(struct tm&, const char *d, std::size_t n, bool extended,
	   clock_type::duration&);

static unsigned int take_two(char const * & d, std::size_t& n);
static unsigned int take_four(char const * & d, std::size_t& n);

static clock_type::time_point to_time_point(struct tm&);
static clock_type::time_point to_local_time_point(struct tm& tm,
						  clock_type::duration&);

template<typename Unit>
static bool fractional(char const *& d, std::size_t& n, clock_type::duration&);

static bool tz(char const *& d, std::size_t& n, clock_type::duration&);

clock_type::time_point
iso8601(const char *d, std::size_t n) {
	clock_type::duration off;
	return iso8601(d, n, off);
}

clock_type::time_point
iso8601(const char *d, std::size_t n, clock_type::duration& off) {
	static constexpr std::size_t COMPACT_DATE = 8;
	static constexpr std::size_t EXTENDED_DATE = 10;
	const char *T = reinterpret_cast<const char *>(memchr(d, 'T', n));
	if (nullptr == T)
		return iso8601_date(d, n, off);
	// both date and time => date must be complete =>
	// it is either 8 chars long (no separator) or
	// 10 chars long (with separator)
	std::size_t date_len = T - d;
	if (COMPACT_DATE == date_len) {
		struct tm tm;
		memset(&tm, 0, sizeof(tm));
		tm.tm_mday = 1;
		tm.tm_isdst = -1;
		const char *p = strptime(d, "%Y%m%d", &tm);
		if (T != p)
			return clock_type::time_point::min();
		return iso8601(tm, T + 1, n - (p + 1 - d), false, off);
	} else if (EXTENDED_DATE == date_len) {
		struct tm tm;
		memset(&tm, 0, sizeof(tm));
		tm.tm_mday = 1;
		tm.tm_isdst = -1;
		const char *p = strptime(d, "%F", &tm);
		if (T != p)
			return clock_type::time_point::min();
		return iso8601(tm, T + 1, n - (p + 1 - d), true, off);
	}
	// neither compact nor extended date
	return clock_type::time_point::min();
}

// no T => either compact representation or no time part
static clock_type::time_point
iso8601_date(const char *d, std::size_t n, clock_type::duration& off) {
	try {
		return do_iso8601(d, n, off);
	} catch (...) {
		return clock_type::time_point::min();
	}
}

// either only date, or compact
static clock_type::time_point
do_iso8601(const char *d, std::size_t n, clock_type::duration& off) {
	struct tm tm;
	memset(&tm, 0, sizeof(tm));
	tm.tm_mday = 1;
	tm.tm_isdst = -1;
	tm.tm_year = take_four(d, n) - 1900;
	if (0 == n)
		return to_local_time_point(tm, off);
	bool extended;
	if ('-' == *d) {
		--n;
		if (0 == n)
			throw Exception(__FILE__, __LINE__, error::INVALID_ARGS,
					": Invalid date");
		++d;
		extended = true;
	} else
		extended = false;
	tm.tm_mon = take_two(d, n);
	if (tm.tm_mon < 1 || tm.tm_mon > 12)
		throw Exception(__FILE__, __LINE__, error::INVALID_ARGS,
				": Invalid month");
	--tm.tm_mon;
	if (0 == n)
		return to_local_time_point(tm, off);
	if (extended) {
		if ('-' != *d)
			throw Exception(__FILE__, __LINE__, error::INVALID_ARGS,
					": No separator in date");
		++d;
		--n;
	}
	tm.tm_mday = take_two(d, n);
	if (tm.tm_mday < 1 || tm.tm_mday > 31)
		Exception(__FILE__, __LINE__, error::INVALID_ARGS,
			  ": Invalid day of month");
	if (0 == n)
		return to_local_time_point(tm, off);
	if (extended)
		throw Exception(__FILE__, __LINE__, error::INVALID_ARGS,
				": Extra input in date");
	return do_iso8601(tm, d, n, false, off);
}

// parse time; the date part is already parsed and stored in tm
static clock_type::time_point
iso8601(struct tm& tm, const char *d, std::size_t n, bool extended,
	clock_type::duration& off) {
	try {
		return do_iso8601(tm, d, n, extended, off);
	} catch (...) {
		return clock_type::time_point::min();
	}
}

static clock_type::time_point
do_iso8601(struct tm& tm, const char *d, std::size_t n, bool extended,
	   clock_type::duration& off) {
	// hour
	tm.tm_hour = take_two(d, n);
	if (tm.tm_hour > 23)
		throw Exception(__FILE__, __LINE__, error::INVALID_ARGS,
				": Hour exceeds 23");
	if (0 == n)
		// HH
		return to_local_time_point(tm, off);
	clock_type::duration fraction(0);
	if (fractional<hours>(d, n, fraction))
		if (0 == n)
			// HH.p...
			return to_local_time_point(tm, off) + fraction;
	if (tz(d, n, off)) {
		if (0 != n)
			throw Exception(__FILE__, __LINE__, error::INVALID_ARGS,
					": Extra input after timezone");
		// HHZ or HH+/-nn[[:]nn]
		return to_time_point(tm) + fraction - off;
	}
	if (extended) {
		if (':' != *d)
			throw Exception(__FILE__, __LINE__, error::INVALID_ARGS,
					": No time separator in extended format");
		++d;
		--n;
	}

	// minute
	tm.tm_min = take_two(d, n);
	if (tm.tm_min > 59)
		throw Exception(__FILE__, __LINE__, error::INVALID_ARGS,
				": Minute exceeds 59");
	if (0 == n)
		// HH[:]mm
		return to_local_time_point(tm, off);
	if (fractional<minutes>(d, n, fraction))
		if (0 == n)
			// HH[:]mm.p...
			return to_local_time_point(tm, off) + fraction;
	if (tz(d, n, off)) {
		if (0 != n)
			throw Exception(__FILE__, __LINE__, error::INVALID_ARGS,
					": Extra input after timezone");
		// HH[:]mm.p...Z or HH[:]mm.p...+/-nn[[:]nn]
		return to_time_point(tm) + fraction - off;
	}
	if (extended) {
		if (':' != *d)
			throw Exception(__FILE__, __LINE__, error::INVALID_ARGS,
					": No time separator in extended format");
		++d;
		--n;
	}

	// second
	tm.tm_sec = take_two(d, n);
	if (tm.tm_sec > 60)
		throw Exception(__FILE__, __LINE__, error::INVALID_ARGS,
				": Second exceeds 60");
	if (0 == n)
		// HH[:]mm[:]ss
		return to_local_time_point(tm, off);
	if (fractional<seconds>(d, n, fraction))
		if (0 == n)
			// HH[:]mm[:]ss.p...
			return to_local_time_point(tm, off) + fraction;
	if (tz(d, n, off)) {
		if (0 != n)
			throw Exception(__FILE__, __LINE__, error::INVALID_ARGS,
					": Extra input after timezone");
		// HH[:]mm[:]ss.p...Z or HH[:]mm[:]ss.p...+/-nn[[:]nn]
		return to_time_point(tm) + fraction - off;
	}
	throw Exception(__FILE__, __LINE__, error::INVALID_ARGS,
			": Extra input");
}

static unsigned int
take_two(char const * & d, std::size_t& n) {
	if (n < 2)
		throw Exception(__FILE__, __LINE__, error::INVALID_ARGS,
				": Less than two digits");
	unsigned int v = 0;
	if (!isdigit(*d))
		throw Exception(__FILE__, __LINE__, error::INVALID_ARGS,
				": No digit");
	v = *d - '0';
	++d;
	--n;
	if (!isdigit(*d))
		throw Exception(__FILE__, __LINE__, error::INVALID_ARGS,
				": No digit");
	v = 10 * v + (*d - '0');
	++d;
	--n;
	return v;
}

static unsigned int
take_four(char const * & d, std::size_t& n) {
	if (n < 4)
		throw Exception(__FILE__, __LINE__, error::INVALID_ARGS,
				": Less than four digits");
	unsigned int v = 0;
	if (!isdigit(*d))
		throw Exception(__FILE__, __LINE__, error::INVALID_ARGS,
				": No digit");
	v = *d - '0';
	++d;
	--n;
	unsigned int i = 1;
	do {
		if (!isdigit(*d))
			throw Exception(__FILE__, __LINE__, error::INVALID_ARGS,
					": No digit");
		v = 10 * v + (*d - '0');
		++i;
		++d;
		--n;
	} while (i < 4);
	return v;
}

static clock_type::time_point
to_time_point(struct tm& tm) {
	time_t t = timegm(&tm);
	if (static_cast<time_t>(-1) == t)
		throw Exception(__FILE__, __LINE__, error::INVALID_ARGS,
				": Invalid time point");
	return clock_type::from_time_t(t);
}

static clock_type::time_point
to_local_time_point(struct tm& tm, clock_type::duration& off) {
	time_t t = mktime(&tm);
	if (static_cast<time_t>(-1) == t)
		throw Exception(__FILE__, __LINE__, error::INVALID_ARGS,
				": Invalid time point");
	struct tm gmt_tm;
	gmtime_r(&t, &gmt_tm);
	tm.tm_isdst = 0;
	off = seconds(mktime(&tm) - mktime(&gmt_tm));
	return clock_type::from_time_t(t);
}

template<typename Unit>
static bool
fractional(char const * & d, std::size_t& n, clock_type::duration& f) {
	if ('.' != *d && ',' != *d)
		return false;
	++d;
	--n;
	unsigned int digits = 0, v = 0;
	for (; n > 0 && isdigit(*d); ++digits, ++d, --n)
		v = 10 * v + (*d - '0');
	if (0 == digits)
		throw Exception(__FILE__, __LINE__, error::INVALID_ARGS,
				": No digit after fractional point");
	uint64_t ticks =
		v * Unit::period::num * clock_type::time_point::period::den /
		(Unit::period::den * clock_type::time_point::period::num);
	for (; digits > 0; --digits)
		ticks /= 10;
	f = clock_type::duration(ticks);
	return true;
}

static bool
tz(char const * & d, std::size_t& n, clock_type::duration& off) {
	if ('Z' == *d) {
		++d;
		--n;
		off = clock_type::duration(0);
		return true;
	}
	if ('-' != *d && '+' != *d)
		return false;
	char sign = *d;
	++d;
	--n;
	unsigned int h = take_two(d, n);
	if (0 == n) {
		off = '+' == sign ? hours(h) : -hours(h);
		return true;
	}
	if (':' == *d) {
		++d;
		--n;
	}
	unsigned int m = take_two(d, n);
	off = '+' == sign ? hours(h) + minutes(m) : -hours(h) - minutes(m);
	return true;
}

}
}
