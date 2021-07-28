#pragma once

#include <type_traits>
#include <ostream>
#include "visibility.h"
#include <string>
#include <utility>
#include "QuoteJSON.hh"
#include "ustring.hh"

namespace pv {
namespace detail {

template<typename T, typename U>
using is_same = typename std::is_same<T, std::decay_t<U> >::type;

template<typename T, typename U>
using is_diff = typename std::__not_<typename std::is_same<T, std::decay_t<U>>::type>::type;

template<typename T, typename U>
using is_ct = typename std::is_constructible<T, U>::type;

template<typename T, typename U>
using is_ass = typename std::is_assignable<T, U>::type;

template<bool dftl, typename Tag = void>
struct dflt_ctor {
	constexpr dflt_ctor() noexcept = default;
	constexpr dflt_ctor(const dflt_ctor&) noexcept = default;
	constexpr dflt_ctor(dflt_ctor&&) noexcept = default;
	dflt_ctor& operator=(const dflt_ctor&) noexcept = default;
	dflt_ctor& operator=(dflt_ctor&&) noexcept = default;
};

template<typename Tag>
struct dflt_ctor<false, Tag> {
	constexpr dflt_ctor() noexcept = delete;
	constexpr dflt_ctor(const dflt_ctor&) noexcept = default;
	constexpr dflt_ctor(dflt_ctor&&) noexcept = default;
	dflt_ctor& operator=(const dflt_ctor&) noexcept = default;
	dflt_ctor& operator=(dflt_ctor&&) noexcept = default;
};

template<bool dftl, typename Tag = void>
struct dflt_dtor {
};

template<typename Tag>
struct dflt_dtor<false, Tag> {
	~dflt_dtor() noexcept = delete;
};

template<bool cp, bool mv, bool cpass, bool mvass, typename Tag = void>
struct dflt_copy_move {
};

template<typename Tag>
struct dflt_copy_move<true, true, true, false, Tag> {
	constexpr dflt_copy_move() noexcept = default;
	constexpr dflt_copy_move(const dflt_copy_move&) noexcept = default;
	constexpr dflt_copy_move(dflt_copy_move&&) noexcept = default;
	dflt_copy_move& operator=(const dflt_copy_move&) noexcept = default;
	dflt_copy_move& operator=(dflt_copy_move&&) noexcept = delete;
};

template<typename Tag>
struct dflt_copy_move<true, true, false, true, Tag> {
	constexpr dflt_copy_move() noexcept = default;
	constexpr dflt_copy_move(const dflt_copy_move&) noexcept = default;
	constexpr dflt_copy_move(dflt_copy_move&&) noexcept = default;
	dflt_copy_move& operator=(const dflt_copy_move&) noexcept = delete;
	dflt_copy_move& operator=(dflt_copy_move&&) noexcept = default;
};

template<typename Tag>
struct dflt_copy_move<true, true, false, false, Tag> {
	constexpr dflt_copy_move() noexcept = default;
	constexpr dflt_copy_move(const dflt_copy_move&) noexcept = default;
	constexpr dflt_copy_move(dflt_copy_move&&) noexcept = default;
	dflt_copy_move& operator=(const dflt_copy_move&) noexcept = delete;
	dflt_copy_move& operator=(dflt_copy_move&&) noexcept = delete;
};

template<typename Tag>
struct dflt_copy_move<true, false, true, true, Tag> {
	constexpr dflt_copy_move() noexcept = default;
	constexpr dflt_copy_move(const dflt_copy_move&) noexcept = default;
	constexpr dflt_copy_move(dflt_copy_move&&) noexcept = delete;
	dflt_copy_move& operator=(const dflt_copy_move&) noexcept = default;
	dflt_copy_move& operator=(dflt_copy_move&&) noexcept = default;
};

template<typename Tag>
struct dflt_copy_move<true, false, true, false, Tag> {
	constexpr dflt_copy_move() noexcept = default;
	constexpr dflt_copy_move(const dflt_copy_move&) noexcept = default;
	constexpr dflt_copy_move(dflt_copy_move&&) noexcept = delete;
	dflt_copy_move& operator=(const dflt_copy_move&) noexcept = default;
	dflt_copy_move& operator=(dflt_copy_move&&) noexcept = delete;
};

template<typename Tag>
struct dflt_copy_move<true, false, false, true, Tag> {
	constexpr dflt_copy_move() noexcept = default;
	constexpr dflt_copy_move(const dflt_copy_move&) noexcept = default;
	constexpr dflt_copy_move(dflt_copy_move&&) noexcept = delete;
	dflt_copy_move& operator=(const dflt_copy_move&) noexcept = delete;
	dflt_copy_move& operator=(dflt_copy_move&&) noexcept = default;
};

template<typename Tag>
struct dflt_copy_move<true, false, false, false, Tag> {
	constexpr dflt_copy_move() noexcept = default;
	constexpr dflt_copy_move(const dflt_copy_move&) noexcept = default;
	constexpr dflt_copy_move(dflt_copy_move&&) noexcept = delete;
	dflt_copy_move& operator=(const dflt_copy_move&) noexcept = delete;
	dflt_copy_move& operator=(dflt_copy_move&&) noexcept = delete;
};

template<typename Tag>
struct dflt_copy_move<false, true, true, true, Tag> {
	constexpr dflt_copy_move() noexcept = default;
	constexpr dflt_copy_move(const dflt_copy_move&) noexcept = delete;
	constexpr dflt_copy_move(dflt_copy_move&&) noexcept = default;
	dflt_copy_move& operator=(const dflt_copy_move&) noexcept = default;
	dflt_copy_move& operator=(dflt_copy_move&&) noexcept = default;
};

template<typename Tag>
struct dflt_copy_move<false, true, true, false, Tag> {
	constexpr dflt_copy_move() noexcept = default;
	constexpr dflt_copy_move(const dflt_copy_move&) noexcept = delete;
	constexpr dflt_copy_move(dflt_copy_move&&) noexcept = default;
	dflt_copy_move& operator=(const dflt_copy_move&) noexcept = default;
	dflt_copy_move& operator=(dflt_copy_move&&) noexcept = delete;
};

template<typename Tag>
struct dflt_copy_move<false, true, false, true, Tag> {
	constexpr dflt_copy_move() noexcept = default;
	constexpr dflt_copy_move(const dflt_copy_move&) noexcept = delete;
	constexpr dflt_copy_move(dflt_copy_move&&) noexcept = default;
	dflt_copy_move& operator=(const dflt_copy_move&) noexcept = delete;
	dflt_copy_move& operator=(dflt_copy_move&&) noexcept = default;
};

template<typename Tag>
struct dflt_copy_move<false, true, false, false, Tag> {
	constexpr dflt_copy_move() noexcept = default;
	constexpr dflt_copy_move(const dflt_copy_move&) noexcept = delete;
	constexpr dflt_copy_move(dflt_copy_move&&) noexcept = default;
	dflt_copy_move& operator=(const dflt_copy_move&) noexcept = delete;
	dflt_copy_move& operator=(dflt_copy_move&&) noexcept = delete;
};

template<typename Tag>
struct dflt_copy_move<false, false, true, true, Tag> {
	constexpr dflt_copy_move() noexcept = default;
	constexpr dflt_copy_move(const dflt_copy_move&) noexcept = delete;
	constexpr dflt_copy_move(dflt_copy_move&&) noexcept = delete;
	dflt_copy_move& operator=(const dflt_copy_move&) noexcept = default;
	dflt_copy_move& operator=(dflt_copy_move&&) noexcept = default;
};

template<typename Tag>
struct dflt_copy_move<false, false, true, false, Tag> {
	constexpr dflt_copy_move() noexcept = default;
	constexpr dflt_copy_move(const dflt_copy_move&) noexcept = delete;
	constexpr dflt_copy_move(dflt_copy_move&&) noexcept = delete;
	dflt_copy_move& operator=(const dflt_copy_move&) noexcept = default;
	dflt_copy_move& operator=(dflt_copy_move&&) noexcept = delete;
};

template<typename Tag>
struct dflt_copy_move<false, false, false, true, Tag> {
	constexpr dflt_copy_move() noexcept = default;
	constexpr dflt_copy_move(const dflt_copy_move&) noexcept = delete;
	constexpr dflt_copy_move(dflt_copy_move&&) noexcept = delete;
	dflt_copy_move& operator=(const dflt_copy_move&) noexcept = delete;
	dflt_copy_move& operator=(dflt_copy_move&&) noexcept = default;
};

template<typename Tag>
struct dflt_copy_move<false, false, false, false, Tag> {
	constexpr dflt_copy_move() noexcept = default;
	constexpr dflt_copy_move(const dflt_copy_move&) noexcept = delete;
	constexpr dflt_copy_move(dflt_copy_move&&) noexcept = delete;
	dflt_copy_move& operator=(const dflt_copy_move&) noexcept = delete;
	dflt_copy_move& operator=(dflt_copy_move&&) noexcept = delete;
};

template<typename T, bool dflt, bool dtor, bool cp, bool mv, bool cpass, bool mvass, typename Tag = void>
struct activate : private dflt_ctor<dflt, Tag>, private dflt_dtor<dtor, Tag>,
		  private dflt_copy_move<cp, mv, cpass, mvass, Tag> {
};

}

class MapKey;
std::ostream& operator<<(std::ostream&, const MapKey&) DSO_LOCAL;

template<typename T>
class DataTmpl;
template<typename T, typename = void>
struct DSO_LOCAL Printer;
template<typename T, typename>
struct Printer {
	static void print(std::ostream& os, const DataTmpl<T>& d);
};

template<typename D>
struct Printer<std::string, D> {
	static void print(std::ostream& os, const DataTmpl<std::string>&);
};

template<typename T>
class DataTmpl : public detail::activate<T, std::is_default_constructible<T>::value,
					 std::is_destructible<T>::value,
					 std::is_copy_constructible<T>::value,
					 std::is_move_constructible<T>::value,
					 std::is_copy_assignable<T>::value,
					 std::is_move_assignable<T>::value>,
		 public Base {
protected:
	T data_;

public:
	typedef T value_type;

	template<typename U = T,
		 std::enable_if_t<
			 std::__and_<
				 detail::is_diff<DataTmpl<T>, U>,
				 detail::is_ct<T, U&&>
			 >::value, int> = 0>
	explicit DataTmpl(U&& u)
		: data_(std::forward<U>(u))
	{
	}

	template<typename U = T,
		 std::enable_if_t<
			 std::__and_<
				 detail::is_diff<DataTmpl<T>, U>,
				 detail::is_ass<T&, U>
			 >::value, int> = 0>
	DataTmpl& operator=(U&& u) {
		data_ = std::forward<U>(u);
		return *this;
	}

	operator const T&() const noexcept { return data_; }
	operator T&() noexcept { return data_; }

	const T& get() const noexcept { return data_; }
	T& get() noexcept { return data_; }

	virtual void print(std::ostream& os) const override { Printer<T>::print(os, *this); }
};

template<typename T, typename D>
void
Printer<T, D>::print(std::ostream& os, const DataTmpl<T>& d) {
	os << d.get();
}

template<typename D>
void
Printer<std::string, D>::print(std::ostream& os, const DataTmpl<std::string>& s) {
	os << '"' << utils::QuoteJSON::quote(s.get()) << '"';
}

template<typename S, typename = void>
class SimpleTmpl: public DataTmpl<S> {
public:
	SimpleTmpl() = delete;
	SimpleTmpl(const SimpleTmpl&) = delete;
	SimpleTmpl(SimpleTmpl&&) = delete;

