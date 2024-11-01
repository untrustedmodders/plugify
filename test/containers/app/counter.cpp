#include "counter.hpp"

#include <cstdlib>// for abort
#include <iostream>
#include <ostream>		// for ostream
#include <stdexcept>	// for runtime_error
#include <unordered_set>// for unordered_set
#include <utility>		// for swap, pair

#define COUNTER_ENABLE_UNORDERED_SET 1

#if COUNTER_ENABLE_UNORDERED_SET
auto singletonConstructedObjects() -> std::unordered_set<Counter::Obj const*>& {
	 static std::unordered_set<Counter::Obj const*> data{};
	 return data;
}
#endif

Counter::Obj::Obj()
	 : _data(0)
	 , _counts(nullptr) {
#if COUNTER_ENABLE_UNORDERED_SET
	 if (!singletonConstructedObjects().emplace(this).second) {
		  std::cerr << std::format("ERROR at {}({}): {}", __FILE__, __LINE__, __func__) << std::endl;
		  std::abort();
	 }
#endif
	 ++staticDefaultCtor;
}

Counter::Obj::Obj(const size_t& data, Counter& counts)
	 : _data(data)
	 , _counts(&counts) {
#if COUNTER_ENABLE_UNORDERED_SET
	 if (!singletonConstructedObjects().emplace(this).second) {
		  std::cerr << std::format("ERROR at {}({}): {}", __FILE__, __LINE__, __func__) << std::endl;
		  std::abort();
	 }
#endif
	 ++_counts->ctor;
}

Counter::Obj::Obj(const Counter::Obj& o)
	 : _data(o._data)
	 , _counts(o._counts) {
#if COUNTER_ENABLE_UNORDERED_SET
	 if (1 != singletonConstructedObjects().count(&o)) {
		  std::cerr << std::format("ERROR at {}({}): {}", __FILE__, __LINE__, __func__) << std::endl;
		  std::abort();
	 }
	 if (!singletonConstructedObjects().emplace(this).second) {
		  std::cerr << std::format("ERROR at {}({}): {}", __FILE__, __LINE__, __func__) << std::endl;
		  std::abort();
	 }
#endif
	 if (nullptr != _counts) {
		  ++_counts->copyCtor;
	 }
}

Counter::Obj::Obj(Counter::Obj&& o) noexcept
	 : _data(o._data)
	 , _counts(o._counts) {
#if COUNTER_ENABLE_UNORDERED_SET
	 if (1 != singletonConstructedObjects().count(&o)) {
		  std::cerr << std::format("ERROR at {}({}): {}", __FILE__, __LINE__, __func__) << std::endl;
		  std::abort();
	 }
	 if (!singletonConstructedObjects().emplace(this).second) {
		  std::cerr << std::format("ERROR at {}({}): {}", __FILE__, __LINE__, __func__) << std::endl;
		  std::abort();
	 }
#endif
	 if (nullptr != _counts) {
		  ++_counts->moveCtor;
	 }
}

Counter::Obj::~Obj() {
#if COUNTER_ENABLE_UNORDERED_SET
	 if (1 != singletonConstructedObjects().erase(this)) {
		  std::cerr << std::format("ERROR at {}({}): {}", __FILE__, __LINE__, __func__) << std::endl;
		  std::abort();
	 }
#endif
	 if (nullptr != _counts) {
		  ++_counts->dtor;
	 } else {
		  ++staticDtor;
	 }
}

void Counter::Obj::set(Counter& c) {
	_counts = &c;
}

auto Counter::Obj::operator==(const Counter::Obj& o) const -> bool {
#if COUNTER_ENABLE_UNORDERED_SET
	 if (1 != singletonConstructedObjects().count(this) || 1 != singletonConstructedObjects().count(&o)) {
		  std::cerr << std::format("ERROR at {}({}): {}", __FILE__, __LINE__, __func__) << std::endl;
		  std::abort();
	 }
#endif
	 if (nullptr != _counts) {
		  ++_counts->equals;
	 }
	 return _data == o._data;
}

auto Counter::Obj::operator<(const Obj& o) const -> bool {
#if COUNTER_ENABLE_UNORDERED_SET
	 if (1 != singletonConstructedObjects().count(this) || 1 != singletonConstructedObjects().count(&o)) {
		  std::cerr << std::format("ERROR at {}({}): {}", __FILE__, __LINE__, __func__) << std::endl;
		  std::abort();
	 }
#endif
	 if (nullptr != _counts) {
		  ++_counts->less;
	 }
	 return _data < o._data;
}

