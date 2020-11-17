#include "catch.hpp"
#include <iostream>
#include <string>
#include <vector>
#include <list>

using namespace std;

template <typename T>
T maximum(T a, T b)
{
    //undeclared():
    puts(__PRETTY_FUNCTION__);
    return a < b ? b : a;
}

const char* maximum(const char* a, const char* b)
{
    puts(__PRETTY_FUNCTION__);
    return strcmp(a, b) < 0 ? b : a;
}

TEST_CASE("function template")
{
    REQUIRE(maximum(1, 2) == 2);

    REQUIRE(maximum(3.14, 2.71) == Approx(3.14));

    REQUIRE(maximum(static_cast<double>(1), 3.14) == Approx(3.14));

    REQUIRE(maximum<double>(1, 3.14) == Approx(3.14));

    std::string str1 = "abc";

    REQUIRE(maximum(str1, "def"s) == "def"s);

    const char* txt1 = "zabc";
    const char* txt2 = "def";

    REQUIRE(maximum(txt1, txt2) == "zabc"s);

    int (*fptr)(int, int) = &maximum;
    auto af = &maximum<int>;

    REQUIRE(fptr(4, 7) == 7);
}

namespace ver1
{
    template <typename TResult, typename T1, typename T2>
    TResult get_max(T1 a, T2 b)
    {
        return a < b ? b : a;
    }
}

namespace ver2
{
    template <typename T1, typename T2>
    auto get_max(T1 a, T2 b)
    {
        return a < b ? b : a;
    }

    // auto vs. decltype(auto)
}

namespace ver3
{
    template <typename T1, typename T2, typename TResult = std::common_type_t<T1, T2>>
    TResult get_max(T1 a, T2 b)
    {
        return a < b ? b : a;
    }

    // auto vs. decltype(auto)
}

template <typename TContainer>
decltype(auto) element_at(TContainer& container, size_t index)
{
    auto it = std::begin(container);
    // for(auto i = 0u; i < index; ++i)
    // {
    //     ++it;
    // }

    //it += index;

    std::advance(it, index);

    return *it;
}

TEST_CASE("templates & return type")
{
    auto r1 = ver1::get_max<double>(1, 3.14);

    auto r2 = ver2::get_max(1, 3.14);

    auto r3 = ver3::get_max<int, double, int>(1, 3.14);

    vector<int> vec = {1, 2, 3, 4};
    REQUIRE(element_at(vec, 2) == 3);

    list<int> lst = {1, 2, 3, 4};
    REQUIRE(element_at(std::as_const(lst), 2) == 3);
    element_at(lst, 1) = 42;

    int tab[10] = {1, 2, 3};
    element_at(tab, 0) = 665;

    vector<bool> vb{0, 0, 1, 1};
    element_at(vb, 1) = true;
}