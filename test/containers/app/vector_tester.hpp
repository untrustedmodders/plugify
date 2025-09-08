//===----------------------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#pragma once

#include <plg/format.hpp>
#include <plg/vector.hpp>

#include <cstddef>
#include <stdexcept>
#include <vector>

template <typename VecA, typename VecB>
void assert_eq(VecA const& a, VecB const& b) {
	 if (a.size() != b.size()) {
		  throw std::runtime_error(std::format("vec size != svec size: {} != {}", a.size(), b.size()));
	 }
	 if (!std::equal(a.begin(), a.end(), b.begin(), b.end())) {
		  throw std::runtime_error(
				std::format("std::vec content != plg::vec content:\n[{{{}}}]\n[{{{}}}]", /*plg::join(a, ", "), plg::join(b, ", ")*/ "array1", "array2"));
	 }
}

template <class T>
class VecTester {
	 std::vector<T> _v{};
	 plg::vector<T> _s{};

public:
	 template <class... Args>
	 void emplace_back(Args&&... args) {
		  _v.emplace_back(std::forward<Args>(args)...);
		  _s.emplace_back(std::forward<Args>(args)...);
		  assert_eq(_v, _s);
	 }

	 template <class... Args>
	 void emplace_at(size_t idx, Args&&... args) {
		  auto it_v = _v.emplace(_v.begin() + idx, std::forward<Args>(args)...);
		  auto it_s = _s.emplace(_s.cbegin() + idx, std::forward<Args>(args)...);
		  REQUIRE(*it_v == *it_s);
		  assert_eq(_v, _s);
	 }

	 [[nodiscard]] auto size() const -> size_t {
		  return _v.size();
	 }
};
