// -*- C++ -*-
//===----------------------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#pragma once

// clang-format off

/*
    string synopsis

#include <compare>
#include <std::initializer_list>

namespace std
{

template <class stateT>
class fpos
{
private:
    stateT st;
public:
    fpos(streamoff = streamoff());

    operator streamoff() const;

    stateT state() const;
    void state(stateT);

    fpos& operator+=(streamoff);
    fpos  operator+ (streamoff) const;
    fpos& operator-=(streamoff);
    fpos  operator- (streamoff) const;
};

template <class stateT> streamoff operator-(const fpos<stateT>& x, const fpos<stateT>& y);

template <class stateT> bool operator==(const fpos<stateT>& x, const fpos<stateT>& y);
template <class stateT> bool operator!=(const fpos<stateT>& x, const fpos<stateT>& y);

template <class charT>
struct char_traits
{
    using char_type           = charT;
    using int_type            = ...;
    using off_type            = streamoff;
    using pos_type            = streampos;
    using state_type          = mbstate_t;
    using comparison_category = strong_ordering; // Since C++20 only for the specializations
                                                 // char, wchar_t, char8_t, char16_t, and char32_t.

    static void assign(char_type& c1, const char_type& c2) noexcept;
    static constexpr bool eq(char_type c1, char_type c2) noexcept;
    static constexpr bool lt(char_type c1, char_type c2) noexcept;

    static int              compare(const char_type* s1, const char_type* s2, size_t n);
    static size_t           length(const char_type* s);
    static const char_type* find(const char_type* s, size_t n, const char_type& a);
    static char_type*       move(char_type* s1, const char_type* s2, size_t n);
    static char_type*       copy(char_type* s1, const char_type* s2, size_t n);
    static char_type*       assign(char_type* s, size_t n, char_type a);

    static constexpr int_type  not_eof(int_type c) noexcept;
    static constexpr char_type to_char_type(int_type c) noexcept;
    static constexpr int_type  to_int_type(char_type c) noexcept;
    static constexpr bool      eq_int_type(int_type c1, int_type c2) noexcept;
    static constexpr int_type  eof() noexcept;
};

template <> struct char_traits<char>;
template <> struct char_traits<wchar_t>;
template <> struct char_traits<char8_t>;  // C++20
template <> struct char_traits<char16_t>;
template <> struct char_traits<char32_t>;

template<class charT, class traits = char_traits<charT>, class Allocator = allocator<charT> >
class basic_string
{
public:
// types:
    typedef traits traits_type;
    typedef typename traits_type::char_type value_type;
    typedef Allocator allocator_type;
    typedef typename allocator_type::size_type size_type;
    typedef typename allocator_type::difference_type difference_type;
    typedef typename allocator_type::reference reference;
    typedef typename allocator_type::const_reference const_reference;
    typedef typename allocator_type::pointer pointer;
    typedef typename allocator_type::const_pointer const_pointer;
    typedef implementation-defined iterator;
    typedef implementation-defined const_iterator;
    typedef std::reverse_iterator<iterator> reverse_iterator;
    typedef std::reverse_iterator<const_iterator> const_reverse_iterator;

    static const size_type npos = -1;

    basic_string()
        noexcept(std::is_nothrow_default_constructible<allocator_type>::value);                      // constexpr since C++20
    explicit basic_string(const allocator_type& a);                                             // constexpr since C++20
    basic_string(const basic_string& str);                                                      // constexpr since C++20
    basic_string(basic_string&& str)
        noexcept(std::is_nothrow_move_constructible<allocator_type>::value);                         // constexpr since C++20
    basic_string(const basic_string& str, size_type pos,
                 const allocator_type& a = allocator_type());                                   // constexpr since C++20
    basic_string(const basic_string& str, size_type pos, size_type n,
                 const Allocator& a = Allocator());                                             // constexpr since C++20
    constexpr basic_string(
        basic_string&& str, size_type pos, const Allocator& a = Allocator());                   // since C++23
    constexpr basic_string(
        basic_string&& str, size_type pos, size_type n, const Allocator& a = Allocator());      // since C++23
    template<class T>
        basic_string(const T& t, size_type pos, size_type n, const Allocator& a = Allocator()); // C++17, constexpr since C++20
    template <class T>
        explicit basic_string(const T& t, const Allocator& a = Allocator());                    // C++17, constexpr since C++20
    basic_string(const value_type* s, const allocator_type& a = allocator_type());              // constexpr since C++20
    basic_string(const value_type* s, size_type n, const allocator_type& a = allocator_type()); // constexpr since C++20
    basic_string(nullptr_t) = delete; // C++23
    basic_string(size_type n, value_type c, const allocator_type& a = allocator_type());        // constexpr since C++20
    template<class InputIterator>
        basic_string(InputIterator begin, InputIterator end,
                     const allocator_type& a = allocator_type());                               // constexpr since C++20
    template<container-compatible-range<charT> R>
      constexpr basic_string(from_range_t, R&& rg, const Allocator& a = Allocator());           // since C++23
    basic_string(std::initializer_list<value_type>, const Allocator& = Allocator());                 // constexpr since C++20
    basic_string(const basic_string&, const Allocator&);                                        // constexpr since C++20
    basic_string(basic_string&&, const Allocator&);                                             // constexpr since C++20

    ~basic_string();                                                                            // constexpr since C++20

    operator std::basic_string_view<charT, traits>() const noexcept;                                 // constexpr since C++20

    basic_string& operator=(const basic_string& str);                                           // constexpr since C++20
    template <class T>
        basic_string& operator=(const T& t);                                                    // C++17, constexpr since C++20
    basic_string& operator=(basic_string&& str)
        noexcept(
             allocator_type::propagate_on_container_move_assignment::value ||
             allocator_type::is_always_equal::value );                                          // C++17, constexpr since C++20
    basic_string& operator=(const value_type* s);                                               // constexpr since C++20
    basic_string& operator=(nullptr_t) = delete; // C++23
    basic_string& operator=(value_type c);                                                      // constexpr since C++20
    basic_string& operator=(std::initializer_list<value_type>);                                      // constexpr since C++20

    iterator       begin() noexcept;                                                            // constexpr since C++20
    const_iterator begin() const noexcept;                                                      // constexpr since C++20
    iterator       end() noexcept;                                                              // constexpr since C++20
    const_iterator end() const noexcept;                                                        // constexpr since C++20

    reverse_iterator       rbegin() noexcept;                                                   // constexpr since C++20
    const_reverse_iterator rbegin() const noexcept;                                             // constexpr since C++20
    reverse_iterator       rend() noexcept;                                                     // constexpr since C++20
    const_reverse_iterator rend() const noexcept;                                               // constexpr since C++20

    const_iterator         cbegin() const noexcept;                                             // constexpr since C++20
    const_iterator         cend() const noexcept;                                               // constexpr since C++20
    const_reverse_iterator crbegin() const noexcept;                                            // constexpr since C++20
    const_reverse_iterator crend() const noexcept;                                              // constexpr since C++20

    size_type size() const noexcept;                                                            // constexpr since C++20
    size_type length() const noexcept;                                                          // constexpr since C++20
    size_type max_size() const noexcept;                                                        // constexpr since C++20
    size_type capacity() const noexcept;                                                        // constexpr since C++20

    void resize(size_type n, value_type c);                                                     // constexpr since C++20
    void resize(size_type n);                                                                   // constexpr since C++20

    template<class Operation>
    constexpr void resize_and_overwrite(size_type n, Operation op); // since C++23

    void reserve(size_type res_arg);                                                            // constexpr since C++20
    void reserve();                                                                             // deprecated in C++20, removed in C++26
    void shrink_to_fit();                                                                       // constexpr since C++20
    void clear() noexcept;                                                                      // constexpr since C++20
    bool empty() const noexcept;                                                                // constexpr since C++20

    const_reference operator[](size_type pos) const;                                            // constexpr since C++20
    reference       operator[](size_type pos);                                                  // constexpr since C++20

    const_reference at(size_type n) const;                                                      // constexpr since C++20
    reference       at(size_type n);                                                            // constexpr since C++20

    basic_string& operator+=(const basic_string& str);                                          // constexpr since C++20
    template <class T>
        basic_string& operator+=(const T& t);                                                   // C++17, constexpr since C++20
    basic_string& operator+=(const value_type* s);                                              // constexpr since C++20
    basic_string& operator+=(value_type c);                                                     // constexpr since C++20
    basic_string& operator+=(std::initializer_list<value_type>);                                     // constexpr since C++20

    basic_string& append(const basic_string& str);                                              // constexpr since C++20
    template <class T>
        basic_string& append(const T& t);                                                       // C++17, constexpr since C++20
    basic_string& append(const basic_string& str, size_type pos, size_type n=npos);             // C++14, constexpr since C++20
    template <class T>
        basic_string& append(const T& t, size_type pos, size_type n=npos);                      // C++17, constexpr since C++20
    basic_string& append(const value_type* s, size_type n);                                     // constexpr since C++20
    basic_string& append(const value_type* s);                                                  // constexpr since C++20
    basic_string& append(size_type n, value_type c);                                            // constexpr since C++20
    template<class InputIterator>
        basic_string& append(InputIterator first, InputIterator last);                          // constexpr since C++20
    template<container-compatible-range<charT> R>
      constexpr basic_string& append_range(R&& rg);                                             // C++23
    basic_string& append(std::initializer_list<value_type>);                                         // constexpr since C++20

    void push_back(value_type c);                                                               // constexpr since C++20
    void pop_back();                                                                            // constexpr since C++20
    reference       front();                                                                    // constexpr since C++20
    const_reference front() const;                                                              // constexpr since C++20
    reference       back();                                                                     // constexpr since C++20
    const_reference back() const;                                                               // constexpr since C++20

    basic_string& assign(const basic_string& str);                                              // constexpr since C++20
    template <class T>
        basic_string& assign(const T& t);                                                       // C++17, constexpr since C++20
    basic_string& assign(basic_string&& str);                                                   // constexpr since C++20
    basic_string& assign(const basic_string& str, size_type pos, size_type n=npos);             // C++14, constexpr since C++20
    template <class T>
        basic_string& assign(const T& t, size_type pos, size_type n=npos);                      // C++17, constexpr since C++20
    basic_string& assign(const value_type* s, size_type n);                                     // constexpr since C++20
    basic_string& assign(const value_type* s);                                                  // constexpr since C++20
    basic_string& assign(size_type n, value_type c);                                            // constexpr since C++20
    template<class InputIterator>
        basic_string& assign(InputIterator first, InputIterator last);                          // constexpr since C++20
    template<container-compatible-range<charT> R>
      constexpr basic_string& assign_range(R&& rg);                                             // C++23
    basic_string& assign(std::initializer_list<value_type>);                                         // constexpr since C++20

    basic_string& insert(size_type pos1, const basic_string& str);                              // constexpr since C++20
    template <class T>
        basic_string& insert(size_type pos1, const T& t);                                       // constexpr since C++20
    basic_string& insert(size_type pos1, const basic_string& str,
                         size_type pos2, size_type n2=npos);                                    // constexpr since C++20
    template <class T>
        basic_string& insert(size_type pos1, const T& t, size_type pos2, size_type n=npos);     // C++17, constexpr since C++20
    basic_string& insert(size_type pos, const value_type* s, size_type n=npos);                 // C++14, constexpr since C++20
    basic_string& insert(size_type pos, const value_type* s);                                   // constexpr since C++20
    basic_string& insert(size_type pos, size_type n, value_type c);                             // constexpr since C++20
    iterator      insert(const_iterator p, value_type c);                                       // constexpr since C++20
    iterator      insert(const_iterator p, size_type n, value_type c);                          // constexpr since C++20
    template<class InputIterator>
        iterator insert(const_iterator p, InputIterator first, InputIterator last);             // constexpr since C++20
    template<container-compatible-range<charT> R>
      constexpr iterator insert_range(const_iterator p, R&& rg);                                // C++23
    iterator      insert(const_iterator p, std::initializer_list<value_type>);                       // constexpr since C++20

    basic_string& erase(size_type pos = 0, size_type n = npos);                                 // constexpr since C++20
    iterator      erase(const_iterator position);                                               // constexpr since C++20
    iterator      erase(const_iterator first, const_iterator last);                             // constexpr since C++20

    basic_string& replace(size_type pos1, size_type n1, const basic_string& str);               // constexpr since C++20
    template <class T>
    basic_string& replace(size_type pos1, size_type n1, const T& t);                            // C++17, constexpr since C++20
    basic_string& replace(size_type pos1, size_type n1, const basic_string& str,
                          size_type pos2, size_type n2=npos);                                   // C++14, constexpr since C++20
    template <class T>
        basic_string& replace(size_type pos1, size_type n1, const T& t,
                              size_type pos2, size_type n2=npos);                               // C++17, constexpr since C++20
    basic_string& replace(size_type pos, size_type n1, const value_type* s, size_type n2);      // constexpr since C++20
    basic_string& replace(size_type pos, size_type n1, const value_type* s);                    // constexpr since C++20
    basic_string& replace(size_type pos, size_type n1, size_type n2, value_type c);             // constexpr since C++20
    basic_string& replace(const_iterator i1, const_iterator i2, const basic_string& str);       // constexpr since C++20
    template <class T>
        basic_string& replace(const_iterator i1, const_iterator i2, const T& t);                // C++17, constexpr since C++20
    basic_string& replace(const_iterator i1, const_iterator i2, const value_type* s, size_type n); // constexpr since C++20
    basic_string& replace(const_iterator i1, const_iterator i2, const value_type* s);           // constexpr since C++20
    basic_string& replace(const_iterator i1, const_iterator i2, size_type n, value_type c);     // constexpr since C++20
    template<class InputIterator>
        basic_string& replace(const_iterator i1, const_iterator i2, InputIterator j1, InputIterator j2); // constexpr since C++20
    template<container-compatible-range<charT> R>
      constexpr basic_string& replace_with_range(const_iterator i1, const_iterator i2, R&& rg); // C++23
    basic_string& replace(const_iterator i1, const_iterator i2, std::initializer_list<value_type>);  // constexpr since C++20

    size_type copy(value_type* s, size_type n, size_type pos = 0) const;                        // constexpr since C++20
    basic_string substr(size_type pos = 0, size_type n = npos) const;                           // constexpr in C++20, removed in C++23
    basic_string substr(size_type pos = 0, size_type n = npos) const&;                          // since C++23
    constexpr basic_string substr(size_type pos = 0, size_type n = npos) &&;                    // since C++23
    void swap(basic_string& str)
        noexcept(allocator_traits<allocator_type>::propagate_on_container_swap::value ||
                 allocator_traits<allocator_type>::is_always_equal::value);                     // C++17, constexpr since C++20

    const value_type* c_str() const noexcept;                                                   // constexpr since C++20
    const value_type* data() const noexcept;                                                    // constexpr since C++20
          value_type* data()       noexcept;                                                    // C++17, constexpr since C++20

    allocator_type get_allocator() const noexcept;                                              // constexpr since C++20

    size_type find(const basic_string& str, size_type pos = 0) const noexcept;                  // constexpr since C++20
    template <class T>
        size_type find(const T& t, size_type pos = 0) const noexcept;                           // C++17, noexcept as an extension, constexpr since C++20
    size_type find(const value_type* s, size_type pos, size_type n) const noexcept;             // constexpr since C++20
    size_type find(const value_type* s, size_type pos = 0) const noexcept;                      // constexpr since C++20
    size_type find(value_type c, size_type pos = 0) const noexcept;                             // constexpr since C++20

    size_type rfind(const basic_string& str, size_type pos = npos) const noexcept;              // constexpr since C++20
    template <class T>
        size_type rfind(const T& t, size_type pos = npos) const noexcept;                       // C++17, noexcept as an extension, constexpr since C++20
    size_type rfind(const value_type* s, size_type pos, size_type n) const noexcept;            // constexpr since C++20
    size_type rfind(const value_type* s, size_type pos = npos) const noexcept;                  // constexpr since C++20
    size_type rfind(value_type c, size_type pos = npos) const noexcept;                         // constexpr since C++20

    size_type find_first_of(const basic_string& str, size_type pos = 0) const noexcept;         // constexpr since C++20
    template <class T>
        size_type find_first_of(const T& t, size_type pos = 0) const noexcept;                  // C++17, noexcept as an extension, constexpr since C++20
    size_type find_first_of(const value_type* s, size_type pos, size_type n) const noexcept;    // constexpr since C++20
    size_type find_first_of(const value_type* s, size_type pos = 0) const noexcept;             // constexpr since C++20
    size_type find_first_of(value_type c, size_type pos = 0) const noexcept;                    // constexpr since C++20

    size_type find_last_of(const basic_string& str, size_type pos = npos) const noexcept;       // constexpr since C++20
    template <class T>
        size_type find_last_of(const T& t, size_type pos = npos) const noexcept noexcept;       // C++17, noexcept as an extension, constexpr since C++20
    size_type find_last_of(const value_type* s, size_type pos, size_type n) const noexcept;     // constexpr since C++20
    size_type find_last_of(const value_type* s, size_type pos = npos) const noexcept;           // constexpr since C++20
    size_type find_last_of(value_type c, size_type pos = npos) const noexcept;                  // constexpr since C++20

    size_type find_first_not_of(const basic_string& str, size_type pos = 0) const noexcept;     // constexpr since C++20
    template <class T>
        size_type find_first_not_of(const T& t, size_type pos = 0) const noexcept;              // C++17, noexcept as an extension, constexpr since C++20
    size_type find_first_not_of(const value_type* s, size_type pos, size_type n) const noexcept; // constexpr since C++20
    size_type find_first_not_of(const value_type* s, size_type pos = 0) const noexcept;         // constexpr since C++20
    size_type find_first_not_of(value_type c, size_type pos = 0) const noexcept;                // constexpr since C++20

    size_type find_last_not_of(const basic_string& str, size_type pos = npos) const noexcept;   // constexpr since C++20
    template <class T>
        size_type find_last_not_of(const T& t, size_type pos = npos) const noexcept;            // C++17, noexcept as an extension, constexpr since C++20
    size_type find_last_not_of(const value_type* s, size_type pos, size_type n) const noexcept; // constexpr since C++20
    size_type find_last_not_of(const value_type* s, size_type pos = npos) const noexcept;       // constexpr since C++20
    size_type find_last_not_of(value_type c, size_type pos = npos) const noexcept;              // constexpr since C++20

    int compare(const basic_string& str) const noexcept;                                        // constexpr since C++20
    template <class T>
        int compare(const T& t) const noexcept;                                                 // C++17, noexcept as an extension, constexpr since C++20
    int compare(size_type pos1, size_type n1, const basic_string& str) const;                   // constexpr since C++20
    template <class T>
        int compare(size_type pos1, size_type n1, const T& t) const;                            // C++17, constexpr since C++20
    int compare(size_type pos1, size_type n1, const basic_string& str,
                size_type pos2, size_type n2=npos) const;                                       // C++14, constexpr since C++20
    template <class T>
        int compare(size_type pos1, size_type n1, const T& t,
                    size_type pos2, size_type n2=npos) const;                                   // C++17, constexpr since C++20
    int compare(const value_type* s) const noexcept;                                            // constexpr since C++20
    int compare(size_type pos1, size_type n1, const value_type* s) const;                       // constexpr since C++20
    int compare(size_type pos1, size_type n1, const value_type* s, size_type n2) const;         // constexpr since C++20

    constexpr bool starts_with(std::basic_string_view<charT, traits> sv) const noexcept;             // C++20
    constexpr bool starts_with(charT c) const noexcept;                                         // C++20
    constexpr bool starts_with(const charT* s) const;                                           // C++20
    constexpr bool ends_with(std::basic_string_view<charT, traits> sv) const noexcept;               // C++20
    constexpr bool ends_with(charT c) const noexcept;                                           // C++20
    constexpr bool ends_with(const charT* s) const;                                             // C++20

    constexpr bool contains(std::basic_string_view<charT, traits> sv) const noexcept;                // C++23
    constexpr bool contains(charT c) const noexcept;                                            // C++23
    constexpr bool contains(const charT* s) const;                                              // C++23
};

template<class InputIterator,
         class Allocator = allocator<typename iterator_traits<InputIterator>::value_type>>
basic_string(InputIterator, InputIterator, Allocator = Allocator())
   -> basic_string<typename iterator_traits<InputIterator>::value_type,
                  char_traits<typename iterator_traits<InputIterator>::value_type>,
                  Allocator>;   // C++17

template<ranges::input_range R,
         class Allocator = allocator<ranges::range_value_t<R>>>
  basic_string(from_range_t, R&&, Allocator = Allocator())
    -> basic_string<ranges::range_value_t<R>, char_traits<ranges::range_value_t<R>>,
                    Allocator>; // C++23

template<class charT,
         class traits,
         class Allocator = allocator<charT>>
  explicit basic_string(std::basic_string_view<charT, traits>, const Allocator& = Allocator())
    -> basic_string<charT, traits, Allocator>; // C++17

template<class charT,
         class traits,
         class Allocator = allocator<charT>>
  basic_string(std::basic_string_view<charT, traits>,
                typename see below::size_type, typename see below::size_type,
                const Allocator& = Allocator())
    -> basic_string<charT, traits, Allocator>; // C++17

template<class charT, class traits, class Allocator>
basic_string<charT, traits, Allocator>
operator+(const basic_string<charT, traits, Allocator>& lhs,
          const basic_string<charT, traits, Allocator>& rhs);                                   // constexpr since C++20

template<class charT, class traits, class Allocator>
basic_string<charT, traits, Allocator>
operator+(const charT* lhs , const basic_string<charT,traits,Allocator>&rhs);                   // constexpr since C++20

template<class charT, class traits, class Allocator>
basic_string<charT, traits, Allocator>
operator+(charT lhs, const basic_string<charT,traits,Allocator>& rhs);                          // constexpr since C++20

template<class charT, class traits, class Allocator>
basic_string<charT, traits, Allocator>
operator+(const basic_string<charT, traits, Allocator>& lhs, const charT* rhs);                 // constexpr since C++20

template<class charT, class traits, class Allocator>
basic_string<charT, traits, Allocator>
operator+(const basic_string<charT, traits, Allocator>& lhs, charT rhs);                        // constexpr since C++20

template<class charT, class traits, class Allocator>
  constexpr basic_string<charT, traits, Allocator>
    operator+(const basic_string<charT, traits, Allocator>& lhs,
              type_identity_t<std::basic_string_view<charT, traits>> rhs);                           // Since C++26
template<class charT, class traits, class Allocator>
  constexpr basic_string<charT, traits, Allocator>
    operator+(basic_string<charT, traits, Allocator>&& lhs,
              type_identity_t<std::basic_string_view<charT, traits>> rhs);                           // Since C++26
template<class charT, class traits, class Allocator>
  constexpr basic_string<charT, traits, Allocator>
    operator+(type_identity_t<std::basic_string_view<charT, traits>> lhs,
              const basic_string<charT, traits, Allocator>& rhs);                               // Since C++26
template<class charT, class traits, class Allocator>
  constexpr basic_string<charT, traits, Allocator>
    operator+(type_identity_t<std::basic_string_view<charT, traits>> lhs,
              basic_string<charT, traits, Allocator>&& rhs);                                    // Since C++26


template<class charT, class traits, class Allocator>
bool operator==(const basic_string<charT, traits, Allocator>& lhs,
                const basic_string<charT, traits, Allocator>& rhs) noexcept;                    // constexpr since C++20

template<class charT, class traits, class Allocator>
bool operator==(const charT* lhs, const basic_string<charT, traits, Allocator>& rhs) noexcept;  // removed in C++20

template<class charT, class traits, class Allocator>
bool operator==(const basic_string<charT,traits,Allocator>& lhs, const charT* rhs) noexcept;    // constexpr since C++20

template<class charT, class traits, class Allocator>
bool operator!=(const basic_string<charT,traits,Allocator>& lhs,
                const basic_string<charT, traits, Allocator>& rhs) noexcept;                    // removed in C++20

template<class charT, class traits, class Allocator>
bool operator!=(const charT* lhs, const basic_string<charT, traits, Allocator>& rhs) noexcept;  // removed in C++20

template<class charT, class traits, class Allocator>
bool operator!=(const basic_string<charT, traits, Allocator>& lhs, const charT* rhs) noexcept;  // removed in C++20

template<class charT, class traits, class Allocator>
bool operator< (const basic_string<charT, traits, Allocator>& lhs,
                const basic_string<charT, traits, Allocator>& rhs) noexcept;                    // removed in C++20

template<class charT, class traits, class Allocator>
bool operator< (const basic_string<charT, traits, Allocator>& lhs, const charT* rhs) noexcept;  // removed in C++20

template<class charT, class traits, class Allocator>
bool operator< (const charT* lhs, const basic_string<charT, traits, Allocator>& rhs) noexcept;  // removed in C++20

template<class charT, class traits, class Allocator>
bool operator> (const basic_string<charT, traits, Allocator>& lhs,
                const basic_string<charT, traits, Allocator>& rhs) noexcept;                    // removed in C++20

template<class charT, class traits, class Allocator>
bool operator> (const basic_string<charT, traits, Allocator>& lhs, const charT* rhs) noexcept;  // removed in C++20

template<class charT, class traits, class Allocator>
bool operator> (const charT* lhs, const basic_string<charT, traits, Allocator>& rhs) noexcept;  // removed in C++20

template<class charT, class traits, class Allocator>
bool operator<=(const basic_string<charT, traits, Allocator>& lhs,
                const basic_string<charT, traits, Allocator>& rhs) noexcept;                    // removed in C++20

template<class charT, class traits, class Allocator>
bool operator<=(const basic_string<charT, traits, Allocator>& lhs, const charT* rhs) noexcept;  // removed in C++20

template<class charT, class traits, class Allocator>
bool operator<=(const charT* lhs, const basic_string<charT, traits, Allocator>& rhs) noexcept;  // removed in C++20

template<class charT, class traits, class Allocator>
bool operator>=(const basic_string<charT, traits, Allocator>& lhs,
                const basic_string<charT, traits, Allocator>& rhs) noexcept;                    // removed in C++20

template<class charT, class traits, class Allocator>
bool operator>=(const basic_string<charT, traits, Allocator>& lhs, const charT* rhs) noexcept;  // removed in C++20

template<class charT, class traits, class Allocator>
bool operator>=(const charT* lhs, const basic_string<charT, traits, Allocator>& rhs) noexcept;  // removed in C++20

template<class charT, class traits, class Allocator>                                            // since C++20
constexpr see below operator<=>(const basic_string<charT, traits, Allocator>& lhs,
                                const basic_string<charT, traits, Allocator>& rhs) noexcept;

template<class charT, class traits, class Allocator>                                            // since C++20
constexpr see below operator<=>(const basic_string<charT, traits, Allocator>& lhs,
                                const charT* rhs) noexcept;

template<class charT, class traits, class Allocator>
void swap(basic_string<charT, traits, Allocator>& lhs,
          basic_string<charT, traits, Allocator>& rhs)
            noexcept(noexcept(lhs.swap(rhs)));                                                  // constexpr since C++20

template<class charT, class traits, class Allocator>
std::basic_istream<charT, traits>&
operator>>(std::basic_istream<charT, traits>& is, basic_string<charT, traits, Allocator>& str);

template<class charT, class traits, class Allocator>
std::basic_ostream<charT, traits>&
operator<<(std::basic_ostream<charT, traits>& os, const basic_string<charT, traits, Allocator>& str);

template<class charT, class traits, class Allocator>
std::basic_istream<charT, traits>&
getline(std::basic_istream<charT, traits>& is, basic_string<charT, traits, Allocator>& str,
        charT delim);

template<class charT, class traits, class Allocator>
std::basic_istream<charT, traits>&
getline(std::basic_istream<charT, traits>& is, basic_string<charT, traits, Allocator>& str);

template<class charT, class traits, class Allocator, class U>
constexpr typename basic_string<charT, traits, Allocator>::size_type
erase(basic_string<charT, traits, Allocator>& c, const U& value);    // C++20
template<class charT, class traits, class Allocator, class Predicate>
constexpr typename basic_string<charT, traits, Allocator>::size_type
erase_if(basic_string<charT, traits, Allocator>& c, Predicate pred); // C++20

typedef basic_string<char>    string;
typedef basic_string<wchar_t> wstring;
typedef basic_string<char8_t> u8string; // C++20
typedef basic_string<char16_t> u16string;
typedef basic_string<char32_t> u32string;

int                stoi  (const string& str, size_t* idx = nullptr, int base = 10);
long               stol  (const string& str, size_t* idx = nullptr, int base = 10);
unsigned long      stoul (const string& str, size_t* idx = nullptr, int base = 10);
long long          stoll (const string& str, size_t* idx = nullptr, int base = 10);
unsigned long long stoull(const string& str, size_t* idx = nullptr, int base = 10);

float       stof (const string& str, size_t* idx = nullptr);
double      stod (const string& str, size_t* idx = nullptr);
long double stold(const string& str, size_t* idx = nullptr);

string to_string(int val);
string to_string(unsigned val);
string to_string(long val);
string to_string(unsigned long val);
string to_string(long long val);
string to_string(unsigned long long val);
string to_string(float val);
string to_string(double val);
string to_string(long double val);

int                stoi  (const wstring& str, size_t* idx = nullptr, int base = 10);
long               stol  (const wstring& str, size_t* idx = nullptr, int base = 10);
unsigned long      stoul (const wstring& str, size_t* idx = nullptr, int base = 10);
long long          stoll (const wstring& str, size_t* idx = nullptr, int base = 10);
unsigned long long stoull(const wstring& str, size_t* idx = nullptr, int base = 10);

float       stof (const wstring& str, size_t* idx = nullptr);
double      stod (const wstring& str, size_t* idx = nullptr);
long double stold(const wstring& str, size_t* idx = nullptr);

wstring to_wstring(int val);
wstring to_wstring(unsigned val);
wstring to_wstring(long val);
wstring to_wstring(unsigned long val);
wstring to_wstring(long long val);
wstring to_wstring(unsigned long long val);
wstring to_wstring(float val);
wstring to_wstring(double val);
wstring to_wstring(long double val);

template <> struct hash<string>;
template <> struct hash<u8string>; // C++20
template <> struct hash<u16string>;
template <> struct hash<u32string>;
template <> struct hash<wstring>;

basic_string<char>     operator""s( const char *str,     size_t len );           // C++14, constexpr since C++20
basic_string<wchar_t>  operator""s( const wchar_t *str,  size_t len );           // C++14, constexpr since C++20
constexpr basic_string<char8_t>  operator""s( const char8_t *str,  size_t len ); // C++20
basic_string<char16_t> operator""s( const char16_t *str, size_t len );           // C++14, constexpr since C++20
basic_string<char32_t> operator""s( const char32_t *str, size_t len );           // C++14, constexpr since C++20

}  // std

*/

