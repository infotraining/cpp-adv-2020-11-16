#include "vector.hpp"
#include "catch.hpp"
#include <algorithm>
#include <sstream>

using namespace std;

SCENARIO("Policy based design for vector", "[Vector]")
{
    GIVEN("Vector with ThrowingErrorPolicy")
    {
        Vector<int, ThrowingRangeChecker> vec = {1, 2, 3};

        WHEN("index is out of range")
        {
            THEN("exception is thrown")
            {
                REQUIRE_THROWS_AS(vec.at(5), std::out_of_range);
            }
        }
    }

    GIVEN("Vector with logging error policy")
    {
        Vector<int, LoggingErrorRangeChecker, StdMutex> vec = {1, 2, 3};
        stringstream mock_log;
        vec.set_log_file(mock_log);

        WHEN("index is out of range")
        {
            auto result = vec.at(5);

            THEN("error is logged into a file")
            {
                REQUIRE_THAT(mock_log.str(), Catch::Matchers::Contains("Error: Index out of range."));
            }

            THEN("last item is returned")
            {
                REQUIRE(result == 3);
            }
        }
    }
}
