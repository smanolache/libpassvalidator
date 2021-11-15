#include "pv/intf.hh"
#include <string>
#include "base45.hh"
#include "ustring.hh"
#include "Zlib.hh"
#include <iterator>
#include <utility>
#include <cbor.h>
#include <openssl/evp.h>
#include <openssl/err.h>
#include <openssl/pem.h>
#include <openssl/ec.h>
#include "CertMgr.hh"
#include <memory>
#include <iomanip>
#include "pv/Exception.hh"
#include "pv/Errors.hh"
#include "QuoteJSON.hh"
#include "Data.hh"
#include <sstream>
#include <chrono>
#include <ctime>
#include "iso8601.hh"

namespace pv {

using clock_type = std::chrono::system_clock;
using time_point = clock_type::time_point;

template<typename T>
using Auto = std::unique_ptr<T, void (*)(T *)>;

static bool do_check(EVP_PKEY *, const unsigned char *msg, std::size_t l,
		     const unsigned char *sig, std::size_t sl);
static unsigned char *make_sig(const unsigned char *sig, std::size_t l,
			       std::size_t *);
static std::string ssl_error(unsigned long code);

static Map::value_type protected_headers(const cbor_item_t *item);
static Map::value_type do_protected_headers(const cbor_item_t *item);
static Map::value_type adapt_protected(Map::value_type&&);
static Map::value_type unprotected_headers(const cbor_item_t *item);
static Data payload(const cbor_item_t *item);
static Data do_payload(const cbor_item_t *item);
static Map::value_type adapt_payload(Map::value_type&&);
static Map::value_type handle_map(const cbor_item_t *item);
static Array::value_type handle_array(const cbor_item_t *item);
static MapKey handle_key(const cbor_item_t *item);
static Data handle_value(const cbor_item_t *item);
static utils::ustring handle_bytestring(const cbor_item_t *item);
static std::string handle_string(const cbor_item_t *item);
static uint64_t handle_uint(const cbor_item_t *item);
static int64_t handle_negint(const cbor_item_t *item);
static Data handle_tag(const cbor_item_t *item);

static Data adapt_ts(Data&& in);
static Data adapt_alg(Data&& in);
static Data adapt_260(Data&& in);

static std::ostream& operator<<(std::ostream& os, const Base& k);
static std::ostream& operator<<(std::ostream& os, const Data& k);

static std::ostream& operator<<(std::ostream& os, const Map::value_type& c);
static std::ostream& operator<<(std::ostream& os, const Array::value_type& c);
static std::ostream& operator<<(std::ostream& os, const utils::ustring&);
static std::ostream& operator<<(std::ostream& os, const time_point&);

struct Claim::impl_type {
	Map prhdr;
	Map unprhdr;
	Data payload;