// clang-format on

#include <bit>
#include <limits>
#include <memory>
#include <string_view>
#include <type_traits>
#include <initializer_list>
#include <algorithm>
#include <iterator>
#include <utility>
#include <compare>
#include <charconv>
#include <concepts>

#include <cstdint>
#include <cstddef>
#include <cstdarg>

#include "plg/allocator.hpp"
#include "plg/guards.hpp"
#include "plg/concepts.hpp"

#ifndef PLUGIFY_STRING_NO_STD_FORMAT
#include "plg/format.hpp"
#endif

// Just in case, because we can't ignore some warnings from `-Wpedantic` (about zero size arrays and anonymous structs when gnu extensions are disabled) on gcc
#if PLUGIFY_COMPILER_CLANG
#  pragma clang system_header
#elif PLUGIFY_COMPILER_GCC
#  pragma GCC system_header
#endif

// from https://github.com/llvm/llvm-project/blob/main/libcxx/include/string
namespace plg {
	// basic_string
	template <class CharT, class Traits, class Allocator>
	class basic_string;

	template <class CharT, class Traits, class Allocator>
	constexpr basic_string<CharT, Traits, Allocator> concatenate_strings(
		const Allocator& alloc,
		std::type_identity_t<std::basic_string_view<CharT, Traits>> str1,
		std::type_identity_t<std::basic_string_view<CharT, Traits>> str2
	);

	template <class Iter>
	inline const bool string_is_trivial_iterator_v = false;

	template <class T>
	concept string_is_trivial_iterator = std::is_arithmetic_v<T>;

	template <class T, class CharT, class Traits>
	concept string_view_convertible = std::is_convertible_v<const T&, std::basic_string_view<CharT, Traits>> && !std::is_convertible_v<const T&, const CharT*>;

	// second concept = the above, but exclude std::basic_string itself
	template <class T, class CharT, class Traits, class Allocator>
	concept string_view_convertible_with_exceptiom = string_view_convertible<T, CharT, Traits> && !std::is_same_v<std::remove_cvref_t<T>, basic_string<CharT, Traits, Allocator>>;

	template <typename T>
	concept string_like = requires(T v) {
		{ std::string_view(v) };
	};

	template <size_t N>
	struct padding {
		char _pad[N];
	};

	template <>
	struct padding<0> {};

	struct uninitialized_size_tag {};

	struct init_with_sentinel_tag {};

	template <class CharT, class Traits = std::char_traits<CharT>, class Allocator = allocator<CharT>>
	class basic_string {
	public:
		// using self = std::basic_string<CharT, Traits, Allocator>;
		using self_view = std::basic_string_view<CharT, Traits>;
		using traits_type = Traits;
		using value_type = CharT;
		using allocator_type = Allocator;
		using alloc_traits = std::allocator_traits<allocator_type>;
		using size_type = typename alloc_traits::size_type;
		using difference_type = typename alloc_traits::difference_type;
		using reference = value_type&;
		using const_reference = const value_type&;
		using pointer = typename alloc_traits::pointer;
		using const_pointer = typename alloc_traits::const_pointer;

		// A basic_string contains the following members which may be trivially relocatable:
		// - pointer: is currently assumed to be trivially relocatable, but is still checked in case
		// that changes
		// - size_type: is always trivially relocatable, since it has to be an integral type
		// - value_type: is always trivially relocatable, since it has to be trivial
		// - unsigned char: is a fundamental type, so it's trivially relocatable
		// - allocator_type: may or may not be trivially relocatable, so it's checked
		//
		// This string implementation doesn't contain any references into itself. It only contains a
		// bit that says whether it is in small or large string mode, so the entire structure is
		// trivially relocatable if its members are.
#if PLUGIFY_INSTRUMENTED_WITH_ASAN
		// When compiling with AddressSanitizer (ASan), basic_string cannot be trivially
		// relocatable. Because the object's memory might be poisoned when its content
		// is kept inside objects memory (short string optimization), instead of in allocated
		// external memory. In such cases, the destructor is responsible for unpoisoning
		// the memory to avoid triggering false positives.
		// Therefore it's crucial to ensure the destructor is called.
		//
		// However, it is replaceable since implementing move-assignment as a destroy +
		// move-construct will maintain the right ASAN state.
		using trivially_relocatable = void;
#else
		using trivially_relocatable = std::conditional_t<
				is_trivially_relocatable<allocator_type>::value &&
				is_trivially_relocatable<pointer>::value,
				basic_string,
				void>;
#endif // PLUGIFY_INSTRUMENTED_WITH_ASAN

#if PLUGIFY_INSTRUMENTED_WITH_ASAN && __has_cpp_attribute(__no_sanitize__)
#  define PLUGIFY_INTERNAL_MEMORY_ACCESS __attribute__((__no_sanitize__("address")))
// This macro disables AddressSanitizer (ASan) instrumentation for a specific function,
// allowing memory accesses that would normally trigger ASan errors to proceed without crashing.
// This is useful for accessing parts of objects memory, which should not be accessed,
// such as unused bytes in short strings, that should never be accessed
// by other parts of the program.
#else
#  define PLUGIFY_INTERNAL_MEMORY_ACCESS
#endif // PLUGIFY_INSTRUMENTED_WITH_ASAN

#if PLUGIFY_INSTRUMENTED_WITH_ASAN
		constexpr pointer asan_volatile_wrapper(pointer const& ptr) const {
			if (std::is_constant_evaluated()) {
				return ptr;
			}

			volatile pointer copy_ptr = ptr;

			return const_cast<pointer&>(copy_ptr);
		}

		constexpr const_pointer asan_volatile_wrapper(const const_pointer& ptr) const {
			if (std::is_constant_evaluated()) {
				return ptr;
			}

			volatile const_pointer copy_ptr = ptr;

			return const_cast<const_pointer&>(copy_ptr);
		}

#  define PLUGIFY_ASAN_VOLATILE_WRAPPER(PTR) asan_volatile_wrapper(PTR)
#else
#  define PLUGIFY_ASAN_VOLATILE_WRAPPER(PTR) PTR
#endif // PLUGIFY_INSTRUMENTED_WITH_ASAN

		static_assert(!std::is_array_v<value_type>, "Character type of basic_string must not be an array");
		static_assert(
			std::is_standard_layout_v<value_type>,
			"Character type of basic_string must be standard-layout"
		);
		static_assert(
			std::is_trivially_default_constructible_v<value_type>,
			"Character type of basic_string must be trivially default constructible"
		);
		static_assert(
			std::is_trivially_copyable_v<value_type>,
			"Character type of basic_string must be trivially copyable"
		);
		static_assert(
			std::is_same_v<CharT, typename traits_type::char_type>,
			"traits_type::char_type must be the same type as CharT"
		);
		static_assert(
			std::is_same_v<typename allocator_type::value_type, value_type>,
			"Allocator::value_type must be same type as value_type"
		);
		// static_assert(std::check_valid_allocator<allocator_type>::value, "");

