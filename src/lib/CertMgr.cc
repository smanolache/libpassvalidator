#include "CertMgr.hh"
#include <mutex>
#include <condition_variable>
#include <string>
#include <openssl/evp.h>
#include <cstddef>
#include <thread>
#include <boost/asio/io_service.hpp>
#include <boost/asio/deadline_timer.hpp>
#include <boost/system/error_code.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <curl/curl.h>
#include "Certs.hh"
#include "CertParser.hh"
#include "base64.hh"
#include <chrono>
#include "pv/Exception.hh"
#include "pv/Errors.hh"

using boost::asio::io_service;
using boost::asio::deadline_timer;
namespace berror = boost::asio::error;
using boost::system::error_code;
using boost::posix_time::hours;
using boost::posix_time::seconds;

using clock_type = std::chrono::system_clock;
using time_point = clock_type::time_point;

namespace pv {

typedef std::mutex mutex_type;
typedef std::condition_variable condition_type;

static mutex_type mtx;
static CertMgr *mgr;

static std::string build_keyid(const unsigned char *, std::size_t);

CertMgr *
CertMgr::cert_mgr(const Config& config) {
	if (mgr)
		return mgr;
	std::unique_lock<mutex_type> lck(mtx);
	if (mgr)
		return mgr;
	mgr = new CertMgr(config);
	return mgr;
}

void
CertMgr::destroy() {
	if (!mgr)
		return;
	std::unique_lock<mutex_type> lck(mtx);
	if (mgr) {
		delete mgr;
		mgr = nullptr;
	}
}

struct CertMgr::impl_type {
	mutex_type mtx, async;
	condition_type cnd;

	std::atomic<Certs *> certs[2];
	time_point updated;
	static const std::size_t ERR_BUF = 8192;
	char err_buf[ERR_BUF];
	CURL *h;

	bool should_stop;
	io_service io;
	deadline_timer timer;
	std::thread t;

	impl_type(const Config&);
	~impl_type();

	void start();
	void stop();

	void main();
	void payload();
	void rearm(const error_code&);

	KeyData key(const unsigned char *keyid, std::size_t l);
	bool update();
	void do_update(bool&);
};

CertMgr::CertMgr(const Config& config)
	: impl(new impl_type(config))
{
	impl->start();
}

CertMgr::~CertMgr() {
	impl->stop();
	delete impl;
}

KeyData
CertMgr::key(const unsigned char *keyid, std::size_t l) const {
	return impl->key(keyid, l);
}

bool
CertMgr::update() const {
	return impl->update();
}

KeyData
CertMgr::impl_type::key(const unsigned char *keyid, std::size_t l) {
	const Certs *c = certs[0];
	const std::string& k = build_keyid(keyid, l);
	Certs::const_iterator i = c->find(k);
	return c->end() != i ? i->second : KeyData();
}

static std::string
build_keyid(const unsigned char *d, std::size_t l) {
	return utils::base64::enc(d, d + l);
}

void
CertMgr::impl_type::start() {
	t = std::thread(&CertMgr::impl_type::main, this);
}

CertMgr::impl_type::impl_type(const Config& config)
	: updated(time_point::min())
	, h(curl_easy_init())
	, should_stop(false)
	, timer(io, hours(4))
{
	err_buf[0] = '\0';
	certs[0] = new Certs();
	certs[1] = new Certs();
	updated = time_point::min();

	if (!h)
		throw Exception(__FILE__, __LINE__, error::OOM,
				": curl handle");
	if (CURLE_OK != curl_easy_setopt(h, CURLOPT_ERRORBUFFER, err_buf))
		throw Exception(__FILE__, __LINE__, error::CURL,
				": setting error buffer");
	if (CURLE_OK != curl_easy_setopt(h, CURLOPT_SSLKEY,
					 config.key().c_str()))
		throw Exception(__FILE__, __LINE__, error::CURL,
				": setting client key: %s", err_buf);
	if (CURLE_OK != curl_easy_setopt(h, CURLOPT_SSLCERT,
					 config.cert().c_str()))
		throw Exception(__FILE__, __LINE__, error::CURL,
				": setting client certificate: %s", err_buf);

	if (CURLE_OK != curl_easy_setopt(h, CURLOPT_URL, config.url().c_str()))
		throw Exception(__FILE__, __LINE__, error::CURL,
				": setting the URL: %s", err_buf);
	if (CURLE_OK != curl_easy_setopt(h, CURLOPT_TIMEOUT_MS, 5000l))
		throw Exception(__FILE__, __LINE__, error::CURL,
				": setting the timeout: %s", err_buf);
	if (CURLE_OK != curl_easy_setopt(h, CURLOPT_CONNECTTIMEOUT_MS, 5000l))
		throw Exception(__FILE__, __LINE__, error::CURL,
				": setting the connect timeout: %s", err_buf);
}

CertMgr::impl_type::~impl_type() {
	stop();
	curl_easy_cleanup(h);
	delete certs[0];
	delete certs[1];
}

void
CertMgr::impl_type::stop() {
	if (!t.joinable())
		return;
	should_stop = true;
	io.post([this]() {
		timer.cancel();
	});
	t.join();
}

struct DSO_LOCAL Ctx;
struct Ctx {
	Exception err;
	CertParser parser;

	Ctx(Certs&);

