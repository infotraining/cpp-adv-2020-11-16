#include "catch.hpp"
#include <iostream>
#include <memory>
#include <string>
#include <vector>
#include <map>

using namespace std;
using namespace Catch::Matchers;

struct X : std::enable_shared_from_this<X>
{
    int value;

    X(int v, const string& name)
        : value {v}
    {
        cout << "X(" << value << ", " << name << ")\n";
    }

    X()
    {
        static int gen_id = 0;
        value = ++gen_id;

        cout << "X(" << value << ")\n";
    }

    X(const X&) = delete;
    X(X&&) = delete;
    X& operator=(const X&) = delete;
    X& operator=(X&&) = delete;

    ~X()
    {
        cout << "~X(" << value << ")\n";
    }

    void register_me_as(std::string key, std::map<std::string, std::shared_ptr<X>>& dev_catalogue)
    {
        dev_catalogue.emplace(key, shared_from_this());
    }
};

namespace legacy_code
{
    X* get_x();

    X* get_x()
    {
        return new X(665, "less evil");
    }

    void use_x(X* ptr);

    void use_x(X* ptr)
    {
        if (ptr)
            cout << "using X: " << ptr->value << endl;

        delete ptr;
    }

    void use(X* ptr);

    void use(X* ptr)
    {
        if (ptr)
            cout << "using X: " << ptr->value << endl;
    }
}

TEST_CASE("legacy hell")
{
    using namespace legacy_code;

    {
        X* ptr = get_x();
        //...
    } // mem-leak

    {
        use(get_x());
    } // mem-leak

    {
        // X x {1, "evil"};
        // X* ptr = &x;
        // use_x(ptr); // seg-fault
    }
}

namespace modernized
{
    std::unique_ptr<X> get_x();

    std::unique_ptr<X> get_x()
    {
        return std::make_unique<X>(665, "less evil");
    }

    void use_x(std::unique_ptr<X> ptr);

    void use_x(std::unique_ptr<X> ptr)
    {
        if (ptr)
            cout << "using X: " << ptr->value << endl;
    }

    void use(X* ptr);

    void use(X* ptr)
    {
        if (ptr)
            cout << "using X: " << ptr->value << endl;
    }
}

TEST_CASE("safe code")
{
    using namespace modernized;

    {
        std::unique_ptr<X> ptr = get_x(); // move semantics
    }

    {
        std::unique_ptr<X> ptr = get_x();
        use(ptr.get());
    }

    {
        X x {1, "evil"};
        X* ptr = &x;
        use(ptr);
    }

    SECTION("transfer ownership")
    {
        use_x(get_x());

        std::unique_ptr<X> ptr = get_x();
        ptr->value = 1024;
        use_x(std::move(ptr));
    }

    SECTION("std::vector + std::unique_ptr")
    {
        std::vector<std::unique_ptr<X>> ptrs;

        ptrs.push_back(std::make_unique<X>(423, "x423"));

        std::unique_ptr<X> ptr = get_x();
        ptrs.push_back(std::move(ptr));
        ptrs.push_back(get_x());

        for (const auto& ptr : ptrs)
            std::cout << ptr->value << "\n";
    }
}

void may_throw()
{
    // throw 42;
}

// TEST_CASE("custom deallocators")
// {
//     SECTION("FILE*")
//     {
//         FILE* file = fopen("text.txt", "w");

//         fprintf(file, "%d", 42);
//         may_throw();

//         fclose(file);
//     }

//     SECTION("unique_ptr<FILE>")
//     {
//         SECTION("deallocator as function")
//         {
//             std::unique_ptr<FILE, int (*)(FILE*)> file {fopen("text2.txt", "w"), fclose};

//             fprintf(file.get(), "%d", 42);
//             may_throw();
//         }

//         SECTION("deallocator as closure")
//         {
//             auto file_closer = [](FILE* f) { std::cout << "file closed...\n"; fclose(f); };
//             std::unique_ptr<FILE, decltype(file_closer)> file_2 {fopen("2text.txt", "w+"), file_closer};

//             fprintf(file_2.get(), "%d", 42);
//             may_throw();
//         }
//     }
// }

TEST_CASE("cooperation with legacy code")
{
    std::unique_ptr<X> ptr(legacy_code::get_x());
    ptr->value = 443;

    legacy_code::use(ptr.get());

    legacy_code::use_x(ptr.release());    
}



TEST_CASE("std::unique_ptr<int[]>")
{
    std::unique_ptr<X[]> tab{new X[10]};

    tab[0].value = 1000;
}

TEST_CASE("shared_ptrs + weak_ptrs")
{
    auto sg1 = std::make_shared<X>(42, "x");

    REQUIRE(sg1.use_count() == 1);   

    auto sg2 = sg1;

    REQUIRE(sg1.get() == sg2.get());
    REQUIRE(sg1.use_count() == 2);

    std::weak_ptr<X> wg = sg1;
    REQUIRE(sg1.use_count() == 2);

    std::cout << sg1->value << "\n";

    SECTION("lock")
    {
        shared_ptr<X> sg3 = wg.lock();
        sg3->value = 68;
        REQUIRE(sg1.use_count() == 3);
    } // use_count == 2

    sg2.reset();
    sg1.reset();

    shared_ptr<X> sg3 = wg.lock();    
    REQUIRE(sg3 == nullptr);

    SECTION("using shared_ptr constructor")
    {
        REQUIRE_THROWS_AS(shared_ptr<X>(wg), std::bad_weak_ptr);
    }
}

TEST_CASE("enable shared from this")
{
    std::map<std::string, std::shared_ptr<X>> devices;

    devices.emplace("dev1", std::make_shared<X>(665, "device #1"));

    auto dev = std::make_shared<X>(42, "device #2");

    dev->register_me_as("dev2", devices);
}