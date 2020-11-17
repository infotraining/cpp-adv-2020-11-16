#include "catch.hpp"
#include <iostream>
#include <memory>
#include <string>
#include <vector>

using namespace std;
using namespace Catch::Matchers;

struct X
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

TEST_CASE("custom deallocators")
{
    SECTION("FILE*")
    {
        FILE* file = fopen("text.txt", "w");

        fprintf(file, "%d", 42);
        may_throw();

        fclose(file);
    }

    SECTION("unique_ptr<FILE>")
    {
        SECTION("deallocator as function")
        {
            std::unique_ptr<FILE, int (*)(FILE*)> file {fopen("text2.txt", "w"), fclose};

            fprintf(file.get(), "%d", 42);
            may_throw();
        }

        SECTION("deallocator as closure")
        {
            auto file_closer = [](FILE* f) { std::cout << "file closed...\n"; fclose(f); };
            std::unique_ptr<FILE, decltype(file_closer)> file_2 {fopen("2text.txt", "w+"), file_closer};

            fprintf(file_2.get(), "%d", 42);
            may_throw();
        }
    }
}

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