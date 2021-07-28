#pragma once

#if __cplusplus >= 201703L

#include <string_view>
#ifndef __HAS_STRING_VIEW
#define __HAS_STRING_VIEW 1
#endif

#elif __cplusplus >= 201402L
#include <experimental/string_view>

#ifndef __HAS_STRING_VIEW
#define __HAS_STRING_VIEW 1
#endif

namespace std {
typedef experimental::string_view string_view;
template<typename Char>
using basic_string_view = experimental::basic_string_view<Char>;
}
#else

#include <iterator>
#include <stdexcept>
#include <limits>

namespace std {

template<typename Char, typename Traits> class basic_string_view;

typedef std::basic_string_view<char, std::char_traits<char> > string_view;
typedef std::basic_string_view<wchar_t, std::char_traits<wchar_t> > wstring_view;

template<typename Char, typename Traits = std::char_traits<Char> >
class basic_string_view {
private:
	const Char *data_;
	std::size_t length_;

public:
	typedef Traits traits_type;
	typedef Char value_type;
	typedef Char *pointer;
	typedef const Char *const_pointer;
	typedef Char& reference;
	typedef const Char& const_reference;
	typedef __gnu_cxx::__normal_iterator<const_pointer, basic_string_view>
	const_iterator;
	typedef const_iterator iterator;
	typedef std::reverse_iterator<iterator> reverse_iterator;
	typedef std::reverse_iterator<const_iterator> const_reverse_iterator;
	typedef std::size_t size_type;
	typedef std::ptrdiff_t difference_type;

	static const size_type npos = size_type(-1);

	basic_string_view() noexcept;
	basic_string_view(const basic_string_view&) noexcept = default;
	basic_string_view(const Char *, size_type);
	basic_string_view(const Char *);

	const_pointer data() const noexcept;
	size_type length() const noexcept;
	size_type size() const noexcept;
	size_type max_size() const noexcept;
	bool empty() const noexcept;

	const_iterator begin() const noexcept;
	const_iterator cbegin() const noexcept;

	const_iterator end() const noexcept;
	const_iterator cend() const noexcept;

	const_reference operator[](size_type) const;
	const_reference at(size_type) const;
	const_reference front() const;
	const_reference back() const;

	void swap(basic_string_view&) noexcept;

	void remove_prefix(size_type);
	void remove_suffix(size_type);

	size_type copy(Char *, size_type, size_type = 0) const;
	basic_string_view substr(size_type = 0, size_type = npos) const;

	int compare(basic_string_view) const noexcept;
	int compare(size_type, size_type, basic_string_view) const;
	int compare(size_type, size_type, basic_string_view, size_type,
		    size_type) const;
	int compare(const Char *) const;
	int compare(size_type, size_type, const Char *) const;
	int compare(size_type, size_type, const Char *, size_type) const;

	size_type find(basic_string_view, size_type = 0) const noexcept;
	size_type find(Char, size_type = 0) const noexcept;
	size_type find(const Char *, size_type, size_type) const;
	size_type find(const Char *, size_type = 0) const;

	size_type rfind(basic_string_view, size_type = npos) const noexcept;
	size_type rfind(Char, size_type = npos) const noexcept;
	size_type rfind(const Char *, size_type, size_type) const;
	size_type rfind(const Char *, size_type = npos) const;

	size_type find_first_of(basic_string_view, size_type = 0) const noexcept;
	size_type find_first_of(Char, size_type = 0) const noexcept;
	size_type find_first_of(const Char *, size_type, size_type) const;
	size_type find_first_of(const Char *, size_type = 0) const;

	size_type find_last_of(basic_string_view, size_type = npos) const noexcept;
	size_type find_last_of(Char, size_type = npos) const noexcept;
	size_type find_last_of(const Char *, size_type, size_type) const;
	size_type find_last_of(const Char *, size_type = npos) const;

