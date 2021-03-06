#define CATCH_CONFIG_MAIN
#include "catch.hpp"

#include <cassert>
#include <iostream>
#include <iterator>
#include <memory>
#include <vector>

using namespace std;

namespace Explain
{
    template <typename TContainer>
    auto begin(TContainer& container)
    {
        return container.begin();
    }

    template <typename T, size_t N>
    auto begin(T (&tab)[N])
    {
        return tab;
    }

    template <typename TContainer>
    auto end(TContainer& container)
    {
        return container.end();
    }

    template <typename T, size_t N>
    auto end(T (&tab)[N])
    {
        return tab + N;
    }

    template <typename TContainer>
    auto size(const TContainer& container)
    {
        return container.size();
    }

    template <typename T, size_t N>
    auto size(T (&tab)[N])
    {
        return N;
    }
}

TEST_CASE("Explain size etc.")
{
    vector<int> vec = {1, 2, 3};
    REQUIRE(Explain::size(vec) == 3);

    int tab[] = {1, 2, 3};
    REQUIRE(Explain::size(tab) == 3);
}

namespace LambdaExpressions
{

}

class Lambda_634527635476234
{
public:
    int operator()(int a) const { return a * 2; }
};

class Lambda_876458753673567
{
public:
    template <typename T>
    auto operator()(const T& a) const { return a * 2; }
};

TEST_CASE("lambda")
{
    SECTION("non-generic")
    {
        auto l1 = [](int a) { return a * 2; };
        REQUIRE(l1(2) == 4);
        //REQUIRE(l1(3.14) == 6);

        SECTION("is interpreted as")
        {
            auto l1 = Lambda_634527635476234 {};
            REQUIRE(l1(2) == 4);
            //REQUIRE(l1(3.14) == 6);
        }
    }

    SECTION("generic lambda")
    {
        auto l1 = [](const auto& a) { return a * 2; };
        REQUIRE(l1(2) == 4);
        REQUIRE(l1(3.14) == Approx(6.28));
    }
}

namespace Solution
{
    namespace ver1
    {

        template <typename ContainerT>
        auto find_null(ContainerT& container)
        {
            for (auto iter = std::begin(container); iter != std::end(container); ++iter)
            {
                if (*iter == nullptr)
                {
                    return iter;
                }
            }
            return std::end(container);
        }
    }

    inline namespace ver2
    {
        template <class TC>
        auto find_null(TC& a_ptrs)
        {
            return std::find_if(std::begin(a_ptrs), std::end(a_ptrs), [](const auto& ptr) { return ptr == nullptr; });
        }
    }
}

TEST_CASE("find_null description")
{
    using namespace Solution;

    SECTION("finds position (returns iterator) of the first null pointer in a std container of raw pointers")
    {
        vector<int*> ptrs = {new int {9}, new int {10}, NULL, new int {20}, nullptr, new int {23}};

        auto first_null_pos = find_null(ptrs);

        REQUIRE(distance(ptrs.begin(), first_null_pos) == 2);

        // clean-up
        for (const auto* ptr : ptrs)
            delete ptr;
    }

    SECTION("finds first null pointer in an array of raw pointers")
    {
        int* ptrs[] = {new int {9}, new int {10}, NULL, new int {20}, nullptr, new int {23}};

        auto first_null_pos = find_null(ptrs);

        REQUIRE(distance(begin(ptrs), first_null_pos) == 2);

        // clean-up
        for (const auto* ptr : ptrs)
            delete ptr;
    }

    SECTION("finds first empty shared_ptr in a initializer-list of shared_ptrs")
    {
        auto il = {make_shared<int>(10), shared_ptr<int> {}, make_shared<int>(3)};

        auto first_null_pos = find_null(il);

        REQUIRE(distance(il.begin(), first_null_pos) == 1);
    }

    SECTION("finds first empty unique_ptr in a container of unique_ptrs")
    {
        vector<unique_ptr<int>> vec_ptr;
        vec_ptr.push_back(std::unique_ptr<int>(new int(10)));
        vec_ptr.push_back(nullptr);
        vec_ptr.push_back(std::unique_ptr<int>(new int(20)));

        auto where_null = find_null(vec_ptr);

        REQUIRE(distance(vec_ptr.begin(), where_null) == 1);
    }

    SECTION("when all pointers are valid returns iterator which equals end()")
    {
        auto il = {make_shared<int>(10), shared_ptr<int> {new int(5)}, make_shared<int>(3)};

        REQUIRE(find_null(il) == il.end());
    }
}