#include <catch2/catch_all.hpp>

#include <plugify/string.h>

TEST_CASE("string operation > swap", "[string]") {

    SECTION("void swap(string& other) noexcept") {
        {
            plg::string a{"123", 3};
            plg::string b{"456", 3};
            a.swap(b);
            REQUIRE(a == "456");
            REQUIRE(b == "123");
        }
        {
            plg::string a{"123", 3};
            plg::string b;
            a.swap(b);
            REQUIRE(a == "");
            REQUIRE(b == "123");
        }
        {
            plg::string a{"longlonglonglonglonglonglonglong", 32};
            plg::string b{"totototototototototototototototo", 32};
            a.swap(b);
            REQUIRE(a == "totototototototototototototototo");
            REQUIRE(b == "longlonglonglonglonglonglonglong");
        }
        {
            plg::string a{"longlonglonglonglonglonglonglong", 32};
            plg::string b;
            a.swap(b);
            REQUIRE(a == "");
            REQUIRE(b == "longlonglonglonglonglonglonglong");
        }
        {
            plg::string a{"longlonglonglonglonglonglonglong", 32};
            plg::string b{"small", 5};
            a.swap(b);
            REQUIRE(a == "small");
            REQUIRE(b == "longlonglonglonglonglonglonglong");
        }
    }

}
