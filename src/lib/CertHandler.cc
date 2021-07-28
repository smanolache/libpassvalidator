#include "CertHandler.hh"
#include "Certs.hh"
#include <cstring>
#include "iso8601.hh"
#include <chrono>
#include "unique.hh"
#include <openssl/pem.h>
#include <openssl/bio.h>
#include <openssl/opensslv.h>
#include <openssl/err.h>
#include <openssl/x509.h>
#include "base64.hh"
#include "pv/Exception.hh"
#include "pv/Errors.hh"

using clock_type = std::chrono::system_clock;
using time_point = clock_type::time_point;

namespace pv {

static const char CERTIFS[] = "certificates";
static const char NUMBER[] = "numberCertificate";
static const char KID[] = "kid";
static const char DATA[] = "data";
static const char UPD[] = "lastUpdatedDate";
static const char FROM[] = "validFrom";
static const char TO[] = "validTo";
static const char TYPE[] = "certificateType";
static const char COUNTRY[] = "countryCode";

static std::string ssl_error(unsigned long code);

CertHandler::CertHandler(Certs& certs__) noexcept
	: certs(certs__)
	, state(State::IDLE)
	, last_known_state(state)
	, level(0)
{
}

void
CertHandler::on_start_array() {
	switch (state) {
	case State::EXP_CERTS:
		state = State::CERTS;
		break;
	case State::UNKNOWN_KEY:
		++level;
		break;
	default:
		throw Exception(__FILE__, __LINE__, error::INVALID_ARGS,
				"Unexpected state: %d", state);
	}
}

void
CertHandler::on_end_array() {
	switch (state) {
	case State::CERTS:
		state = State::RESPONSE;
		break;
	case State::UNKNOWN_KEY:
		if (0 == --level) {
			state = last_known_state;
		}
		break;
	default:
		throw Exception(__FILE__, __LINE__, error::INVALID_ARGS,
				"Unexpected state: %d", state);
	}
}

void
CertHandler::on_start_map() {
	switch (state) {
	case State::IDLE:
		state = State::RESPONSE;
		break;
	case State::CERTS:
		crt.reset(new Cert());
		state = State::CERT;
		break;
	case State::UNKNOWN_KEY:
		++level;
		break;
	default:
		throw Exception(__FILE__, __LINE__, error::INVALID_ARGS,
				"Unexpected state: %d", state);
	}
}

void
CertHandler::on_end_map() {
	switch (state) {
	case State::RESPONSE:
		state = State::DONE;
		break;
	case State::CERT:
		add_cert();
		crt.reset();
		state = State::CERTS;
		break;
	case State::UNKNOWN_KEY:
		if (0 == --level) {
			state = last_known_state;
		}
		break;
	default:
		throw Exception(__FILE__, __LINE__, error::INVALID_ARGS,
				"Unexpected state: %d", state);
	}
}

void
CertHandler::on_map_key(const char *d, YAJL_UINT n) {
	switch (state) {
	case State::RESPONSE:
		on_response(d, n);
		break;
	case State::CERT:
		on_cert(d, n);
		break;
	case State::UNKNOWN_KEY:
		break;
	default:
		throw Exception(__FILE__, __LINE__, error::INVALID_ARGS,
				"Unexpected state: %d", state);
	}
}

void
CertHandler::on_response(const char *d, YAJL_UINT n) noexcept {
	if (n == sizeof(CERTIFS) - 1 &&
	    0 == strncasecmp(d, CERTIFS, sizeof(CERTIFS) - 1))
		state = State::EXP_CERTS;
	else if (n == sizeof(NUMBER) - 1 &&
		 0 == strncasecmp(d, NUMBER, sizeof(NUMBER) - 1)) {
		state = State::NUMBER;
	} else {
		last_known_state = state;
		state = State::UNKNOWN_KEY;
	}
}

void
CertHandler::on_cert(const char *d, YAJL_UINT n) noexcept {
	if (n == sizeof(KID) - 1 &&
	    0 == strncasecmp(d, KID, sizeof(KID) - 1)) {
		state = State::KID;
	} else if (n == sizeof(DATA) - 1 &&
		   0 == strncasecmp(d, DATA, sizeof(DATA) - 1)) {
		state = State::DATA;
	} else if (n == sizeof(UPD) - 1 &&
		   0 == strncasecmp(d, UPD, sizeof(UPD) - 1)) {
		state = State::UPD;
	} else if (n == sizeof(FROM) - 1 &&
		   0 == strncasecmp(d, FROM, sizeof(FROM) - 1)) {
		state = State::FROM;
	} else if (n == sizeof(TO) - 1 &&
		   0 == strncasecmp(d, TO, sizeof(TO) - 1)) {
		state = State::TO;
	} else if (n == sizeof(TYPE) - 1 &&
		   0 == strncasecmp(d, TYPE, sizeof(TYPE) - 1)) {
		state = State::TYPE;
	} else if (n == sizeof(COUNTRY) - 1 &&
		   0 == strncasecmp(d, COUNTRY, sizeof(COUNTRY) - 1)) {
		state = State::CC;
	} else {
		last_known_state = state;
		state = State::UNKNOWN_KEY;
	}
}

void
CertHandler::add_cert() {
	Certs::iterator i = certs.lower_bound(crt->kid);
	if (certs.end() != i && !certs.key_comp()(crt->kid, i->first))
		throw Exception(__FILE__, __LINE__, error::INVALID_ARGS,
				": duplicate keyid");
	std::string der = utils::base64::dec(
		crt->data.data(), crt->data.data() + crt->data.length());
#if OPENSSL_VERSION >= 0x10002000
	utils::Auto<BIO> b(BIO_new_mem_buf(der.data(), der.length()),
			   &BIO_free_all);
#else
	utils::Auto<BIO> b(BIO_new_mem_buf(const_cast<char *>(der.data()),
					   der.length()), &BIO_free_all);
#endif
        if (!b)
		throw Exception(__FILE__, __LINE__, error::OOM,
				": Building BIO object");
	X509 *x509 = d2i_X509_bio(b.get(), nullptr);
	if (!x509)
		throw Exception(__FILE__, __LINE__, error::INVALID_ARGS,
				": Could not read certificate: %s",
				ssl_error(ERR_get_error()).c_str());
	utils::Auto<X509> x509_(x509, &X509_free);

	struct tm tm;
	if (!ASN1_TIME_to_tm(X509_get0_notAfter(x509), &tm))
		throw Exception(__FILE__, __LINE__, error::INVALID_ARGS,
				": Reading certificate expiration time: %s",
				ssl_error(ERR_get_error()).c_str());
	time_t not_after = mktime(&tm);
	if (static_cast<time_t>(-1) == not_after)
		throw Exception(__FILE__, __LINE__, error::INVALID_ARGS,
				": Invalid certificate expiration time");
	crt->info.cert_to_ = clock_type::from_time_t(not_after);

	if (!ASN1_TIME_to_tm(X509_get0_notBefore(x509), &tm))
		throw Exception(__FILE__, __LINE__, error::INVALID_ARGS,
				": Reading certificate start time: %s",
				ssl_error(ERR_get_error()).c_str());
	time_t not_before = mktime(&tm);
	if (static_cast<time_t>(-1) == not_before)
		throw Exception(__FILE__, __LINE__, error::INVALID_ARGS,
				": Invalid certificate start time");
	crt->info.cert_from_ = clock_type::from_time_t(not_before);

	std::shared_ptr<EVP_PKEY> pub(X509_get_pubkey(&*x509), &EVP_PKEY_free);
	if (!pub)
		throw Exception(__FILE__, __LINE__, error::CRYPTO,
				": Building public key: %s",
				ssl_error(ERR_get_error()).c_str());
	certs.emplace_hint(i, std::move(crt->kid), KeyData(std::move(crt->info),
							   std::move(pub)));
}

void
CertHandler::on_string(const char *s, YAJL_UINT n) {
	switch (state) {
	case State::KID:
		crt->kid = std::string(s, n);
		state = State::CERT;
		break;
	case State::DATA:
		crt->data = std::string(s, n);
		state = State::CERT;
		break;
	case State::UPD:
		state = State::CERT;
		break;
	case State::FROM:
		crt->info.from_ = utils::iso8601(s, n);
		state = State::CERT;
		break;
	case State::TO:
		crt->info.to_ = utils::iso8601(s, n);
		state = State::CERT;
		break;
	case State::CC:
		crt->info.country_ = std::string(s, n);
		state = State::CERT;
		break;
	case State::TYPE:
		state = State::CERT;
		break;
	case State::UNKNOWN_KEY:
		if (0 == level) {
			state = last_known_state;
		}
		break;
	default:
		throw Exception(__FILE__, __LINE__, error::INVALID_ARGS,
				"Unexpected state: %d", state);
	}
}

void
CertHandler::on_int(YAJL_LONG) {
	switch (state) {
	case State::NUMBER:
		state = State::RESPONSE;
		break;
	case State::UNKNOWN_KEY:
		if (0 == level) {
			state = last_known_state;
		}
		break;
	default:
		throw Exception(__FILE__, __LINE__, error::INVALID_ARGS,
				"Unexpected state: %d", state);
	}
}

void
CertHandler::on_null() {
	switch (state) {
	case State::EXP_CERTS:
		state = State::RESPONSE;
		break;
	case State::CERTS:
		break;
	case State::KID:
		state = State::CERT;
		break;
	case State::DATA:
		state = State::CERT;
		break;
	case State::UPD:
		state = State::CERT;
		break;
	case State::FROM:
		state = State::CERT;
		break;
	case State::TO:
		state = State::CERT;
		break;
	case State::CC:
		state = State::CERT;
		break;
	case State::TYPE:
		state = State::CERT;
		break;
	case State::NUMBER:
		state = State::CERT;
		break;
	case State::UNKNOWN_KEY:
		if (0 == level) {
			state = last_known_state;
		}
		break;
	default:
		throw Exception(__FILE__, __LINE__, error::INVALID_ARGS,
				"Unexpected state: %d", state);
	}
}

void
CertHandler::on_bool(bool) {
	switch (state) {
	case State::UNKNOWN_KEY:
		if (0 == level) {
			state = last_known_state;
		}
		break;
	default:
		throw Exception(__FILE__, __LINE__, error::INVALID_ARGS,
				"Unexpected state: %d", state);
	}
}

void
CertHandler::on_double(double) {
	switch (state) {
	case State::UNKNOWN_KEY:
		if (0 == level) {
			state = last_known_state;
		}
		break;
	default:
		throw Exception(__FILE__, __LINE__, error::INVALID_ARGS,
				"Unexpected state: %d", state);
	}
}

static std::string
ssl_error(unsigned long code) {
	char buf[1024];
        ERR_error_string_n(code, buf, sizeof(buf));
	return std::string(buf);
}

}
