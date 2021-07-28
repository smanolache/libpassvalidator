#pragma once

#include <openssl/evp.h>
#include <cstddef>
#include "visibility.h"

namespace pv {

extern EVP_PKEY *key(const unsigned char *, std::size_t) DSO_LOCAL;

}