	bool operator<(const SimpleTmpl& other) const noexcept;
	bool operator>(const SimpleTmpl& other) const noexcept;
	bool operator<=(const SimpleTmpl& other) const noexcept;
	bool operator>=(const SimpleTmpl& other) const noexcept;
	bool operator==(const SimpleTmpl& other) const noexcept;
	bool operator!=(const SimpleTmpl& other) const noexcept;
};

template<typename S>
class SimpleTmpl<S, std::enable_if_t<
			    std::__or_<
				    typename std::is_integral<S>::type,
				    typename std::is_floating_point<S>::type,
				    typename std::is_same<S, std::string>::type,
				    typename std::is_same<S, utils::ustring>::type
			    >::value> > : public DataTmpl<S> {
public:
	typedef typename DataTmpl<S>::value_type value_type;

	template<typename U = S,
		 std::enable_if_t<
			 std::__and_<
				 detail::is_diff<SimpleTmpl<S>, U>,
				 detail::is_ct<S, U&&>
			 >::value, int> = 0>
	explicit SimpleTmpl(U&& u)
		: DataTmpl<S>(std::forward<U>(u))
	{
	}

	template<typename U = S,
		 std::enable_if_t<
			 std::__and_<
				 detail::is_diff<SimpleTmpl<S>, U>,
				 detail::is_ass<S&, U>
			 >::value, int> = 0>
	SimpleTmpl& operator=(U&& u) {
		DataTmpl<S>::operator=(std::forward<U>(u));
		return *this;
	}

	bool operator<(const SimpleTmpl& other) const noexcept { return this->get() < other.get(); }
	bool operator>(const SimpleTmpl& other) const noexcept { return other < *this; }
	bool operator<=(const SimpleTmpl& other) const noexcept { return !(other < *this); }
	bool operator>=(const SimpleTmpl& other) const noexcept { return !(*this < other); }
	bool operator==(const SimpleTmpl& other) const noexcept { return this->get() == other.get(); }
	bool operator!=(const SimpleTmpl& other) const noexcept { return !(*this == other); }
};

class MapKey {
private:
	struct DSO_LOCAL Type;
	struct Type {
		typedef enum {
			UINT_8,
			UINT_16,
			UINT_32,
			UINT_64,
			INT_8,
			INT_16,
			INT_32,
			INT_64,
			TEXT,
			BYTESTRING,
		} type;
	};
	template<typename K, typename = void>
	struct DSO_LOCAL helper;
	template<typename K, typename>
	struct helper {
	};
	template<typename D>
	struct helper<uint8_t, D> {
		static Type::type type() noexcept { return Type::UINT_8; }
	};
	template<typename D>
	struct helper<uint16_t, D> {
		static Type::type type() noexcept { return Type::UINT_16; }
	};
	template<typename D>
	struct helper<uint32_t, D> {
		static Type::type type() noexcept { return Type::UINT_32; }
	};
	template<typename D>
	struct helper<uint64_t, D> {
		static Type::type type() noexcept { return Type::UINT_64; }
	};
	template<typename D>
	struct helper<int8_t, D> {
		static Type::type type() noexcept { return Type::INT_8; }
	};
	template<typename D>
	struct helper<int16_t, D> {
		static Type::type type() noexcept { return Type::INT_16; }
	};
	template<typename D>
	struct helper<int32_t, D> {
		static Type::type type() noexcept { return Type::INT_32; }
	};
	template<typename D>
	struct helper<int64_t, D> {
		static Type::type type() noexcept { return Type::INT_64; }
	};
	template<typename D>
	struct helper<std::string, D> {
		static Type::type type() noexcept { return Type::TEXT; }
	};
	template<typename D>
	struct helper<utils::ustring, D> {
		static Type::type type() noexcept { return Type::BYTESTRING; }
	};

