#pragma once

namespace pv {

struct error {
	static constexpr const unsigned int INTERNAL = 500;
	static constexpr const unsigned int OOM = 501;
	static constexpr const unsigned int ZLIB = 502;
	static constexpr const unsigned int CRYPTO = 503;
	static constexpr const unsigned int CURL = 504;
	static constexpr const unsigned int INVALID_ARGS = 400;
};

}
