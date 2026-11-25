#include <catch2/catch_all.hpp>

#include "lab2/series/series.hpp"

namespace ls = lab2::series;

using Catch::Approx;

TEST_CASE("create and destroy manage series lifetime", "[series][lifecycle]")
{
    ls::MeasurementSeries s;

    SECTION("creates non-empty series")
    {
        REQUIRE(ls::create(s, 4));
        REQUIRE(s.size == 4);
        REQUIRE(s.data != nullptr);

        ls::destroy(s);
        REQUIRE(s.size == 0);
        REQUIRE(s.data == nullptr);
    }

    SECTION("creates zero-sized series")
    {
        REQUIRE(ls::create(s, 0));
        REQUIRE(s.size == 0);

        ls::destroy(s);
        REQUIRE(s.size == 0);
        REQUIRE(s.data == nullptr);
    }
}

TEST_CASE("min_value, max_value, mean and stddev compute statistics", "[series][stats]")
{
    ls::MeasurementSeries s;
    REQUIRE(ls::create(s, 4));
    s.data[0] = 10.0;
    s.data[1] = 20.0;
    s.data[2] = 30.0;
    s.data[3] = 40.0;

    double min_v = 0.0;
    double max_v = 0.0;
    double mean_v = 0.0;
    double std_v = 0.0;

    REQUIRE(ls::min_value(s, min_v));
    REQUIRE(ls::max_value(s, max_v));
    REQUIRE(ls::mean(s, mean_v));
    REQUIRE(ls::stddev(s, std_v));

    REQUIRE(min_v == Approx(10.0));
    REQUIRE(max_v == Approx(40.0));
    REQUIRE(mean_v == Approx(25.0));
    // Для [10, 20, 30, 40] среднее = 25, отклонения [-15, -5, 5, 15]:
    // среднеквадратичное отклонение = sqrt((225 + 25 + 25 + 225) / 4) = sqrt(125) ≈ 11.1803398875
    REQUIRE(std_v == Approx(11.1803398875).margin(1e-7));

    ls::destroy(s);
}

TEST_CASE("statistics return false on empty series", "[series][stats][edge]")
{
    ls::MeasurementSeries empty;
    double out = -1.0;

    REQUIRE_FALSE(ls::min_value(empty, out));
    REQUIRE_FALSE(ls::max_value(empty, out));
    REQUIRE_FALSE(ls::mean(empty, out));
    REQUIRE_FALSE(ls::stddev(empty, out));
}

TEST_CASE("count_greater counts elements strictly above threshold", "[series][query]")
{
    ls::MeasurementSeries s;
    REQUIRE(ls::create(s, 5));
    s.data[0] = -5.0;
    s.data[1] = 10.0;
    s.data[2] = 25.0;
    s.data[3] = 25.0;
    s.data[4] = 40.0;

    REQUIRE(ls::count_greater(s, 25.0) == 1);
    REQUIRE(ls::count_greater(s, 10.0) == 3);
    REQUIRE(ls::count_greater(s, 100.0) == 0);

    ls::destroy(s);

    ls::MeasurementSeries empty;
    REQUIRE(ls::count_greater(empty, 0.0) == 0);
}

TEST_CASE("shift adds delta to every element", "[series][transform]")
{
    ls::MeasurementSeries s;
    REQUIRE(ls::create(s, 3));
    s.data[0] = 1.0;
    s.data[1] = -2.0;
    s.data[2] = 5.5;

    ls::shift(s, 2.5);

    REQUIRE(s.data[0] == Approx(3.5));
    REQUIRE(s.data[1] == Approx(0.5));
    REQUIRE(s.data[2] == Approx(8.0));

    ls::destroy(s);
}

TEST_CASE("normalize centers and scales series or rejects degenerate input", "[series][transform]")
{
    SECTION("normalizes non-constant series")
    {
        ls::MeasurementSeries s;
        REQUIRE(ls::create(s, 4));
        s.data[0] = 10.0;
        s.data[1] = 20.0;
        s.data[2] = 30.0;
        s.data[3] = 40.0;

        REQUIRE(ls::normalize(s));

        double mean_v = 123.0;
        double std_v = 123.0;
        REQUIRE(ls::mean(s, mean_v));
        REQUIRE(ls::stddev(s, std_v));

        REQUIRE(mean_v == Approx(0.0).margin(1e-9));
        REQUIRE(std_v == Approx(1.0).margin(1e-9));

        ls::destroy(s);
    }

    SECTION("returns false when standard deviation is zero or series is empty")
    {
        ls::MeasurementSeries constant;
        REQUIRE(ls::create(constant, 3));
        constant.data[0] = 7.0;
        constant.data[1] = 7.0;
        constant.data[2] = 7.0;

        REQUIRE_FALSE(ls::normalize(constant));
        ls::destroy(constant);

        ls::MeasurementSeries empty;
        REQUIRE_FALSE(ls::normalize(empty));
    }
}

TEST_CASE("filter_greater writes matching elements into empty output series", "[series][filter]")
{
    ls::MeasurementSeries s;
    REQUIRE(ls::create(s, 5));
    s.data[0] = 3.0;
    s.data[1] = 12.0;
    s.data[2] = 8.0;
    s.data[3] = 20.0;
    s.data[4] = 1.5;

    ls::MeasurementSeries filtered;
    REQUIRE(ls::filter_greater(s, 8.0, filtered));
    REQUIRE(filtered.size == 2);
    REQUIRE(filtered.data[0] == Approx(12.0));
    REQUIRE(filtered.data[1] == Approx(20.0));

    ls::destroy(filtered);
    ls::destroy(s);
}