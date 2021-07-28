#pragma once

#include <type_traits>

namespace pv {
namespace utils {
namespace helpers {

template<typename Iterator>
struct is_random_access {
private:
	template<typename T,
		 typename = decltype(std::declval<T>() - std::declval<T>())>
	static std::true_type test(const T&);
	static std::false_type test(...);
	template<typename T>
	using do_test = decltype(test(std::declval<T>()));
public:
	typedef do_test<Iterator> type;
	static const bool value = type::value;
};

template<typename Iterator>
struct is_bidirectional {
private:
	template<typename T,
		 typename = decltype(--std::declval<T>())>
	static std::true_type test(const T&);
	static std::false_type test(...);
	template<typename T>
	using do_test = decltype(test(std::declval<T>()));
public:
	typedef do_test<Iterator> type;
	static const bool value = type::value;
};

}
}
}
