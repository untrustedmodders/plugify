#pragma once

namespace plugify {
	template<typename Cnt, typename Pr = std::equal_to<typename Cnt::value_type>>
	constexpr bool RemoveDuplicates(Cnt &cnt, Pr cmp = Pr()) {
		auto size = std::size(cnt);
		Cnt result;
		result.reserve(size);

		std::copy_if(
				std::make_move_iterator(std::begin(cnt)),
				std::make_move_iterator(std::end(cnt)),
				std::back_inserter(result),
				[&](const typename Cnt::value_type& what) {
					return std::find_if(std::begin(result), std::end(result), [&](const typename Cnt::value_type& existing) {
							   return cmp(what, existing);
						   }) == std::end(result);
				}
		);

		cnt = std::move(result);
		return std::size(cnt) != size;
	}
} // namespace plugify