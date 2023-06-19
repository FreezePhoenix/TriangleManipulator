#include <stddef.h>

#include <functional>
#include <algorithm>
#include <iterator>
#include <utility>
#include <vector>
#include <optional>

template <typename K, typename T, typename C = std::less<K>,
	class A = std::allocator<std::pair<K, T>>>
	requires std::predicate<C, K, K>
class flat_multimap {
public:
	typedef std::pair<K, T> value_type;
	typedef T mapped_type;
	typedef K key_type;
	typedef C key_compare;
	typedef std::vector<value_type, A>::size_type size_type;
	typedef std::vector<value_type, A>::difference_type difference_type;
	typedef std::vector<value_type, A>::iterator iterator;
	typedef std::vector<value_type, A>::const_iterator const_iterator;
	typedef value_type& reference;
	typedef const value_type& const_reference;
	typedef std::allocator_traits<A>::pointer pointer;
	typedef std::allocator_traits<A>::const_pointer const_pointer;
	typedef std::reverse_iterator<iterator> reverse_iterator;
	typedef std::reverse_iterator<const_iterator> const_reverse_iterator;
	typedef std::vector<value_type, A>::allocator_type allocator_type;
private:
	std::vector<value_type, A> buffer;
	C comparator;
public:
	constexpr flat_multimap() : buffer(), comparator() {
	};
	explicit constexpr flat_multimap(const A& allocator)
		: buffer(allocator), comparator() {
	};
	explicit constexpr flat_multimap(const C& comparator,
									 const A& allocator = A())
		: buffer(allocator), comparator(comparator) {
	};
	constexpr flat_multimap(const flat_multimap& other) noexcept
		: buffer(other.buffer), comparator(other.comparator) {
	}
	constexpr flat_multimap(flat_multimap&& other) noexcept
		: buffer(std::move(other.buffer)),
		comparator(std::move(other.comparator)) {
	}
	constexpr flat_multimap& operator=(const flat_multimap& other) noexcept {
		return *this = flat_multimap(other);
	}
	constexpr flat_multimap& operator=(flat_multimap&& other) noexcept {
		std::swap(buffer, other.buffer);
		std::swap(comparator, other.comparator);
		return *this;
	}
	constexpr ~flat_multimap() {
	}
	constexpr allocator_type get_allocator() const noexcept {
		return buffer.get_allocator();
	}
	constexpr size_type size() const noexcept {
		return buffer.size();
	}
	constexpr size_type max_size() const noexcept {
		return buffer.max_size();
	}
	constexpr size_type capacity() const noexcept {
		return buffer.capacity();
	}
	constexpr void resize(size_type count) {
		buffer.resize(count);
	}
	constexpr void reserve(size_type new_cap) {
		buffer.reserve(new_cap);
	}
	constexpr void shrink_to_fit() {
		buffer.shrink_to_fit();
	}
	constexpr bool empty() const noexcept {
		return buffer.empty();
	}
	constexpr void clear() noexcept {
		buffer.clear();
	}
	constexpr pointer data() noexcept {
		return buffer.data();
	}
	constexpr const_pointer data() const noexcept {
		return buffer.data();
	}
    constexpr iterator insert(const value_type& value) {
        return buffer.insert(
            std::upper_bound(
                cbegin(), cend(), value,
                [](const value_type& left, const value_type& right) {
                    return C()(left.first, right.first);
                }),
            value);
    }
    constexpr iterator insert(value_type&& value) {
        const_iterator destination = std::upper_bound(
            cbegin(), cend(), value,
            [](const value_type& left, const value_type& right) {
                return C()(left.first, right.first);
            });
        return buffer.insert(destination, std::move(value));
    }
	constexpr size_type erase(const key_type& key) {
		const auto [first, last] = equal_range(key);
		size_type dist = std::distance(first, last);
		buffer.erase(first, last);
		return dist;
	}
	constexpr size_type count(const key_type& key) const {
		const auto [first, last] = equal_range(key);
		return std::distance(first, last);
	}
	constexpr bool contains(const key_type& key) const {
		const_iterator first = lower_bound(key);
		return (!(first == buffer.cend()) and !(comparator(key, first->first)));
	}
	constexpr std::pair<iterator, iterator> equal_range(const key_type& key) {
		return std::make_pair(lower_bound(key), upper_bound(key));
	}
	constexpr std::pair<const_iterator, const_iterator> equal_range(
		const key_type& key) const {
		return std::make_pair(lower_bound(key), upper_bound(key));
	}
	constexpr iterator lower_bound(const key_type& key) {
		return std::lower_bound(begin(), end(), key,
			[this](const value_type& left, const key_type& right) {
				return comparator(left.first, right);
			});
	}
	constexpr const_iterator lower_bound(const key_type& key) const {
		return std::lower_bound(cbegin(), cend(), key,
			[this](const value_type& left, const key_type& right) {
				return comparator(left.first, right);
			});
	}
	constexpr iterator upper_bound(const key_type& key) {
		return std::upper_bound(
			begin(), end(), key,
			[this](const key_type& left, const value_type& right) {
				return comparator(left, right.first);
			});
	}
	constexpr const_iterator upper_bound(const key_type& key) const {
		return std::upper_bound(
			cbegin(), cend(), key,
			[this](const key_type& left, const value_type& right) {
				return comparator(left, right.first);
			});
	}
	constexpr void merge(flat_multimap<K, T, C, A>& source) {
		std::vector<value_type, A> vec(size() + source.size(),
									   buffer.get_allocator());
		std::swap(buffer, vec);
		std::merge(vec.cbegin(), vec.cend(), source.buffer.cbegin(),
				   source.buffer.cend(), buffer.begin(),
				   [this](const value_type& left, const value_type& right) {
					   return comparator(left.first, right.first);
				   });
		source.clear();
	}
	
	constexpr void merge(flat_multimap<K, T, C, A>&& source) {
		std::vector<value_type, A> vec(size() + source.size(),
									   buffer.get_allocator());
		std::swap(buffer, vec);
		std::merge(vec.cbegin(), vec.cend(), source.buffer.cbegin(),
				   source.buffer.cend(), buffer.begin(),
				   [this](const value_type& left, const value_type& right) {
					   return comparator(left.first, right.first);
				   });
		source.clear();
	}
	constexpr iterator begin() noexcept {
		return buffer.begin();
	}
	constexpr const_iterator begin() const noexcept {
		return buffer.begin();
	}
	constexpr const_iterator cbegin() const noexcept {
		return buffer.cbegin();
	}
	constexpr iterator end() noexcept {
		return buffer.end();
	}
	constexpr const_iterator end() const noexcept {
		return buffer.end();
	}
	constexpr const_iterator cend() const noexcept {
		return buffer.cend();
	}
	constexpr reverse_iterator rbegin() noexcept {
		return buffer.rbegin();
	}
	constexpr const_reverse_iterator rbegin() const noexcept {
		return buffer.rbegin();
	}
	constexpr const_reverse_iterator crbegin() const noexcept {
		return buffer.crbegin();
	}
	constexpr reverse_iterator rend() noexcept {
		return buffer.rend();
	}
	constexpr const_reverse_iterator rend() const noexcept {
		return buffer.rend();
	}
	constexpr const_reverse_iterator crend() const noexcept {
		return buffer.crend();
	}
};