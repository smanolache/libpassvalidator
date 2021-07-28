#pragma once

#include "visibility.h"
#include "Certs.hh"
#include "yajl_defs.h"
#include <chrono>
#include <memory>

namespace pv {

/*
  idle { response
    "certificates": exp_certs [ certs
      { cert
         kid:
         data:
         lastUpdate:
         validFrom:
         validTo:
         countryCode:
         certificateType:
      } certs
    ] response
    numberCertificate:
  } done
 */

class DSO_LOCAL CertHandler;
class CertHandler {
private:
	Certs& certs;

	struct DSO_LOCAL State;
	struct State {
		typedef enum {
			IDLE,
			RESPONSE,
			EXP_CERTS,
			CERTS,
			CERT,
			KID,
			DATA,
			UPD,
			FROM,
			TO,
			CC,
			TYPE,
			NUMBER,
			DONE,
			UNKNOWN_KEY,
		} type;
	};

	State::type state;
	State::type last_known_state;
	unsigned int level;

	struct DSO_LOCAL Cert;
	struct Cert {
		std::string kid;
		std::string data;
		KeyInfo info;
	};
	std::unique_ptr<Cert> crt;

public:
	CertHandler(Certs&) noexcept;

	void on_start_array();
	void on_end_array();
	void on_start_map();
	void on_end_map();
	void on_map_key(const char *, YAJL_UINT);

	void on_string(const char *, YAJL_UINT);
	void on_int(YAJL_LONG);
	void on_null();
	void on_bool(bool);
	void on_double(double);

	void on_response(const char *, YAJL_UINT) noexcept;
	void on_cert(const char *, YAJL_UINT) noexcept;

	void add_cert();
};

}
