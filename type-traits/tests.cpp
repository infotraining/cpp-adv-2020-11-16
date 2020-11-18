#include "catch.hpp"
#include <iostream>
#include <string>
#include <type_traits>
#include <vector>

using namespace std;
using namespace Catch::Matchers;

namespace Cpp98
{
    template <typename TContainer>
    typename TContainer::value_type sum(const TContainer& container)
    {
        typedef typename TContainer::value_type TResult;
        TResult result = TResult();
        for (typename TContainer::const_iterator it = container.begin(); it != container.end(); ++it)
        {
            result += *it;
        }

        return result;
    }
}

int foo(int n)
{
    return 2 * n;
}

template <size_t N>
struct Foo
{
    static constexpr size_t value = 2 * N;
};

template <typename T>
struct Identity
{
    using type = T;
};

template <typename T>
struct RemoveReference
{
    using type = T;
};

template <typename T>
struct RemoveReference<T&>
{
    using type = T;
};

template <typename T>
struct RemoveReference<T&&>
{
    using type = T;
};

template <typename T>
using RemoveReference_t = typename RemoveReference<T>::type;

TEST_CASE("meta function")
{
    REQUIRE(foo(2) == 4);
    REQUIRE(Foo<2>::value == 4);

    static_assert(std::is_same<Identity<int>::type, int>::value, "Error");

    static_assert(std::is_same<RemoveReference<int>::type, int>::value, "Error");
    static_assert(std::is_same<RemoveReference<int&>::type, int>::value, "Error");
    static_assert(std::is_same_v<RemoveReference<int&&>::type, int>, "Error");
}

namespace ModernCpp
{
    template <typename TContainer>
    auto sum(const TContainer& container)
    {
        using TResult = std::remove_cv_t<std::remove_reference_t<decltype(*begin(container))>>;
        TResult result = TResult();
        for (auto it = std::begin(container); it != std::end(container); ++it)
        {
            result += *it;
        }

        return result;
    }
}

template <typename T>
struct SumTraits
{
    using SumType = T;
    inline static const T zero = T();
};

template <>
struct SumTraits<uint8_t>
{
    using SumType = uint64_t;
    inline static constexpr uint64_t zero = 0;
};

template <typename T1, typename T2>
struct SumPolicy
{
    static void sum(T1& total, const T2& item)
    {
        total += item;
    }
};

namespace Traits
{
    template <typename TContainer>
    auto sum(const TContainer& container)
    {
        using T = std::remove_cv_t<std::remove_reference_t<decltype(*begin(container))>>;

        using TResult = typename SumTraits<T>::SumType;
        TResult result = SumTraits<T>::zero;
        for (auto it = std::begin(container); it != std::end(container); ++it)
        {
            //result += *it;
            SumPolicy<TResult, T>::sum(result, *it);
        }

        return result;
    }
}

TEST_CASE("Test")
{
    vector<int> vec = {1, 2, 3};

    REQUIRE(Cpp98::sum(vec) == 6);

    int tab[3] = {1, 2, 3};

    REQUIRE(ModernCpp::sum(tab) == 6);

    // vector<vector<int>> data = {{1, 2, 3}, {4, 5, 6}};
    // auto result = ModernCpp::sum(data);

    vector<uint8_t> numbers = {250, 50};

    REQUIRE(Traits::sum(numbers) == 300);

    vector<string> words = {"a", "bc", "d"};
    REQUIRE(Traits::sum(words) == "abcd"s);
}
