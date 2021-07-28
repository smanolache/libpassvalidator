#include "base45.hh"
#include "ustring.hh"
#include "pv/Exception.hh"
#include "pv/Errors.hh"
#include <string>

namespace pv {
namespace utils {

static unsigned char do_map(char c);

ustring
base45_dec(const std::string& in) {
	return base45_dec(in.data(), in.length());
}

ustring
base45_dec(const char *s, std::size_t l) {
	return base45_dec(s, s + l);
}

ustring
base45_dec(const char *s, const char *e) {
	ustring r;
	while (e != s) {
		unsigned short p = do_map(*s++);
		if (e == s) {
			r.append(1, static_cast<unsigned char>(p));
			return r;
		}
		p += do_map(*s++) * 45;
		if (e == s) {
			r.append(1, static_cast<unsigned char>(p >> 8));
			r.append(1, static_cast<unsigned char>(p & 0xff));
			return r;
		}
		p += do_map(*s++) * 2025;
		r.append(1, static_cast<unsigned char>(p >> 8));
		r.append(1, static_cast<unsigned char>(p & 0xff));
	}
	return r;
}

static unsigned char
do_map(char c) {
	if (c < '0')
		switch (c) {
		case ' ':
			return 36;
		case '$':
			return 37;
		case '%':
			return 38;
		case '*':
			return 39;
		case '+':
			return 40;
		case '-':
			return 41;
		case '.':
			return 42;
		case '/':
			return 43;
		default:
			throw Exception(__FILE__, __LINE__, error::INVALID_ARGS,
					": %02x is not a valid base45 char",
					static_cast<int>(c));
		}
	if (c <= '9')
		return c - '0';
	if (':' == c)
		return 44;
	if (c < 'A' || c > 'Z')
		throw Exception(__FILE__, __LINE__, error::INVALID_ARGS,
				": %02x is not a valid base45 char",
				static_cast<int>(c));
	return 10 + c - 'A';
	
}

}
}
