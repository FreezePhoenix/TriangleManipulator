#pragma once

#ifndef _FLAT_MULTIMAP_HPP
#define _FLAT_MULTIMAP_HPP

#include <functional>
#include <algorithm>
#include <utility>
#include <vector>

template <typename K, typename T, typename C = std::less<K>,
          typename A = std::allocator<std::pair<K, T>>>
    requires std::predicate<C, K, K> &&
             std::same_as<std::pair<K, T>, typename A::value_type>
class flat_multimap {
   public:
    using value_type = std::pair<K, T>;
    using mapped_type = T;
    using key_type = K;
    using key_compare = C;
    using size_type = std::vector<value_type, A>::size_type;
    using difference_type = std::vector<value_type, A>::difference_type;
    using iterator = std::vector<value_type, A>::iterator;
    using const_iterator = std::vector<value_type, A>::const_iterator;
    using reference = std::pair<const key_type&, value_type&>;
    using const_reference = std::pair<const key_type&, const value_type&>;
    using reverse_iterator = std::reverse_iterator<iterator>;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;
    using allocator_type = std::vector<value_type, A>::allocator_type;
    using pointer = std::allocator_traits<A>::pointer;
    using const_pointer = std::allocator_traits<A>::const_pointer;

   private:
    std::vector<value_type, A> buffer;
    [[no_unique_address]] struct compare_helper {
        [[no_unique_address]] C comparator;
        compare_helper(const C& c) : comparator(c) {}
        constexpr bool operator()(const value_type& lhs,
                                  const value_type& rhs) const {
            return comparator(lhs.first, rhs.first);
        }
        template <typename KK>
            requires std::predicate<C, key_type, KK>
        constexpr bool operator()(const value_type& lhs, const KK& rhs) const {
            return comparator(lhs.first, rhs);
        }
        template <typename KK>
            requires std::predicate<C, KK, key_type>
        constexpr bool operator()(const KK& lhs, const value_type& rhs) const {
            return comparator(lhs, rhs.first);
        }
        template <typename K1, typename K2>
            requires std::predicate<C, K1, K2>
        constexpr bool operator()(const K1& lhs, const K2& rhs) const {
            return comparator(lhs, rhs);
        }
    } comparator;

   public:
    constexpr flat_multimap() : buffer(), comparator(C()){};
    explicit constexpr flat_multimap(const A& allocator)
        : buffer(allocator), comparator(C()){};
    explicit constexpr flat_multimap(const C& comparator,
                                     const A& allocator = A())
        : buffer(allocator), comparator(comparator){};
    constexpr allocator_type get_allocator() const noexcept {
        return buffer.get_allocator();
    }
    constexpr size_type size() const noexcept { return buffer.size(); }
    constexpr size_type max_size() const noexcept { return buffer.max_size(); }
    constexpr size_type capacity() const noexcept { return buffer.capacity(); }
    constexpr void resize(size_type count) { buffer.resize(count); }
    constexpr void reserve(size_type new_cap) { buffer.reserve(new_cap); }
    constexpr void shrink_to_fit() { buffer.shrink_to_fit(); }
    constexpr bool empty() const noexcept { return buffer.empty(); }
    constexpr void clear() noexcept { buffer.clear(); }
    constexpr pointer data() noexcept { return buffer.data(); }
    constexpr const_pointer data() const noexcept { return buffer.data(); }
    constexpr iterator insert(const value_type& value) {
        return buffer.insert(
            std::upper_bound(cbegin(), cend(), value, comparator), value);
    }
    constexpr iterator insert(value_type&& value) {
        return buffer.insert(std::upper_bound(cbegin(), cend(), value, comparator), std::move(value));
    }
    template <class... Args>
    constexpr iterator emplace(Args&&... args) {
        value_type value(std::forward<Args>(args)...);
        const_iterator destination =
            std::upper_bound(cbegin(), cend(), value, comparator);
        return buffer.insert(destination, std::move(value));
    }
    constexpr iterator erase(const_iterator pos) {
        return buffer.erase(pos);
    }

    constexpr iterator erase(const_iterator first, const_iterator last) {
        return buffer.erase(first, last);
    }

    template <typename KK>
        requires std::predicate<C, key_type, KK> &&
                 std::predicate<C, KK, key_type>
    constexpr size_type erase(const KK& key) {
        const auto [first, last] = equal_range(key);
        size_type dist = std::distance(first, last);
        buffer.erase(first, last);
        return dist;
    }

    template <typename KK>
        requires std::predicate<C, key_type, KK> &&
                 std::predicate<C, KK, key_type>
    constexpr size_type count(const KK& key) const {
        auto [first, last] = equal_range(key);
        return std::distance(first, last);
    }

    template <typename KK>
        requires std::predicate<C, key_type, KK> &&
                 std::predicate<C, KK, key_type>
    constexpr bool contains(const KK& key) const {
        return std::binary_search(cbegin(), cend(), key, comparator);
    }

    template <typename KK>
        requires std::predicate<C, key_type, KK> &&
                 std::predicate<C, KK, key_type>
    constexpr std::pair<iterator, iterator> equal_range(const KK& key) {
        return std::equal_range(begin(), end(), key, comparator);
    }

    template <typename KK>
        requires std::predicate<C, key_type, KK> &&
                 std::predicate<C, KK, key_type>
    constexpr std::pair<const_iterator, const_iterator> equal_range(
        const KK& key) const {
        return std::equal_range(cbegin(), cend(), key, comparator);
    }

    template <typename KK>
        requires std::predicate<C, key_type, KK>
    constexpr iterator lower_bound(const KK& key) {
        return std::lower_bound(begin(), end(), key, comparator);
    }

    template <typename KK>
        requires std::predicate<C, key_type, KK>
    constexpr const_iterator lower_bound(const KK& key) const {
        return std::lower_bound(cbegin(), cend(), key, comparator);
    }
    template <typename KK>
        requires std::predicate<C, KK, key_type>
    constexpr iterator upper_bound(const KK& key) {
        return std::upper_bound(begin(), end(), key, comparator);
    }
    
    template <typename KK>
        requires std::predicate<C, KK, key_type>
    constexpr const_iterator upper_bound(const KK& key) const {
        return std::upper_bound(cbegin(), cend(), key, comparator);
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
    constexpr iterator begin() noexcept { return buffer.begin(); }
    constexpr const_iterator begin() const noexcept { return buffer.begin(); }
    constexpr const_iterator cbegin() const noexcept { return buffer.cbegin(); }
    constexpr iterator end() noexcept { return buffer.end(); }
    constexpr const_iterator end() const noexcept { return buffer.end(); }
    constexpr const_iterator cend() const noexcept { return buffer.cend(); }
    constexpr reverse_iterator rbegin() noexcept { return buffer.rbegin(); }
    constexpr const_reverse_iterator rbegin() const noexcept {
        return buffer.rbegin();
    }
    constexpr const_reverse_iterator crbegin() const noexcept {
        return buffer.crbegin();
    }
    constexpr reverse_iterator rend() noexcept { return buffer.rend(); }
    constexpr const_reverse_iterator rend() const noexcept {
        return buffer.rend();
    }
    constexpr const_reverse_iterator crend() const noexcept {
        return buffer.crend();
    }
};

#endif