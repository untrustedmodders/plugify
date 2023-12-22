#pragma once

namespace wizard {
    /**
     * Helper class for C++ strings.
     */
    class WIZARD_API String {
    public:
        String() = delete;

        /**
         * Converts a string to a valid boolean.
         * @param str The string.
         * @return The converted bool.
         */
        static bool ConvertBool(std::string str);

        /**
         * Splits a string by a separator.
         * @param str The string.
         * @param sep The separator.
         * @return The split string vector.
         */
        static std::vector<std::string> Split(const std::string& str, char sep);

        /**
         * Concatenates a strings, using the specified separator between each member.
         * @param strings A collection that contains the strings to concatenate.
         * @param separator The string to use as a separator is included in the returned string only if values has more than one element.
         * @return A string that consists of the elements of values delimited by the separator string.
         * @link https://programming-idioms.org/idiom/53/join-a-list-of-strings/1552/cpp
         */
        static std::string Join(std::span<const std::string> strings, std::string_view separator);

        /**
         * Gets if a string starts with a token.
         * @param str The string.
         * @param token The token.
         * @return If a string starts with the token.
         */
        static bool StartsWith(std::string_view str, std::string_view token);

        /**
         * Gets if a string contains a token.
         * @param str The string.
         * @param token The token.
         * @return If a string contains the token.
         */
        static bool Contains(std::string_view str, std::string_view token) noexcept;

        /**
         * Gets if a character is a whitespace.
         * @param c The character.
         * @return If a character is a whitespace.
         */
        static bool IsWhitespace(char c) noexcept;

        /**
         * Gets if a string is a number.
         * @param str The string.
         * @return If a string is a number.
         */
        static bool IsNumber(std::string_view str) noexcept;

        /**
         * Gets the first char index in the string.
         * @param str The string.
         * @param c The char to look for.
         * @return The char index.
         */
        static size_t FindCharPos(std::string_view str, char c) noexcept;

        /**
         * Trims the left and right side of a string of whitespace.
         * @param str The string.
         * @param whitespace The whitespace type.
         * @return The trimmed string.
         */
        static std::string_view Trim(std::string_view str, std::string_view whitespace = " \t\n\r");

        /**
         * Extract string between the left and right side of a string of delimiter.
         * @param str The string.
         * @param start The start delimiter.
         * @param stop The stop delimiter.
         * @return The extracted string.
         */
        static std::string_view Extract(std::string_view str, std::string_view start, std::string_view stop);

        /**
         * Removes all tokens from a string.
         * @param str The string.
         * @param token The token.
         * @return The string with the tokens removed.
         */
        static std::string RemoveAll(std::string str, char token);

        /**
         * Removes the last token from a string.
         * @param str The string.
         * @param token The token.
         * @return The string with the last token removed.
         */
        static std::string RemoveLast(std::string str, char token);

        /**
         * Replaces all tokens from a string.
         * @param str The string.
         * @param token The token.
         * @param to The string to replace the tokens with.
         * @return The string with the tokens replaced.
         */
        static std::string ReplaceAll(std::string str, std::string_view token, std::string_view to);

        /**
         * Replaces the first token from a string.
         * @param str The string.
         * @param token The token.
         * @param to The string to replace the tokens with.
         * @return The string with the tokens replaced.
         */
        static std::string ReplaceFirst(std::string str, std::string_view token, std::string_view to);

        /**
         * Fixes all tokens return line tokens from a string.
         * @param str The string.
         * @return The string with return lines fixed.
         */
        static std::string FixEscapedChars(std::string str);

        /**
         * Unfixes all tokens return line tokens from a string.
         * @param str The string.
         * @return The string with return lines unfixed.
         */
        static std::string UnfixEscapedChars(std::string str);

        /**
         * Lower cases a string.
         * @param str The string.
         * @return The lowercased string.
         */
        static std::string Lowercase(std::string str);

        /**
         * Uppercases a string.
         * @param str The string.
         * @return The uppercased string.
         */
        static std::string Uppercase(std::string str);

        /**
         * Quotes a string.
         * @param str The string.
         * @return The uppercased string.
         */
        static std::string Quoted(const std::string& str);

        /**
         * Find position of a string.
         * @param str The string.
         * @param pattern The pattern
         * @param pos Index of character to search from (default 0).
         * @return Index of start of first occurrence.
         */
        static size_t FindInsensitive(std::string str, std::string pattern, size_t pos = 0);
    };
}