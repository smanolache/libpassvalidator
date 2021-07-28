#pragma once

#include "visibility.h"
#include "pv/intf.hh"
#include <string>
#include <memory>
#include <openssl/evp.h>
#include <map>
#include <utility>

namespace pv {

typedef std::shared_ptr<EVP_PKEY> Key;

class DSO_LOCAL KeyData;
class KeyData {
private:
	KeyInfo info_;
	Key key_;

public:
	KeyData() noexcept = default;
	KeyData(KeyInfo&&, Key&&) noexcept;
	KeyData(const KeyData&) = default;
	KeyData(KeyData&&) noexcept = default;
	const KeyInfo& info() const noexcept;
	Key key() const noexcept;

private:
	void set_key(Key&&) noexcept;
	KeyInfo& info() noexcept;

friend class CertHandler;
};

typedef std::map<std::string, KeyData> Certs;

inline
KeyData::KeyData(KeyInfo&& info__, Key&& key__) noexcept
	: info_(std::move(info__))
	, key_(std::move(key__))
{
}

inline const KeyInfo&
KeyData::info() const noexcept {
	return info_;
}

inline KeyInfo&
KeyData::info() noexcept {
	return info_;
}

inline Key
KeyData::key() const noexcept {
	return key_;
}

inline void
KeyData::set_key(Key&& key__) noexcept {
	key_ = std::move(key__);
}

}