		using iterator = pointer;
		using const_iterator = const_pointer;
		using reverse_iterator = std::reverse_iterator<iterator>;
		using const_reverse_iterator = std::reverse_iterator<const_iterator>;

		using alloc_result = allocation_result<pointer, size_type>;

	private:
		static constexpr int char_bit = std::numeric_limits<uint8_t>::digits + std::numeric_limits<uint8_t>::is_signed;
		static_assert(char_bit == 8, "This implementation assumes that one byte contains 8 bits");

		struct long_ {
			constexpr long_() = default;

			constexpr long_(alloc_result alloc, size_type size)
				: _data(alloc.ptr)
				, _size(size)
				, _cap(alloc.count / endian_factor)
				, _is_long(true) {
				PLUGIFY_ASSERT(!fits_in_sso(alloc.count), "Long capacity should always be larger than the SSO");
			}

			pointer _data;
			size_type _size;
			size_type _cap : sizeof(size_type) * char_bit - 1;
			size_type _is_long : 1;
		};

		static constexpr size_type min_cap = ((sizeof(long_) - 1) / sizeof(value_type) > 2 ? (sizeof(long_) - 1) / sizeof(value_type) : 2) + 1;

		struct short_ {
			constexpr short_()
				: _data{}
				, _spare_size(min_cap - 1)
				, _is_long(false) {
			}

			value_type _data[min_cap - 1];
			PLUGIFY_NO_UNIQUE_ADDRESS padding<sizeof(value_type) - 1> _padding;
			uint8_t _spare_size : 7;
			uint8_t _is_long : 1;
		};

		// The endian_factor is required because the field we use to store the size
		// has one fewer bit than it would if it were not a bitfield.
		//
		// If the LSB is used to store the short-flag in the short string representation,
		// we have to multiply the size by two when it is stored and divide it by two when
		// it is loaded to make sure that we always store an even number. In the long string
		// representation, we can ignore this because we can assume that we always allocate
		// an even amount of value_types.
		//
		// If the MSB is used for the short-flag, the max_size() is numeric_limits<size_type>::max()
		// / 2. This does not impact the short string representation, since we never need the MSB
		// for representing the size of a short string anyway.

		static constexpr size_type endian_factor = std::endian::native == std::endian::big ? 2 : 1;

		static_assert(
			sizeof(short_) == (sizeof(value_type) * min_cap),
			"short has an unexpected size."
		);
		static_assert(
			sizeof(short_) == sizeof(long_),
			"short and long layout structures must be the same size"
		);

		union rep {
			short_ s{};
			long_ l;
		} _rep;
		PLUGIFY_NO_UNIQUE_ADDRESS
		allocator_type _alloc;

		// annotate the string with its size() at scope exit. The string has to be in a valid state
		// at that point.
		struct annotate_new_size {
			basic_string& _str;

			constexpr explicit annotate_new_size(basic_string& str)
				: _str(str) {
			}

			constexpr void operator()() {
				_str.annotate_new(_str.size());
			}
		};

		// Construct a string with the given allocator and enough storage to hold `size` characters,
		// but don't initialize the characters. The contents of the string, including the null
		// terminator, must be initialized separately.
		constexpr /*explicit*/ basic_string(uninitialized_size_tag, size_type size, const allocator_type& a)
			: _alloc(a) {
			init_internal_buffer(size);
		}

		template <class Iter, class Sent>
		constexpr basic_string(init_with_sentinel_tag, Iter first, Sent last, const allocator_type& a)
			: _alloc(a) {
			init_with_sentinel(std::move(first), std::move(last));
		}

		constexpr iterator make_iterator(pointer p) {
			return iterator(p);
		}

		constexpr const_iterator make_const_iterator(const_pointer p) const {
			return const_iterator(p);
		}

	public:
		static const size_type npos = static_cast<size_type>(-1);

		constexpr basic_string() noexcept(std::is_nothrow_default_constructible_v<allocator_type>)
			: _rep(short_()) {
			annotate_new(0);
		}

		constexpr /*explicit*/ basic_string(const allocator_type& a) noexcept
			: _rep(short_())
			, _alloc(a) {
			annotate_new(0);
		}

		constexpr PLUGIFY_INTERNAL_MEMORY_ACCESS basic_string(const basic_string& str)
			: _alloc(alloc_traits::select_on_container_copy_construction(str._alloc)) {
			if (!str.is_long()) {
				_rep = str._rep;
				annotate_new(get_short_size());
			} else {
				init_copy_ctor_external(std::to_address(str.get_long_pointer()), str.get_long_size());
			}
		}

		constexpr PLUGIFY_INTERNAL_MEMORY_ACCESS
		basic_string(const basic_string& str, const allocator_type& a)
			: _alloc(a) {
			if (!str.is_long()) {
				_rep = str._rep;
				annotate_new(get_short_size());
			} else {
				init_copy_ctor_external(std::to_address(str.get_long_pointer()), str.get_long_size());
			}
		}

		constexpr basic_string(basic_string&& str) noexcept
			// Turning off ASan instrumentation for variable initialization with
			// PLUGIFY_INTERNAL_MEMORY_ACCESS does not work consistently during
			// initialization of r_, so we instead unpoison str's memory manually first. str's
			// memory needs to be unpoisoned only in the case where it's a short string.
			: _rep([](basic_string& s) -> decltype(s._rep)&& {
				if (!s.is_long()) {
					s.annotate_delete();
				}
				return std::move(s._rep);
			}(str))
			, _alloc(std::move(str._alloc)) {
			str._rep = rep();
			str.annotate_new(0);
			if (!is_long()) {
				annotate_new(size());
			}
		}

		constexpr basic_string(basic_string&& str, const allocator_type& a)
			: _alloc(a) {
			if (str.is_long() && a != str._alloc) {	 // copy, not move
				init(std::to_address(str.get_long_pointer()), str.get_long_size());
			} else {
				if (std::is_constant_evaluated()) {
					_rep = rep();
				}
				if (!str.is_long()) {
					str.annotate_delete();
				}
				_rep = str._rep;
				str._rep = rep();
				str.annotate_new(0);
				if (!is_long() && this != std::addressof(str)) {
					annotate_new(size());
				}
			}
		}

		constexpr basic_string(const CharT* PLUGIFY_NO_NULL s)
			requires(is_allocator<Allocator>)
		{
			PLUGIFY_ASSERT(s != nullptr, "basic_string(const char*) detected nullptr");
			init(s, traits_type::length(s));
		}

		constexpr basic_string(const CharT* PLUGIFY_NO_NULL s, const Allocator& a)
			requires(is_allocator<Allocator>)
			: _alloc(a) {
			PLUGIFY_ASSERT(s != nullptr, "basic_string(const char*, allocator) detected nullptr");
			init(s, traits_type::length(s));
		}

		basic_string(std::nullptr_t) = delete;

		constexpr basic_string(const CharT* s, size_type n) {
			PLUGIFY_ASSERT(n == 0 || s != nullptr, "basic_string(const char*, n) detected nullptr");
			init(s, n);
		}

		constexpr basic_string(const CharT* s, size_type n, const Allocator& a)
			: _alloc(a) {
			PLUGIFY_ASSERT(
				n == 0 || s != nullptr,
				"basic_string(const char*, n, allocator) detected nullptr"
			);
			init(s, n);
		}

		constexpr basic_string(size_type n, CharT c) {
			init(n, c);
		}

		constexpr basic_string(basic_string&& str, size_type pos, const Allocator& alloc = Allocator())
			: basic_string(std::move(str), pos, npos, alloc) {
		}

		constexpr basic_string(
			basic_string&& str,
			size_type pos,
			size_type n,
			const Allocator& alloc = Allocator()
		)
			: _alloc(alloc) {
			if (pos > str.size()) {
				this->throw_out_of_range();
			}

			auto len = std::min<size_type>(n, str.size() - pos);
			if (alloc_traits::is_always_equal::value || alloc == str._alloc) {
				move_assign(std::move(str), pos, len);
			} else {
				// Perform a copy because the allocators are not compatible.
				init(str.data() + pos, len);
			}
		}

		constexpr basic_string(size_type n, CharT c, const Allocator& a)
			requires(is_allocator<Allocator>)
			: _alloc(a) {
			init(n, c);
		}

		constexpr basic_string(
			const basic_string& str,
			size_type pos,
			size_type n,
			const Allocator& a = Allocator()
		)
			: _alloc(a) {
			size_type str_sz = str.size();
			if (pos > str_sz) {
				this->throw_out_of_range();
			}
			init(str.data() + pos, std::min(n, str_sz - pos));
		}

		constexpr basic_string(const basic_string& str, size_type pos, const Allocator& a = Allocator())
			: _alloc(a) {
			size_type str_sz = str.size();
			if (pos > str_sz) {
				this->throw_out_of_range();
			}
			init(str.data() + pos, str_sz - pos);
		}

		template <string_view_convertible_with_exceptiom<CharT, Traits, Allocator> T>
		constexpr basic_string(
			const T& t,
			size_type pos,
			size_type n,
			const allocator_type& a = allocator_type()
		)
			: _alloc(a) {
			self_view sv0 = t;
			self_view sv = sv0.substr(pos, n);
			init(sv.data(), sv.size());
		}

		template <string_view_convertible_with_exceptiom<CharT, Traits, Allocator> T>
		constexpr /*explicit*/ basic_string(const T& t) {
			self_view sv = t;
			init(sv.data(), sv.size());
		}

		template <string_view_convertible_with_exceptiom<CharT, Traits, Allocator> T>
		constexpr /*explicit*/ basic_string(const T& t, const allocator_type& a)
			: _alloc(a) {
			self_view sv = t;
			init(sv.data(), sv.size());
		}

		template <std::input_iterator InputIterator>
		constexpr basic_string(InputIterator first, InputIterator last) {
			init(first, last);
		}

		template <std::input_iterator InputIterator>
		constexpr basic_string(InputIterator first, InputIterator last, const allocator_type& a)
			: _alloc(a) {
			init(first, last);
		}

#if PLUGIFY_HAS_CXX23
		template <container_compatible_range<CharT> Range>
		constexpr basic_string(std::from_range_t, Range&& range, const allocator_type& a = allocator_type())
			: _alloc(a) {
			if constexpr (std::ranges::forward_range<Range> || std::ranges::sized_range<Range>) {
				init_with_size(
					std::ranges::begin(range),
					std::ranges::end(range),
					std::ranges::distance(range)
				);
			} else {
				init_with_sentinel(std::ranges::begin(range), std::ranges::end(range));
			}
		}
#endif

		constexpr basic_string(std::initializer_list<CharT> il) {
			init(il.begin(), il.end());
		}

		constexpr basic_string(std::initializer_list<CharT> il, const Allocator& a)
			: _alloc(a) {
			init(il.begin(), il.end());
		}

		inline constexpr ~basic_string() {
			reset_internal_buffer();
		}

		constexpr operator self_view() const noexcept {
			return self_view(begin(), end());
		}

		constexpr PLUGIFY_INTERNAL_MEMORY_ACCESS basic_string& operator=(const basic_string& str);

		template <string_view_convertible_with_exceptiom<CharT, Traits, Allocator> T>
		constexpr basic_string& operator=(const T& t) {
			self_view sv = t;
			return assign(sv);
		}

		constexpr basic_string& operator=(basic_string&& str
		) noexcept(alloc_traits::propagate_on_container_move_assignment::value) {
			move_assign(
				str,
				std::integral_constant<bool, alloc_traits::propagate_on_container_move_assignment::value>()
			);
			return *this;
		}

		constexpr basic_string& operator=(std::initializer_list<value_type> il) {
			return assign(il.begin(), il.size());
		}

		constexpr basic_string& operator=(const value_type* PLUGIFY_NO_NULL s) {
			return assign(s);
		}

		basic_string& operator=(std::nullptr_t) = delete;
		constexpr basic_string& operator=(value_type c);

		constexpr iterator begin() noexcept {
			return make_iterator(get_pointer());
		}

		constexpr const_iterator begin() const noexcept {
			return make_const_iterator(get_pointer());
		}

		constexpr iterator end() noexcept {
			return make_iterator(get_pointer() + size());
		}

		constexpr const_iterator end() const noexcept {
			return make_const_iterator(get_pointer() + size());
		}

		constexpr reverse_iterator rbegin() noexcept {
			return reverse_iterator(end());
		}

		constexpr const_reverse_iterator rbegin() const noexcept {
			return const_reverse_iterator(end());
		}

		constexpr reverse_iterator rend() noexcept {
			return reverse_iterator(begin());
		}

		constexpr const_reverse_iterator rend() const noexcept {
			return const_reverse_iterator(begin());
		}

		constexpr const_iterator cbegin() const noexcept {
			return begin();
		}

		constexpr const_iterator cend() const noexcept {
			return end();
		}

		constexpr const_reverse_iterator crbegin() const noexcept {
			return rbegin();
		}

		constexpr const_reverse_iterator crend() const noexcept {
			return rend();
		}

		constexpr size_type size() const noexcept {
			return is_long() ? get_long_size() : get_short_size();
		}

		constexpr size_type length() const noexcept {
			return size();
		}

		constexpr size_type max_size() const noexcept {
			constexpr bool uses_lsb = endian_factor == 2;

			if (size_type m = alloc_traits::max_size(_alloc);
				m <= std::numeric_limits<size_type>::max() / 2) {
				size_type res = m - alignment;

				// When the endian_factor == 2, our string representation assumes that the capacity
				// (including the null terminator) is always even, so we have to make sure the
				// lowest bit isn't set when the string grows to max_size()
				if constexpr (uses_lsb) {
					res &= ~size_type(1);
				}

				// We have to allocate space for the null terminator, but max_size() doesn't include
				// it.
				return res - 1;
			} else {
				return uses_lsb ? m - alignment - 1 : (m / 2) - alignment - 1;
			}
		}

		constexpr size_type capacity() const noexcept {
			return (is_long() ? get_long_cap() : min_cap) - 1;
		}

		constexpr void resize(size_type n, value_type c);

		constexpr void resize(size_type n) {
			resize(n, value_type());
		}

		constexpr void reserve(size_type requested_capacity);

#if PLUGIFY_HAS_CXX23
		/*template <class Op>
		constexpr void resize_and_overwrite(size_type n, Op op) {
			size_type sz = size();
			size_type cap = capacity();
			if (n > cap) {
				grow_by_without_replace(cap, n - cap, sz, sz, 0);
			}
			annotate_delete();
			set_size(n);
			annotate_new(n);
			erase_to_end(std::move(op)(data(), auto(n)));
		}*/
#endif

		constexpr void shrink_to_fit() noexcept;
		constexpr void clear() noexcept;

		[[nodiscard]] constexpr bool empty() const noexcept {
			return size() == 0;
		}

		constexpr const_reference operator[](size_type pos) const noexcept {
			PLUGIFY_ASSERT(pos <= size(), "string index out of bounds");
			if (__builtin_constant_p(pos) && !fits_in_sso(pos)) {
				return *(get_long_pointer() + pos);
			}
			return *(data() + pos);
		}

		constexpr reference operator[](size_type pos) noexcept {
			PLUGIFY_ASSERT(pos <= size(), "string index out of bounds");
			if (__builtin_constant_p(pos) && !fits_in_sso(pos)) {
				return *(get_long_pointer() + pos);
			}
			return *(get_pointer() + pos);
		}

		constexpr const_reference at(size_type n) const;
		constexpr reference at(size_type n);

		constexpr basic_string& operator+=(const basic_string& str) {
			return append(str);
		}

		template <string_view_convertible_with_exceptiom<CharT, Traits, Allocator> T>
		constexpr basic_string& operator+=(const T& t) {
			self_view sv = t;
			return append(sv);
		}

		constexpr basic_string& operator+=(const value_type* PLUGIFY_NO_NULL s) {
			return append(s);
		}

		constexpr basic_string& operator+=(value_type c) {
			push_back(c);
			return *this;
		}

		constexpr basic_string& operator+=(std::initializer_list<value_type> il) {
			return append(il);
		}

		constexpr basic_string& append(const basic_string& str) {
			return append(str.data(), str.size());
		}

		template <string_view_convertible_with_exceptiom<CharT, Traits, Allocator> T>
		constexpr basic_string& append(const T& t) {
			self_view sv = t;
			return append(sv.data(), sv.size());
		}

		constexpr basic_string& append(const basic_string& str, size_type pos, size_type n = npos);

		template <string_view_convertible_with_exceptiom<CharT, Traits, Allocator> T>
		constexpr basic_string& append(const T& t, size_type pos, size_type n = npos) {
			self_view sv = t;
			size_type sz = sv.size();
			if (pos > sz) {
				throw_out_of_range();
			}
			return append(sv.data() + pos, std::min(n, sz - pos));
		}

		constexpr basic_string& append(const value_type* s, size_type n);
		constexpr basic_string& append(const value_type* PLUGIFY_NO_NULL s);
		constexpr basic_string& append(size_type n, value_type c);

		template <std::input_iterator InputIterator>
		constexpr basic_string& append(InputIterator first, InputIterator last) {
			const basic_string temp(first, last, _alloc);
			append(temp.data(), temp.size());
			return *this;
		}

		template <std::forward_iterator ForwardIterator>
		constexpr basic_string& append(ForwardIterator first, ForwardIterator last) {
			size_type sz = size();
			size_type cap = capacity();
			size_type n = static_cast<size_type>(std::distance(first, last));
			if (n == 0) {
				return *this;
			}

			if (string_is_trivial_iterator_v<ForwardIterator> && !addr_in_range(*first)) {
				if (cap - sz < n) {
					grow_by_without_replace(cap, sz + n - cap, sz, sz, 0);
				}
				annotate_increase(n);
				auto end = copy_non_overlapping_range(first, last, std::to_address(get_pointer() + sz));
				traits_type::assign(*end, value_type());
				set_size(sz + n);
				return *this;
			} else {
				const basic_string temp(first, last, _alloc);
				return append(temp.data(), temp.size());
			}
		}

#if PLUGIFY_HAS_CXX23
		template <container_compatible_range<CharT> Range>
		constexpr basic_string& append_range(Range&& range) {
			insert_range(end(), std::forward<Range>(range));
			return *this;
		}
#endif

		constexpr basic_string& append(std::initializer_list<value_type> il) {
			return append(il.begin(), il.size());
		}

		constexpr void push_back(value_type c);
		constexpr void pop_back();

		constexpr reference front() noexcept {
			PLUGIFY_ASSERT(!empty(), "string::front(): string is empty");
			return *get_pointer();
		}

