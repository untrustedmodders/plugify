//===----------------------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#pragma once

#include <cstddef>                   // for size_t
#include <functional>                // for hash
#include <iosfwd>                    // for ostream
#include <string>                    // for allocator, string
#include <string_view>               // for hash, string_view

#include <plg/format.hpp> // for format_context, format_parse_context, format_to

struct Counter {
	 // Obj for only swaps & equals. Used for optimizing.
	 // Can't use static counters here because I want to do it in parallel.
	 class Obj {
	 public:
		  // required for operator[]
		  Obj();
		  Obj(const size_t& data, Counter& counts);
		  Obj(const Obj& o);
		  Obj(Obj&& o) noexcept;
		  ~Obj();

		  auto operator==(const Obj& o) const -> bool;
		  auto operator<(const Obj& o) const -> bool;
		  auto operator=(const Obj& o) -> Obj&;
		  auto operator=(Obj&& o) noexcept -> Obj&;

		  [[nodiscard]] auto get() const -> size_t const&;
		  auto get() -> size_t&;

		  void swap(Obj& other);
		  [[nodiscard]] auto getForHash() const -> size_t;

		  // helper to set counter when not yet set
		  void set(Counter& counter);

		auto operator<=>(const Obj&) const = default;

	 private:
		  size_t _data;
		  Counter* _counts;
	 };

	 Counter();
	 ~Counter();

	 void check_all_done() const;

	 size_t ctor{};
	 size_t defaultCtor{};
	 size_t copyCtor{};
	 size_t dtor{};
	 size_t equals{};
	 size_t less{};
	 size_t assign{};
	 size_t swaps{};
	 size_t get{};
	 size_t constGet{};
	 size_t hash{};
	 size_t moveCtor{};
	 size_t moveAssign{};

	 std::string _records =
		  "\n	  ctor  defctor  cpyctor	  dtor	assign	 swaps		get  cnstget	  hash	equals	  less	ctormv assignmv|	total |\n";

	 void operator()(std::string_view title);

	 [[nodiscard]] auto total() const -> size_t;

	 static inline size_t staticDefaultCtor = 0;
	 static inline size_t staticCopyCtor = 0;
	 static inline size_t staticMoveCtor = 0;
	 static inline size_t staticDtor = 0;
};

// Throws an exception, this overload should never be taken!
inline auto operator new(size_t s, Counter::Obj* ptr) -> void*;

auto operator<<(std::ostream& os, Counter const& c) -> std::ostream&;

namespace std {

template <>
struct hash<Counter::Obj> {
	 [[nodiscard]] auto operator()(const Counter::Obj& c) const noexcept -> size_t {
		  return hash<size_t>{}(c.getForHash());
	 }
};

} // namespace std

template <>
struct std::formatter<Counter::Obj> {
	 static constexpr auto parse(format_parse_context& ctx) -> decltype(ctx.begin()) {
		  return ctx.end();
	 }
	 static auto format(Counter::Obj const& o, std::format_context& ctx) -> decltype(ctx.out()) {
		  return format_to(ctx.out(), "{}", o.get());
	 }
};
