#include "catch.hpp"
#include <iostream>
#include <memory>
#include <string>
#include <vector>

using namespace std;
using namespace Catch::Matchers;

string full_name(const string& first, const string& last)
{
    return first + " " + last;
}

TEST_CASE("reference binding")
{
    std::string name = "jan";

    SECTION("C++98")
    {
        std::string& ref_name = name;

        const std::string& ref_full_name = full_name(name, "kowalski");

        //ref_full_name[0] = 'J';
    }

    SECTION("C++11")
    {
        std::string&& rv_ref_full_name = full_name(name, "kowalski");
        rv_ref_full_name[0] = 'J';

        REQUIRE(rv_ref_full_name == "Jan kowalski");

        //std::string&& rv_name = name;

        SECTION("std::move()")
        {
            std::string&& rv_name = std::move(name);
        }
    }
}

namespace Explain
{
    template <typename T>
    class vector
    {
    public:
        void push_back(const T& item)
        {
            std::cout << "push_back " << item << " by copy\n";
        }

        void push_back(T&& item)
        {
            std::cout << "push_back " << item << " by move\n";
        }
    };
}

TEST_CASE("vector + push_back")
{
    std::string name = "Jan";
    std::string* ptr_name = &name;

    Explain::vector<std::string> vec;

    vec.push_back(name); // copy - const string&
    vec.push_back("Adam"); // move - string&&
    vec.push_back(name + name); // move - string&&
    vec.push_back(std::move(name)); // move - string&& - name is after
    name = "Ewa"; // safe
    std::string* ptr_after_move_and_reinit = &name;

    REQUIRE(ptr_name == ptr_after_move_and_reinit);
} // safe

class Gadget
{
public:
    int id_ = 0;
    std::string name_;

    Gadget() = default; // user-declared

    Gadget(int id, const std::string& name)
        : id_ {id}
        , name_ {name}
    {
        std::cout << "Gadget(" << id_ << ", " << name_ << ")\n";
    }

    Gadget(const Gadget&) = default;
    Gadget& operator=(const Gadget&) = default;
    Gadget(Gadget&&) = default;
    Gadget& operator=(Gadget&&) = default;

    ~Gadget()
    {
        std::cout << "~Gadget(" << id_ << ", " << name_ << ")\n";
    }

    void use() const
    {
        std::cout << "Using Gadget(" << id_ << ", " << name_ << ")\n";
    }
};

TEST_CASE("Gadget")
{
    Gadget g1 {1, "ipad"};

    Gadget g2 = g1;

    Gadget g3 = std::move(g1);

    REQUIRE(g3.id_ == 1);
    REQUIRE(g3.name_ == "ipad"s);

    SECTION("not safe")
    {
        REQUIRE(g1.id_ == 1);
        //REQUIRE(g1.name_ == ""s);
    }
}

template <typename T>
class UniquePtr
{
    T* ptr_;

public:
    explicit UniquePtr(T* ptr)
        : ptr_ {ptr}
    {
    }

    UniquePtr(const UniquePtr&) = delete;
    UniquePtr& operator=(const UniquePtr&) = delete;

    // move constructor
    UniquePtr(UniquePtr&& source)
        : ptr_ {std::move(source.ptr_)}
    {
        source.ptr_ = nullptr;
    }

    // move assignment
    UniquePtr& operator=(UniquePtr&& source) // a = std::move(b) = std::move(c)
    {
        if (this != &source) // protection from: a = std::move(a)
        {
            delete ptr_;

            ptr_ = std::move(source.ptr_);

            source.ptr_ = nullptr;
        }

        return *this;
    }

    ~UniquePtr()
    {
        delete ptr_;
    }

    explicit operator bool() const
    {
        return ptr_ != nullptr;
    }

    T& operator*() const
    {
        return *ptr_;
    }

    T* operator->() const
    {
        return ptr_;
    }

    T* get() const
    {
        return ptr_;
    }
};

UniquePtr<Gadget> create_gadget()
{
    static int id = 0;

    ++id;
    return UniquePtr<Gadget>(new Gadget(id, "gadget"s + std::to_string(id)));
}

TEST_CASE("UniquePtr & move semantics")
{
    std::unique_ptr<Gadget> uptr;

    std::cout << "\n=======================\n";
    UniquePtr<Gadget> ptr_g1(new Gadget(1, "ipad"));

    if (ptr_g1)
        ptr_g1->use();

    UniquePtr<Gadget> ptr_g2 = std::move(ptr_g1);

    if (ptr_g2)
        ptr_g2->use();

    REQUIRE(ptr_g1.get() == nullptr);

    ptr_g2 = UniquePtr<Gadget>(new Gadget(2, "tablet"));

    ptr_g2->use();

    ptr_g2 = create_gadget();

    ptr_g2->use();
}

TEST_CASE("move does not move")
{
    SECTION("const objects can not be moved")
    {
        const std::string str = "text";

        std::string target = std::move(str);
        REQUIRE(str == "text"s);
        REQUIRE(target == "text"s);

        const Gadget g {1, "g1"};
        Gadget g_target = std::move(g);
        REQUIRE(g.name_ == "g1"s);
    }
}

struct AllSpecialFunctionsAreDefault
{
    int id;
    std::string name;
    std::vector<int> data;

    AllSpecialFunctionsAreDefault() = default;

    AllSpecialFunctionsAreDefault(int id, std::string n, std::vector<int> d)
        : id {id}
        , name {std::move(n)}
        , data {std::move(d)}
    {
    }

    void print() const
    {
        std::cout << "ASFAD(" << id << ", " << name << ", [ ";
        for (const auto& item : data)
            std::cout << item << " ";
        std::cout << "])\n";
    }
};

struct RuleOfFive
{
    int id;
    std::string name;
    std::vector<int> data;

    RuleOfFive() = default;

    RuleOfFive(int id, std::string n, std::vector<int> d)
        : id {id}
        , name {std::move(n)}
        , data {std::move(d)}
    {
    }

    RuleOfFive(const RuleOfFive&) = default;
    RuleOfFive& operator=(const RuleOfFive&) = default;
    RuleOfFive(RuleOfFive&&) = default;
    RuleOfFive& operator=(RuleOfFive&&) = default;
    ~RuleOfFive() { }

    void print() const
    {
        std::cout << "ASFAD(" << id << ", " << name << ", [ ";
        for (const auto& item : data)
            std::cout << item << " ";
        std::cout << "])\n";
    }
};

TEST_CASE("Special functions")
{
    string text = "text";
    AllSpecialFunctionsAreDefault asfad1 {1, text, {1, 2, 3}};
    asfad1.print();

    AllSpecialFunctionsAreDefault backup = asfad1;
    backup.print();

    AllSpecialFunctionsAreDefault target = std::move(asfad1);
    target.print();

    REQUIRE(asfad1.data.size() == 0);
}