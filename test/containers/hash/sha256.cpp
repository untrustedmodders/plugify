#include <catch_amalgamated.hpp>

#include "../../../src/utils/sha256.hpp"

TEST_CASE("plugify::Sha256 vectors", "[sha256]") {
    std::vector<std::pair<std::string, std::string>> vectors = {
        // 📦 Baseline & well-known vectors
        {"", "e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855"},
        {"abc", "ba7816bf8f01cfea414140de5dae2223b00361a396177a9cb410ff61f20015ad"},
        {"message digest", "f7846f55cf23e14eebeab5b4e1550cad5b509e3348fbc4efa3a1413d393cb650"},
        {"abcdefghijklmnopqrstuvwxyz", "71c480df93d6ae2f1efad1447c66c9525e316218cf51fc8d9ed832f2daf18b73"},
        {"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789", "db4bfcbd4da0cd85a60c3c37d3fbd8805c77f15fc6b1fdfe614ee0a7c8fdb4c0"},
        {"1234567890", "c775e7b757ede630cd0aa1113bd102661ab38829ca52a6422ab782862f268646"},

        // 🔁 Repeated characters
        {std::string(1000, 'a'), "41edece42d63e8d9bf515a9ba6932e1c20cbc9f5a5d134645adb5db1b9737ea3"},
        {std::string(1000000, 'a'), "cdc76e5c9914fb9281a1c7e284d73e67f1809a48a497200e046d39ccc7112cd0"},

        // 🧪 Cryptographic phrases
        {"The quick brown fox jumps over the lazy dog", "d7a8fbb307d7809469ca9abcb0082e4f8d5651e46d3cdb762d02d0bf37c9e592"},
        {"The quick brown fox jumps over the lazy dog.", "ef537f25c895bfa782526529a9b63d97aa631564d5d789c2b765448c8635fb6c"},
        {"abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq", "248d6a61d20638b8e5c026930c3e6039a33ce45964ff2167f6ecedd419db06c1"},

        // ⚙️ Structured / JSON-like / source-ish strings
        {"{\"name\":\"test\",\"value\":42}", "9a304be829134dbe6b6ccc54e7e78e6e0b477c48a75b8e3042389dbd61617569"},
        {"int main() { return 0; }", "80a7161009ffaf868641acac3f5e49bc5f86021ee1d177f3b1cbb47573513649"},

        // 🧬 Binary-like input (non-printable bytes)
        {std::string("\x00\x01\x02\x03\x04\x05", 6), "17e88db187afd62c16e5debf3e6527cd006bc012bc90b51a810cd80c2d511f43"},

        // 🧠 Hash of hash (double SHA-256)
        {
            [] {
                plugify::Sha256 h1;
            	std::string_view input = "abc";
                h1.update(std::vector<uint8_t>(input.begin(), input.end()));
                auto res = h1.finalize();
                plugify::Sha256 h2;
                h2.update(res);
                h2.finalize();
                return h2.to_string();
            }(),
            "81860363b1e0d0166c35137166721dc1c3d58c8655c0375e6bed77ea31c6bc9e"
        }
    };

    for (const auto& [input, expected] : vectors) {
        SECTION("Input (len=" + std::to_string(input.size()) + "): " + (input.size() <= 50 ? input : input.substr(0, 47) + "...")) {
            plugify::Sha256 hasher;
            hasher.update(std::vector<uint8_t>(input.begin(), input.end()));
            hasher.finalize();
            REQUIRE(hasher.to_string() == expected);
        }
    }
}