
#include <nlohmann/json.hpp>
using json = nlohmann::json;

int main() {
    auto j3 = json::parse(R"({"happy": true, "pi": 3.141})");

    std::string s = j3.dump();    // {"happy":true,"pi":3.141}

    std::cout << j3.dump(4) << std::endl;

    return 0;
}
