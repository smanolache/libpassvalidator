#include "Zlib.hh"
#include <string>
// #include "string_view.hh"

namespace pv {
namespace utils {

template bool
Zlib::is_zlib<std::string>(std::string&&) noexcept;
template bool
Zlib::is_zlib<std::string&>(std::string&) noexcept;
template bool
Zlib::is_zlib<const std::string&>(const std::string&) noexcept;

template bool
Zlib::is_zlib<std::string::iterator,
	      std::string::iterator, 0>(
		      std::string::iterator&&,
		      const std::string::iterator&) noexcept;
template bool
Zlib::is_zlib<std::string::iterator&,
	      std::string::iterator, 0>(
		      std::string::iterator&,
		      const std::string::iterator&) noexcept;
template bool
Zlib::is_zlib<std::string::const_iterator,
	      std::string::const_iterator, 0>(
		      std::string::const_iterator&&,
		      const std::string::const_iterator&) noexcept;
template bool
Zlib::is_zlib<std::string::const_iterator&,
	      std::string::const_iterator, 0>(
		      std::string::const_iterator&,
		      const std::string::const_iterator&) noexcept;

// template bool
// Zlib::is_zlib<std::string_view>(std::string_view&&) noexcept;
// template bool
// Zlib::is_zlib<std::string_view&>(std::string_view&) noexcept;
// template bool
// Zlib::is_zlib<const std::string_view&>(const std::string_view&) noexcept;

// template bool
// Zlib::is_zlib<std::string_view::const_iterator,
// 	      std::string_view::const_iterator, 0>(
// 		      std::string_view::const_iterator&&,
// 		      const std::string_view::const_iterator&) noexcept;
// template bool
// Zlib::is_zlib<std::string_view::const_iterator&,
// 	      std::string_view::const_iterator, 0>(
// 		      std::string_view::const_iterator&,
// 		      const std::string_view::const_iterator&) noexcept;

template bool
Zlib::is_zlib<char const *, char const *, 0>(
	char const * &&, char const * const &) noexcept;
template bool
Zlib::is_zlib<char *, char *, 0>(char * &&, char * const &) noexcept;
template bool
Zlib::is_zlib<char const * &, char const *, 0>(
	char const * &, char const * const &) noexcept;
template bool
Zlib::is_zlib<char * &, char *, 0>(char * &, char * const &) noexcept;

// decompress
template std::string
Zlib::decompress<std::string>(std::string&&);
template std::string
Zlib::decompress<std::string&>(std::string&);
template std::string
Zlib::decompress<const std::string&>(const std::string&);

template std::string
Zlib::decompress<std::string::iterator,
		 std::string::iterator, 0>(
			 std::string::iterator&&,
			 const std::string::iterator&);
template std::string
Zlib::decompress<std::string::iterator&,
		 std::string::iterator, 0>(
			 std::string::iterator&,
			 const std::string::iterator&);
template std::string
Zlib::decompress<std::string::const_iterator,
		 std::string::const_iterator, 0>(
			 std::string::const_iterator&&,
			 const std::string::const_iterator&);
template std::string
Zlib::decompress<std::string::const_iterator&,
		 std::string::const_iterator, 0>(
			 std::string::const_iterator&,
			 const std::string::const_iterator&);

// template std::string
// Zlib::decompress<std::string_view>(std::string_view&&);
// template std::string
// Zlib::decompress<std::string_view&>(std::string_view&);
// template std::string
// Zlib::decompress<const std::string_view&>(const std::string_view&);

// template std::string
// Zlib::decompress<std::string_view::const_iterator,
// 		 std::string_view::const_iterator, 0>(
// 			 std::string_view::const_iterator&&,
// 			 const std::string_view::const_iterator&);
// template std::string
// Zlib::decompress<std::string_view::const_iterator&,
// 		 std::string_view::const_iterator, 0>(
// 			 std::string_view::const_iterator&,
// 			 const std::string_view::const_iterator&);

template std::string
Zlib::decompress<char const *, char const *, 0>(
	char const * &&, char const * const &);
template std::string
Zlib::decompress<char *, char *, 0>(char * &&, char * const &);
template std::string
Zlib::decompress<char const * &, char const *, 0>(
	char const * &, char const * const &);
template std::string
Zlib::decompress<char * &, char *, 0>(char * &, char * const &);

// compress
template std::string
Zlib::compress<std::string>(std::string&&);
template std::string
Zlib::compress<std::string&>(std::string&);
template std::string
Zlib::compress<const std::string&>(const std::string&);

template std::string
Zlib::compress<std::string::iterator,
	       std::string::iterator, 0>(std::string::iterator&&,
					 const std::string::iterator&);
template std::string
Zlib::compress<std::string::iterator&,
	       std::string::iterator, 0>(std::string::iterator&,
					 const std::string::iterator&);
template std::string
Zlib::compress<std::string::const_iterator,
	       std::string::const_iterator, 0>(
		       std::string::const_iterator&&,
		       const std::string::const_iterator&);
template std::string
Zlib::compress<std::string::const_iterator&,
	       std::string::const_iterator, 0>(
		       std::string::const_iterator&,
		       const std::string::const_iterator&);

// template std::string
// Zlib::compress<std::string_view>(std::string_view&&);
// template std::string
// Zlib::compress<std::string_view&>(std::string_view&);
// template std::string
// Zlib::compress<const std::string_view&>(const std::string_view&);

// template std::string
// Zlib::compress<std::string_view::const_iterator,
// 	       std::string_view::const_iterator, 0>(
// 		       std::string_view::const_iterator&&,
// 		       const std::string_view::const_iterator&);
// template std::string
// Zlib::compress<std::string_view::const_iterator&,
// 	       std::string_view::const_iterator, 0>(
// 		       std::string_view::const_iterator&,
// 		       const std::string_view::const_iterator&);

template std::string
Zlib::compress<char const *, char const *, 0>(
	char const * &&, char const * const &);
template std::string
Zlib::compress<char *, char *, 0>(char * &&, char * const &);
template std::string
Zlib::compress<char const * &, char const *, 0>(
	char const * &, char const * const &);
template std::string
Zlib::compress<char * &, char *, 0>(char * &, char * const &);

}
}
