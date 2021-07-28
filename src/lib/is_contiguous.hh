#pragma once

#include <type_traits>
#include <vector>
#include <string>
// #include "string_view.hh"

namespace pv {
namespace utils {
namespace helpers {

template<typename Iterator, typename Char = char>
struct is_contiguous : public std::false_type {};

template<typename Char>
struct is_contiguous<typename std::vector<Char>::const_iterator, Char> :
		public std::true_type {};

template<typename Char>
struct is_contiguous<typename std::vector<Char>::iterator, Char> :
		public std::true_type {};

template<typename Char>
struct is_contiguous<typename std::basic_string<Char>::const_iterator, Char> :
		public std::true_type {};

template<typename Char>
struct is_contiguous<typename std::basic_string<Char>::iterator, Char> :
		public std::true_type {};

template<typename Char>
struct is_contiguous<Char *, Char> : public std::true_type {};

template<typename Char>
struct is_contiguous<const Char *, Char> : public std::true_type {};

// template<typename Char>
// struct is_contiguous<typename std::basic_string_view<Char>::const_iterator,
// 		     Char> :
// 		public std::true_type {};

}
}
}