		constexpr const_reference front() const noexcept {
			PLUGIFY_ASSERT(!empty(), "string::front(): string is empty");
			return *data();
		}

		constexpr reference back() noexcept {
			PLUGIFY_ASSERT(!empty(), "string::back(): string is empty");
			return *(get_pointer() + size() - 1);
		}

		constexpr const_reference back() const noexcept {
			PLUGIFY_ASSERT(!empty(), "string::back(): string is empty");
			return *(data() + size() - 1);
		}

		template <string_view_convertible<CharT, Traits> T>
		constexpr basic_string& assign(const T& t) {
			self_view sv = t;
			return assign(sv.data(), sv.size());
		}

		constexpr void move_assign(basic_string&& str, size_type pos, size_type len) {
			// Pilfer the allocation from str.
			PLUGIFY_ASSERT(_alloc == str._alloc, "move_assign called with wrong allocator");
			size_type old_sz = str.size();
			if (!str.is_long()) {
				str.annotate_delete();
			}
			_rep = str._rep;
			str._rep = rep();
			str.annotate_new(0);

			Traits::move(data(), data() + pos, len);
			set_size(len);
			Traits::assign(data()[len], value_type());

			if (!is_long()) {
				annotate_new(len);
			} else if (old_sz > len) {
				annotate_shrink(old_sz);
			}
		}

		constexpr basic_string& assign(const basic_string& str) {
			return *this = str;
		}

		constexpr basic_string&
		assign(basic_string&& str) noexcept(alloc_traits::propagate_on_container_move_assignment::value
		) {
			*this = std::move(str);
			return *this;
		}

		constexpr basic_string& assign(const basic_string& str, size_type pos, size_type n = npos);

		template <string_view_convertible_with_exceptiom<CharT, Traits, Allocator> T>
		constexpr basic_string& assign(const T& t, size_type pos, size_type n = npos) {
			self_view sv = t;
			size_type sz = sv.size();
			if (pos > sz) {
				throw_out_of_range();
			}
			return assign(sv.data() + pos, std::min(n, sz - pos));
		}

		constexpr basic_string& assign(const value_type* s, size_type n);
		constexpr basic_string& assign(const value_type* PLUGIFY_NO_NULL s);
		constexpr basic_string& assign(size_type n, value_type c);

		template <std::input_iterator InputIterator>
		constexpr basic_string& assign(InputIterator first, InputIterator last) {
			assign_with_sentinel(first, last);
			return *this;
		}

		template <std::forward_iterator ForwardIterator>
		constexpr basic_string& assign(ForwardIterator first, ForwardIterator last) {
			if (string_is_trivial_iterator_v<ForwardIterator>) {
				size_type n = static_cast<size_type>(std::distance(first, last));
				assign_trivial(first, last, n);
			} else {
				assign_with_sentinel(first, last);
			}

			return *this;
		}

#if PLUGIFY_HAS_CXX23
		template <container_compatible_range<CharT> Range>
		constexpr basic_string& assign_range(Range&& range) {
			if constexpr (string_is_trivial_iterator_v<std::ranges::iterator_t<Range>>
						  && (std::ranges::forward_range<Range> || std::ranges::sized_range<Range>) ) {
				size_type n = static_cast<size_type>(std::ranges::distance(range));
				assign_trivial(std::ranges::begin(range), std::ranges::end(range), n);

			} else {
				assign_with_sentinel(std::ranges::begin(range), std::ranges::end(range));
			}

			return *this;
		}
#endif

		constexpr basic_string& assign(std::initializer_list<value_type> il) {
			return assign(il.begin(), il.size());
		}

		constexpr basic_string& insert(size_type pos1, const basic_string& str) {
			return insert(pos1, str.data(), str.size());
		}

		template <string_view_convertible<CharT, Traits> T>
		constexpr basic_string& insert(size_type pos1, const T& t) {
			self_view sv = t;
			return insert(pos1, sv.data(), sv.size());
		}

		template <string_view_convertible_with_exceptiom<CharT, Traits, Allocator> T>
		constexpr basic_string&
		insert(size_type pos1, const T& t, size_type pos2, size_type n = npos) {
			self_view sv = t;
			size_type str_sz = sv.size();
			if (pos2 > str_sz) {
				throw_out_of_range();
			}
			return insert(pos1, sv.data() + pos2, std::min(n, str_sz - pos2));
		}

		constexpr basic_string&
		insert(size_type pos1, const basic_string& str, size_type pos2, size_type n = npos);
		constexpr basic_string& insert(size_type pos, const value_type* s, size_type n);
		constexpr basic_string& insert(size_type pos, const value_type* PLUGIFY_NO_NULL s);
		constexpr basic_string& insert(size_type pos, size_type n, value_type c);
		constexpr iterator insert(const_iterator pos, value_type c);

#if PLUGIFY_HAS_CXX23
		template <container_compatible_range<CharT> Range>
		constexpr iterator insert_range(const_iterator position, Range&& range) {
			if constexpr (std::ranges::forward_range<Range> || std::ranges::sized_range<Range>) {
				auto n = static_cast<size_type>(std::ranges::distance(range));
				return insert_with_size(position, std::ranges::begin(range), std::ranges::end(range), n);

			} else {
				basic_string temp(std::from_range, std::forward<Range>(range), _alloc);
				return insert(position, temp.data(), temp.data() + temp.size());
			}
		}
#endif

		constexpr iterator insert(const_iterator pos, size_type n, value_type c) {
			difference_type p = pos - begin();
			insert(static_cast<size_type>(p), n, c);
			return begin() + p;
		}

		template <std::input_iterator InputIterator>
		constexpr iterator insert(const_iterator pos, InputIterator first, InputIterator last) {
			const basic_string temp(first, last, _alloc);
			return insert(pos, temp.data(), temp.data() + temp.size());
		}

		template <std::forward_iterator ForwardIterator>
		constexpr iterator insert(const_iterator pos, ForwardIterator first, ForwardIterator last) {
			auto n = static_cast<size_type>(std::distance(first, last));
			return insert_with_size(pos, first, last, n);
		}

		constexpr iterator insert(const_iterator pos, std::initializer_list<value_type> il) {
			return insert(pos, il.begin(), il.end());
		}

		constexpr basic_string& erase(size_type pos = 0, size_type n = npos);
		constexpr iterator erase(const_iterator pos);
		constexpr iterator erase(const_iterator first, const_iterator last);

		constexpr basic_string& replace(size_type pos1, size_type n1, const basic_string& str) {
			return replace(pos1, n1, str.data(), str.size());
		}

		template <string_view_convertible<CharT, Traits> T>
		constexpr basic_string& replace(size_type pos1, size_type n1, const T& t) {
			self_view sv = t;
			return replace(pos1, n1, sv.data(), sv.size());
		}

		constexpr basic_string&
		replace(size_type pos1, size_type n1, const basic_string& str, size_type pos2, size_type n2 = npos);

		template <string_view_convertible_with_exceptiom<CharT, Traits, Allocator> T>
		constexpr basic_string&
		replace(size_type pos1, size_type n1, const T& t, size_type pos2, size_type n2 = npos) {
			self_view sv = t;
			size_type str_sz = sv.size();
			if (pos2 > str_sz) {
				throw_out_of_range();
			}
			return replace(pos1, n1, sv.data() + pos2, std::min(n2, str_sz - pos2));
		}

		constexpr basic_string&
		replace(size_type pos, size_type n1, const value_type* s, size_type n2);
		constexpr basic_string&
		replace(size_type pos, size_type n1, const value_type* PLUGIFY_NO_NULL s);
		constexpr basic_string& replace(size_type pos, size_type n1, size_type n2, value_type c);

		constexpr basic_string&
		replace(const_iterator i1, const_iterator i2, const basic_string& str) {
			return replace(
				static_cast<size_type>(i1 - begin()),
				static_cast<size_type>(i2 - i1),
				str.data(),
				str.size()
			);
		}

		template <string_view_convertible<CharT, Traits> T>
		constexpr basic_string& replace(const_iterator i1, const_iterator i2, const T& t) {
			self_view sv = t;
			return replace(i1 - begin(), i2 - i1, sv);
		}

		constexpr basic_string&
		replace(const_iterator i1, const_iterator i2, const value_type* s, size_type n) {
			return replace(static_cast<size_type>(i1 - begin()), static_cast<size_type>(i2 - i1), s, n);
		}

		constexpr basic_string& replace(const_iterator i1, const_iterator i2, const value_type* s) {
			return replace(static_cast<size_type>(i1 - begin()), static_cast<size_type>(i2 - i1), s);
		}

		constexpr basic_string&
		replace(const_iterator i1, const_iterator i2, size_type n, value_type c) {
			return replace(static_cast<size_type>(i1 - begin()), static_cast<size_type>(i2 - i1), n, c);
		}

		template <std::input_iterator InputIterator>
		constexpr basic_string&
		replace(const_iterator i1, const_iterator i2, InputIterator j1, InputIterator j2) {
			const basic_string temp(j1, j2, _alloc);
			return replace(i1, i2, temp);
		}

#if PLUGIFY_HAS_CXX23
		template <container_compatible_range<CharT> Range>
		constexpr basic_string&
		replace_with_range(const_iterator i1, const_iterator i2, Range&& range) {
			basic_string temp(std::from_range, std::forward<Range>(range), _alloc);
			return replace(i1, i2, temp);
		}
#endif

		constexpr basic_string&
		replace(const_iterator i1, const_iterator i2, std::initializer_list<value_type> il) {
			return replace(i1, i2, il.begin(), il.end());
		}

		constexpr size_type copy(value_type* s, size_type n, size_type pos = 0) const;

		constexpr basic_string substr(size_type pos = 0, size_type n = npos) const& {
			return basic_string(*this, pos, n);
		}

		constexpr basic_string substr(size_type pos = 0, size_type n = npos) && {
			return basic_string(std::move(*this), pos, n);
		}

		constexpr void swap(basic_string& str) noexcept;

		// [string.ops]
		// ------------

		constexpr const value_type* c_str() const noexcept {
			return data();
		}

		constexpr const value_type* data() const noexcept {
			return std::to_address(get_pointer());
		}

		constexpr value_type* data() noexcept {
			return std::to_address(get_pointer());
		}

		constexpr allocator_type get_allocator() const noexcept {
			return _alloc;
		}

		// find

		constexpr size_type find(const basic_string& str, size_type pos = 0) const noexcept {
			return operator self_view().find(str.data(), pos, str.size());
		}

		template <string_view_convertible<CharT, Traits> T>
		constexpr size_type find(const T& t, size_type pos = 0) const noexcept {
			self_view sv = t;
			return operator self_view().find(sv.data(), pos, sv.size());
		}

		constexpr size_type find(const value_type* s, size_type pos, size_type n) const noexcept {
			PLUGIFY_ASSERT(n == 0 || s != nullptr, "string::find(): received nullptr");
			return operator self_view().find(s, pos, n);
		}

		constexpr size_type
		find(const value_type* PLUGIFY_NO_NULL s, size_type pos = 0) const noexcept {
			PLUGIFY_ASSERT(s != nullptr, "string::find(): received nullptr");
			return operator self_view().find(s, pos, traits_type::length(s));
		}

		constexpr size_type find(value_type c, size_type pos = 0) const noexcept {
			return operator self_view().find(c, pos);
		}

		// rfind

		constexpr size_type rfind(const basic_string& str, size_type pos = npos) const noexcept {
			return operator self_view().rfind(str.data(), pos, str.size());
		}

		template <string_view_convertible<CharT, Traits> T>
		constexpr size_type rfind(const T& t, size_type pos = npos) const noexcept {
			self_view sv = t;
			return operator self_view().rfind(sv.data(), pos, sv.size());
		}

		constexpr size_type rfind(const value_type* s, size_type pos, size_type n) const noexcept {
			PLUGIFY_ASSERT(n == 0 || s != nullptr, "string::rfind(): received nullptr");
			return operator self_view().rfind(s, pos, n);
		}

		constexpr size_type
		rfind(const value_type* PLUGIFY_NO_NULL s, size_type pos = npos) const noexcept {
			PLUGIFY_ASSERT(s != nullptr, "string::rfind(): received nullptr");
			return operator self_view().rfind(s, pos, traits_type::length(s));
		}

		constexpr size_type rfind(value_type c, size_type pos = npos) const noexcept {
			return operator self_view().rfind(c, pos);
		}

		// find_first_of

		constexpr size_type find_first_of(const basic_string& str, size_type pos = 0) const noexcept {
			return operator self_view().find_first_of(str.data(), pos, str.size());
		}

		template <string_view_convertible<CharT, Traits> T>
		constexpr size_type find_first_of(const T& t, size_type pos = 0) const noexcept {
			self_view sv = t;
			return operator self_view().find_first_of(sv.data(), pos, sv.size());
		}

		constexpr size_type
		find_first_of(const value_type* s, size_type pos, size_type n) const noexcept {
			PLUGIFY_ASSERT(n == 0 || s != nullptr, "string::find_first_of(): received nullptr");
			return operator self_view().find_first_of(s, pos, n);
		}

		constexpr size_type
		find_first_of(const value_type* PLUGIFY_NO_NULL s, size_type pos = 0) const noexcept {
			PLUGIFY_ASSERT(s != nullptr, "string::find_first_of(): received nullptr");
			return operator self_view().find_first_of(s, pos, traits_type::length(s));
		}

		constexpr size_type find_first_of(value_type c, size_type pos = 0) const noexcept {
			return find(c, pos);
		}

		// find_last_of

		constexpr size_type
		find_last_of(const basic_string& str, size_type pos = npos) const noexcept {
			return operator self_view().find_last_of(str.data(), pos, str.size());
		}

		template <string_view_convertible<CharT, Traits> T>
		constexpr size_type find_last_of(const T& t, size_type pos = npos) const noexcept {
			self_view sv = t;
			return operator self_view().find_last_of(sv.data(), pos, sv.size());
		}

		constexpr size_type
		find_last_of(const value_type* s, size_type pos, size_type n) const noexcept {
			PLUGIFY_ASSERT(n == 0 || s != nullptr, "string::find_last_of(): received nullptr");
			return operator self_view().find_last_of(s, pos, n);
		}

		constexpr size_type
		find_last_of(const value_type* PLUGIFY_NO_NULL s, size_type pos = npos) const noexcept {
			PLUGIFY_ASSERT(s != nullptr, "string::find_last_of(): received nullptr");
			return operator self_view().find_last_of(s, pos, traits_type::length(s));
		}

		constexpr size_type find_last_of(value_type c, size_type pos = npos) const noexcept {
			return rfind(c, pos);
		}

		// find_first_not_of

		constexpr size_type
		find_first_not_of(const basic_string& str, size_type pos = 0) const noexcept {
			return operator self_view().find_first_not_of(str.data(), pos, str.size());
		}

		template <string_view_convertible<CharT, Traits> T>
		constexpr size_type find_first_not_of(const T& t, size_type pos = 0) const noexcept {
			self_view sv = t;
			return operator self_view().find_first_not_of(sv.data(), pos, sv.size());
		}

		constexpr size_type
		find_first_not_of(const value_type* s, size_type pos, size_type n) const noexcept {
			PLUGIFY_ASSERT(n == 0 || s != nullptr, "string::find_first_not_of(): received nullptr");
			return operator self_view().find_first_not_of(s, pos, n);
		}

		constexpr size_type
		find_first_not_of(const value_type* PLUGIFY_NO_NULL s, size_type pos = 0) const noexcept {
			PLUGIFY_ASSERT(s != nullptr, "string::find_first_not_of(): received nullptr");
			return operator self_view().find_first_not_of(s, pos, traits_type::length(s));
		}

		constexpr size_type find_first_not_of(value_type c, size_type pos = 0) const noexcept {
			return operator self_view().find_first_not_of(c, pos);
		}

		// find_last_not_of

		constexpr size_type
		find_last_not_of(const basic_string& str, size_type pos = npos) const noexcept {
			return operator self_view().find_last_not_of(str.data(), pos, str.size());
		}

		template <string_view_convertible<CharT, Traits> T>
		constexpr size_type find_last_not_of(const T& t, size_type pos = npos) const noexcept {
			self_view sv = t;
			return operator self_view().find_last_not_of(sv.data(), pos, sv.size());
		}

		constexpr size_type
		find_last_not_of(const value_type* s, size_type pos, size_type n) const noexcept {
			PLUGIFY_ASSERT(n == 0 || s != nullptr, "string::find_last_not_of(): received nullptr");
			return operator self_view().find_last_not_of(s, pos, n);
		}

		constexpr size_type
		find_last_not_of(const value_type* PLUGIFY_NO_NULL s, size_type pos = npos) const noexcept {
			PLUGIFY_ASSERT(s != nullptr, "string::find_last_not_of(): received nullptr");
			return operator self_view().find_last_not_of(s, pos, traits_type::length(s));
		}

		constexpr size_type find_last_not_of(value_type c, size_type pos = npos) const noexcept {
			return operator self_view().find_last_not_of(c, pos);
		}

		// compare

		constexpr int compare(const basic_string& str) const noexcept {
			return compare(self_view(str));
		}

		template <string_view_convertible<CharT, Traits> T>
		constexpr int compare(const T& t) const noexcept {
			self_view sv = t;
			size_t lhs_sz = size();
			size_t rhs_sz = sv.size();
			int result = traits_type::compare(data(), sv.data(), std::min(lhs_sz, rhs_sz));
			if (result != 0) {
				return result;
			}
			if (lhs_sz < rhs_sz) {
				return -1;
			}
			if (lhs_sz > rhs_sz) {
				return 1;
			}
			return 0;
		}

		template <string_view_convertible<CharT, Traits> T>
		constexpr int compare(size_type pos1, size_type n1, const T& t) const {
			self_view sv = t;
			return compare(pos1, n1, sv.data(), sv.size());
		}

		constexpr int compare(size_type pos1, size_type n1, const basic_string& str) const {
			return compare(pos1, n1, str.data(), str.size());
		}

		constexpr int compare(
			size_type pos1,
			size_type n1,
			const basic_string& str,
			size_type pos2,
			size_type n2 = npos
		) const {
			return compare(pos1, n1, self_view(str), pos2, n2);
		}

		template <string_view_convertible_with_exceptiom<CharT, Traits, Allocator> T>
		inline constexpr int
		compare(size_type pos1, size_type n1, const T& t, size_type pos2, size_type n2 = npos) const {
			self_view sv = t;
			return self_view(*this).substr(pos1, n1).compare(sv.substr(pos2, n2));
		}

		constexpr int compare(const value_type* PLUGIFY_NO_NULL s) const noexcept {
			PLUGIFY_ASSERT(s != nullptr, "string::compare(): received nullptr");
			return compare(0, npos, s, traits_type::length(s));
		}