	impl_type();
	void print(std::ostream& os) const;
	time_point expires() const;
};

Claim::impl_type::impl_type()
	: prhdr{Map::value_type{}}
	, unprhdr{Map::value_type{}}
{
}

Claim::Claim(impl_type *impl__) noexcept
	: impl(impl__)
{
}

Claim::Claim(Claim&& other) noexcept
	: impl(other.impl)
{
	other.impl = nullptr;
}

Claim::~Claim() {
	delete impl;
}

time_point
Claim::expires() const {
	return impl->expires();
}

Result
App::check(const std::string& in) {
	static const char PFX[] = "HC1:";
	if (in.length() < sizeof(PFX) - 1)
		throw Exception(__FILE__, __LINE__, error::INVALID_ARGS,
				": Not a health certificate");
	utils::ustring in2 = utils::base45_dec(in.data() + sizeof(PFX) - 1,
					       in.data() + in.length());
	if (utils::Zlib::is_zlib(in2)) {
		utils::ustring r;
		utils::Zlib::decompress(in2.begin(), in2.end(),
					std::back_inserter(r));
		in2 = std::move(r);
	}

	if (in2.empty())
		throw Exception(__FILE__, __LINE__, error::INVALID_ARGS,
				": Empty data");

	switch (in2[0] >> 5) {
	case CBOR_TYPE_TAG:
		break;
	default:
		throw Exception(__FILE__, __LINE__, error::INVALID_ARGS,
				": Expected tag");
	}
	if (18 != (in2[0] & 0x1f))
		throw Exception(__FILE__, __LINE__, error::INVALID_ARGS,
				": Expected tag 0x18");

	cbor_load_result rlt;
	cbor_item_t *item = cbor_load(in2.data() + 1, in2.length() - 1, &rlt);
	if (!item)
		throw Exception(__FILE__, __LINE__, error::INVALID_ARGS,
				": Parsing cbor data: position: %zu; code: %d; "
				"read: %zu", rlt.error.position, rlt.error.code,
				rlt.read);
	Auto<cbor_item_t> item_(item, &cbor_intermediate_decref);

	switch (item->type) {
	case CBOR_TYPE_ARRAY:
		if (!cbor_array_is_definite(item) || 4 != cbor_array_size(item))
			throw Exception(__FILE__, __LINE__, error::INVALID_ARGS,
				": Expected four-element array");
		break;
	default:
		throw Exception(__FILE__, __LINE__, error::INVALID_ARGS,
				": Expected array");
	}

	cbor_item_t *e0 = cbor_array_get(item, 0);
	if (!e0)
		throw Exception(__FILE__, __LINE__, error::INVALID_ARGS,
				": Null protected headers");
	Auto<cbor_item_t> e0_(e0, &cbor_intermediate_decref);

	cbor_item_t *e1 = cbor_array_get(item, 1);
	if (!e1)
		throw Exception(__FILE__, __LINE__, error::INVALID_ARGS,
				": Null unprotected headers");
	Auto<cbor_item_t> e1_(e1, &cbor_intermediate_decref);

	cbor_item_t *e2 = cbor_array_get(item, 2);
	if (!e2)
		throw Exception(__FILE__, __LINE__, error::INVALID_ARGS,
				": Null payload");
	Auto<cbor_item_t> e2_(e2, &cbor_intermediate_decref);

	std::unique_ptr<Claim::impl_type> claim(new Claim::impl_type());
	claim->prhdr = protected_headers(e0);
	claim->unprhdr = unprotected_headers(e1);
	claim->payload = payload(e2);

	cbor_item_t *e3 = cbor_array_get(item, 3);
	if (!e3)
		throw Exception(__FILE__, __LINE__, error::INVALID_ARGS,
				": Null signature");
	Auto<cbor_item_t> e3_(e3, &cbor_intermediate_decref);

	cbor_item_t *n = cbor_new_definite_array(4);
	if (!n)
		throw Exception(__FILE__, __LINE__, error::OOM,
				": Allocating signature array object");
	Auto<cbor_item_t> n_(n, &cbor_intermediate_decref);
	cbor_item_t *s = cbor_build_string("Signature1");
	if (!s)
		throw Exception(__FILE__, __LINE__, error::OOM,
				": Allocating cbor string");
	Auto<cbor_item_t> s_(s, &cbor_intermediate_decref);
	if (!cbor_array_set(n, 0, s))
		throw Exception(__FILE__, __LINE__, error::OOM,
				": Adding string to array");
	if (!cbor_array_set(n, 1, e0))
		throw Exception(__FILE__, __LINE__, error::OOM,
				": Adding protected headers to array");
	cbor_item_t *b = cbor_build_bytestring(
		reinterpret_cast<cbor_data>(""), 0);
	if (!b)
		throw Exception(__FILE__, __LINE__, error::OOM,
				": Allocating bytestring");
	Auto<cbor_item_t> b_(b, &cbor_intermediate_decref);
	if (!cbor_array_set(n, 2, b))
		throw Exception(__FILE__, __LINE__, error::OOM,
				": Adding bytestring to array");
	if (!cbor_array_set(n, 3, e2))
		throw Exception(__FILE__, __LINE__, error::OOM,
				": Adding payload to array");
	cbor_mutable_data buf = nullptr;
	std::size_t sz = 0, tmp = 0;
	sz = cbor_serialize_alloc(n, &buf, &tmp);
	if (!sz)
		throw Exception(__FILE__, __LINE__, error::OOM,
				": Serializing cbor array object");
	std::unique_ptr<unsigned char, void(*)(unsigned char *)> buf_(
		buf, reinterpret_cast<void (*)(unsigned char *)>(&free));

	switch (e3->type) {
	case CBOR_TYPE_BYTESTRING:
		if (!cbor_bytestring_is_definite(e3))
			throw Exception(__FILE__, __LINE__, error::INVALID_ARGS,
					": The payload must be a definite "
					"bytestring");
		break;
	default:
		throw Exception(__FILE__, __LINE__, error::INVALID_ARGS,
				": The signature must be a bytestring");
	}

	const CertMgr *cert_mgr = CertMgr::cert_mgr(config);
	if (!cert_mgr)
		throw Exception(__FILE__, __LINE__, error::INTERNAL,
				": Error building the certificate manager");
	Map::value_type::const_iterator k =
		claim->prhdr.get().find(MapKey(std::string("kid")));
	if (claim->prhdr.get().end() == k) {
		k = claim->unprhdr.get().find(MapKey(std::string("kid")));
		if (claim->prhdr.get().end() == k)
			throw Exception(__FILE__, __LINE__, error::INVALID_ARGS,
					": No key id in the protected headers");
	}
	const ByteString *kid = dynamic_cast<const ByteString *>(k->second.get());
	const KeyData& key = cert_mgr->key(kid->get().data(),
					   kid->get().length());
	if (!key.key())
		return Result{Claim(claim.release()), key.info(),
			Status::UnknownKey,
			std::string(reinterpret_cast<const char *>(buf), sz)};
	time_point now = clock_type::now();
	if (now > key.info().to() || now > key.info().cert_to() ||
	    now < key.info().from() || now < key.info().cert_from())
		return Result{Claim(claim.release()), key.info(),
			Status::ExpiredClaim,
			std::string(reinterpret_cast<const char *>(buf), sz)};

	bool r = do_check(key.key().get(), buf, sz, cbor_bytestring_handle(e3),
			  cbor_bytestring_length(e3));

	return Result{Claim(claim.release()), key.info(),
		r ? Status::Ok : Status::InvalidSignature,
		std::string(reinterpret_cast<const char *>(buf), sz)};
}

static Map::value_type
protected_headers(const cbor_item_t *item) {
	switch (item->type) {
	case CBOR_TYPE_BYTESTRING:
		if (!cbor_bytestring_is_definite(item))
			throw Exception(__FILE__, __LINE__, error::INVALID_ARGS,
					": The protected headers must be a "
					"definite bytestring");
		break;
	default:
		throw Exception(__FILE__, __LINE__, error::INVALID_ARGS,
				": The protected headers must be a bytestring");
	}

	cbor_mutable_data d = cbor_bytestring_handle(item);
	if (!d)
		throw Exception(__FILE__, __LINE__, error::INVALID_ARGS,
				": The protected headers are null");
	std::size_t l = cbor_bytestring_length(item);
	cbor_load_result rlt;
	cbor_item_t *inner = cbor_load(d, l, &rlt);
	if (!inner)
		throw Exception(__FILE__, __LINE__, error::INVALID_ARGS,
				": Parsing the protected headers: position: "
				"%zu; code: %d; read: %zu",
				rlt.error.position, rlt.error.code, rlt.read);
	Auto<cbor_item_t> inner_(inner, &cbor_intermediate_decref);

	return do_protected_headers(inner);
}

static Map::value_type
do_protected_headers(const cbor_item_t *item) {
	switch (item->type) {
	case CBOR_TYPE_MAP:
		if (!cbor_map_is_definite(item))
			throw Exception(__FILE__, __LINE__, error::INVALID_ARGS,
					": The inner protected headers must "
					"be a definite map");
		break;
	default:
		throw Exception(__FILE__, __LINE__, error::INVALID_ARGS,
				": The inner protected headers must be a map");
	}

	return adapt_protected(handle_map(item));
}

static Map::value_type
adapt_protected(Map::value_type&& in) {
	Map::value_type r;
	for (Map::value_type::value_type& e: in)
		if (e.first.is_uint())
			switch (e.first.uint64()) {
			case 1:
				r.emplace(MapKey(std::string("alg")),
					  adapt_alg(std::move(e.second)));
				break;
			case 2:
				r.emplace(MapKey(std::string("crit")),
					  std::move(e.second));
				break;
			case 3:
				r.emplace(MapKey(std::string("content_type")),
					  std::move(e.second));
				break;
			case 4:
				r.emplace(MapKey(std::string("kid")),
					  std::move(e.second));
				break;
			case 5:
				r.emplace(MapKey(std::string("IV")),
					  std::move(e.second));
				break;
			case 6:
				r.emplace(MapKey(std::string("partial_IV")),
					  std::move(e.second));
				break;
			case 7:
				r.emplace(MapKey(std::string("counter_signature")),
					  std::move(e.second));
				break;
			default:
				std::ostringstream ss;
				ss << e.first.uint64();
				r.emplace(MapKey(ss.str()), std::move(e.second));
				break;
			}
		else if (e.first.is_int()) {
			std::ostringstream ss;
			ss << e.first.int64();
			r.emplace(MapKey(ss.str()), std::move(e.second));
		} else
			r.emplace(std::move(const_cast<MapKey&>(e.first)),
				  std::move(e.second));
	return r;
}

static Map::value_type
unprotected_headers(const cbor_item_t *item) {
	switch (item->type) {
	case CBOR_TYPE_MAP:
		if (!cbor_map_is_definite(item))
			throw Exception(__FILE__, __LINE__, error::INVALID_ARGS,
					": The unprotected headers must be a "
					"definite map");
		break;
	default:
		throw Exception(__FILE__, __LINE__, error::INVALID_ARGS,
				": The unprotected headers must be a map");
	}

	return adapt_protected(handle_map(item));
}

static Data
payload(const cbor_item_t *item) {
	switch (item->type) {
	case CBOR_TYPE_BYTESTRING:
		if (!cbor_bytestring_is_definite(item))
			throw Exception(__FILE__, __LINE__, error::INVALID_ARGS,
					": The payload must be a definite "
					"bytestring");
		break;
	default:
		throw Exception(__FILE__, __LINE__, error::INVALID_ARGS,
				": The payload must be a bytestring");
	}

	cbor_mutable_data d = cbor_bytestring_handle(item);
	if (!d)
		throw Exception(__FILE__, __LINE__, error::INVALID_ARGS,
				": The payload is null");
	std::size_t l = cbor_bytestring_length(item);
	cbor_load_result rlt;
	cbor_item_t *inner = cbor_load(d, l, &rlt);
	if (!inner)
		throw Exception(__FILE__, __LINE__, error::INVALID_ARGS,
				": Parsing the payload: position: %zu; code: "
				"%d; read: %zu", rlt.error.position,
				rlt.error.code, rlt.read);
	Auto<cbor_item_t> inner_(inner, &cbor_intermediate_decref);
	return do_payload(inner);
}

static Data
do_payload(const cbor_item_t *item) {
	switch (item->type) {
	case CBOR_TYPE_MAP:
		if (!cbor_map_is_definite(item))
			throw Exception(__FILE__, __LINE__, error::INVALID_ARGS,
					": The inner payload must be a "
					"definite map");
		break;
	default:
		throw Exception(__FILE__, __LINE__, error::INVALID_ARGS,
				": The inner payload must be a map");
	}

	return Data(new Map(adapt_payload(handle_map(item))));
}

static Map::value_type
adapt_payload(Map::value_type&& in) {
	Map::value_type r;
	for (Map::value_type::value_type& e: in)
		if (e.first.is_uint())
			switch (e.first.uint64()) {
			case 1:
				r.emplace(MapKey(std::string("iss")),
					  std::move(e.second));
				break;
			case 2:
				r.emplace(MapKey(std::string("sub")),
					  std::move(e.second));
				break;
			case 3:
				r.emplace(MapKey(std::string("aud")),
					  std::move(e.second));
				break;
			case 4:
				r.emplace(MapKey(std::string("exp")),
					  adapt_ts(std::move(e.second)));
				break;
			case 5:
				r.emplace(MapKey(std::string("nbf")),
					  adapt_ts(std::move(e.second)));
				break;
			case 6:
				r.emplace(MapKey(std::string("iat")),
					  adapt_ts(std::move(e.second)));
				break;
			case 7:
				r.emplace(MapKey(std::string("cti")),
					  std::move(e.second));
				break;
			default:
				std::ostringstream ss;
				ss << e.first.uint64();
				r.emplace(MapKey(ss.str()), std::move(e.second));
				break;
			}
		else if (e.first.is_int()) {
			switch (e.first.int64()) {
			case -260:
				r.emplace(MapKey(std::string("claim")),
					  adapt_260(std::move(e.second)));
				break;
			default:
				std::ostringstream ss;
				ss << e.first.uint64();
				r.emplace(MapKey(ss.str()), std::move(e.second));
				break;
			}
		} else
			r.emplace(std::move(const_cast<MapKey&>(e.first)),
				  std::move(e.second));
	return r;
}

static Data
adapt_ts(Data&& in) {
	const Base *b = in.get();
	const UInt64 *d = dynamic_cast<const UInt64 *>(b);
	if (!d)
		return std::move(in);
	struct tm r;
	time_t t = *d;
	if (!gmtime_r(&t, &r))
		return std::move(in);
	char buf[21];
	strftime(buf, sizeof(buf), "%FT%TZ", &r);
	return Data(new Text(buf));
}

static Data
adapt_alg(Data&& in) {
	const Base *b = in.get();
	const Int64 *d = dynamic_cast<const Int64 *>(b);
	if (!d)
		return std::move(in);
	switch (*d) {
	case -7:
		return Data(new Text("ECDSA-SHA256"));
	case -35:
		return Data(new Text("ECDSA-SHA384"));
	case -36:
		return Data(new Text("ECDSA-SHA512"));
	default:
		return std::move(in);
	}
}

static Data
adapt_260(Data&& in) {
	Base *b = in.get();
	Map *d = dynamic_cast<Map *>(b);
	if (!d)
		return std::move(in);
	Map::value_type r;
	for (Map::value_type::value_type& e: d->get())
		if (e.first.is_uint()) {
			std::ostringstream ss;
			ss << e.first.uint64();
			r.emplace(MapKey(ss.str()), std::move(e.second));
		} else if (e.first.is_int()) {
			std::ostringstream ss;
			ss << e.first.int64();
			r.emplace(MapKey(ss.str()), std::move(e.second));
		} else
			r.emplace(std::move(const_cast<MapKey&>(e.first)),
				  std::move(e.second));
	return Data(new Map(std::move(r)));
}

static Map::value_type
handle_map(const cbor_item_t *item) {
	std::size_t l = cbor_map_size(item);
	const cbor_pair *c = cbor_map_handle(item);
	Map::value_type r;
	for (std::size_t i = 0; i < l; ++i) {
		MapKey k(handle_key(c[i].key));
		Data v = handle_value(c[i].value);
		r.emplace(std::move(k), std::move(v));
	}
	return r;
}

static Array::value_type
handle_array(const cbor_item_t *item) {
	std::size_t l = cbor_array_size(item);
	cbor_item_t **c = cbor_array_handle(item);
	Array::value_type r;
	for (std::size_t i = 0; i < l; ++i)
		r.emplace_back(handle_value(c[i]));
	return r;
}

static MapKey
handle_key(const cbor_item_t *item) {
	switch (item->type) {
	case CBOR_TYPE_UINT:
		return MapKey(handle_uint(item));
	case CBOR_TYPE_NEGINT:
		return MapKey(handle_negint(item));
	case CBOR_TYPE_BYTESTRING:
		if (!cbor_bytestring_is_definite(item))
			throw Exception(__FILE__, __LINE__, error::INVALID_ARGS,
					": The map key must be definite");
		return MapKey(handle_bytestring(item));
	case CBOR_TYPE_STRING:
		if (!cbor_string_is_definite(item))
			throw Exception(__FILE__, __LINE__, error::INVALID_ARGS,
					": The map key must be definite");
		return MapKey(handle_string(item));
	case CBOR_TYPE_ARRAY:
		throw Exception(__FILE__, __LINE__, error::INVALID_ARGS,
				": A map key cannot be of array type");
	case CBOR_TYPE_MAP:
		throw Exception(__FILE__, __LINE__, error::INVALID_ARGS,
				": A map key cannot be of map type");
	case CBOR_TYPE_TAG:
		throw Exception(__FILE__, __LINE__, error::INVALID_ARGS,
				": A map key cannot be of tag type");
	case CBOR_TYPE_FLOAT_CTRL:
		throw Exception(__FILE__, __LINE__, error::INVALID_ARGS,
				": A map key cannot be of float or ctrl type");
	default:
		throw Exception(__FILE__, __LINE__, error::INTERNAL,
				": Unknown type for a map key");
	}
}

static Data
handle_value(const cbor_item_t *item) {
	switch (item->type) {
	case CBOR_TYPE_UINT:
		return Data(new UInt64(handle_uint(item)));
	case CBOR_TYPE_NEGINT:
		return Data(new Int64(handle_negint(item)));
	case CBOR_TYPE_BYTESTRING:
		if (!cbor_bytestring_is_definite(item))
			throw Exception(__FILE__, __LINE__, error::INVALID_ARGS,
					": A bytestring value must be "
					"definite");
		return Data(new ByteString(handle_bytestring(item)));
	case CBOR_TYPE_STRING:
		if (!cbor_string_is_definite(item))
			throw Exception(__FILE__, __LINE__, error::INVALID_ARGS,
					": A text value must be definite");
		return Data(new Text(handle_string(item)));
	case CBOR_TYPE_ARRAY:
		if (!cbor_array_is_definite(item))
			throw Exception(__FILE__, __LINE__, error::INVALID_ARGS,
					": An array value must be definite");
		return Data(new Array(handle_array(item)));
	case CBOR_TYPE_MAP:
		if (!cbor_map_is_definite(item))
			throw Exception(__FILE__, __LINE__, error::INVALID_ARGS,
					": A map value must be definite");
		return Data(new Map(handle_map(item)));
	case CBOR_TYPE_TAG:
		return handle_tag(item);
	case CBOR_TYPE_FLOAT_CTRL:
		throw Exception(__FILE__, __LINE__, error::INTERNAL,
				": Float values are not yet supported");
	default:
		throw Exception(__FILE__, __LINE__, error::INTERNAL,
				": Unknown value type");
	}
}

static utils::ustring
handle_bytestring(const cbor_item_t *item) {
	return utils::ustring(cbor_bytestring_handle(item),
			      cbor_bytestring_length(item));
}

static std::string
handle_string(const cbor_item_t *item) {
	return std::string(reinterpret_cast<const char *>(
				   cbor_string_handle(item)),
			   cbor_string_length(item));
}

static uint64_t
handle_uint(const cbor_item_t *item) {
	return cbor_get_int(item);
}

static int64_t
handle_negint(const cbor_item_t *item) {
	return -1 - static_cast<int64_t>(cbor_get_int(item));
}

static Data
handle_tag(const cbor_item_t *item) {
	cbor_item_t *t = cbor_tag_item(item);
	if (!t)
		throw Exception(__FILE__, __LINE__, error::INVALID_ARGS,
				": empty tagged data");
	return handle_value(t);
}

static bool
do_check(EVP_PKEY *key, const unsigned char *msg, std::size_t l,
	 const unsigned char *sig, std::size_t sl) {
	EVP_MD_CTX *c = EVP_MD_CTX_new();
	if (!c)
		throw std::bad_alloc();
	std::unique_ptr<EVP_MD_CTX, void (*)(EVP_MD_CTX *)> c_(
		c, &EVP_MD_CTX_free);
	const EVP_MD *md = EVP_get_digestbyname("SHA256");
	if (!md)
		throw Exception(__FILE__, __LINE__, error::INTERNAL,
				": No SHA256 digests");
 	if (!EVP_DigestVerifyInit(c, nullptr, md, nullptr, key))
		throw Exception(__FILE__, __LINE__, error::CRYPTO,
				": Cannot initialize verification: %s",
				ssl_error(ERR_get_error()).c_str());
	if (!EVP_DigestUpdate(c, msg, l))
		throw Exception(__FILE__, __LINE__, error::CRYPTO,
				": Verification error: %s",
				ssl_error(ERR_get_error()).c_str());

	std::size_t sz;
	unsigned char *p = make_sig(sig, sl, &sz);
	int r = EVP_DigestVerifyFinal(c, p, sz);
	delete [] p;
	switch (r) {
	case 1:
		return true;
	case 0:
		return false;
	default:
		throw Exception(__FILE__, __LINE__, error::CRYPTO,
				": Verification error: %s",
				ssl_error(ERR_get_error()).c_str());
	}
}

static unsigned char *
make_sig(const unsigned char *sig, std::size_t l, std::size_t *sz) {
	ECDSA_SIG *ec = ECDSA_SIG_new();
	if (!ec)
		throw std::bad_alloc();
	Auto<ECDSA_SIG> ec_(ec, &ECDSA_SIG_free);

	BIGNUM *r = BN_bin2bn(sig, 32, nullptr);
	if (!r)
		throw Exception(__FILE__, __LINE__, error::CRYPTO,
				": %s", ssl_error(ERR_get_error()).c_str());
	Auto<BIGNUM> r_(r, &BN_free);
	BIGNUM *s = BN_bin2bn(sig + 32, 32, nullptr);
	if (!s)
		throw Exception(__FILE__, __LINE__, error::CRYPTO,
				": %s", ssl_error(ERR_get_error()).c_str());
	Auto<BIGNUM> s_(s, &BN_free);
	if (!ECDSA_SIG_set0(ec, r, s))
		throw Exception(__FILE__, __LINE__, error::CRYPTO,
				": %s", ssl_error(ERR_get_error()).c_str());
	s_.release();
	r_.release();
	*sz = i2d_ECDSA_SIG(ec, nullptr);
	unsigned char *p = new unsigned char[*sz];
	unsigned char *pp = p;
	i2d_ECDSA_SIG(ec, &pp);
	return p;
}

static std::string
ssl_error(unsigned long code) {
	char buf[1024];
        ERR_error_string_n(code, buf, sizeof(buf));
	return std::string(buf);
}

void
App::shutdown() {
	CertMgr::destroy();
}

bool
App::update() {
	const CertMgr *cert_mgr = CertMgr::cert_mgr(config);
	if (!cert_mgr)
		throw Exception(__FILE__, __LINE__, error::INTERNAL,
				": Error building the certificate manager");
	return cert_mgr->update();
}

time_point
Claim::impl_type::expires() const {
	const Map *m = dynamic_cast<const Map *>(payload.get());
	if (!m)
		return time_point::min();
	Map::value_type::const_iterator i =
		m->get().find(MapKey(std::string("exp")));
	if (m->get().end() == i)
		return time_point::min();
	const Text *t = dynamic_cast<const Text *>(i->second.get());
	if (!m)
		return time_point::min();
	return utils::iso8601(t->get().data(), t->get().length());
}

std::ostream&
operator<<(std::ostream& os, const Claim& c) {
	c.impl->print(os);
	return os;
}

void
Claim::impl_type::print(std::ostream& os) const {
	os << "{\"protected\":" << prhdr.get() << ",\"unprotected\":"
	   << unprhdr.get() << ",\"claim\":" << payload << '}';
}

static std::ostream&
operator<<(std::ostream& os, const Map::value_type& c) {
	Map::value_type::const_iterator i = c.begin(), __li = c.end();
	os << '{';
	if (__li != i) {
		os << i->first << ":" << i->second;
		for (++i; __li != i; ++i)
			os << ',' << i->first << ":" << i->second;
	}
	os << '}';
	return os;
}

std::ostream&
operator<<(std::ostream& os, const MapKey& k) {
	switch (k.type) {
	case MapKey::Type::UINT_8:
		return os << k.u.uint8.get();
	case MapKey::Type::UINT_16:
		return os << k.u.uint16.get();
	case MapKey::Type::UINT_32:
		return os << k.u.uint32.get();
	case MapKey::Type::UINT_64:
		return os << k.u.uint64.get();
	case MapKey::Type::INT_8:
		return os << k.u.int8.get();
	case MapKey::Type::INT_16:
		return os << k.u.int16.get();
	case MapKey::Type::INT_32:
		return os << k.u.int32.get();
	case MapKey::Type::INT_64:
		return os << k.u.int64.get();
	case MapKey::Type::TEXT:
		return os << '"' << utils::QuoteJSON::quote(k.u.text.get())
			  << '"';
	case MapKey::Type::BYTESTRING:
		return os << k.u.bytestring.get();
	default:
		return os;
	}
}

std::ostream&
operator<<(std::ostream& os, const Array::value_type& c) {
	os << '[';
	Array::value_type::const_iterator i = c.begin(), __li = c.end();
	if (__li != i) {
		os << *i;
		for (++i; __li != i; ++i)
			os << ',' << *i;
	}
	os << ']';
	return os;
}

static std::ostream&
operator<<(std::ostream& os, const Data& k) {
	if (!k)
		return os << "null";
	return os << *k.get();
}

static std::ostream&
operator<<(std::ostream& os, const Base& k) {
	k.print(os);
	return os;
}

static std::ostream&
operator<<(std::ostream& os, const utils::ustring& s) {
	os << '"';
	utils::ustring::const_iterator i = s.begin(), __li = s.end();
	if (__li != i) {
		os << std::hex;
		os << std::setw(2) << std::setfill('0')
		   << static_cast<unsigned int>(*i);
		for (++i; __li != i; ++i)
			os << ':' << std::setw(2) << std::setfill('0')
			   << static_cast<unsigned int>(*i);
		os << std::dec;
	}
	os << '"';
	return os;
}

std::ostream&
operator<<(std::ostream& os, const KeyInfo& ki) {
	os << "{\"country\":\"" << utils::QuoteJSON::quote(ki.country())
	   // << "\",\"from\":\"" << ki.from()
	   // << "\",\"to\":\"" << ki.to()
	   << "\",\"notBefore\":\"" << ki.cert_from()
	   << "\",\"notAfter\":\"" << ki.cert_to()
	   << "\"}";
	return os;
}

std::ostream&
operator<<(std::ostream& os, const time_point& t) {
	time_t p = clock_type::to_time_t(t);
	struct tm tm;
	if (!gmtime_r(&p, &tm))
		throw Exception(__FILE__, __LINE__, error::INTERNAL);
	char buf[21];
	strftime(buf, sizeof(buf), "%FT%TZ", &tm);
	os << buf;
	return os;
}

}