	union All {
		~All() {}
		All() {}
		// All(const All& other) noexcept { memcpy(&uint8, &other.uint8, sizeof(All)); }
		// All(All&& other) noexcept { std::cerr << "memcpy" << std::endl; memcpy(&uint8, &other.uint8, sizeof(All)); }
		All(SimpleTmpl<uint8_t>&& e) noexcept : uint8(std::move(e)) {}
		All(SimpleTmpl<uint16_t>&& e) noexcept : uint16(std::move(e)) {}
		All(SimpleTmpl<uint32_t>&& e) noexcept : uint32(std::move(e)) {}
		All(SimpleTmpl<uint64_t>&& e) noexcept : uint64(std::move(e)) {}
		All(SimpleTmpl<int8_t>&& e) noexcept : int8(std::move(e)) {}
		All(SimpleTmpl<int16_t>&& e) noexcept : int16(std::move(e)) {}
		All(SimpleTmpl<int32_t>&& e) noexcept : int32(std::move(e)) {}
		All(SimpleTmpl<int64_t>&& e) noexcept : int64(std::move(e)) {}
		All(SimpleTmpl<std::string>&& e) noexcept : text(std::move(e)) {}
		All(SimpleTmpl<utils::ustring>&& e) noexcept : bytestring(std::move(e)) {}
		SimpleTmpl<uint8_t> uint8;
		SimpleTmpl<uint16_t> uint16;
		SimpleTmpl<uint32_t> uint32;
		SimpleTmpl<uint64_t> uint64;
		SimpleTmpl<int8_t> int8;
		SimpleTmpl<int16_t> int16;
		SimpleTmpl<int32_t> int32;
		SimpleTmpl<int64_t> int64;
		SimpleTmpl<std::string> text;
		SimpleTmpl<utils::ustring> bytestring;
	} u;

