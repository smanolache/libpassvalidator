#pragma once

#include "Base.hh"
#include <memory>
#include <cstdint>
#include <string>
#include "ustring.hh"
#include <map>
#include <vector>

namespace pv {

using Data = std::unique_ptr<Base>;

template<typename T> class DataTmpl;
class MapKey;

using UInt8 = DataTmpl<uint8_t>;
using UInt16 = DataTmpl<uint16_t>;
using UInt32 = DataTmpl<uint32_t>;
using UInt64 = DataTmpl<uint64_t>;
using Int8 = DataTmpl<uint8_t>;
using Int16 = DataTmpl<int16_t>;
using Int32 = DataTmpl<int32_t>;
using Int64 = DataTmpl<int64_t>;
using Text = DataTmpl<std::string>;
using ByteString = DataTmpl<utils::ustring>;
using Map = DataTmpl<std::map<MapKey, Data>>;
using Array = DataTmpl<std::vector<Data>>;

}
