#include "base64.hh"
#include <string>
// #include "string_view.hpp"

namespace pv {
namespace utils {
// common instantiations
// is_base64
template bool
base64::is_base64<std::string>(std::string&&);
template bool
base64::is_base64<std::string&>(std::string&);
template bool
base64::is_base64<const std::string&>(const std::string&);

template bool
base64::is_base64<std::string::iterator,
		  std::string::iterator, 0>(
			  std::string::iterator&&,
			  const std::string::iterator&);
template bool
base64::is_base64<std::string::iterator&,
		  std::string::iterator, 0>(
			  std::string::iterator&,
			  const std::string::iterator&);
template bool
base64::is_base64<std::string::const_iterator,
		  std::string::const_iterator, 0>(
			  std::string::const_iterator&&,
			  const std::string::const_iterator&);
template bool
base64::is_base64<std::string::const_iterator&,
		  std::string::const_iterator, 0>(
			  std::string::const_iterator&,
			  const std::string::const_iterator&);

// template bool
// base64::is_base64<std::string_view>(std::string_view&&);
// template bool
// base64::is_base64<std::string_view&>(std::string_view&);
// template bool
// base64::is_base64<const std::string_view&>(const std::string_view&);

// template bool
// base64::is_base64<std::string_view::const_iterator,
// 		  std::string_view::const_iterator, 0>(
// 			  std::string_view::const_iterator&&,
// 			  const std::string_view::const_iterator&);
// template bool
// base64::is_base64<std::string_view::const_iterator&,
// 		  std::string_view::const_iterator, 0>(
// 			  std::string_view::const_iterator&,
// 			  const std::string_view::const_iterator&);

template bool
base64::is_base64<char const *, char const *, 0>(
	char const * &&, char const * const &);
template bool
base64::is_base64<char *, char *, 0>(char * &&, char * const &);
template bool
base64::is_base64<char const * &, char const *, 0>(
	char const * &, char const * const &);
template bool
base64::is_base64<char * &, char *, 0>(
	char * &, char * const &);

// dec
template std::string
base64::dec<std::string>(std::string&&);
template std::string
base64::dec<std::string&>(std::string&);
template std::string
base64::dec<const std::string&>(const std::string&);

template std::string
base64::dec<std::string::iterator,
	    std::string::iterator, 0>(
		    std::string::iterator&&,
		    const std::string::iterator&);
template std::string
base64::dec<std::string::iterator&,
	    std::string::iterator, 0>(
		    std::string::iterator&,
		    const std::string::iterator&);
template std::string
base64::dec<std::string::const_iterator,
	    std::string::const_iterator, 0>(
		    std::string::const_iterator&&,
		    const std::string::const_iterator&);
template std::string
base64::dec<std::string::const_iterator&,
	    std::string::const_iterator, 0>(
		    std::string::const_iterator&,
		    const std::string::const_iterator&);

// template std::string
// base64::dec<std::string_view>(std::string_view&&);
// template std::string
// base64::dec<std::string_view&>(std::string_view&);
// template std::string
// base64::dec<const std::string_view&>(const std::string_view&);

// template std::string
// base64::dec<std::string_view::const_iterator,
// 	    std::string_view::const_iterator, 0>(
// 		    std::string_view::const_iterator&&,
// 		    const std::string_view::const_iterator&);
// template std::string
// base64::dec<std::string_view::const_iterator&,
// 	    std::string_view::const_iterator, 0>(
// 		    std::string_view::const_iterator&,
// 		    const std::string_view::const_iterator&);

template std::string
base64::dec<char const *, char const *, 0>(
	char const * &&, char const * const &);
template std::string
base64::dec<char *, char *, 0>(char * &&, char * const &);
template std::string
base64::dec<char const * &, char const *, 0>(
	char const * &, char const * const &);
template std::string
base64::dec<char * &, char *, 0>(char * &, char * const &);

// enc
template std::string
base64::enc<std::string>(std::string&&);
template std::string
base64::enc<std::string&>(std::string&);
template std::string
base64::enc<const std::string&>(const std::string&);

template std::string
base64::enc<std::string::iterator,
	    std::string::iterator, 0>(std::string::iterator&&,
				      const std::string::iterator&);
template std::string
base64::enc<std::string::iterator&,
	    std::string::iterator, 0>(std::string::iterator&,
				      const std::string::iterator&);
template std::string
base64::enc<std::string::const_iterator,
	    std::string::const_iterator, 0>(
		    std::string::const_iterator&&,
		    const std::string::const_iterator&);
template std::string
base64::enc<std::string::const_iterator&,
	    std::string::const_iterator, 0>(
		    std::string::const_iterator&,
		    const std::string::const_iterator&);

// template std::string
// base64::enc<std::string_view>(std::string_view&&);
// template std::string
// base64::enc<std::string_view&>(std::string_view&);
// template std::string
// base64::enc<const std::string_view&>(const std::string_view&);

// template std::string
// base64::enc<std::string_view::const_iterator,
// 	    std::string_view::const_iterator, 0>(
// 		    std::string_view::const_iterator&&,
// 		    const std::string_view::const_iterator&);
// template std::string
// base64::enc<std::string_view::const_iterator&,
// 	    std::string_view::const_iterator, 0>(
// 		    std::string_view::const_iterator&,
// 		    const std::string_view::const_iterator&);

template std::string
base64::enc<char const *, char const *, 0>(
	char const * &&, char const * const &);
template std::string
base64::enc<char *, char *, 0>(char * &&, char * const &);
template std::string
base64::enc<char const * &, char const *, 0>(
	char const * &, char const * const &);
template std::string
base64::enc<char * &, char *, 0>(char * &, char * const &);

// common instantiations
// is_base64
template bool
ubase64::is_base64<std::string>(std::string&&);
template bool
ubase64::is_base64<std::string&>(std::string&);
template bool
ubase64::is_base64<const std::string&>(const std::string&);

template bool
ubase64::is_base64<std::string::iterator,
		   std::string::iterator, 0>(
			   std::string::iterator&&,
			   const std::string::iterator&);
template bool
ubase64::is_base64<std::string::iterator&,
		   std::string::iterator, 0>(
			   std::string::iterator&,
			   const std::string::iterator&);
template bool
ubase64::is_base64<std::string::const_iterator,
		   std::string::const_iterator, 0>(
			   std::string::const_iterator&&,
			   const std::string::const_iterator&);
template bool
ubase64::is_base64<std::string::const_iterator&,
		   std::string::const_iterator, 0>(
			   std::string::const_iterator&,
			   const std::string::const_iterator&);

// template bool
// ubase64::is_base64<std::string_view>(std::string_view&&);
// template bool
// ubase64::is_base64<std::string_view&>(std::string_view&);
// template bool
// ubase64::is_base64<const std::string_view&>(const std::string_view&);

// template bool
// ubase64::is_base64<std::string_view::const_iterator,
// 		   std::string_view::const_iterator, 0>(
// 			   std::string_view::const_iterator&&,
// 			   const std::string_view::const_iterator&);
// template bool
// ubase64::is_base64<std::string_view::const_iterator&,
// 		   std::string_view::const_iterator, 0>(
// 			   std::string_view::const_iterator&,
// 			   const std::string_view::const_iterator&);

template bool
ubase64::is_base64<char const *, char const *, 0>(
	char const * &&, char const * const &);
template bool
ubase64::is_base64<char *, char *, 0>(char * &&, char * const &);
template bool
ubase64::is_base64<char const * &, char const *, 0>(
	char const * &, char const * const &);
template bool
ubase64::is_base64<char * &, char *, 0>(
	char * &, char * const &);

// dec
template std::string
ubase64::dec<std::string>(std::string&&);
template std::string
ubase64::dec<std::string&>(std::string&);
template std::string
ubase64::dec<const std::string&>(const std::string&);

template std::string
ubase64::dec<std::string::iterator,
	     std::string::iterator, 0>(
		     std::string::iterator&&,
		     const std::string::iterator&);
template std::string
ubase64::dec<std::string::iterator&,
	     std::string::iterator, 0>(
		     std::string::iterator&,
		     const std::string::iterator&);
template std::string
ubase64::dec<std::string::const_iterator,
	     std::string::const_iterator, 0>(
		     std::string::const_iterator&&,
		     const std::string::const_iterator&);
template std::string
ubase64::dec<std::string::const_iterator&,
	     std::string::const_iterator, 0>(
		     std::string::const_iterator&,
		     const std::string::const_iterator&);

// template std::string
// ubase64::dec<std::string_view>(std::string_view&&);
// template std::string
// ubase64::dec<std::string_view&>(std::string_view&);
// template std::string
// ubase64::dec<const std::string_view&>(const std::string_view&);

// template std::string
// ubase64::dec<std::string_view::const_iterator,
// 	     std::string_view::const_iterator, 0>(
// 		     std::string_view::const_iterator&&,
// 		     const std::string_view::const_iterator&);
// template std::string
// ubase64::dec<std::string_view::const_iterator&,
// 	     std::string_view::const_iterator, 0>(
// 		     std::string_view::const_iterator&,
// 		     const std::string_view::const_iterator&);

template std::string
ubase64::dec<char const *, char const *, 0>(
	char const * &&, char const * const &);
template std::string
ubase64::dec<char *, char *, 0>(char * &&, char * const &);
template std::string
ubase64::dec<char const * &, char const *, 0>(
	char const * &, char const * const &);
template std::string
ubase64::dec<char * &, char *, 0>(char * &, char * const &);

// enc
template std::string
ubase64::enc<std::string>(std::string&&);
template std::string
ubase64::enc<std::string&>(std::string&);
template std::string
ubase64::enc<const std::string&>(const std::string&);

template std::string
ubase64::enc<std::string::iterator,
	     std::string::iterator, 0>(std::string::iterator&&,
				       const std::string::iterator&);
template std::string
ubase64::enc<std::string::iterator&,
	     std::string::iterator, 0>(std::string::iterator&,
				       const std::string::iterator&);
template std::string
ubase64::enc<std::string::const_iterator,
	     std::string::const_iterator, 0>(
		     std::string::const_iterator&&,
		     const std::string::const_iterator&);
template std::string
ubase64::enc<std::string::const_iterator&,
	     std::string::const_iterator, 0>(
		     std::string::const_iterator&,
		     const std::string::const_iterator&);

// template std::string
// ubase64::enc<std::string_view>(std::string_view&&);
// template std::string
// ubase64::enc<std::string_view&>(std::string_view&);
// template std::string
// ubase64::enc<const std::string_view&>(const std::string_view&);

// template std::string
// ubase64::enc<std::string_view::const_iterator,
// 	     std::string_view::const_iterator, 0>(
// 		     std::string_view::const_iterator&&,
// 		     const std::string_view::const_iterator&);
// template std::string
// ubase64::enc<std::string_view::const_iterator&,
// 	     std::string_view::const_iterator, 0>(
// 		     std::string_view::const_iterator&,
// 		     const std::string_view::const_iterator&);

template std::string
ubase64::enc<char const *, char const *, 0>(
	char const * &&, char const * const &);
template std::string
ubase64::enc<char *, char *, 0>(char * &&, char * const &);
template std::string
ubase64::enc<char const * &, char const *, 0>(
	char const * &, char const * const &);
template std::string
ubase64::enc<char * &, char *, 0>(char * &, char * const &);

}
}
