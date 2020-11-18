#include "catch.hpp"
#include <array>
#include <iostream>
#include <string>
#include <vector>

using namespace std;
using namespace Catch::Matchers;

constexpr int id = 665;
constexpr int next_id = id + 1;

const int size_m = 1024;
constexpr int size_l = 2 * size_m;

namespace Cpp11
{
    constexpr int factorial(int n)
    {
        return (n == 0) ? 1 : n * factorial(n - 1);
    }
}

constexpr int factorial(int n)
{
    if (n == 0)
        return 1;
    else
        return n * factorial(n - 1);
}

TEST_CASE("factorial")
{
    static_assert(Cpp11::factorial(3) == 6, "Error");

    int n = 4;
    REQUIRE(Cpp11::factorial(n) == 24);

    int tab[factorial(4)];
    static_assert(sizeof(tab) == 24 * sizeof(int), "Error");
}

template <size_t N>
constexpr std::array<uint64_t, N> create_factorial_lookup()
{
    std::array<uint64_t, N> result {};

    for (size_t i = 0; i < N; ++i)
        result[i] = factorial(i);

    return result;
}

TEST_CASE("factorial lookup")
{
    constexpr auto size = 5u;
    constexpr std::array<uint64_t, size> factorial_lookup = create_factorial_lookup<size>();

    static_assert(factorial_lookup[4] == 24, "Error");

    auto constexpr_lambda = [](int a) { return factorial(2 * a); };

    static_assert(constexpr_lambda(2) == 24);
}

constexpr int check(int i)
{
    constexpr int low = 0;
    constexpr int high = 99;

    return (low <= i && i < high)
        ? i
        : throw std::out_of_range("range error");
}

TEST_CASE("constexpr + throw")
{
    constexpr int value = check(59);
    static_assert(value == 59);
}