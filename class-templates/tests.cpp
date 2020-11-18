#include "catch.hpp"
#include <array>
#include <bitset>
#include <iostream>
#include <list>
#include <map>
#include <string>
#include <vector>
#include <string_view>

using namespace std;
using namespace Catch::Matchers;

template <typename T>
class Holder
{
    T item_;

public:
    //typedef T value_type;
    using value_type = T;

    Holder(T& item)
        : item_ {item}
    {
    }

    Holder(T&& item)
        : item_ {std::move(item)}
    {
    }

    T& value()
    {
        return item_;
    }

    const T& value() const;

    void info() const
    {
        std::cout << "Holder<T: " << typeid(T).name() << ">(" << item_ << ")\n";
    }
};

// deduction guide - C++17 (CTAD)
template <typename T>
Holder(T) -> Holder<T>;

template <typename T>
const T& Holder<T>::value() const
{
    return item_;
}

template <typename T>
class Holder<T*>
{
    std::unique_ptr<T> item_;
public:
    using value_type = T;

    Holder(T* item)
        : item_ {item}
    {
    }

    T& value()
    {
        return *item_;
    }

    const T& value() const
    {
        return *item_;
    }

    T* get() const
    {
        return item_;
    }

    void info() const
    {
        std::cout << "Holder<T*: " << typeid(T).name() << ">(" << item_ 
        << " - " << *item_ << ")\n";
    }
};

template <>
class Holder<const char*>
{
    const char* item_;
public:
    using value_type = const char*;

    Holder(const char* item)
        : item_ {item}
    {
    }

    std::string_view value() const
    {
        return item_;
    }

    void info() const
    {
        std::cout << "Holder<const char*>(" << item_ << ")\n";
    }
};

template <typename T>
Holder<T> make_holder(T item)
{
    return Holder<T>(item);
}

template <typename T>
struct Data
{
    Holder<T> value;
};

template <typename T>
using StringKeyMap = std::map<std::string, T>;

StringKeyMap<int> dict = {{"one", 1}, {"two", 2}};

TEST_CASE("Holder")
{
    Holder<int> h1 {42};
    REQUIRE(h1.value() == 42);
    h1.info();

    Holder<int>::value_type x = h1.value();

    using HolderString = Holder<string>;

    HolderString h2 {"text"};
    REQUIRE(h2.value() == "text"s);

    Holder<int*> h3(new int(13));
    REQUIRE(h3.value() == 13);
    h3.info();

    Holder<const char*> h4 = "text";
    h4.info();

    auto h5 = make_holder(7); // Holder<int>

    Holder h6(665); // Holder<int>
    h6.info();

    Holder h7(new string("text"));
    h7.info();

    Holder h8("abc");
    h8.info();
}

TEST_CASE("CTAD - std")
{
    vector vec = {1, 2, 3};

    array tab = {1, 2, 3};
}

template <typename T, size_t N>
struct Array
{
    T items[N];

    using value_type = T;

    size_t size() const noexcept
    {
        return N;
    }

    int& operator[](size_t index) noexcept
    {
        return items[index];
    }

    const int& operator[](size_t index) const noexcept
    {
        return items[index];
    }
};

TEST_CASE("Array")
{
    std::array<int, 8> arr1 = {1, 2, 3, 4};
    for (size_t i = 0; i < arr1.size(); ++i)
        std::cout << arr1[i] << " ";
    std::cout << "\n";

    std::bitset<16> bs1(1256);
    std::cout << bs1 << std::endl;

    // tuple<int, double, string> tpl(1, 3.14, "text");
    // REQUIRE(select<0, 2>(tpl) == tuple<int, string>(1, "text");
}

namespace ver1
{
    template <typename T, typename TContainer = std::vector<T>>
    struct Stack
    {
        TContainer items;

        void push(const T& item)
        {
            items.push_back(item);
        }
    };
}

template <typename T, template<typename, typename> class Container, typename A = std::allocator<T>>
struct Stack
{
    Container<T, A> items;

    void push(const T& item)
    {
        items.push_back(item);
    }
};

TEST_CASE("Stack")
{
    ver1::Stack<int, std::list<int>> s0;

    Stack<int, std::list> s1;
    s1.push(1);

    vector<int> vec = {1, 2, 3};
    
    vector<bool> flags = {0, 1, 1, 0};
    flags.flip();
}