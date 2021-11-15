#pragma once

#include <ostream>
#include <tuple>
#include "Config.hh"
#include <chrono>

namespace pv {

class Claim;
extern std::ostream& operator<<(std::ostream&, const Claim&);

class App;
class Claim {
private:
	struct impl_type;
	impl_type *impl;

public:
	Claim(const Claim&) = delete;
	Claim(Claim&&) noexcept;
	Claim& operator=(const Claim&) = delete;
	Claim& operator=(Claim&&) = delete;
	~Claim();

	std::chrono::system_clock::time_point expires() const;

private:
	Claim(impl_type *) noexcept;

friend class App;
friend std::ostream& operator<<(std::ostream&, const Claim&);
};

struct Status {
	typedef enum {
		Ok,
		UnknownKey,
		InvalidSignature,
		ExpiredClaim,
	} type;
};

class KeyInfo {
private:
	std::chrono::system_clock::time_point from_;
	std::chrono::system_clock::time_point to_;
	std::chrono::system_clock::time_point cert_from_;
	std::chrono::system_clock::time_point cert_to_;
	std::string country_;

public:
	KeyInfo() noexcept;

	std::chrono::system_clock::time_point from() const noexcept;
	std::chrono::system_clock::time_point to() const noexcept;
	std::chrono::system_clock::time_point cert_from() const noexcept;
	std::chrono::system_clock::time_point cert_to() const noexcept;
	const std::string& country() const noexcept;

friend class CertHandler;
};

extern std::ostream& operator<<(std::ostream&, const KeyInfo&);

inline
KeyInfo::KeyInfo() noexcept
	: from_(std::chrono::system_clock::time_point::max())
	, to_(std::chrono::system_clock::time_point::min())
	, cert_from_(std::chrono::system_clock::time_point::max())
	, cert_to_(std::chrono::system_clock::time_point::min())
{
}

inline std::chrono::system_clock::time_point
KeyInfo::from() const noexcept {
	return from_;
}

inline std::chrono::system_clock::time_point
KeyInfo::to() const noexcept {
	return to_;
}

inline std::chrono::system_clock::time_point
KeyInfo::cert_from() const noexcept {
	return from_;
}

inline std::chrono::system_clock::time_point
KeyInfo::cert_to() const noexcept {
	return to_;
}

inline const std::string&
KeyInfo::country() const noexcept {
	return country_;
}

typedef std::tuple<Claim, KeyInfo, Status::type, std::string> Result;

class App {
private:
	const Config& config;

public:
	App(const Config&) noexcept;

	Result check(const std::string&);
	bool update();

	static void shutdown();
};

inline
App::App(const Config& config__) noexcept
	: config(config__)
{
}

}