	Type::type type;

public:
	template<typename K, std::enable_if_t<!std::is_same<K, std::decay<MapKey>::type>::value, bool> = true>
	explicit MapKey(K&& k)
		: u(SimpleTmpl<K>(std::forward<K>(k)))
		, type(helper<K>::type())
	{
	}

	MapKey(const MapKey&) = delete;
	MapKey(MapKey&& other)
		: type(other.type)
	{
		switch (type) {
		case Type::UINT_8:
			new (&u.uint8) SimpleTmpl<uint8_t>(std::move(other.u.uint8));
			break;
		case Type::UINT_16:
			new (&u.uint16) SimpleTmpl<uint16_t>(std::move(other.u.uint16));
			break;
		case Type::UINT_32:
			new (&u.uint32) SimpleTmpl<uint32_t>(std::move(other.u.uint32));
			break;
		case Type::UINT_64:
			new (&u.uint64) SimpleTmpl<uint64_t>(std::move(other.u.uint64));
			break;
		case Type::INT_8:
			new (&u.int8) SimpleTmpl<int8_t>(std::move(other.u.int8));
			break;
		case Type::INT_16:
			new (&u.int16) SimpleTmpl<int16_t>(std::move(other.u.int16));
			break;
		case Type::INT_32:
			new (&u.int32) SimpleTmpl<int32_t>(std::move(other.u.int32));
			break;
		case Type::INT_64:
			new (&u.int64) SimpleTmpl<int64_t>(std::move(other.u.int64));
			break;
		case Type::TEXT:
			new (&u.text) SimpleTmpl<std::string>(std::move(other.u.text));
			break;
		case Type::BYTESTRING:
			new (&u.bytestring) SimpleTmpl<utils::ustring>(std::move(other.u.bytestring));
			break;
		default:
			break;
		}
	}
	MapKey& operator=(const MapKey&) = delete;
	MapKey& operator=(MapKey&&) = delete;

