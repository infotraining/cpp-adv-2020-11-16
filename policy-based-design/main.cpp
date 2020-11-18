#include "vector.hpp"
#include <iostream>

using namespace std;

int main()
{
    Vector<int, ThrowingRangeChecker> vec = {1, 2, 3};

    cout << "vec: ";
    for (size_t i = 0; i < vec.size(); ++i)
        cout << vec.at(i) << " ";
    cout << endl;
}
