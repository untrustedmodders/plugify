#include <catch2/catch_all.hpp>

#include <plugify/string.h>

TEST_CASE("string operation > compare", "[string]") {

    SECTION("int compare(string const& str ) const noexcept") {
        {
            plg::string a;
            plg::string b;
            REQUIRE(a.compare(b) == 0);
        }
        {
            plg::string a{"toto", 4};
            plg::string b{"toto", 4};
            REQUIRE(a.compare(b) == 0);
        }
        {
            plg::string a{"longlonglonglonglonglonglonglong", 32};
            plg::string b{"longlonglonglonglonglonglonglong", 32};
            REQUIRE(a.compare(b) == 0);
        }

        {
            plg::string a{"1", 1};
            plg::string b{"1", 1};
            REQUIRE(a.compare(b) == 0);
        }
        {
            plg::string a{"1", 1};
            plg::string b{"2", 1};
            REQUIRE(a.compare(b) < 0);
        }
        {
            plg::string a{"2", 1};
            plg::string b{"1", 1};
            REQUIRE(a.compare(b) > 0);
        }

        {
            plg::string a{"12", 1};
            plg::string b{"12", 1};
            REQUIRE(a.compare(b) == 0);
        }
        {
            plg::string a{"12", 2};
            plg::string b{"13", 2};
            REQUIRE(a.compare(b) < 0);
        }
        {
            plg::string a{"12", 2};
            plg::string b{"22", 2};
            REQUIRE(a.compare(b) < 0);
        }
        {
            plg::string a{"13", 2};
            plg::string b{"12", 2};
            REQUIRE(a.compare(b) > 0);
        }
        {
            plg::string a{"22", 2};
            plg::string b{"12", 2};
            REQUIRE(a.compare(b) > 0);
        }

        {
            plg::string a{"123", 3};
            plg::string b{"123", 3};
            REQUIRE(a.compare(b) == 0);
        }
        {
            plg::string a{"123", 3};
            plg::string b{"223", 3};
            REQUIRE(a.compare(b) < 0);
        }
        {
            plg::string a{"123", 3};
            plg::string b{"133", 3};
            REQUIRE(a.compare(b) < 0);
        }
        {
            plg::string a{"123", 3};
            plg::string b{"124", 3};
            REQUIRE(a.compare(b) < 0);
        }
        {
            plg::string a{"223", 3};
            plg::string b{"123", 3};
            REQUIRE(a.compare(b) > 0);
        }
        {
            plg::string a{"133", 3};
            plg::string b{"123", 3};
            REQUIRE(a.compare(b) > 0);
        }
        {
            plg::string a{"124", 3};
            plg::string b{"123", 3};
            REQUIRE(a.compare(b) > 0);
        }
    }

    SECTION("int compare(size_type pos1, size_type count1, string const& str) const") {
        {
            plg::string a{"123", 3};
            plg::string b{"123", 3};
            REQUIRE(a.compare(0, plg::string::npos, b) == 0);
            REQUIRE(a.compare(2, plg::string::npos, b) > 0);
            REQUIRE(a.compare(1, 1, b) > 0);
            REQUIRE(a.compare(1, 3, b) > 0);
            REQUIRE(a.compare(1, 3, b) > 0);
        }
        {
            plg::string const a{"123", 3};
            plg::string const b{"421", 3};
            REQUIRE(a.compare(0, plg::string::npos, b) < 0);
            REQUIRE(a.compare(2, plg::string::npos, b) < 0);
            REQUIRE(a.compare(1, 1, b) < 0);
            REQUIRE(a.compare(1, 3, b) < 0);
            REQUIRE(a.compare(1, 3, b) < 0);
        }
    }

    SECTION("int compare(size_type pos1, size_type count1, string const& str, size_type pos2, size_type count2 = npos) const") {
        {
            plg::string a{"123", 3};
            plg::string b{"123", 3};
            REQUIRE(a.compare(0, plg::string::npos, b, 0, plg::string::npos) == 0);
            REQUIRE(a.compare(1, 1, b, 1, 1) == 0);
            REQUIRE(a.compare(1, 1, b, 2, 1) < 0);
        }
        {
            plg::string a{"123", 3};
            plg::string b{"421", 3};
            REQUIRE(a.compare(0, plg::string::npos, b, 0 , plg::string::npos) < 0);
            REQUIRE(a.compare(0, 1, b, 2 , 1) == 0);
        }
    }

    SECTION("int compare(const_pointer s) const") {
        {
            plg::string a{"123", 3};
            REQUIRE(a.compare("123") == 0);
        }
        {
            plg::string a{"123", 3};
            REQUIRE(a.compare("223") < 0);
        }
        {
            plg::string a{"223", 3};
            REQUIRE(a.compare("123") > 0);
        }
    }

    SECTION("int compare(size_type pos1, size_type count1, const_pointer s) const") {
        {
            plg::string const a{"123", 3};
            REQUIRE(a.compare(0, plg::string::npos, "123") == 0);
        }
        {
            plg::string const a{"123", 3};
            REQUIRE(a.compare(0, plg::string::npos, "421") < 0);
        }
    }

    SECTION("int compare(size_type pos1, size_type count1, const_pointer s, size_type count2) const") {
        {
            plg::string const a{"123", 3};
            REQUIRE(a.compare(0, plg::string::npos, "123", 0, plg::string::npos) == 0);
            REQUIRE(a.compare(1, 1, "123", 1, 1) == 0);
            REQUIRE(a.compare(1, 1, "123", 2, 1) < 0);
        }
        {
            plg::string const a{"123", 3};
            REQUIRE(a.compare(0, plg::string::npos, "421", 0 , plg::string::npos) < 0);
            REQUIRE(a.compare(0, 1, "421", 2 , 1) == 0);
        }
    }

    SECTION("int compare(T const& t) const noexcept") {
        {
            plg::string a{"123", 3};
            std::string_view b{"123", 3};
            REQUIRE(a.compare(b) == 0);
        }
        {
            plg::string a{"123", 3};
            std::string_view b{"223", 3};
            REQUIRE(a.compare(b) < 0);
        }
        {
            plg::string a{"223", 3};
            std::string_view b{"123", 3};
            REQUIRE(a.compare(b) > 0);
        }
    }

    SECTION("int compare(size_type pos1, size_type count1, T const& t) const") {
        {
            plg::string const a{"123", 3};
            std::string_view b{"123", 3};
            REQUIRE(a.compare(0, plg::string::npos, b) == 0);
        }
        {
            plg::string const a{"123", 3};
            std::string_view b{"421", 3};
            REQUIRE(a.compare(0, plg::string::npos, b) < 0);
        }
    }

    SECTION("int compare(size_type pos1, size_type count1, T const& t, size_type pos2, size_type count2 = npos) const") {
        {
            plg::string const a{"123", 3};
            std::string_view b{"123", 3};
            REQUIRE(a.compare(0, plg::string::npos, b, 0, plg::string::npos) == 0);
            REQUIRE(a.compare(1, 1, b, 1, 1) == 0);
            REQUIRE(a.compare(1, 1, b, 2, 1) < 0);
        }
        {
            plg::string const a{"123", 3};
            std::string_view b{"421", 3};
            REQUIRE(a.compare(0, plg::string::npos, b, 0 , plg::string::npos) < 0);
            REQUIRE(a.compare(0, 1, b, 2 , 1) == 0);
        }
    }

}
