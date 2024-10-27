#include <catch2/catch_all.hpp>

#include <plugify/string.h>

TEST_CASE("string operation > substr", "[string]") {

    SECTION("string substr(size_type pos = 0, size_type count = plg::string::npos ) const") {
        {
            plg::string a{"123"};
            REQUIRE(a.substr(0) == "123");
        }
        {
            plg::string a{"123"};
            REQUIRE(a.substr(0, 2) == "12");
        }
        {
            plg::string a{"123"};
            REQUIRE(a.substr(1, 2) == "23");
        }
        {
            plg::string a{"longlonglonglonglonglonglonglong"};
            REQUIRE(a.substr(4, 4) == "long");
        }
        {
            plg::string a{"longlonglonglonglonglonglonglong"};
            REQUIRE(a.substr(4, 24) == "longlonglonglonglonglong");
        }
    }

}
