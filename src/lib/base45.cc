#include "base45.hh"
#include "ustring.hh"
#include "pv/Exception.hh"
#include "pv/Errors.hh"
#include <string>

namespace pv {
namespace utils {

static unsigned char do_map(char c);
static char rmap(unsigned char c);

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
			if (p >= 256)
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

std::string
base45_enc(const utils::ustring& in) {
	return base45_enc(in.data(), in.length());
}

std::string
base45_enc(const unsigned char *buf, std::size_t l) {
	return base45_enc(buf, buf + l);
}

std::string
base45_enc(const unsigned char *s, const unsigned char *e) {
	std::string r;
	while (e != s) {
		unsigned short p = *s++;
		if (e == s) {
			unsigned char c1 = p / 45;
			unsigned char c2 = p % 45;
			r.append(1, rmap(c2));
			r.append(1, rmap(c1));
			return r;
		}
		p <<= 8;
		p |= *s++;
		unsigned char c0 = p / 2025;
		unsigned short tmp0 = p % 2025;
		unsigned char c1 = tmp0 / 45;
		unsigned char c2 = tmp0 % 45;
		r.append(1, rmap(c2));
		r.append(1, rmap(c1));
		r.append(1, rmap(c0));
	}
	return r;
}

static char
rmap(unsigned char c) {
	// 0 <= c < 45
	if (c >= 36)
		switch (c) {
		case 36:
			return ' ';
		case 37:
			return '$';
		case 38:
			return '%';
		case 39:
			return '*';
		case 40:
			return '+';
		case 41:
			return '-';
		case 42:
			return '.';
		case 43:
			return '/';
		case 44:
			return ':';
		default:
			throw Exception(__FILE__, __LINE__, error::INVALID_ARGS,
					": out of range: %u",
					static_cast<unsigned int>(c));
		}
	if (c <= 9)
		return c + '0';
	// 10 <= c < 36
	return c - 10 + 'A';
}

}
}