	size_type find_first_not_of(basic_string_view, size_type = 0) const noexcept;
	size_type find_first_not_of(Char, size_type = 0) const noexcept;
	size_type find_first_not_of(const Char *, size_type, size_type) const;
	size_type find_first_not_of(const Char *, size_type = 0) const;

	size_type find_last_not_of(basic_string_view, size_type = npos) const noexcept;
	size_type find_last_not_of(Char, size_type = npos) const noexcept;
	size_type find_last_not_of(const Char *, size_type, size_type) const;
	size_type find_last_not_of(const Char *, size_type = npos) const;

private:
	size_type kmp(basic_string_view v, size_type, size_type,
		      bool) const noexcept;

};

template<typename Char, typename Traits>
inline
basic_string_view<Char, Traits>::basic_string_view() noexcept
	: data_(nullptr)
	, length_(0)
{
}

template<typename Char, typename Traits>
inline
basic_string_view<Char, Traits>::basic_string_view(const Char *data__,
						   size_type length__)
	: data_(data__)
	, length_(length__)
{
}

template<typename Char, typename Traits>
inline
basic_string_view<Char, Traits>::basic_string_view(const Char *data__)
	: data_(data__)
	, length_(nullptr == data__ ? 0 : traits_type::length(data__))
{
}

template<typename Char, typename Traits>
inline typename basic_string_view<Char, Traits>::const_pointer
basic_string_view<Char, Traits>::data() const noexcept {
	return data_;
}

template<typename Char, typename Traits>
inline typename basic_string_view<Char, Traits>::size_type
basic_string_view<Char, Traits>::length() const noexcept {
	return length_;
}

template<typename Char, typename Traits>
inline typename basic_string_view<Char, Traits>::size_type
basic_string_view<Char, Traits>::size() const noexcept {
	return length_;
}

template<typename Char, typename Traits>
inline bool
basic_string_view<Char, Traits>::empty() const noexcept {
	return 0 == length_;
}

template<typename Char, typename Traits>
inline typename basic_string_view<Char, Traits>::const_iterator
basic_string_view<Char, Traits>::begin() const noexcept {
	return const_iterator(data_);
}

template<typename Char, typename Traits>
inline typename basic_string_view<Char, Traits>::const_iterator
basic_string_view<Char, Traits>::cbegin() const noexcept {
	return const_iterator(data_);
}

template<typename Char, typename Traits>
inline typename basic_string_view<Char, Traits>::const_iterator
basic_string_view<Char, Traits>::end() const noexcept {
	return const_iterator(data_ + length_);
}

template<typename Char, typename Traits>
inline typename basic_string_view<Char, Traits>::const_iterator
basic_string_view<Char, Traits>::cend() const noexcept {
	return const_iterator(data_ + length_);
}

template<typename Char, typename Traits>
inline typename basic_string_view<Char, Traits>::const_reference
basic_string_view<Char, Traits>::operator[](size_type n) const {
	return data_[n];
}

template<typename Char, typename Traits>
inline typename basic_string_view<Char, Traits>::const_reference
basic_string_view<Char, Traits>::at(size_type n) const {
	if (n >= length_)
		throw out_of_range(__FUNCTION__);
	return data_[n];
}

template<typename Char, typename Traits>
inline typename basic_string_view<Char, Traits>::const_reference
basic_string_view<Char, Traits>::front() const {
	return data_[0];
}

template<typename Char, typename Traits>
inline typename basic_string_view<Char, Traits>::const_reference
basic_string_view<Char, Traits>::back() const {
	return data_[length_ - 1];
}

template<typename Char, typename Traits>
inline typename basic_string_view<Char, Traits>::size_type
basic_string_view<Char, Traits>::max_size() const noexcept {
	return std::numeric_limits<size_type>::max();
}

template<typename Char, typename Traits>
inline void
basic_string_view<Char, Traits>::swap(basic_string_view& other) noexcept {
	std::swap(data_, other.data_);
	std::swap(length_, other.length_);
}

template<typename Char, typename Traits>
inline void
basic_string_view<Char, Traits>::remove_prefix(size_type n) {
	data_ += n;
	length_ -= n;
}

template<typename Char, typename Traits>
inline void
basic_string_view<Char, Traits>::remove_suffix(size_type n) {
	length_ -= n;
}

template<typename Char, typename Traits>
inline typename basic_string_view<Char, Traits>::size_type
basic_string_view<Char, Traits>::
copy(Char *dst, size_type n, size_type pos) const {
	if (pos >= size())
		return 0;
	traits_type::copy(dst, data() + pos, n);
	size_type q = size() - pos;
	return n < q ? n : q;
}

template<typename Char, typename Traits>
inline basic_string_view<Char, Traits>
basic_string_view<Char, Traits>::substr(size_type pos, size_type count) const {
	if (pos > size())
		throw out_of_range(__FUNCTION__);
	size_type s = size() - pos;
	return basic_string_view(data_ + pos, s < count ? s : count);
}

template<typename Char, typename Traits>
inline int
basic_string_view<Char, Traits>::compare(basic_string_view v) const noexcept {
	if (size() < v.size()) {
		int r = traits_type::compare(data(), v.data(), size());
		return 0 == r ? -1 : r;
	}
	if (size() > v.size()) {
		int r = traits_type::compare(data(), v.data(), v.size());
		return 0 == r ? 1 : r;
	}
	return traits_type::compare(data(), v.data(), size());
}

template<typename Char, typename Traits>
inline int
basic_string_view<Char, Traits>::compare(size_type pos1, size_type count1,
					 basic_string_view v) const {
	return substr(pos1, count1).compare(v);
}

template<typename Char, typename Traits>
inline int
basic_string_view<Char, Traits>::compare(size_type pos1, size_type count1,
					 basic_string_view v, size_type pos2,
					 size_type count2) const {
	return substr(pos1, count1).compare(v.substr(pos2, count2));
}

template<typename Char, typename Traits>
inline int
basic_string_view<Char, Traits>::compare(const Char *s) const {
	return compare(basic_string_view(s));
}

template<typename Char, typename Traits>
inline int
basic_string_view<Char, Traits>::compare(size_type pos1, size_type count1,
					 const Char *s) const {
	return substr(pos1, count1).compare(basic_string_view(s));
}

template<typename Char, typename Traits>
inline int
basic_string_view<Char, Traits>::compare(size_type pos1, size_type count1,
					 const Char *s,
					 size_type count2) const {
	return substr(pos1, count1).compare(basic_string_view(s, count2));
}

template<typename Char, typename Traits>
typename basic_string_view<Char, Traits>::size_type
basic_string_view<Char, Traits>::
kmp(basic_string_view v, size_type pos, size_type last, bool f) const noexcept {
	if (0 == v.length())
		return 0;

	// knuth-morris-pratt
	size_type T[v.length() + 1];

	T[0] = npos;

	size_type i = 1, k = 0;

	for (; i < v.length(); ++i) {
		if (traits_type::eq(v[i], v[k])) {
			T[i] = T[k];
			++k;
		} else {
			T[i] = k;
			k = T[k];
			if (k != npos) {
				while (!traits_type::eq(v[i], v[k])) {
					if (npos == T[k]) {
						k = 0;
						break;
					} else
						k = T[k];
				}
			} else
				++k;
		}
	}
	T[i] = k;

	i = pos;
	k = 0;

	size_type r = npos;
	while (i < last) {
		if (traits_type::eq(v[k], (*this)[i])) {
			++i;
			++k;
			if (k == v.length()) {
				r = i - k;
				if (f)
					return r;
				k = T[k];
			}
		} else {
			k = T[k];
			if (npos == k) {
				++i;
				++k;
			}
		}
	}

	return r;
}

template<typename Char, typename Traits>
inline typename basic_string_view<Char, Traits>::size_type
basic_string_view<Char, Traits>::
find(basic_string_view v, size_type pos) const noexcept {
	return kmp(v, pos, length(), true);
}

template<typename Char, typename Traits>
inline typename basic_string_view<Char, Traits>::size_type
basic_string_view<Char, Traits>::find(Char ch, size_type pos) const noexcept {
	return find(basic_string_view(addressof(ch), 1), pos);
}

template<typename Char, typename Traits>
inline typename basic_string_view<Char, Traits>::size_type
basic_string_view<Char, Traits>::find(const Char *s, size_type pos,
				      size_type count) const {
	return find(basic_string_view(s, count), pos);
}

template<typename Char, typename Traits>
inline typename basic_string_view<Char, Traits>::size_type
basic_string_view<Char, Traits>::find(const Char *s, size_type pos) const {
	return find(basic_string_view(s), pos);
}

template<typename Char, typename Traits>
inline typename basic_string_view<Char, Traits>::size_type
basic_string_view<Char, Traits>::
rfind(basic_string_view v, size_type pos) const noexcept {
	if (npos == pos)
		return kmp(v, 0, length(), false);
	size_type n = pos + v.size();
	return kmp(v, 0, n > length() ? length() : n, false);
}

template<typename Char, typename Traits>
inline typename basic_string_view<Char, Traits>::size_type
basic_string_view<Char, Traits>::rfind(Char ch, size_type pos) const noexcept {
	return rfind(basic_string_view(addressof(ch), 1), pos);
}

template<typename Char, typename Traits>
inline typename basic_string_view<Char, Traits>::size_type
basic_string_view<Char, Traits>::rfind(const Char *s, size_type pos,
				       size_type count) const {
	return rfind(basic_string_view(s, count), pos);
}

template<typename Char, typename Traits>
inline typename basic_string_view<Char, Traits>::size_type
basic_string_view<Char, Traits>::rfind(const Char *s, size_type pos) const {
	return rfind(basic_string_view(s), pos);
}

template<typename Char, typename Traits>
typename basic_string_view<Char, Traits>::size_type
basic_string_view<Char, Traits>::
find_first_of(basic_string_view v, size_type pos) const noexcept {
	for (size_type i = pos; i < size(); ++i)
		if (npos != v.find((*this)[i]))
			return i;
	return npos;
}

template<typename Char, typename Traits>
inline typename basic_string_view<Char, Traits>::size_type
basic_string_view<Char, Traits>::
find_first_of(Char ch, size_type pos) const noexcept {
	return find_first_of(basic_string_view(addressof(ch), 1), pos);
}

template<typename Char, typename Traits>
inline typename basic_string_view<Char, Traits>::size_type
basic_string_view<Char, Traits>::
find_first_of(const Char *s, size_type pos, size_type count) const {
	return find_first_of(basic_string_view(s, count), pos);
}

template<typename Char, typename Traits>
inline typename basic_string_view<Char, Traits>::size_type
basic_string_view<Char, Traits>::
find_first_of(const Char *s, size_type pos) const {
	return find_first_of(basic_string_view(s), pos);
}

template<typename Char, typename Traits>
typename basic_string_view<Char, Traits>::size_type
basic_string_view<Char, Traits>::
find_last_of(basic_string_view v, size_type pos) const noexcept {
	if (pos >= size())
		pos = size();
	else
		++pos;
	for (size_type i = pos; i > 0;) {
		--i;
		if (npos != v.find((*this)[i]))
			return i;
	}
	return npos;
}

template<typename Char, typename Traits>
inline typename basic_string_view<Char, Traits>::size_type
basic_string_view<Char, Traits>::
find_last_of(Char ch, size_type pos) const noexcept {
	return find_last_of(basic_string_view(addressof(ch), 1), pos);
}

template<typename Char, typename Traits>
inline typename basic_string_view<Char, Traits>::size_type
basic_string_view<Char, Traits>::
find_last_of(const Char *s, size_type pos, size_type count) const {
	return find_last_of(basic_string_view(s, count), pos);
}

template<typename Char, typename Traits>
inline typename basic_string_view<Char, Traits>::size_type
basic_string_view<Char, Traits>::
find_last_of(const Char *s, size_type pos) const {
	return find_last_of(basic_string_view(s), pos);
}

template<typename Char, typename Traits>
typename basic_string_view<Char, Traits>::size_type
basic_string_view<Char, Traits>::
find_first_not_of(basic_string_view v, size_type pos) const noexcept {
	for (size_type i = pos; i < size(); ++i)
		if (npos == v.find((*this)[i]))
			return i;
	return npos;
}

template<typename Char, typename Traits>
inline typename basic_string_view<Char, Traits>::size_type
basic_string_view<Char, Traits>::
find_first_not_of(Char ch, size_type pos) const noexcept {
	return find_first_not_of(basic_string_view(addressof(ch), 1), pos);
}

template<typename Char, typename Traits>
inline typename basic_string_view<Char, Traits>::size_type
basic_string_view<Char, Traits>::
find_first_not_of(const Char *s, size_type pos, size_type count) const {
	return find_first_not_of(basic_string_view(s, count), pos);
}

template<typename Char, typename Traits>
inline typename basic_string_view<Char, Traits>::size_type
basic_string_view<Char, Traits>::
find_first_not_of(const Char *s, size_type pos) const {
	return find_first_not_of(basic_string_view(s), pos);
}

template<typename Char, typename Traits>
typename basic_string_view<Char, Traits>::size_type
basic_string_view<Char, Traits>::
find_last_not_of(basic_string_view v, size_type pos) const noexcept {
	if (pos >= size())
		pos = size();
	else
		++pos;
	for (size_type i = pos; i > 0;) {
		--i;
		if (npos == v.find((*this)[i]))
			return i;
	}
	return npos;
}

template<typename Char, typename Traits>
inline typename basic_string_view<Char, Traits>::size_type
basic_string_view<Char, Traits>::
find_last_not_of(Char ch, size_type pos) const noexcept {
	return find_last_not_of(basic_string_view(addressof(ch), 1), pos);
}

template<typename Char, typename Traits>
inline typename basic_string_view<Char, Traits>::size_type
basic_string_view<Char, Traits>::
find_last_not_of(const Char *s, size_type pos, size_type count) const {
	return find_last_not_of(basic_string_view(s, count), pos);
}

template<typename Char, typename Traits>
inline typename basic_string_view<Char, Traits>::size_type
basic_string_view<Char, Traits>::
find_last_not_of(const Char *s, size_type pos) const {
	return find_last_not_of(basic_string_view(s), pos);
}

template<typename Char, typename Traits>
inline bool
operator==(const basic_string_view<Char, Traits>& v1,
	   const basic_string_view<Char, Traits>& v2) noexcept {
	return 0 == v1.compare(v2);
}

template<typename Char, typename Traits>
inline bool
operator!=(const basic_string_view<Char, Traits>& v1,
	   const basic_string_view<Char, Traits>& v2) noexcept {
	return 0 != v1.compare(v2);
}

template<typename Char, typename Traits>
inline bool
operator>=(const basic_string_view<Char, Traits>& v1,
	   const basic_string_view<Char, Traits>& v2) noexcept {
	return v1.compare(v2) >= 0;
}

template<typename Char, typename Traits>
inline bool
operator<=(const basic_string_view<Char, Traits>& v1,
	   const basic_string_view<Char, Traits>& v2) noexcept {
	return v1.compare(v2) <= 0;
}

template<typename Char, typename Traits>
inline bool
operator>(const basic_string_view<Char, Traits>& v1,
	  const basic_string_view<Char, Traits>& v2) noexcept {
	return v1.compare(v2) > 0;
}

template<typename Char, typename Traits>
inline bool
operator<(const basic_string_view<Char, Traits>& v1,
	  const basic_string_view<Char, Traits>& v2) noexcept {
	return v1.compare(v2) < 0;
}

}

#endif