		constexpr int
		compare(size_type pos1, size_type n1, const value_type* PLUGIFY_NO_NULL s) const {
			PLUGIFY_ASSERT(s != nullptr, "string::compare(): received nullptr");
			return compare(pos1, n1, s, traits_type::length(s));
		}

		constexpr int compare(size_type pos1, size_type n1, const value_type* s, size_type n2) const;

		// starts_with

		constexpr bool starts_with(self_view sv) const noexcept {
			return self_view(typename self_view::assume_valid(), data(), size()).starts_with(sv);
		}

		constexpr bool starts_with(value_type c) const noexcept {
			return !empty() && Traits::eq(front(), c);
		}

		constexpr bool starts_with(const value_type* PLUGIFY_NO_NULL s) const noexcept {
			return starts_with(self_view(s));
		}

		// ends_with

		constexpr bool ends_with(self_view sv) const noexcept {
			return self_view(typename self_view::assume_valid(), data(), size()).ends_with(sv);
		}

		constexpr bool ends_with(value_type c) const noexcept {
			return !empty() && Traits::eq(back(), c);
		}

		constexpr bool ends_with(const value_type* PLUGIFY_NO_NULL s) const noexcept {
			return ends_with(self_view(s));
		}

		// contains

		constexpr bool contains(self_view sv) const noexcept {
			return self_view(typename self_view::assume_valid(), data(), size()).contains(sv);
		}

		constexpr bool contains(value_type c) const noexcept {
			return self_view(typename self_view::assume_valid(), data(), size()).contains(c);
		}

		constexpr bool contains(const value_type* PLUGIFY_NO_NULL s) const {
			return self_view(typename self_view::assume_valid(), data(), size()).contains(s);
		}

		[[nodiscard]] constexpr bool invariants() const;

	private:
		[[nodiscard]] constexpr PLUGIFY_INTERNAL_MEMORY_ACCESS bool is_long() const noexcept {
			if (std::is_constant_evaluated() && __builtin_constant_p(_rep.l._is_long)) {
				return _rep.l._is_long;
			}
			return _rep.s._is_long;
		}

		static constexpr bool fits_in_sso(size_type sz) {
			return sz < min_cap;
		}

		template <class Iterator, class Sentinel>
		constexpr void assign_trivial(Iterator first, Sentinel last, size_type n);

		template <class Iterator, class Sentinel>
		constexpr void assign_with_sentinel(Iterator first, Sentinel last);

		// Copy [first, last) into [dest, dest + (last - first)). Assumes that the ranges don't
		// overlap.
		template <class ForwardIter, class Sent>
		static constexpr value_type*
		copy_non_overlapping_range(ForwardIter first, Sent last, value_type* dest) {
			if constexpr (std::contiguous_iterator<ForwardIter>
						  && std::is_same_v<value_type, std::remove_cvref_t<decltype(*first)>>
						  && std::is_same_v<ForwardIter, Sent>) {
				PLUGIFY_ASSERT(
					!is_overlapping_range(std::to_address(first), std::to_address(last), dest),
					"copy_non_overlapping_range called with an overlapping range!"
				);
				traits_type::copy(dest, std::to_address(first), last - first);
				return dest + (last - first);
			} else {
				for (; first != last; ++first) {
					traits_type::assign(*dest++, *first);
				}
				return dest;
			}
		}

		template <class ForwardIterator, class Sentinel>
		constexpr iterator
		insert_from_safe_copy(size_type n, size_type ip, ForwardIterator first, Sentinel last) {
			size_type sz = size();
			size_type cap = capacity();
			value_type* p;
			if (cap - sz >= n) {
				annotate_increase(n);
				p = std::to_address(get_pointer());
				size_type n_move = sz - ip;
				if (n_move != 0) {
					traits_type::move(p + ip + n, p + ip, n_move);
				}
			} else {
				grow_by_without_replace(cap, sz + n - cap, sz, ip, 0, n);
				p = std::to_address(get_long_pointer());
			}
			sz += n;
			set_size(sz);
			traits_type::assign(p[sz], value_type());
			copy_non_overlapping_range(std::move(first), std::move(last), p + ip);

			return begin() + ip;
		}

		template <class Iterator, class Sentinel>
		constexpr iterator
		insert_with_size(const_iterator pos, Iterator first, Sentinel last, size_type n);

		// internal buffer accessors
		// -------------------------

		constexpr PLUGIFY_INTERNAL_MEMORY_ACCESS void set_short_size(size_type s) noexcept {
			PLUGIFY_ASSERT(
				s < min_cap,
				"s should never be greater than or equal to the short string capacity"
			);
			_rep.s._spare_size = (min_cap - 1) - s;
			_rep.s._is_long = false;
		}

		constexpr PLUGIFY_INTERNAL_MEMORY_ACCESS size_type get_short_size() const noexcept {
			PLUGIFY_ASSERT(!_rep.s._is_long, "String has to be short when trying to get the short size");
			return (min_cap - 1) - _rep.s._spare_size;
		}

		constexpr void set_long_size(size_type s) noexcept {
			_rep.l._size = s;
		}

		constexpr size_type get_long_size() const noexcept {
			PLUGIFY_ASSERT(_rep.l._is_long, "String has to be long when trying to get the long size");
			return _rep.l._size;
		}

		constexpr void set_size(size_type s) noexcept {
			if (is_long()) {
				set_long_size(s);
			} else {
				set_short_size(s);
			}
		}

		constexpr size_type get_long_cap() const noexcept {
			PLUGIFY_ASSERT(_rep.l._is_long, "String has to be long when trying to get the long capacity");
			return _rep.l._cap * endian_factor;
		}

		constexpr pointer get_long_pointer() noexcept {
			PLUGIFY_ASSERT(_rep.l._is_long, "String has to be long when trying to get the long pointer");
			return PLUGIFY_ASAN_VOLATILE_WRAPPER(_rep.l._data);
		}

		constexpr const_pointer get_long_pointer() const noexcept {
			PLUGIFY_ASSERT(_rep.l._is_long, "String has to be long when trying to get the long pointer");
			return PLUGIFY_ASAN_VOLATILE_WRAPPER(_rep.l._data);
		}

		constexpr PLUGIFY_INTERNAL_MEMORY_ACCESS pointer get_short_pointer() noexcept {
			return PLUGIFY_ASAN_VOLATILE_WRAPPER(
				std::pointer_traits<pointer>::pointer_to(_rep.s._data[0])
			);
		}

		constexpr PLUGIFY_INTERNAL_MEMORY_ACCESS const_pointer get_short_pointer() const noexcept {
			return PLUGIFY_ASAN_VOLATILE_WRAPPER(
				std::pointer_traits<const_pointer>::pointer_to(_rep.s._data[0])
			);
		}

		constexpr pointer get_pointer() noexcept {
			return is_long() ? get_long_pointer() : get_short_pointer();
		}

		constexpr const_pointer get_pointer() const noexcept {
			return is_long() ? get_long_pointer() : get_short_pointer();
		}

		// Internal buffer management
		// --------------------------
		//
		// These functions are only responsible for managing the buffer itself, not the value inside
		// the buffer. As such, none of these facilities ensure that there is a null terminator at
		// `data()[size()]`.

		// Allocate a buffer of capacity size with alloc and return it
		static constexpr long_ allocate_long_buffer(Allocator& alloc, size_type capacity) {
			auto buffer = allocate_at_least(alloc, recommend(capacity) + 1);

			if (std::is_constant_evaluated()) {
				for (size_type i = 0; i != buffer.count; ++i) {
					std::construct_at(std::addressof(buffer.ptr[i]));
				}
			}

			return long_(buffer, capacity);
		}

		// Deallocate the long buffer if it exists and clear the short buffer so we are an empty
		// string
		constexpr void reset_internal_buffer() {
			annotate_delete();
			if (is_long()) {
				alloc_traits::deallocate(_alloc, get_long_pointer(), get_long_cap());
			}
			_rep.s = short_();
		}

		// Replace the current buffer with alloc; the first size elements constitute a string
		constexpr void replace_internal_buffer(long_ alloc) {
			reset_internal_buffer();
			_rep.l = alloc;
		}

		// Initialize the internal buffer to hold size elements
		// The elements and null terminator have to be set by the caller
		constexpr pointer init_internal_buffer(size_type size) {
			if (std::is_constant_evaluated()) {
				_rep = rep();
			}

			if (size > max_size()) {
				throw_length_error();
			}

			if (fits_in_sso(size)) {
				set_short_size(size);
				annotate_new(size);
				return get_short_pointer();
			} else {
				_rep.l = allocate_long_buffer(_alloc, size);
				annotate_new(size);
				return get_long_pointer();
			}
		}

		// ASan annotation helpers
		// -----------------------

		// The following functions are no-ops outside of AddressSanitizer mode.
		constexpr void annotate_contiguous_container(
			const void* old_mid,
			const void* new_mid
		) const {
			if (!is_long()) {
				return;
			}

			plg::annotate_contiguous_container<Allocator>(
				data(),
				data() + capacity() + 1,
				old_mid,
				new_mid
			);
		}

		constexpr void annotate_new(size_type current_size) const noexcept {
			annotate_contiguous_container(data() + capacity() + 1, data() + current_size + 1);
		}

		constexpr void annotate_delete() const noexcept {
			annotate_contiguous_container(data() + size() + 1, data() + capacity() + 1);
		}

		constexpr void annotate_increase(size_type n) const noexcept {
			annotate_contiguous_container(data() + size() + 1, data() + size() + 1 + n);
		}

		constexpr void annotate_shrink(size_type old_size) const noexcept {
			annotate_contiguous_container(data() + old_size + 1, data() + size() + 1);
		}

		// Disable ASan annotations and enable them again when going out of scope. It is assumed
		// that the string is in a valid state at that point, so `size()` can be called safely.
		struct [[nodiscard]] annotation_guard {
			annotation_guard(const annotation_guard&) = delete;
			annotation_guard& operator=(const annotation_guard&) = delete;

			constexpr annotation_guard(basic_string& str)
				: str(str) {
				str.annotate_delete();
			}

			constexpr ~annotation_guard() {
				str.annotate_new(str.size());
			}

			basic_string& str;
		};

		template <size_type a>
		static constexpr size_type align_it(size_type s) noexcept {
			return (s + (a - 1)) & ~(a - 1);
		}

		enum { alignment = 8 };

		static constexpr size_type recommend(size_type s) noexcept {
			if (s < min_cap) {
				return min_cap - 1;
			}
			const size_type boundary = sizeof(value_type) < alignment ? alignment / sizeof(value_type)  : endian_factor;
			size_type guess = align_it<boundary>(s + 1) - 1;
			if (guess == min_cap) {
				guess += endian_factor;
			}

			PLUGIFY_ASSERT(guess >= s, "recommendation is below the requested size");
			return guess;
		}

		inline constexpr void init(const value_type* s, size_type sz);
		inline constexpr void init(size_type n, value_type c);

		// Slow path for the (inlined) copy constructor for 'long' strings.
		// Always externally instantiated and not inlined.
		// Requires that s is zero terminated.
		// The main reason for this function to exist is because for unstable, we
		// want to allow inlining of the copy constructor. However, we don't want
		// to call the init() functions as those are marked as inline which may
		// result in over-aggressive inlining by the compiler, where our aim is
		// to only inline the fast path code directly in the ctor.
		PLUGIFY_NOINLINE constexpr void init_copy_ctor_external(const value_type* s, size_type sz);

		template <std::input_iterator InputIterator>
		inline constexpr void init(InputIterator first, InputIterator last);

		template <std::forward_iterator ForwardIterator>
		inline constexpr void init(ForwardIterator first, ForwardIterator last);

		template <class InputIterator, class Sentinel>
		constexpr void init_with_sentinel(InputIterator first, Sentinel last);
		template <class InputIterator, class Sentinel>
		constexpr void init_with_size(InputIterator first, Sentinel last, size_type sz);

		constexpr void grow_by_without_replace(
			size_type old_cap,
			size_type delta_cap,
			size_type old_sz,
			size_type n_copy,
			size_type n_del,
			size_type n_add = 0
		);
		constexpr void grow_by_and_replace(
			size_type old_cap,
			size_type delta_cap,
			size_type old_sz,
			size_type n_copy,
			size_type n_del,
			size_type n_add,
			const value_type* p_new_stuff
		);

		// assign_no_alias is invoked for assignment operations where we
		// have proof that the input does not alias the current instance.
		// For example, operator=(basic_string) performs a 'self' check.
		template <bool is_short>
		PLUGIFY_NOINLINE constexpr basic_string& assign_no_alias(const value_type* s, size_type n);

		constexpr void erase_to_end(size_type pos) {
			PLUGIFY_ASSERT(
				pos <= capacity(),
				"Trying to erase at position outside the strings capacity!"
			);
			null_terminate_at(std::to_address(get_pointer()), pos);
		}

		// erase_external_with_move is invoked for erase() invocations where
		// `n ~= npos`, likely requiring memory moves on the string data.
		PLUGIFY_NOINLINE constexpr void erase_external_with_move(size_type pos, size_type n);

		constexpr void copy_assign_alloc(const basic_string& str) {
			copy_assign_alloc(
				str,
				std::integral_constant<bool, alloc_traits::propagate_on_container_copy_assignment::value>()
			);
		}

		constexpr void copy_assign_alloc(const basic_string& str, std::true_type) {
			if (_alloc == str._alloc) {
				_alloc = str._alloc;
			} else {
				if (!str.is_long()) {
					reset_internal_buffer();
					_alloc = str._alloc;
				} else {
					annotate_delete();
					[[maybe_unused]] auto guard = make_scope_guard(annotate_new_size(*this));
					auto alloc = str._alloc;
					replace_internal_buffer(allocate_long_buffer(alloc, str.size()));
					_alloc = std::move(alloc);
				}
			}
		}

		constexpr void copy_assign_alloc(const basic_string&, std::false_type) noexcept {
		}

		constexpr void
		move_assign(basic_string& str, std::false_type) noexcept(alloc_traits::is_always_equal::value);
		constexpr PLUGIFY_INTERNAL_MEMORY_ACCESS void
		move_assign(basic_string& str, std::true_type) noexcept;

		constexpr void move_assign_alloc(
			basic_string& str
		) noexcept(!alloc_traits::propagate_on_container_move_assignment::value || std::is_nothrow_move_assignable_v<allocator_type>) {
			move_assign_alloc(
				str,
				std::integral_constant<bool, alloc_traits::propagate_on_container_move_assignment::value>()
			);
		}

		constexpr void move_assign_alloc(
			basic_string& c,
			std::true_type
		) noexcept(std::is_nothrow_move_assignable_v<allocator_type>) {
			_alloc = std::move(c._alloc);
		}

		constexpr void move_assign_alloc(basic_string&, std::false_type) noexcept {
		}

		PLUGIFY_NOINLINE constexpr basic_string& assign_external(const value_type* s);
		PLUGIFY_NOINLINE constexpr basic_string& assign_external(const value_type* s, size_type n);

		// Assigns the value in s, guaranteed to be n < min_cap in length.
		inline constexpr basic_string& assign_short(const value_type* s, size_type n) {
			size_type old_size = size();
			if (n > old_size) {
				annotate_increase(n - old_size);
			}
			pointer p;
			if (is_long()) {
				set_long_size(n);
				p = get_long_pointer();
			} else {
				set_short_size(n);
				p = get_short_pointer();
			}
			traits_type::move(std::to_address(p), s, n);
			traits_type::assign(p[n], value_type());
			if (old_size > n) {
				annotate_shrink(old_size);
			}
			return *this;
		}

		constexpr basic_string& null_terminate_at(value_type* p, size_type newsz) {
			size_type old_size = size();
			if (newsz > old_size) {
				annotate_increase(newsz - old_size);
			}
			set_size(newsz);
			traits_type::assign(p[newsz], value_type());
			if (old_size > newsz) {
				annotate_shrink(old_size);
			}
			return *this;
		}

		template <class T>
		constexpr bool addr_in_range(const T& v) const {
			return is_pointer_in_range(data(), data() + size() + 1, std::addressof(v));
		}

		[[noreturn]] static void throw_length_error() {
			PLUGIFY_THROW("constructed string size would exceed max_size()", std::length_error);
		}

		[[noreturn]] static void throw_out_of_range() {
			PLUGIFY_THROW("input index is out of bounds", std::out_of_range);
		}

		friend constexpr basic_string
		concatenate_strings<>(const Allocator&, std::type_identity_t<self_view>, std::type_identity_t<self_view>);

		template <class CharT2, class Traits2, class Allocator2>
		friend inline constexpr bool
		operator==(const basic_string<CharT2, Traits2, Allocator2>&, const CharT2*) noexcept;
	};

	template <
		std::input_iterator InputIterator,
		class CharT = std::iter_value_t<InputIterator>,
		is_allocator Allocator = allocator<CharT>>
	basic_string(InputIterator, InputIterator, Allocator = Allocator())
		-> basic_string<CharT, std::char_traits<CharT>, Allocator>;

	template <class CharT, is_char_traits Traits, is_allocator Allocator = allocator<CharT>>
	explicit basic_string(std::basic_string_view<CharT, Traits>, const Allocator& = Allocator())
		-> basic_string<CharT, Traits, Allocator>;

	template <
		class CharT,
		is_char_traits Traits,
		is_allocator Allocator = allocator<CharT>,
		class Sz = typename std::allocator_traits<Allocator>::size_type>
	basic_string(std::basic_string_view<CharT, Traits>, Sz, Sz, const Allocator& = Allocator())
		-> basic_string<CharT, Traits, Allocator>;

#if PLUGIFY_HAS_CXX23
	template <
		std::ranges::input_range Range,
		is_allocator Allocator = std::allocator<std::ranges::range_value_t<Range>>>
	basic_string(std::from_range_t, Range&&, Allocator = Allocator()) -> basic_string<
		std::ranges::range_value_t<Range>,
		std::char_traits<std::ranges::range_value_t<Range>>,
		Allocator>;
#endif

	template <class CharT, class Traits, class Allocator>
	constexpr void basic_string<CharT, Traits, Allocator>::init(const value_type* s, size_type sz) {
		pointer p = init_internal_buffer(sz);
		traits_type::copy(std::to_address(p), s, sz);
		traits_type::assign(p[sz], value_type());
	}

	template <class CharT, class Traits, class Allocator>
	PLUGIFY_NOINLINE constexpr void
	basic_string<CharT, Traits, Allocator>::init_copy_ctor_external(const value_type* s, size_type sz) {
		pointer p = init_internal_buffer(sz);
		traits_type::copy(std::to_address(p), s, sz + 1);
	}

	template <class CharT, class Traits, class Allocator>
	constexpr void basic_string<CharT, Traits, Allocator>::init(size_type n, value_type c) {
		pointer p = init_internal_buffer(n);
		traits_type::assign(std::to_address(p), n, c);
		traits_type::assign(p[n], value_type());
	}

