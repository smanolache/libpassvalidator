#pragma once

#include <memory>

namespace pv {
namespace utils {

template<typename T>
using Auto = std::unique_ptr<T, void (*)(T *)>;

}
}
