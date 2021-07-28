#pragma once

#include "visibility.h"
#include "pv/Config.hh"
#include <string>
#include "Certs.hh"
#include <cstddef>

namespace pv {

class DSO_LOCAL CertMgr;
class CertMgr {
private:
	struct DSO_LOCAL impl_type;
	impl_type *impl;

	CertMgr(const Config&);
	~CertMgr();
	CertMgr(const CertMgr&) = delete;
	CertMgr(CertMgr&&) = delete;
	CertMgr& operator=(const CertMgr&) = delete;
	CertMgr& operator=(CertMgr&&) = delete;

public:
	static CertMgr *cert_mgr(const Config&);
	static void destroy();

	KeyData key(const unsigned char *, std::size_t) const;
	bool update() const;
};

}