	~MapKey();

	bool operator<(const MapKey& other) const noexcept;
	bool operator>(const MapKey& other) const noexcept { return other < *this; }
	bool operator<=(const MapKey& other) const noexcept { return !(other < *this); }
	bool operator>=(const MapKey& other) const noexcept { return !(*this < other); }
	bool operator==(const MapKey& other) const noexcept;
	bool operator!=(const MapKey& other) const noexcept { return !(*this == other); }

	bool is_uint() const noexcept;
	bool is_int() const noexcept;
	uint64_t uint64() const noexcept;
	int64_t int64() const noexcept;

friend std::ostream& operator<<(std::ostream&, const MapKey&);
};

inline
MapKey::~MapKey() {
	using SKText = SimpleTmpl<std::string>;
	using SKByteString = SimpleTmpl<utils::ustring>;

	switch (type) {
	case Type::TEXT:
		u.text.~SKText();
		break;
	case Type::BYTESTRING:
		u.bytestring.~SKByteString();
		break;
	default:
		break;
	}
}

inline bool
MapKey::operator<(const MapKey& other) const noexcept {
	if (type < other.type)
		return true;
	if (type > other.type)
		return false;
	switch (type) {
	case Type::UINT_8:
		return u.uint8 < other.u.uint8;
	case Type::UINT_16:
		return u.uint16 < other.u.uint16;
	case Type::UINT_32:
		return u.uint32 < other.u.uint32;
	case Type::UINT_64:
		return u.uint64 < other.u.uint64;
	case Type::INT_8:
		return u.int8 < other.u.int8;
	case Type::INT_16:
		return u.int16 < other.u.int16;
	case Type::INT_32:
		return u.int32 < other.u.int32;
	case Type::INT_64:
		return u.int64 < other.u.int64;
	case Type::TEXT:
		return u.text < other.u.text;
	case Type::BYTESTRING:
		return u.bytestring < other.u.bytestring;
	default:
		return false;
	}
}

inline bool
MapKey::operator==(const MapKey& other) const noexcept {
	if (type != other.type)
		return false;
	switch (type) {
	case Type::UINT_8:
		return u.uint8 == other.u.uint8;
	case Type::UINT_16:
		return u.uint16 == other.u.uint16;
	case Type::UINT_32:
		return u.uint32 == other.u.uint32;
	case Type::UINT_64:
		return u.uint64 == other.u.uint64;
	case Type::INT_8:
		return u.int8 == other.u.int8;
	case Type::INT_16:
		return u.int16 == other.u.int16;
	case Type::INT_32:
		return u.int32 == other.u.int32;
	case Type::INT_64:
		return u.int64 == other.u.int64;
	case Type::TEXT:
		return u.text == other.u.text;
	case Type::BYTESTRING:
		return u.bytestring == other.u.bytestring;
	default:
		return false;
	}
}

inline bool
MapKey::is_uint() const noexcept {
	switch (type) {
	case Type::UINT_8:
	case Type::UINT_16:
	case Type::UINT_32:
	case Type::UINT_64:
		return true;
	default:
		return false;
	}
}

inline bool
MapKey::is_int() const noexcept {
	switch (type) {
	case Type::INT_8:
	case Type::INT_16:
	case Type::INT_32:
	case Type::INT_64:
		return true;
	default:
		return false;
	}
}

inline uint64_t
MapKey::uint64() const noexcept {
	switch (type) {
	case Type::UINT_8:
		return u.uint8.get();
	case Type::UINT_16:
		return u.uint16.get();
	case Type::UINT_32:
		return u.uint32.get();
	case Type::UINT_64:
		return u.uint64.get();
	default:
		return 0;
	}
}

inline int64_t
MapKey::int64() const noexcept {
	switch (type) {
	case Type::INT_8:
		return u.int8.get();
	case Type::INT_16:
		return u.int16.get();
	case Type::INT_32:
		return u.int32.get();
	case Type::INT_64:
		return u.int64.get();
	default:
		return 0;
	}
}

}
