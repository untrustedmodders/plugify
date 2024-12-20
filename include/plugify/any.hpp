#pragma once

#include <plugify/numerics.hpp>
#include <plugify/string.hpp>
#include <plugify/variant.hpp>
#include <plugify/vector.hpp>

namespace plg {
	struct invalid {};
	struct none {};
	union function { void* ptr; };

	// TODO: Add comments explaing tags
	using any = variant<
			invalid,

			none,
			bool,
			char,
			char16_t,
			int8_t,
			int16_t,
			int32_t,
			int64_t,
			uint8_t,
			uint16_t,
			uint32_t,
			uint64_t,
			void*,
			float,
			double,
			function,
			string,
			variant<none>,
			vector<bool>,
			vector<char>,
			vector<char16_t>,
			vector<int8_t>,
			vector<int16_t>,
			vector<int32_t>,
			vector<int64_t>,
			vector<uint8_t>,
			vector<uint16_t>,
			vector<uint32_t>,
			vector<uint64_t>,
			vector<void*>,
			vector<float>,
			vector<double>,
			vector<string>,
			vector<variant<none>>,
			vector<vec2>,
			vector<vec3>,
			vector<vec4>,
			vector<mat4x4>,
			vec2,
			vec3,
			vec4
			//mat4x4
			>;
} // namespace plg
