#include "QuoteJSON.hh"
#include <string>
#include <utility>
#include <cstring>
#include <cctype>
#include <codecvt>
#include <locale>

namespace pv {
namespace utils {

std::string
QuoteJSON::quote(const std::string& s) {
	return quote(s.data());
}

std::string
QuoteJSON::quote(const char *s) {
	const char *p = strpbrk(s, "\\\"\t\r\n\f\b");
	if (nullptr == p)
		return s;
	std::string r(s, p);
	do {
		r.append(1, '\\');
		switch (*p) {
		case '\\':
		case '"':
			r.append(1, *p);
			break;
		case '\t':
			r.append(1, 't');
			break;
		case '\r':
			r.append(1, 'r');
			break;
		case '\n':
			r.append(1, 'n');
			break;
		case '\f':
			r.append(1, 'f');
			break;
		case '\b':
			r.append(1, 'b');
			break;
		default:
			break;
		}
		s = p + 1;
		p = strpbrk(s, "\\\"\t\r\n\f\b");
		if (nullptr == p) {
			r.append(s);
			return r;
		}
		r.append(s, p);
	} while (true);
}

std::string
QuoteJSON::unquote(const std::string& s) {
	return unquote(s.data());
}

std::string
QuoteJSON::unquote(const char *s) {
	const char *p = strchr(s, '\\');
	if (nullptr == p)
		return s;
	std::string r(s, p);
	do {
		++p;
		switch (*p) {
		case '\\':
		case '"':
		case '/':
			r.append(1, *p);
			s = p + 1;
			break;
		case 't':
			r.append(1, '\t');
			s = p + 1;
			break;
		case 'r':
			r.append(1, '\r');
			s = p + 1;
			break;
		case 'n':
			r.append(1, '\n');
			s = p + 1;
			break;
		case 'f':
			r.append(1, '\f');
			s = p + 1;
			break;
		case 'b':
			r.append(1, '\b');
			s = p + 1;
			break;
		case 'u': {
			if (strlen(p + 1) < 4 || !isxdigit(p[1]) ||
			    !isxdigit(p[2]) || !isxdigit(p[3]) ||
			    !isxdigit(p[4])) {
				// error: \u not folowed by 4 hex chars
				r.append(1, '\\');
				s = p;
				break;
			}
			unsigned char hi, lo;
			hi = ((p[1] >= 'A' ? (p[1] & 0xdf) - '7' : p[1]) << 4) |
			      (p[2] >= 'A' ? (p[2] & 0xdf) - '7' : (p[2] & 0x0f));
			lo = ((p[3] >= 'A' ? (p[3] & 0xdf) - '7' : p[3]) << 4) |
			      (p[4] >= 'A' ? (p[4] & 0xdf) - '7' : (p[4] & 0x0f));
			std::wstring_convert<std::codecvt_utf8<char16_t>,
					     char16_t > cnv;
			r.append(cnv.to_bytes(std::u16string(1, hi << 8 | lo)));
			s = p + 5;
			break;
		}
		case '\0': // error, a json-string ending with a backslash.
			r.append(1, '\\');
			return r;
		default: // error, backslash followed by an unknown char.
			r.append(1, '\\');
			s = p;
			break;
		}
		p = strchr(s, '\\');
		if (nullptr == p) {
			r.append(s);
			return r;
		}
		r.append(s, p);
	} while (true);
}

}
}
