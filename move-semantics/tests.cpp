#include "catch.hpp"
#include <iostream>
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