	template <class CharT, class Traits, class Allocator>
	template <std::input_iterator InputIterator>
	constexpr void
	basic_string<CharT, Traits, Allocator>::init(InputIterator first, InputIterator last) {
		init_with_sentinel(std::move(first), std::move(last));
	}

	template <class CharT, class Traits, class Allocator>
	template <class InputIterator, class Sentinel>
	constexpr void
	basic_string<CharT, Traits, Allocator>::init_with_sentinel(InputIterator first, Sentinel last) {
		_rep = rep();
		annotate_new(0);

#if PLUGIFY_HAS_EXCEPTIONS
		try {
#endif	// PLUGIFY_HAS_EXCEPTIONS
			for (; first != last; ++first) {
				push_back(*first);
			}
#if PLUGIFY_HAS_EXCEPTIONS
		} catch (...) {
			reset_internal_buffer();
			throw;
		}
#endif	// PLUGIFY_HAS_EXCEPTIONS
	}

	template <class CharT, class Traits, class Allocator>
	template <std::forward_iterator ForwardIterator>
	constexpr void
	basic_string<CharT, Traits, Allocator>::init(ForwardIterator first, ForwardIterator last) {
		size_type sz = static_cast<size_type>(std::distance(first, last));
		init_with_size(first, last, sz);
	}

	template <class CharT, class Traits, class Allocator>
	template <class InputIterator, class Sentinel>
	constexpr void basic_string<CharT, Traits, Allocator>::init_with_size(
		InputIterator first,
		Sentinel last,
		size_type sz
	) {
		pointer p = init_internal_buffer(sz);

#if PLUGIFY_HAS_EXCEPTIONS
		try {
#endif	// PLUGIFY_HAS_EXCEPTIONS
			auto end = copy_non_overlapping_range(std::move(first), std::move(last), std::to_address(p));
			traits_type::assign(*end, value_type());
#if PLUGIFY_HAS_EXCEPTIONS
		} catch (...) {
			reset_internal_buffer();
			throw;
		}
#endif	// PLUGIFY_HAS_EXCEPTIONS
	}

	template <class CharT, class Traits, class Allocator>
	constexpr void basic_string<CharT, Traits, Allocator>::grow_by_and_replace(
		size_type old_cap,
		size_type delta_cap,
		size_type old_sz,
		size_type n_copy,
		size_type n_del,
		size_type n_add,
		const value_type* p_new_stuff
	) {
		size_type ms = max_size();
		if (delta_cap > ms - old_cap) {
			throw_length_error();
		}
		pointer old_p = get_pointer();
		size_type cap = old_cap < ms / 2 - alignment
							? recommend(std::max(old_cap + delta_cap, 2 * old_cap))
							: ms;
		annotate_delete();
		[[maybe_unused]] auto guard = make_scope_guard(annotate_new_size(*this));
		long_ buffer = allocate_long_buffer(_alloc, cap);
		if (n_copy != 0) {
			traits_type::copy(std::to_address(buffer._data), std::to_address(old_p), n_copy);
		}
		if (n_add != 0) {
			traits_type::copy(std::to_address(buffer._data) + n_copy, p_new_stuff, n_add);
		}
		size_type sec_cp_sz = old_sz - n_del - n_copy;
		if (sec_cp_sz != 0) {
			traits_type::copy(
				std::to_address(buffer._data) + n_copy + n_add,
				std::to_address(old_p) + n_copy + n_del,
				sec_cp_sz
			);
		}
		buffer._size = n_copy + n_add + sec_cp_sz;
		traits_type::assign(buffer._data[buffer._size], value_type());
		replace_internal_buffer(buffer);
	}

	template <class CharT, class Traits, class Allocator>
	constexpr void basic_string<CharT, Traits, Allocator>::grow_by_without_replace(
		size_type old_cap,
		size_type delta_cap,
		size_type old_sz,
		size_type n_copy,
		size_type n_del,
		size_type n_add
	) {
		annotate_delete();
		[[maybe_unused]] auto guard = make_scope_guard(annotate_new_size(*this));
		size_type ms = max_size();
		if (delta_cap > ms - old_cap) {
			this->throw_length_error();
		}
		pointer old_p = get_pointer();
		size_type cap = old_cap < ms / 2 - alignment
							? recommend(std::max(old_cap + delta_cap, 2 * old_cap))
							: ms;
		long_ buffer = allocate_long_buffer(_alloc, cap);
		if (n_copy != 0) {
			traits_type::copy(std::to_address(buffer._data), std::to_address(old_p), n_copy);
		}
		size_type sec_cp_sz = old_sz - n_del - n_copy;
		if (sec_cp_sz != 0) {
			traits_type::copy(
				std::to_address(buffer._data) + n_copy + n_add,
				std::to_address(old_p) + n_copy + n_del,
				sec_cp_sz
			);
		}

		// This is -1 to make sure the caller sets the size properly, since old versions of this
		// function didn't set the size at all.
		buffer._size = npos;
		replace_internal_buffer(buffer);
		set_long_size(old_sz - n_del + n_add);
	}

	// assign

	template <class CharT, class Traits, class Allocator>
	template <bool is_short>
	PLUGIFY_NOINLINE constexpr basic_string<CharT, Traits, Allocator>&
	basic_string<CharT, Traits, Allocator>::assign_no_alias(const value_type* s, size_type n) {
		const auto cap = is_short ? min_cap : get_long_cap();
		const auto size = is_short ? get_short_size() : get_long_size();
		if (n >= cap) {
			grow_by_and_replace(cap - 1, n - cap + 1, size, 0, size, n, s);
			return *this;
		}

		annotate_delete();
		[[maybe_unused]] auto guard = make_scope_guard(annotate_new_size(*this));
		pointer p;
		if (is_short) {
			p = get_short_pointer();
			set_short_size(n);
		} else {
			p = get_long_pointer();
			set_long_size(n);
		}
		traits_type::copy(std::to_address(p), s, n);
		traits_type::assign(p[n], value_type());
		return *this;
	}

	template <class CharT, class Traits, class Allocator>
	PLUGIFY_NOINLINE constexpr basic_string<CharT, Traits, Allocator>&
	basic_string<CharT, Traits, Allocator>::assign_external(const value_type* s, size_type n) {
		const auto cap = capacity();
		const auto sz = size();
		if (cap >= n) {
			if (n > sz) {
				annotate_increase(n - sz);
			}
			value_type* p = std::to_address(get_pointer());
			traits_type::move(p, s, n);
			return null_terminate_at(p, n);
		} else {
			grow_by_and_replace(cap, n - cap, sz, 0, sz, n, s);
			return *this;
		}
	}

	template <class CharT, class Traits, class Allocator>
	constexpr basic_string<CharT, Traits, Allocator>&
	basic_string<CharT, Traits, Allocator>::assign(const value_type* s, size_type n) {
		PLUGIFY_ASSERT(n == 0 || s != nullptr, "string::assign received nullptr");
		return (__builtin_constant_p(n) && fits_in_sso(n)) ? assign_short(s, n)
														   : assign_external(s, n);
	}

	template <class CharT, class Traits, class Allocator>
	constexpr basic_string<CharT, Traits, Allocator>&
	basic_string<CharT, Traits, Allocator>::assign(size_type n, value_type c) {
		size_type cap = capacity();
		size_type old_size = size();
		if (cap < n) {
			grow_by_without_replace(cap, n - cap, old_size, 0, old_size);
			annotate_increase(n);
		} else if (n > old_size) {
			annotate_increase(n - old_size);
		}
		value_type* p = std::to_address(get_pointer());
		traits_type::assign(p, n, c);
		return null_terminate_at(p, n);
	}

	template <class CharT, class Traits, class Allocator>
	constexpr basic_string<CharT, Traits, Allocator>&
	basic_string<CharT, Traits, Allocator>::operator=(value_type c) {
		size_type old_size = size();
		if (old_size == 0) {
			annotate_increase(1);
		}
		pointer p;
		if (is_long()) {
			p = get_long_pointer();
			set_long_size(1);
		} else {
			p = get_short_pointer();
			set_short_size(1);
		}
		traits_type::assign(*p, c);
		traits_type::assign(*++p, value_type());
		if (old_size > 1) {
			annotate_shrink(old_size);
		}
		return *this;
	}

	template <class CharT, class Traits, class Allocator>
	constexpr PLUGIFY_INTERNAL_MEMORY_ACCESS basic_string<CharT, Traits, Allocator>&
	basic_string<CharT, Traits, Allocator>::operator=(const basic_string& str) {
		if (this == std::addressof(str)) {
			return *this;
		}

		copy_assign_alloc(str);

		if (is_long()) {
			return assign_no_alias<false>(str.data(), str.size());
		}

		if (str.is_long()) {
			return assign_no_alias<true>(str.data(), str.size());
		}

		annotate_delete();
		[[maybe_unused]] auto guard = make_scope_guard(annotate_new_size(*this));
		_rep = str._rep;

		return *this;
	}

	template <class CharT, class Traits, class Allocator>
	inline constexpr void
	basic_string<CharT, Traits, Allocator>::move_assign(basic_string& str, std::false_type) noexcept(
		alloc_traits::is_always_equal::value
	) {
		if (_alloc != str._alloc) {
			assign(str);
		} else {
			move_assign(str, std::true_type());
		}
	}

	template <class CharT, class Traits, class Allocator>
	inline constexpr PLUGIFY_INTERNAL_MEMORY_ACCESS void
	basic_string<CharT, Traits, Allocator>::move_assign(basic_string& str, std::true_type) noexcept {
		annotate_delete();
		if (is_long()) {
			reset_internal_buffer();
		}
		size_type str_old_size = str.size();
		bool str_was_short = !str.is_long();

		move_assign_alloc(str);
		_rep = str._rep;
		str.set_short_size(0);
		traits_type::assign(str.get_short_pointer()[0], value_type());

		if (str_was_short && this != std::addressof(str)) {
			str.annotate_shrink(str_old_size);
		} else {
			// ASan annotations: was long, so object memory is unpoisoned as new.
			// Or is same as *this, and annotate_delete() was called.
			str.annotate_new(0);
		}

		// ASan annotations: Guard against `std::string s; s = std::move(s);`
		// You can find more here: https://en.cppreference.com/w/cpp/utility/move
		// Quote: "Unless otherwise specified, all standard library objects that have been moved
		// from are placed in a "valid but unspecified state", meaning the object's class
		// invariants hold (so functions without preconditions, such as the assignment operator,
		// can be safely used on the object after it was moved from):"
		// Quote: "v = std::move(v); // the value of v is unspecified"
		if (!is_long() && std::addressof(str) != this) {
			// If it is long string, delete was never called on original str's buffer.
			annotate_new(get_short_size());
		}
	}

	template <class CharT, class Traits, class Allocator>
	template <class InputIterator, class Sentinel>
	constexpr void
	basic_string<CharT, Traits, Allocator>::assign_with_sentinel(InputIterator first, Sentinel last) {
		const basic_string temp(init_with_sentinel_tag(), std::move(first), std::move(last), _alloc);
		assign(temp.data(), temp.size());
	}

	template <class CharT, class Traits, class Allocator>
	template <class Iterator, class Sentinel>
	constexpr void
	basic_string<CharT, Traits, Allocator>::assign_trivial(Iterator first, Sentinel last, size_type n) {
		PLUGIFY_ASSERT(
			string_is_trivial_iterator_v<Iterator>,
			"The iterator type given to `assign_trivial` must be trivial"
		);

		size_type old_size = size();
		size_type cap = capacity();
		if (cap < n) {
			// Unlike `append` functions, if the input range points into the string itself, there is
			// no case that the input range could get invalidated by reallocation:
			// 1. If the input range is a subset of the string itself, its size cannot exceed the
			// capacity of the string,
			//    thus no reallocation would happen.
			// 2. In the exotic case where the input range is the byte representation of the string
			// itself, the string
			//    object itself stays valid even if reallocation happens.
			size_type sz = size();
			grow_by_without_replace(cap, n - cap, sz, 0, sz);
			annotate_increase(n);
		} else if (n > old_size) {
			annotate_increase(n - old_size);
		}
		pointer p = get_pointer();
		for (; first != last; ++p, (void) ++first) {
			traits_type::assign(*p, *first);
		}
		traits_type::assign(*p, value_type());
		set_size(n);
		if (n < old_size) {
			annotate_shrink(old_size);
		}
	}

	template <class CharT, class Traits, class Allocator>
	constexpr basic_string<CharT, Traits, Allocator>&
	basic_string<CharT, Traits, Allocator>::assign(const basic_string& str, size_type pos, size_type n) {
		size_type sz = str.size();
		if (pos > sz) {
			this->throw_out_of_range();
		}
		return assign(str.data() + pos, std::min(n, sz - pos));
	}

	template <class CharT, class Traits, class Allocator>
	PLUGIFY_NOINLINE constexpr basic_string<CharT, Traits, Allocator>&
	basic_string<CharT, Traits, Allocator>::assign_external(const value_type* s) {
		return assign_external(s, traits_type::length(s));
	}

	template <class CharT, class Traits, class Allocator>
	constexpr basic_string<CharT, Traits, Allocator>&
	basic_string<CharT, Traits, Allocator>::assign(const value_type* s) {
		PLUGIFY_ASSERT(s != nullptr, "string::assign received nullptr");
		if (auto len = traits_type::length(s); __builtin_constant_p(len) && fits_in_sso(len)) {
			return assign_short(s, len);
		}
		return assign_external(s);
	}

	// append

	template <class CharT, class Traits, class Allocator>
	constexpr basic_string<CharT, Traits, Allocator>&
	basic_string<CharT, Traits, Allocator>::append(const value_type* s, size_type n) {
		PLUGIFY_ASSERT(n == 0 || s != nullptr, "string::append received nullptr");
		size_type cap = capacity();
		size_type sz = size();
		if (cap - sz < n) {
			grow_by_and_replace(cap, sz + n - cap, sz, sz, 0, n, s);
			return *this;
		}

		if (n == 0) {
			return *this;
		}

		annotate_increase(n);
		value_type* p = std::to_address(get_pointer());
		traits_type::copy(p + sz, s, n);
		sz += n;
		set_size(sz);
		traits_type::assign(p[sz], value_type());
		return *this;
	}

	template <class CharT, class Traits, class Allocator>
	constexpr basic_string<CharT, Traits, Allocator>&
	basic_string<CharT, Traits, Allocator>::append(size_type n, value_type c) {
		if (n == 0) {
			return *this;
		}

		size_type cap = capacity();
		size_type sz = size();
		if (cap - sz < n) {
			grow_by_without_replace(cap, sz + n - cap, sz, sz, 0);
		}
		annotate_increase(n);
		pointer p = get_pointer();
		traits_type::assign(std::to_address(p) + sz, n, c);
		sz += n;
		set_size(sz);
		traits_type::assign(p[sz], value_type());
		return *this;
	}

	template <class CharT, class Traits, class Allocator>
	constexpr void basic_string<CharT, Traits, Allocator>::push_back(value_type c) {
		bool is_short = !is_long();
		size_type cap;
		size_type sz;
		if (is_short) {
			cap = min_cap - 1;
			sz = get_short_size();
		} else {
			cap = get_long_cap() - 1;
			sz = get_long_size();
		}
		if (sz == cap) {
			grow_by_without_replace(cap, 1, sz, sz, 0);
			is_short = false;  // the string is always long after grow_by
		}
		annotate_increase(1);
		pointer p;
		if (is_short) {
			p = get_short_pointer() + sz;
			set_short_size(sz + 1);
		} else {
			p = get_long_pointer() + sz;
			set_long_size(sz + 1);
		}
		traits_type::assign(*p, c);
		traits_type::assign(*++p, value_type());
	}

	template <class CharT, class Traits, class Allocator>
	constexpr basic_string<CharT, Traits, Allocator>&
	basic_string<CharT, Traits, Allocator>::append(const basic_string& str, size_type pos, size_type n) {
		size_type sz = str.size();
		if (pos > sz) {
			this->throw_out_of_range();
		}
		return append(str.data() + pos, std::min(n, sz - pos));
	}

	template <class CharT, class Traits, class Allocator>
	constexpr basic_string<CharT, Traits, Allocator>&
	basic_string<CharT, Traits, Allocator>::append(const value_type* s) {
		PLUGIFY_ASSERT(s != nullptr, "string::append received nullptr");
		return append(s, traits_type::length(s));
	}

	// insert

	template <class CharT, class Traits, class Allocator>
	constexpr basic_string<CharT, Traits, Allocator>&
	basic_string<CharT, Traits, Allocator>::insert(size_type pos, const value_type* s, size_type n) {
		PLUGIFY_ASSERT(n == 0 || s != nullptr, "string::insert received nullptr");
		size_type sz = size();
		if (pos > sz) {
			this->throw_out_of_range();
		}
		size_type cap = capacity();

		if (cap - sz < n) {
			grow_by_and_replace(cap, sz + n - cap, sz, pos, 0, n, s);
			return *this;
		}

		if (n == 0) {
			return *this;
		}

		annotate_increase(n);
		value_type* p = std::to_address(get_pointer());
		size_type n_move = sz - pos;
		if (n_move != 0) {
			if (is_pointer_in_range(p + pos, p + sz, s)) {
				s += n;
			}
			traits_type::move(p + pos + n, p + pos, n_move);
		}
		traits_type::move(p + pos, s, n);
		sz += n;
		set_size(sz);
		traits_type::assign(p[sz], value_type());
		return *this;
	}

	template <class CharT, class Traits, class Allocator>
	constexpr basic_string<CharT, Traits, Allocator>&
	basic_string<CharT, Traits, Allocator>::insert(size_type pos, size_type n, value_type c) {
		size_type sz = size();
		if (pos > sz) {
			this->throw_out_of_range();
		}

		if (n == 0) {
			return *this;
		}

		size_type cap = capacity();
		value_type* p;
		if (cap - sz >= n) {
			annotate_increase(n);
			p = std::to_address(get_pointer());
			size_type n_move = sz - pos;
			if (n_move != 0) {
				traits_type::move(p + pos + n, p + pos, n_move);
			}
		} else {
			grow_by_without_replace(cap, sz + n - cap, sz, pos, 0, n);
			p = std::to_address(get_long_pointer());
		}
		traits_type::assign(p + pos, n, c);
		sz += n;
		set_size(sz);
		traits_type::assign(p[sz], value_type());
		return *this;
	}

	template <class CharT, class Traits, class Allocator>
	template <class Iterator, class Sentinel>
	constexpr typename basic_string<CharT, Traits, Allocator>::iterator
	basic_string<CharT, Traits, Allocator>::insert_with_size(
		const_iterator pos,
		Iterator first,
		Sentinel last,
		size_type n
	) {
		size_type ip = static_cast<size_type>(pos - begin());
		if (n == 0) {
			return begin() + ip;
		}

		if (string_is_trivial_iterator_v<Iterator> && !addr_in_range(*first)) {
			return insert_from_safe_copy(n, ip, std::move(first), std::move(last));
		} else {
			const basic_string temp(init_with_sentinel_tag(), std::move(first), std::move(last), _alloc);
			return insert_from_safe_copy(n, ip, temp.begin(), temp.end());
		}
	}