	static std::size_t on_data(char *, std::size_t, std::size_t,
				   Ctx *) noexcept;
	static std::size_t on_headers(char *, std::size_t, std::size_t,
				      Ctx *) noexcept;

	std::size_t do_on_data(const char *, std::size_t);
	std::size_t do_on_headers(const char *, std::size_t);

	const Exception& error() const noexcept;
	void error(const std::exception&);
	void error(const Exception&);

	void eos();
};

Ctx::Ctx(Certs& certs)
	: err(Exception(__FILE__, __LINE__, 0))
	, parser(certs)
{
}

const Exception&
Ctx::error() const noexcept {
	return err;
}

void
Ctx::error(const std::exception& e) {
	err = Exception(__FILE__, __LINE__, error::INTERNAL, ": %s", e.what());
}

void
Ctx::error(const Exception& e) {
	err = e;
}

void
CertMgr::impl_type::main() {
	try {
		std::unique_lock<mutex_type> lck(mtx);
		time_point now = clock_type::now();
		if (now >= updated + std::chrono::seconds(10)) {
			payload();
			updated = now;
		}
	} catch (const std::exception& e) {
		std::cerr << e.what() << std::endl;
		timer.expires_from_now(seconds(10));
	}
	timer.async_wait([this](const error_code& ec) {
		rearm(ec);
	});
	io.run();
}

void
CertMgr::impl_type::rearm(const error_code& ec) {
	if (berror::operation_aborted == ec)
		return;
	if (should_stop)
		return;
	try {
		{
			std::unique_lock<mutex_type> lck(mtx);
			time_point now = clock_type::now();
			if (now >= updated + std::chrono::seconds(10)) {
				payload();
				updated = now;
			}
		}
		timer.expires_from_now(hours(4));
	} catch (const std::exception& e) {
		std::cerr << e.what() << std::endl;
		timer.expires_from_now(seconds(10));
	}
	timer.async_wait([this](const error_code& ec) {
		rearm(ec);
	});
}

void
CertMgr::impl_type::payload() {
	Certs c;
	Ctx ctx(c);

	if (CURLE_OK != curl_easy_setopt(h, CURLOPT_WRITEFUNCTION,
					 &Ctx::on_data))
		throw Exception(__FILE__, __LINE__, error::CURL,
				": setting the write callback: %s", err_buf);
	if (CURLE_OK != curl_easy_setopt(h, CURLOPT_WRITEDATA, &ctx))
		throw Exception(__FILE__, __LINE__, error::CURL,
				": setting the write callback data: %s",
				err_buf);
	if (CURLE_OK != curl_easy_setopt(h, CURLOPT_HEADERFUNCTION,
					 &Ctx::on_headers))
		throw Exception(__FILE__, __LINE__, error::CURL,
				": setting the headers callback: %s", err_buf);
	if (CURLE_OK != curl_easy_setopt(h, CURLOPT_HEADERDATA, &ctx))
		throw Exception(__FILE__, __LINE__, error::CURL,
				": setting the headers callback data: %s",
				err_buf);
	if (CURLE_OK != curl_easy_perform(h))
		throw Exception(__FILE__, __LINE__, error::CURL,
				": performing the call: %s", err_buf);

	ctx.eos();
	*certs[1] = std::move(c);
	certs[1] = certs[0].exchange(certs[1]);
}

bool
CertMgr::impl_type::update() {
	bool r = false;
	std::unique_lock<mutex_type> lck(async);
	io.post([this, &r]() {
		try {
			do_update(r);
		} catch (...) {
			r = false;
		}
	});
	cnd.wait(lck);
	return r;
}

void
CertMgr::impl_type::do_update(bool& r) {
	std::unique_lock<mutex_type> lck(mtx, std::try_to_lock_t{});
	if (lck.owns_lock()) {
		time_point now = clock_type::now();
		if (now >= updated + std::chrono::seconds(10))
			try {
				payload();
				updated = now;
				r = true;
			} catch (const std::exception& e) {
				std::cerr << "Caught: " << e.what()
					  << std::endl;
				r = false;
			}
		else
			r = false;
		lck.unlock();
	} else {
		r = false;
	}
	std::unique_lock<mutex_type> lck2(async);
	cnd.notify_one();
}

std::size_t
Ctx::on_data(char *d, std::size_t n, std::size_t m, Ctx *ctx) noexcept {
	if (!ctx)
		return n * m;
	if (ctx->error().code())
		return n * m;
	try {
		return ctx->do_on_data(d, n * m);
	} catch (const Exception& e) {
		ctx->error(e);
		return n * m;
	} catch (const std::exception& e) {
		ctx->error(e);
		return n * m;
	}
}

std::size_t
Ctx::on_headers(char *d, std::size_t n, std::size_t m, Ctx *ctx) noexcept {
	if (!ctx)
		return n * m;
	if (ctx->error().code())
		return n * m;
	try {
		return ctx->do_on_headers(d, n * m);
	} catch (const Exception& e) {
		ctx->error(e);
		return n * m;
	} catch (const std::exception& e) {
		ctx->error(e);
		return n * m;
	}
}

std::size_t
Ctx::do_on_data(const char *d, std::size_t n) {
	parser.parse(d, n);
	return n;
}

std::size_t
Ctx::do_on_headers(const char *d, std::size_t n) {
	return n;
}

void
Ctx::eos() {
	const Exception& e = error();
	if (e.code())
		throw e;
	parser.eos();
}

}
