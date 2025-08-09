#pragma once

#include <memory>
#include <string>

namespace plugify {
	/**
	 * @struct ErrorData
	 * @brief Holds information about an error.
	 *
	 * The ErrorData structure contains a string describing an error.
	 */
	struct ErrorData {
		ErrorData(std::string_view str) {
			error = std::make_shared<char[]>(str.size() + 1);
			std::memcpy(error.get(), str.data(), str.size() * sizeof(char));
			error[str.size()] = 0; // Null-terminate the string
		}

		std::string_view string() const noexcept {
			return error.get();
		}

	private:
		std::shared_ptr<char[]> error; ///< Description of the error.
	};
} // namespace plugify