	template <class CharT, class Traits, class Allocator>
	constexpr basic_string<CharT, Traits, Allocator>& basic_string<CharT, Traits, Allocator>::insert(
		size_type pos1,
		const basic_string& str,
		size_type pos2,
		size_type n
	) {
		size_type str_sz = str.size();
		if (pos2 > str_sz) {
			this->throw_out_of_range();
		}
		return insert(pos1, str.data() + pos2, std::min(n, str_sz - pos2));
	}

	template <class CharT, class Traits, class Allocator>
	constexpr basic_string<CharT, Traits, Allocator>&
	basic_string<CharT, Traits, Allocator>::insert(size_type pos, const value_type* s) {
		PLUGIFY_ASSERT(s != nullptr, "string::insert received nullptr");
		return insert(pos, s, traits_type::length(s));
	}

	template <class CharT, class Traits, class Allocator>
	constexpr typename basic_string<CharT, Traits, Allocator>::iterator
	basic_string<CharT, Traits, Allocator>::insert(const_iterator pos, value_type c) {
		size_type ip = static_cast<size_type>(pos - begin());
		size_type sz = size();
		size_type cap = capacity();
		value_type* p;
		if (cap == sz) {
			grow_by_without_replace(cap, 1, sz, ip, 0, 1);
			p = std::to_address(get_long_pointer());
		} else {
			annotate_increase(1);
			p = std::to_address(get_pointer());
			size_type n_move = sz - ip;
			if (n_move != 0) {
				traits_type::move(p + ip + 1, p + ip, n_move);
			}
		}
		traits_type::assign(p[ip], c);
		traits_type::assign(p[++sz], value_type());
		set_size(sz);
		return begin() + static_cast<difference_type>(ip);
	}

	// replace

	template <class CharT, class Traits, class Allocator>
	constexpr basic_string<CharT, Traits, Allocator>&
	basic_string<CharT, Traits, Allocator>::replace(
		size_type pos,
		size_type n1,
		const value_type* s,
		size_type n2
	) {
		PLUGIFY_ASSERT(n2 == 0 || s != nullptr, "string::replace received nullptr");
		size_type sz = size();
		if (pos > sz) {
			this->throw_out_of_range();
		}
		n1 = std::min(n1, sz - pos);
		size_type cap = capacity();
		if (cap - sz + n1 < n2) {
			grow_by_and_replace(cap, sz - n1 + n2 - cap, sz, pos, n1, n2, s);
			return *this;
		}

		value_type* p = std::to_address(get_pointer());
		if (n1 != n2) {
			if (n2 > n1) {
				annotate_increase(n2 - n1);
			}
			size_type n_move = sz - pos - n1;
			if (n_move != 0) {
				if (n1 > n2) {
					traits_type::move(p + pos, s, n2);
					traits_type::move(p + pos + n2, p + pos + n1, n_move);
					return null_terminate_at(p, sz + (n2 - n1));
				}
				if (is_pointer_in_range(p + pos + 1, p + sz, s)) {
					if (p + pos + n1 <= s) {
						s += n2 - n1;
					} else {  // p + pos < s < p + pos + n1
						traits_type::move(p + pos, s, n1);
						pos += n1;
						s += n2;
						n2 -= n1;
						n1 = 0;
					}
				}
				traits_type::move(p + pos + n2, p + pos + n1, n_move);
			}
		}
		traits_type::move(p + pos, s, n2);
		return null_terminate_at(p, sz + (n2 - n1));
	}

	template <class CharT, class Traits, class Allocator>
	constexpr basic_string<CharT, Traits, Allocator>&
	basic_string<CharT, Traits, Allocator>::replace(size_type pos, size_type n1, size_type n2, value_type c) {
		size_type sz = size();
		if (pos > sz) {
			this->throw_out_of_range();
		}
		n1 = std::min(n1, sz - pos);
		size_type cap = capacity();
		value_type* p;
		if (cap - sz + n1 >= n2) {
			p = std::to_address(get_pointer());
			if (n1 != n2) {
				if (n2 > n1) {
					annotate_increase(n2 - n1);
				}
				size_type n_move = sz - pos - n1;
				if (n_move != 0) {
					traits_type::move(p + pos + n2, p + pos + n1, n_move);
				}
			}
		} else {
			grow_by_without_replace(cap, sz - n1 + n2 - cap, sz, pos, n1, n2);
			p = std::to_address(get_long_pointer());
		}
		traits_type::assign(p + pos, n2, c);
		return null_terminate_at(p, sz - (n1 - n2));
	}

	template <class CharT, class Traits, class Allocator>
	constexpr basic_string<CharT, Traits, Allocator>&
	basic_string<CharT, Traits, Allocator>::replace(
		size_type pos1,
		size_type n1,
		const basic_string& str,
		size_type pos2,
		size_type n2
	) {
		size_type str_sz = str.size();
		if (pos2 > str_sz) {
			this->throw_out_of_range();
		}
		return replace(pos1, n1, str.data() + pos2, std::min(n2, str_sz - pos2));
	}

	template <class CharT, class Traits, class Allocator>
	constexpr basic_string<CharT, Traits, Allocator>&
	basic_string<CharT, Traits, Allocator>::replace(size_type pos, size_type n1, const value_type* s) {
		PLUGIFY_ASSERT(s != nullptr, "string::replace received nullptr");
		return replace(pos, n1, s, traits_type::length(s));
	}

	// erase

	// 'externally instantiated' erase() implementation, called when n != npos.
	// Does not check pos against size()
	template <class CharT, class Traits, class Allocator>
	PLUGIFY_NOINLINE constexpr void
	basic_string<CharT, Traits, Allocator>::erase_external_with_move(size_type pos, size_type n) {
		if (n == 0) {
			return;
		}

		size_type sz = size();
		value_type* p = std::to_address(get_pointer());
		n = std::min(n, sz - pos);
		size_type n_move = sz - pos - n;
		if (n_move != 0) {
			traits_type::move(p + pos, p + pos + n, n_move);
		}
		null_terminate_at(p, sz - n);
	}

	template <class CharT, class Traits, class Allocator>
	constexpr basic_string<CharT, Traits, Allocator>&
	basic_string<CharT, Traits, Allocator>::erase(size_type pos, size_type n) {
		if (pos > size()) {
			this->throw_out_of_range();
		}
		if (n == npos) {
			erase_to_end(pos);
		} else {
			erase_external_with_move(pos, n);
		}
		return *this;
	}

	template <class CharT, class Traits, class Allocator>
	inline constexpr typename basic_string<CharT, Traits, Allocator>::iterator
	basic_string<CharT, Traits, Allocator>::erase(const_iterator pos) {
		PLUGIFY_ASSERT(pos != end(), "string::erase(iterator) called with a non-dereferenceable iterator");
		iterator b = begin();
		size_type r = static_cast<size_type>(pos - b);
		erase(r, 1);
		return b + static_cast<difference_type>(r);
	}

	template <class CharT, class Traits, class Allocator>
	inline constexpr typename basic_string<CharT, Traits, Allocator>::iterator
	basic_string<CharT, Traits, Allocator>::erase(const_iterator first, const_iterator last) {
		PLUGIFY_ASSERT(first <= last, "string::erase(first, last) called with invalid range");
		iterator b = begin();
		size_type r = static_cast<size_type>(first - b);
		erase(r, static_cast<size_type>(last - first));
		return b + static_cast<difference_type>(r);
	}

	template <class CharT, class Traits, class Allocator>
	inline constexpr void basic_string<CharT, Traits, Allocator>::pop_back() {
		PLUGIFY_ASSERT(!empty(), "string::pop_back(): string is already empty");
		erase_to_end(size() - 1);
	}

	template <class CharT, class Traits, class Allocator>
	inline constexpr void basic_string<CharT, Traits, Allocator>::clear() noexcept {
		size_type old_size;
		if (is_long()) {
			old_size = get_long_size();
			traits_type::assign(*get_long_pointer(), value_type());
			set_long_size(0);
		} else {
			old_size = get_short_size();
			traits_type::assign(*get_short_pointer(), value_type());
			set_short_size(0);
		}
		annotate_shrink(old_size);
	}

	template <class CharT, class Traits, class Allocator>
	constexpr void basic_string<CharT, Traits, Allocator>::resize(size_type n, value_type c) {
		size_type sz = size();
		if (n > sz) {
			append(n - sz, c);
		} else {
			erase_to_end(n);
		}
	}

	template <class CharT, class Traits, class Allocator>
	constexpr void basic_string<CharT, Traits, Allocator>::reserve(size_type requested_capacity) {
		if (requested_capacity > max_size()) {
			this->throw_length_error();
		}

		// Make sure reserve(n) never shrinks. This is technically only required in C++20
		// and later (since P0966R1), however we provide consistent behavior in all Standard
		// modes because this function is instantiated in the shared library.
		if (requested_capacity <= capacity()) {
			return;
		}

		[[maybe_unused]] annotation_guard g(*this);
		long_ buffer = allocate_long_buffer(_alloc, requested_capacity);
		buffer._size = size();
		traits_type::copy(std::to_address(buffer._data), data(), buffer._size + 1);
		replace_internal_buffer(buffer);
	}

	template <class CharT, class Traits, class Allocator>
	inline constexpr void basic_string<CharT, Traits, Allocator>::shrink_to_fit() noexcept {
		size_type target_capacity = recommend(size());
		if (target_capacity == capacity()) {
			return;
		}

		PLUGIFY_ASSERT(is_long(), "Trying to shrink small string");

		// We're a long string and we're shrinking into the small buffer.
		const auto ptr = get_long_pointer();
		const auto size = get_long_size();
		const auto cap = get_long_cap();

		if (fits_in_sso(target_capacity)) {
			[[maybe_unused]] annotation_guard g(*this);
			set_short_size(size);
			traits_type::copy(std::to_address(get_short_pointer()), std::to_address(ptr), size + 1);
			alloc_traits::deallocate(_alloc, ptr, cap);
			return;
		}

#if PLUGIFY_HAS_EXCEPTIONS
		try {
#endif	// PLUGIFY_HAS_EXCEPTIONS
			[[maybe_unused]] annotation_guard g(*this);
			long_ buffer = allocate_long_buffer(_alloc, size);

			// The Standard mandates shrink_to_fit() does not increase the capacity.
			// With equal capacity keep the existing buffer. This avoids extra work
			// due to swapping the elements.
			if (buffer._cap * endian_factor - 1 >= capacity()) {
				alloc_traits::deallocate(_alloc, buffer._data, buffer._cap * endian_factor);
				return;
			}

			traits_type::copy(
				std::to_address(buffer._data),
				std::to_address(get_long_pointer()),
				size + 1
			);
			replace_internal_buffer(buffer);
#if PLUGIFY_HAS_EXCEPTIONS
		} catch (...) {
			return;
		}
#endif	// PLUGIFY_HAS_EXCEPTIONS
	}

	template <class CharT, class Traits, class Allocator>
	constexpr typename basic_string<CharT, Traits, Allocator>::const_reference
	basic_string<CharT, Traits, Allocator>::at(size_type n) const {
		if (n >= size()) {
			this->throw_out_of_range();
		}
		return (*this)[n];
	}

	template <class CharT, class Traits, class Allocator>
	constexpr typename basic_string<CharT, Traits, Allocator>::reference
	basic_string<CharT, Traits, Allocator>::at(size_type n) {
		if (n >= size()) {
			this->throw_out_of_range();
		}
		return (*this)[n];
	}

	template <class CharT, class Traits, class Allocator>
	constexpr typename basic_string<CharT, Traits, Allocator>::size_type
	basic_string<CharT, Traits, Allocator>::copy(value_type* s, size_type n, size_type pos) const {
		size_type sz = size();
		if (pos > sz) {
			this->throw_out_of_range();
		}
		size_type rlen = std::min(n, sz - pos);
		traits_type::copy(s, data() + pos, rlen);
		return rlen;
	}

	template <class CharT, class Traits, class Allocator>
	inline constexpr void basic_string<CharT, Traits, Allocator>::swap(basic_string& str) noexcept {
		PLUGIFY_ASSERT(
			alloc_traits::propagate_on_container_swap::value || alloc_traits::is_always_equal::value || _alloc == str._alloc,
			"swapping non-equal allocators"
		);
		if (!is_long()) {
			annotate_delete();
		}
		if (this != std::addressof(str) && !str.is_long()) {
			str.annotate_delete();
		}
		std::swap(_rep, str._rep);
		swap_allocator(_alloc, str._alloc);
		if (!is_long()) {
			annotate_new(get_short_size());
		}
		if (this != std::addressof(str) && !str.is_long()) {
			str.annotate_new(str.get_short_size());
		}
	}

	// compare

	template <class CharT, class Traits, class Allocator>
	inline constexpr int basic_string<CharT, Traits, Allocator>::compare(
		size_type pos1,
		size_type n1,
		const value_type* s,
		size_type n2
	) const {
		PLUGIFY_ASSERT(n2 == 0 || s != nullptr, "string::compare(): received nullptr");
		size_type sz = size();
		if (pos1 > sz || n2 == npos) {
			this->throw_out_of_range();
		}
		size_type rlen = std::min(n1, sz - pos1);
		int r = traits_type::compare(data() + pos1, s, std::min(rlen, n2));
		if (r == 0) {
			if (rlen < n2) {
				r = -1;
			} else if (rlen > n2) {
				r = 1;
			}
		}
		return r;
	}

	// invariants

	template <class CharT, class Traits, class Allocator>
	inline constexpr bool basic_string<CharT, Traits, Allocator>::invariants() const {
		if (size() > capacity()) {
			return false;
		}
		if (capacity() < min_cap - 1) {
			return false;
		}
		if (data() == nullptr) {
			return false;
		}
		if (!Traits::eq(data()[size()], value_type())) {
			return false;
		}
		return true;
	}

	// operator==

	template <class CharT, class Traits, class Allocator>
	inline constexpr bool operator==(
		const basic_string<CharT, Traits, Allocator>& lhs,
		const basic_string<CharT, Traits, Allocator>& rhs
	) noexcept {
		size_t lhs_sz = lhs.size();
		return lhs_sz == rhs.size() && Traits::compare(lhs.data(), rhs.data(), lhs_sz) == 0;
	}

	template <class CharT, class Traits, class Allocator>
	inline constexpr bool operator==(
		const basic_string<CharT, Traits, Allocator>& lhs,
		const CharT* PLUGIFY_NO_NULL rhs
	) noexcept {
		PLUGIFY_ASSERT(rhs != nullptr, "operator==(basic_string, char*): received nullptr");

		using String = basic_string<CharT, Traits, Allocator>;

		size_t rhs_len = Traits::length(rhs);
		if (__builtin_constant_p(rhs_len) && !String::fits_in_sso(rhs_len)) {
			if (!lhs.is_long()) {
				return false;
			}
		}
		if (rhs_len != lhs.size()) {
			return false;
		}
		return lhs.compare(0, String::npos, rhs, rhs_len) == 0;
	}

	template <class CharT, class Traits, class Allocator>
	constexpr auto operator<=>(
		const basic_string<CharT, Traits, Allocator>& lhs,
		const basic_string<CharT, Traits, Allocator>& rhs
	) noexcept {
		return std::basic_string_view<CharT, Traits>(lhs)
			   <=> std::basic_string_view<CharT, Traits>(rhs);
	}

	template <class CharT, class Traits, class Allocator>
	constexpr auto operator<=>(const basic_string<CharT, Traits, Allocator>& lhs, const CharT* rhs) {
		return std::basic_string_view<CharT, Traits>(lhs)
			   <=> std::basic_string_view<CharT, Traits>(rhs);
	}

	// operator +

	template <class CharT, class Traits, class Allocator>
	constexpr basic_string<CharT, Traits, Allocator> concatenate_strings(
		const Allocator& alloc,
		std::type_identity_t<std::basic_string_view<CharT, Traits>> str1,
		std::type_identity_t<std::basic_string_view<CharT, Traits>> str2
	) {
		using String = basic_string<CharT, Traits, Allocator>;
		String r(
			uninitialized_size_tag(),
			str1.size() + str2.size(),
			String::alloc_traits::select_on_container_copy_construction(alloc)
		);
		auto ptr = std::to_address(r.get_pointer());
		Traits::copy(ptr, str1.data(), str1.size());
		Traits::copy(ptr + str1.size(), str2.data(), str2.size());
		Traits::assign(ptr[str1.size() + str2.size()], CharT());
		return r;
	}

	template <class CharT, class Traits, class Allocator>
	constexpr basic_string<CharT, Traits, Allocator> operator+(
		const basic_string<CharT, Traits, Allocator>& lhs,
		const basic_string<CharT, Traits, Allocator>& rhs
	) {
		return concatenate_strings<CharT, Traits>(lhs.get_allocator(), lhs, rhs);
	}

	template <class CharT, class Traits, class Allocator>
	constexpr basic_string<CharT, Traits, Allocator>
	operator+(const CharT* lhs, const basic_string<CharT, Traits, Allocator>& rhs) {
		return concatenate_strings<CharT, Traits>(rhs.get_allocator(), lhs, rhs);
	}

	// extern template string operator+ <char, std::char_traits<char>, allocator<char> >(char
	// const*, string const&);

	template <class CharT, class Traits, class Allocator>
	constexpr basic_string<CharT, Traits, Allocator>
	operator+(CharT lhs, const basic_string<CharT, Traits, Allocator>& rhs) {
		return concatenate_strings<CharT, Traits>(
			rhs.get_allocator(),
			std::basic_string_view<CharT, Traits>(std::addressof(lhs), 1),
			rhs
		);
	}

	template <class CharT, class Traits, class Allocator>
	constexpr basic_string<CharT, Traits, Allocator>
	operator+(const basic_string<CharT, Traits, Allocator>& lhs, const CharT* rhs) {
		return concatenate_strings<CharT, Traits>(lhs.get_allocator(), lhs, rhs);
	}

	template <class CharT, class Traits, class Allocator>
	constexpr basic_string<CharT, Traits, Allocator>
	operator+(const basic_string<CharT, Traits, Allocator>& lhs, CharT rhs) {
		return concatenate_strings<CharT, Traits>(
			lhs.get_allocator(),
			lhs,
			std::basic_string_view<CharT, Traits>(std::addressof(rhs), 1)
		);
	}
#if PLUGIFY_HAS_CXX26

