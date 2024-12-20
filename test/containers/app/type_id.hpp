//===----------------------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#pragma once

#include <functional>
#include <string>
#include <cstdio>
#include <cassert>

// TypeID - Represent a unique identifier for a type. TypeID allows equality
// comparisons between different types.
struct TypeID {
	friend bool operator==(TypeID const& LHS, TypeID const& RHS)
	{return LHS.m_id == RHS.m_id; }
	friend bool operator!=(TypeID const& LHS, TypeID const& RHS)
	{return LHS.m_id != RHS.m_id; }

	[[nodiscard]] std::string_view name() const {
		return m_id;
	}

private:
	explicit constexpr TypeID(const char* xid) : m_id(xid) {}

	TypeID(const TypeID&) = delete;
	TypeID& operator=(TypeID const&) = delete;

	const char* const m_id;
	template <class T> friend TypeID const& makeTypeIDImp();
};

// makeTypeID - Return the TypeID for the specified type 'T'.
template <class T>
inline TypeID const& makeTypeIDImp() {
#ifdef _MSC_VER
	static const TypeID id(__FUNCSIG__);
#else
	static const TypeID id(__PRETTY_FUNCTION__);
#endif // _MSC_VER
	return id;
}

template <class T>
struct TypeWrapper {};

template <class T>
inline  TypeID const& makeTypeID() {
	return makeTypeIDImp<TypeWrapper<T>>();
}

template <class ...Args>
struct ArgumentListID {};

// makeArgumentID - Create and return a unique identifier for a given set
// of arguments.
template <class ...Args>
inline  TypeID const& makeArgumentID() {
	return makeTypeIDImp<ArgumentListID<Args...>>();
}


// COMPARE_TYPEID(...) is a utility macro for generating diagnostics when
// two typeid's are expected to be equal
#define COMPARE_TYPEID(LHS, RHS) CompareTypeIDVerbose(#LHS, LHS, #RHS, RHS)

inline bool CompareTypeIDVerbose(const char* LHSString, TypeID const* LHS,
								 const char* RHSString, TypeID const* RHS) {
	if (*LHS == *RHS)
		return true;
	std::printf("TypeID's not equal:\n");
	std::printf("%s: %s\n----------\n%s: %s\n",
				LHSString, LHS->name().data(),
				RHSString, RHS->name().data());
	return false;
}