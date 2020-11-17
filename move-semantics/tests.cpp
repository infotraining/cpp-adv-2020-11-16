#include "catch.hpp"
#include <iostream>
#include <memory>
#include <string>
#include <vector>
#include <map>

using namespace std;
using namespace Catch::Matchers;

std::string full_name(const std::string& first, const std::string& last)
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
        std::vector<T> items_;
    public:
        void push_back(const T& item)
        {
            std::cout << "push_back " << item << " by copy\n";
            items_.push_back(item);
        }

        void push_back(T&& item)
        {
            std::cout << "push_back " << item << " by move\n";
            items_.push_back(std::move(item));
        }

        template <typename TArg>
        void emplace_back(TArg&& arg)
        {
            items_.emplace_back(std::forward<TArg>(arg));
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

    Gadget(const Gadget& source)
        : id_ {source.id_}
        , name_ {source.name_}
    {
        std::cout << "Gadget(cc: " << id_ << ", " << name_ << ")\n";
    }

    Gadget& operator=(const Gadget&) = default;

    Gadget(Gadget&& source) noexcept
        : id_ {std::move(source.id_)}
        , name_ {std::move(source.name_)}
    {
        try
        {
            std::cout << "Gadget(mv: " << id_ << ", " << name_ << ")\n";
        }
        catch (...)
        {
            //
        }
    }

    Gadget& operator=(Gadget&&) = default;

    ~Gadget() noexcept
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

    SECTION("use after move is not safe")
    {
        REQUIRE(g1.id_ == 1);
        REQUIRE(g1.name_ == ""s);
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
    UniquePtr(UniquePtr&& source) noexcept
        : ptr_ {std::move(source.ptr_)}
    {
        source.ptr_ = nullptr;
    }

    // move assignment
    UniquePtr& operator=(UniquePtr&& source) noexcept // a = std::move(b) = std::move(c)
    {
        if (this != &source) // protection from: a = std::move(a)
        {
            delete ptr_;

            ptr_ = std::move(source.ptr_);

            source.ptr_ = nullptr;
        }

        return *this;
    }

    ~UniquePtr() noexcept
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

template <typename T, typename TArg1, typename TArg2>
UniquePtr<T> MakeUnique(TArg1&& arg1, TArg2&& arg2)
{
    return UniquePtr<T>(new T(std::forward<TArg1>(arg1), std::forward<TArg2>(arg2)));
}

TEST_CASE("UniquePtr & move semantics")
{
    std::unique_ptr<Gadget> uptr;

    std::cout << "\n=======================\n";
    //UniquePtr<Gadget> ptr_g1(new Gadget(1, "ipad"));
    UniquePtr<Gadget> ptr_g1 = MakeUnique<Gadget>(1, "ipad");

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

struct All5SpecialFunctionsAreDefault
{
    int id;
    std::string name;
    std::vector<int> data;

    All5SpecialFunctionsAreDefault() = default;

    All5SpecialFunctionsAreDefault(int id, std::string n, std::vector<int> d)
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
    All5SpecialFunctionsAreDefault asfad1 {1, text, {1, 2, 3}};
    asfad1.print();

    All5SpecialFunctionsAreDefault backup = asfad1;
    backup.print();

    All5SpecialFunctionsAreDefault target = std::move(asfad1);
    target.print();

    REQUIRE(asfad1.data.size() == 0);
}

TEST_CASE("vector + move semantics")
{
    std::cout << "\n----------------------\n";
    std::vector<Gadget> gadgets;

    gadgets.push_back(Gadget(1, "ipad"));
    std::cout << "----------------------\n";

    gadgets.push_back(Gadget(2, "ipad"));
    std::cout << "----------------------\n";

    gadgets.push_back(Gadget(3, "ipad"));
    std::cout << "----------------------\n";

    gadgets.push_back(Gadget(4, "ipad"));
    std::cout << "----------------------\n";

    gadgets.push_back(Gadget(5, "ipad"));
    std::cout << "----------------------\n";
}

///////////////////////////////////////////////////////////
// PERFECT FORWARDING
//

void have_fun(Gadget& g)
{
	puts(__PRETTY_FUNCTION__);
	std::cout << "Having fun with " << g.name_ << "\n";
}

void have_fun(const Gadget& g)
{
	puts(__PRETTY_FUNCTION__);
	std::cout << "Having fun with " << g.name_ << "\n";
}

void have_fun(Gadget&& g)
{
	puts(__PRETTY_FUNCTION__);
	std::cout << "Having fun with " << g.name_ << "\n";
}

// void use(Gadget& g)
// {
//     puts(__PRETTY_FUNCTION__);
//     have_fun(g);
// }

// void use(const Gadget& g)
// {
//     puts(__PRETTY_FUNCTION__);
//     have_fun(g);
// }

// void use(Gadget&& g)
// {
//     puts(__PRETTY_FUNCTION__);
//     have_fun(std::move(g));
// }

template <typename TGadget>
void use(TGadget&& g)
{
    puts(__PRETTY_FUNCTION__);
    have_fun(std::forward<TGadget>(g));    
}

// namespace Cpp20
// {    
//     void use(auto&& g)
//     {
//         puts(__PRETTY_FUNCTION__);
//         have_fun(std::forward<TGadget>(g));    
//     }
// }

TEST_CASE("perfect forwarding")
{
    std::cout << "\n\n----------------------\n";
    Gadget g1{1, "g1"};
    const Gadget cg2{2, "cg2"};

    use(g1);
    std::cout << "----------------------\n";
    use(cg2);
    std::cout << "----------------------\n";
    use(Gadget{3, "temp g3"});
}

TEST_CASE("auto & universal reference")
{
    int x = 10;
    auto&& uref1 = x; // int&

    auto&& uref2 = 10; // int&&
}

TEST_CASE("perfect forwarding - use cases")
{
    std::cout << "\n\n----------------------\n";

    SECTION("container + emplace")
    {
        std::vector<Gadget> vec;
        //vec.push_back(Gadget{1, "ipad"});
        vec.emplace_back(1, "ipad");

        std::map<int, std::string> dict;
        dict.insert(make_pair(1, "one"s));
        dict.emplace(2, "two");
    }

    SECTION("std::make_unique || std::make_shared")
    {
        std::unique_ptr<Gadget> ptrg = std::make_unique<Gadget>(42, "smartthing");   
        ptrg->use();     
    }
}

TEST_CASE("drawing")
{
    std::vector<int> vec = {1, 2, 3};

    std::vector<int> backup = vec;  // copy

    std::vector<int> target = std::move(vec);

    vec = vector<int>{4, 5, 6};
}

