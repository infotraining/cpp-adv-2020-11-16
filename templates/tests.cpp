#include "catch.hpp"
#include <iostream>
#include <string>
#include <vector>

using namespace std;
using namespace Catch::Matchers;

struct X {
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