	template <class CharT, class Traits, class Allocator>
	constexpr basic_string<CharT, Traits, Allocator> operator+(
		const basic_string<CharT, Traits, Allocator>& lhs,
		std::type_identity_t<std::basic_string_view<CharT, Traits>> rhs
	) {
		return concatenate_strings<CharT, Traits>(lhs.get_allocator(), lhs, rhs);
	}

	template <class CharT, class Traits, class Allocator>
	constexpr basic_string<CharT, Traits, Allocator> operator+(
		std::type_identity_t<std::basic_string_view<CharT, Traits>> lhs,
		const basic_string<CharT, Traits, Allocator>& rhs
	) {
		return concatenate_strings<CharT, Traits>(rhs.get_allocator(), lhs, rhs);
	}

#endif	// PLUGIFY_HAS_CXX26

	template <class CharT, class Traits, class Allocator>
	inline constexpr basic_string<CharT, Traits, Allocator> operator+(
		basic_string<CharT, Traits, Allocator>&& lhs,
		const basic_string<CharT, Traits, Allocator>& rhs
	) {
		return std::move(lhs.append(rhs));
	}

	template <class CharT, class Traits, class Allocator>
	inline constexpr basic_string<CharT, Traits, Allocator> operator+(
		const basic_string<CharT, Traits, Allocator>& lhs,
		basic_string<CharT, Traits, Allocator>&& rhs
	) {
		return std::move(rhs.insert(0, lhs));
	}

	template <class CharT, class Traits, class Allocator>
	inline constexpr basic_string<CharT, Traits, Allocator>
	operator+(basic_string<CharT, Traits, Allocator>&& lhs, basic_string<CharT, Traits, Allocator>&& rhs) {
		return std::move(lhs.append(rhs));
	}

	template <class CharT, class Traits, class Allocator>
	inline constexpr basic_string<CharT, Traits, Allocator>
	operator+(const CharT* lhs, basic_string<CharT, Traits, Allocator>&& rhs) {
		return std::move(rhs.insert(0, lhs));
	}

	template <class CharT, class Traits, class Allocator>
	inline constexpr basic_string<CharT, Traits, Allocator>
	operator+(CharT lhs, basic_string<CharT, Traits, Allocator>&& rhs) {
		rhs.insert(rhs.begin(), lhs);
		return std::move(rhs);
	}

	template <class CharT, class Traits, class Allocator>
	inline constexpr basic_string<CharT, Traits, Allocator>
	operator+(basic_string<CharT, Traits, Allocator>&& lhs, const CharT* rhs) {
		return std::move(lhs.append(rhs));
	}

	template <class CharT, class Traits, class Allocator>
	inline constexpr basic_string<CharT, Traits, Allocator>
	operator+(basic_string<CharT, Traits, Allocator>&& lhs, CharT rhs) {
		lhs.push_back(rhs);
		return std::move(lhs);
	}

#if PLUGIFY_HAS_CXX26

	template <class CharT, class Traits, class Allocator>
	constexpr basic_string<CharT, Traits, Allocator> operator+(
		basic_string<CharT, Traits, Allocator>&& lhs,
		std::type_identity_t<std::basic_string_view<CharT, Traits>> rhs
	) {
		return std::move(lhs.append(rhs));
	}

	template <class CharT, class Traits, class Allocator>
	constexpr basic_string<CharT, Traits, Allocator> operator+(
		std::type_identity_t<std::basic_string_view<CharT, Traits>> lhs,
		basic_string<CharT, Traits, Allocator>&& rhs
	) {
		return std::move(rhs.insert(0, lhs));
	}

#endif	// PLUGIFY_HAS_CXX26

	// swap

	template <class CharT, class Traits, class Allocator>
	inline constexpr void swap(
		basic_string<CharT, Traits, Allocator>& lhs,
		basic_string<CharT, Traits, Allocator>& rhs
	) noexcept(noexcept(lhs.swap(rhs))) {
		lhs.swap(rhs);
	}

	template <class CharT, class Traits, class Allocator, class Up>
	inline constexpr typename basic_string<CharT, Traits, Allocator>::size_type
	erase(basic_string<CharT, Traits, Allocator>& str, const Up& v) {
		auto old_size = str.size();
		str.erase(std::remove(str.begin(), str.end(), v), str.end());
		return old_size - str.size();
	}

	template <class CharT, class Traits, class Allocator, class Predicate>
	inline constexpr typename basic_string<CharT, Traits, Allocator>::size_type
	erase_if(basic_string<CharT, Traits, Allocator>& str, Predicate pred) {
		auto old_size = str.size();
		str.erase(std::remove_if(str.begin(), str.end(), pred), str.end());
		return old_size - str.size();
	}

	// basic_string typedef-names
	using string = basic_string<char>;
	using u8string = basic_string<char8_t>;
	using u16string = basic_string<char16_t>;
	using u32string = basic_string<char32_t>;
	using wstring = basic_string<wchar_t>;

#ifndef PLUGIFY_STRING_NO_NUMERIC_CONVERSIONS
	// numeric conversions
	inline int stoi(const string& str, std::size_t* pos = nullptr, int base = 10) {
		auto cstr = str.c_str();
		char* ptr = const_cast<char*>(cstr);

		auto ret = strtol(cstr, &ptr, base);
		if (pos != nullptr) {
			*pos = static_cast<size_t>(cstr - ptr);
		}

		return static_cast<int>(ret);
	}

	inline long stol(const string& str, std::size_t* pos = nullptr, int base = 10) {
		auto cstr = str.c_str();
		char* ptr = const_cast<char*>(cstr);

		auto ret = strtol(cstr, &ptr, base);
		if (pos != nullptr) {
			*pos = static_cast<size_t>(cstr - ptr);
		}

		return ret;
	}

	inline long long stoll(const string& str, std::size_t* pos = nullptr, int base = 10) {
		auto cstr = str.c_str();
		char* ptr = const_cast<char*>(cstr);

		auto ret = strtoll(cstr, &ptr, base);
		if (pos != nullptr) {
			*pos = static_cast<size_t>(cstr - ptr);
		}

		return ret;
	}

	inline unsigned long stoul(const string& str, std::size_t* pos = nullptr, int base = 10) {
		auto cstr = str.c_str();
		char* ptr = const_cast<char*>(cstr);

		auto ret = strtoul(cstr, &ptr, base);
		if (pos != nullptr) {
			*pos = static_cast<size_t>(cstr - ptr);
		}

		return ret;
	}

	inline unsigned long long stoull(const string& str, std::size_t* pos = nullptr, int base = 10) {
		auto cstr = str.c_str();
		char* ptr = const_cast<char*>(cstr);

		auto ret = strtoull(cstr, &ptr, base);
		if (pos != nullptr) {
			*pos = static_cast<size_t>(cstr - ptr);
		}

		return ret;
	}

	inline float stof(const string& str, std::size_t* pos = nullptr) {
		auto cstr = str.c_str();
		char* ptr = const_cast<char*>(cstr);

		auto ret = strtof(cstr, &ptr);
		if (pos != nullptr) {
			*pos = static_cast<size_t>(cstr - ptr);
		}

		return ret;
	}

	inline double stod(const string& str, std::size_t* pos = nullptr) {
		auto cstr = str.c_str();
		char* ptr = const_cast<char*>(cstr);

		auto ret = strtod(cstr, &ptr);
		if (pos != nullptr) {
			*pos = static_cast<size_t>(cstr - ptr);
		}

		return ret;
	}

	inline long double stold(const string& str, std::size_t* pos = nullptr) {
		auto cstr = str.c_str();
		char* ptr = const_cast<char*>(cstr);

		auto ret = strtold(cstr, &ptr);
		if (pos != nullptr) {
			*pos = static_cast<size_t>(cstr - ptr);
		}

		return ret;
	}

	namespace detail {
		template <typename S, typename V>
		constexpr S to_string(V v) {
			//  numeric_limits::digits10 returns value less on 1 than desired for unsigned numbers.
			//  For example, for 1-byte unsigned value digits10 is 2 (999 can not be represented),
			//  so we need +1 here.
			constexpr std::size_t bufSize = std::numeric_limits<V>::digits10 + 2;  // +1 for minus,
																				   // +1 for
																				   // digits10
			char buf[bufSize];
			const auto res = std::to_chars(buf, buf + bufSize, v);
			return S(buf, res.ptr);
		}

		typedef int (*wide_printf)(wchar_t* __restrict, std::size_t, const wchar_t* __restrict, ...);

#if PLUGIFY_COMPILER_MSVC
		inline int truncate_snwprintf(
			wchar_t* __restrict buffer,
			std::size_t count,
			const wchar_t* __restrict format,
			...
		) {
			int r;
			va_list args;
			va_start(args, format);
			r = _vsnwprintf_s(buffer, count, _TRUNCATE, format, args);
			va_end(args);
			return r;
		}
#endif

		constexpr wide_printf get_swprintf() noexcept {
#if PLUGIFY_COMPILER_MSVC
			return static_cast<
				int(__cdecl*)(wchar_t* __restrict, std::size_t, const wchar_t* __restrict, ...)>(
				truncate_snwprintf
			);
#else
			return swprintf;
#endif
		}

		template <typename S, typename P, typename V>
		constexpr S as_string(P sprintf_like, const typename S::value_type* fmt, V v) {
			typedef typename S::size_type size_type;
			S s;
			s.resize(s.capacity());
			size_type available = s.size();
			while (true) {
				int status = sprintf_like(&s[0], available + 1, fmt, v);
				if (status >= 0) {
					auto used = static_cast<size_type>(status);
					if (used <= available) {
						s.resize(used);
						break;
					}
					available = used;  // Assume this is advice of how much space we need.
				} else {
					available = available * 2 + 1;
				}
				s.resize(available);
			}
			return s;
		}
	}  // namespace detail

	inline string to_string(int val) { return detail::to_string<string>(val); }
	inline string to_string(unsigned val) { return detail::to_string<string>(val); }
	inline string to_string(long val) { return detail::to_string<string>(val); }
	inline string to_string(unsigned long val) { return detail::to_string<string>(val); }
	inline string to_string(long long val) { return detail::to_string<string>(val); }
	inline string to_string(unsigned long long val) { return detail::to_string<string>(val); }
	inline string to_string(float val) { return detail::as_string<string>(snprintf, "%f", val); }
	inline string to_string(double val) { return detail::as_string<string>(snprintf, "%f", val); }
	inline string to_string(long double val) { return detail::as_string<string>(snprintf, "%Lf", val); }

	inline wstring to_wstring(int val) { return detail::to_string<wstring>(val); }
	inline wstring to_wstring(unsigned val) { return detail::to_string<wstring>(val); }
	inline wstring to_wstring(long val) { return detail::to_string<wstring>(val); }
	inline wstring to_wstring(unsigned long val) { return detail::to_string<wstring>(val); }
	inline wstring to_wstring(long long val) { return detail::to_string<wstring>(val); }
	inline wstring to_wstring(unsigned long long val) { return detail::to_string<wstring>(val); }
	inline wstring to_wstring(float val) { return detail::as_string<wstring>(detail::get_swprintf(), L"%f", val); }
	inline wstring to_wstring(double val) { return detail::as_string<wstring>(detail::get_swprintf(), L"%f", val); }
	inline wstring to_wstring(long double val) { return detail::as_string<wstring>(detail::get_swprintf(), L"%Lf", val); }
#endif	// PLUGIFY_STRING_NO_NUMERIC_CONVERSIONS

#ifndef PLUGIFY_STRING_NO_STD_HASH
	// hash support
	namespace detail {
		template <
			typename Char,
			typename Allocator,
			typename String = basic_string<Char, std::char_traits<Char>, Allocator>
		>
		struct string_hash_base {
			constexpr std::size_t operator()(const String& str) const noexcept {
				return std::hash<typename String::self_view>{}(typename String::self_view(str));
			}
		};
	}  // namespace detail
#endif	// PLUGIFY_STRING_NO_STD_HASH

#ifndef PLUGIFY_STRING_NO_STD_FORMAT
	// format support
	namespace detail {
		template <typename Char>
		static constexpr const Char* format_string() {
			if constexpr (std::is_same_v<Char, char> || std::is_same_v<Char, char8_t>) {
				return "{}";
			}
			if constexpr (std::is_same_v<Char, wchar_t>) {
				return L"{}";
			}
			if constexpr (std::is_same_v<Char, char16_t>) {
				return u"{}";
			}
			if constexpr (std::is_same_v<Char, char32_t>) {
				return U"{}";
			}
			return "";
		}

		template <
			typename Char,
			typename Allocator,
			typename String = basic_string<Char, std::char_traits<Char>, Allocator>
		>
		struct string_formatter_base {
			constexpr auto parse(std::format_parse_context& ctx) {
				return ctx.begin();
			}

			template <class FormatContext>
			auto format(const String& str, FormatContext& ctx) const {
				return std::format_to(ctx.out(), format_string<Char>(), str.c_str());
			}
		};
	}
#endif	// PLUGIFY_STRING_NO_STD_FORMAT

	inline namespace literals {
		inline namespace string_literals {
			PLUGIFY_WARN_PUSH()

#if PLUGIFY_COMPILER_CLANG
			PLUGIFY_WARN_IGNORE("-Wuser-defined-literals")
#elif PLUGIFY_COMPILER_GCC
			PLUGIFY_WARN_IGNORE("-Wliteral-suffix")
#elif PLUGIFY_COMPILER_MSVC
			PLUGIFY_WARN_IGNORE(4455)
#endif

			// suffix for basic_string literals
			constexpr string operator""s(const char* str, std::size_t len) { return string{str, len}; }
			constexpr u8string operator""s(const char8_t* str, std::size_t len) { return u8string{str, len}; }
			constexpr u16string operator""s(const char16_t* str, std::size_t len) { return u16string{str, len}; }
			constexpr u32string operator""s(const char32_t* str, std::size_t len) { return u32string{str, len}; }
			constexpr wstring operator""s(const wchar_t* str, std::size_t len) { return wstring{str, len}; }

			PLUGIFY_WARN_POP()
		}  // namespace string_literals
	}  // namespace literals
}  // namespace plg

#ifndef PLUGIFY_STRING_NO_STD_HASH
// hash support
namespace std {
	template <typename Allocator>
	struct hash<plg::basic_string<char, std::char_traits<char>, Allocator>>
		: plg::detail::string_hash_base<char, Allocator> {};

	template <typename Allocator>
	struct hash<plg::basic_string<char8_t, std::char_traits<char8_t>, Allocator>>
		: plg::detail::string_hash_base<char8_t, Allocator> {};

	template <typename Allocator>
	struct hash<plg::basic_string<char16_t, std::char_traits<char16_t>, Allocator>>
		: plg::detail::string_hash_base<char16_t, Allocator> {};

	template <typename Allocator>
	struct hash<plg::basic_string<char32_t, std::char_traits<char32_t>, Allocator>>
		: plg::detail::string_hash_base<char32_t, Allocator> {};

	template <typename Allocator>
	struct hash<plg::basic_string<wchar_t, std::char_traits<wchar_t>, Allocator>>
		: plg::detail::string_hash_base<wchar_t, Allocator> {};
}  // namespace std
#endif	// PLUGIFY_STRING_NO_STD_HASH

#ifndef PLUGIFY_STRING_NO_STD_FORMAT
// format support
#ifdef FMT_HEADER_ONLY
namespace fmt {
#else
namespace std {
#endif
	template <typename Allocator>
	struct formatter<plg::basic_string<char, std::char_traits<char>, Allocator>>
		: plg::detail::string_formatter_base<char, Allocator> {};

	template <typename Allocator>
	struct formatter<plg::basic_string<char8_t, std::char_traits<char8_t>, Allocator>>
		: plg::detail::string_formatter_base<char8_t, Allocator> {};

	template <typename Allocator>
	struct formatter<plg::basic_string<char16_t, std::char_traits<char16_t>, Allocator>>
		: plg::detail::string_formatter_base<char16_t, Allocator> {};

	template <typename Allocator>
	struct formatter<plg::basic_string<char32_t, std::char_traits<char32_t>, Allocator>>
		: plg::detail::string_formatter_base<char32_t, Allocator> {};

	template <typename Allocator>
	struct formatter<plg::basic_string<wchar_t, std::char_traits<wchar_t>, Allocator>>
		: plg::detail::string_formatter_base<wchar_t, Allocator> {};
}  // namespace std
#endif	// PLUGIFY_STRING_NO_STD_FORMAT

template <typename Char, typename Traits, typename Alloc>
std::ostream& operator<<(std::ostream& os, const plg::basic_string<Char, Traits, Alloc>& str) {
	os << str.c_str();
	return os;
}

#ifndef PLUGIFY_STRING_NO_STD_FORMAT
#include <functional>

namespace plg {
	template <typename Range>
	constexpr string join(const Range& range, std::string_view separator) {
		string result;

		auto it = range.cbegin();
		auto end = range.cend();

		if (it == end) {
			return result;
		}

		// First pass: compute total size
		size_t total_size = 0;
		size_t count = 0;

		for (auto tmp = it; tmp != end; ++tmp) {
			using elem = std::decay_t<decltype(*tmp)>;
			if constexpr (string_like<elem>) {
				total_size += std::string_view(*tmp).size();
			} else {
				total_size += std::formatted_size("{}", *tmp);
			}
			++count;
		}
		if (count > 1) {
			total_size += (count - 1) * separator.size();
		}
		result.reserve(total_size);

		auto in = std::back_inserter(result);

		// Second pass: actual formatting
		/*if (it != end)*/ { std::format_to(in, "{}", *it++); }
		while (it != end) {
			std::format_to(in, "{}{}", separator, *it++);
		}

		return result;
	}

	template <typename Range, typename Proj>
	constexpr string join(const Range& range, Proj&& proj, std::string_view separator) {
		string result;

		auto it = range.cbegin();
		auto end = range.cend();

		if (it == end) {
			return result;
		}

		// First pass: compute total size
		size_t total_size = 0;
		size_t count = 0;

		for (auto tmp = it; tmp != end; ++tmp) {
			auto&& projected = std::invoke(std::forward<Proj>(proj), *tmp);
			using elem = std::decay_t<decltype(projected)>;
			if constexpr (string_like<elem>) {
				total_size += std::string_view(*projected).size();
			} else {
				total_size += std::formatted_size("{}", projected);
			}
			++count;
		}
		if (count > 1) {
			total_size += (count - 1) * separator.size();
		}
		result.reserve(total_size);

		auto out = std::back_inserter(result);

		// Second pass: actual formatting
		{
			auto&& projected = std::invoke(std::forward<Proj>(proj), *it++);
			std::format_to(out, "{}", projected);
		}
		while (it != end) {
			auto&& projected = std::invoke(std::forward<Proj>(proj), *it++);
			std::format_to(out, "{}{}", separator, projected);
		}

		return result;
	}
}  // namespace plugify
#endif	// PLUGIFY_STRING_NO_STD_FORMAT