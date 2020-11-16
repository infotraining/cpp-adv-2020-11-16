#include "catch.hpp"
#include <iostream>
#include <vector>
#include "paragraph.hpp"

using namespace std;

TEST_CASE("Moving paragraph")
{
    LegacyCode::Paragraph p("***");
    LegacyCode::Paragraph mp = move(p);

    REQUIRE(mp.get_paragraph() == string("***"));
    REQUIRE(p.get_paragraph() == nullptr);
}

TEST_CASE("Moving text shape")
{
    Text txt{10, 20, "text"};
    Text mtxt = move(txt);

    REQUIRE(mtxt.text() == string("text"));
    REQUIRE(txt.text() == string());
}

std::vector<int> load_big_data()
{
    std::vector<int> vec(1'000'000);

    int i = 0;
    for(auto& item : vec)
        item = ++i;

    return vec; 
}

TEST_CASE("C++98")
{
    std::vector<int> data = load_big_data();
}