auto Counter::Obj::operator=(const Counter::Obj& o) -> Counter::Obj& {
#if COUNTER_ENABLE_UNORDERED_SET
	 if (1 != singletonConstructedObjects().count(this) || 1 != singletonConstructedObjects().count(&o)) {
		  std::cerr << std::format("ERROR at {}({}): {}", __FILE__, __LINE__, __func__) << std::endl;
		  std::abort();
	 }
#endif
	_counts = o._counts;
	 if (nullptr != _counts) {
		  ++_counts->assign;
	 } else {
		  ++staticCopyCtor;
	 }
	_data = o._data;
	 return *this;
}

auto Counter::Obj::operator=(Counter::Obj&& o) noexcept -> Counter::Obj& {
#if COUNTER_ENABLE_UNORDERED_SET
	 if (1 != singletonConstructedObjects().count(this) || 1 != singletonConstructedObjects().count(&o)) {
		  std::cerr << std::format("ERROR at {}({}): {}", __FILE__, __LINE__, __func__) << std::endl;
		  std::abort();
	 }
#endif
	 if (nullptr != o._counts) {
		_counts = o._counts;
	 }
	_data = o._data;
	 if (nullptr != _counts) {
		  ++_counts->moveAssign;
	 } else {
		  ++staticMoveCtor;
	 }
	 return *this;
}

auto Counter::Obj::get() const -> size_t const& {
	 if (nullptr != _counts) {
		  ++_counts->constGet;
	 }
	 return _data;
}

auto Counter::Obj::get() -> size_t& {
	 if (nullptr != _counts) {
		  ++_counts->get;
	 }
	 return _data;
}

void Counter::Obj::swap(Obj& other) {
#if COUNTER_ENABLE_UNORDERED_SET
	 if (1 != singletonConstructedObjects().count(this) || 1 != singletonConstructedObjects().count(&other)) {
		  std::cerr << std::format("ERROR at {}({}): {}", __FILE__, __LINE__, __func__) << std::endl;
		  std::abort();
	 }
#endif
	 using std::swap;
	 swap(_data, other._data);
	 swap(_counts, other._counts);
	 if (nullptr != _counts) {
		  ++_counts->swaps;
	 }
}

auto Counter::Obj::getForHash() const -> size_t {
	 if (nullptr != _counts) {
		  ++_counts->hash;
	 }
	 return _data;
}

Counter::Counter() {
	 Counter::staticDefaultCtor = 0;
	 Counter::staticCopyCtor = 0;
	 Counter::staticMoveCtor = 0;
	 Counter::staticDtor = 0;
}

void Counter::check_all_done() const {
#if COUNTER_ENABLE_UNORDERED_SET
	 // check that all are destructed
	 if (!singletonConstructedObjects().empty()) {
		  std::cerr << std::format("ERROR at ~Counter(): got {} objects still alive!", singletonConstructedObjects().size()) << std::endl;
		  std::abort();
	 }
	 if (dtor + staticDtor != ((ctor + staticDefaultCtor) + (copyCtor + staticCopyCtor) + defaultCtor + (moveCtor + staticMoveCtor))) {
		  std::cerr << std::format("ERROR at ~Counter(): number of counts does not match!\\n") << std::format(
				"{} dtor + {} staticDtor != ({} ctor + {} staticDefaultCtor) + ({} copyCtor + {} staticCopyCtor) + {} defaultCtor + ({} moveCtor + {} staticMoveCtor)",
				dtor,
				staticDtor,
				ctor,
				staticDefaultCtor,
				copyCtor,
				staticCopyCtor,
				defaultCtor,
				moveCtor,
				staticMoveCtor) << std::endl;
		  std::abort();
	 }
#endif
}

Counter::~Counter() {
	 check_all_done();
}

auto Counter::total() const -> size_t {
	 return (ctor + staticDefaultCtor) + copyCtor + (dtor + staticDtor) + equals + less + assign + swaps + get + constGet + hash + moveCtor + moveAssign;
}

void Counter::operator()(std::string_view title) {
	std::format_to(std::back_inserter(_records),
					"{:9}{:9}{:9}{:9}{:9}{:9}{:9}{:9}{:9}{:9}{:9}{:9}{:9}|{:9}| {}\n",
					ctor,
					staticDefaultCtor,
					copyCtor,
					dtor + staticDtor,
					assign,
					swaps,
					get,
					constGet,
					hash,
					equals,
					less,
					moveCtor,
					moveAssign,
					total(),
					title);
}

auto operator<<(std::ostream& os, Counter const& c) -> std::ostream& {
	 return os << c._records;
}

auto operator new(size_t /*unused*/, Counter::Obj* /*unused*/) -> void* {
	 throw std::runtime_error("operator new overload is taken! Cast to void* to ensure the void pointer overload is taken.");
}
