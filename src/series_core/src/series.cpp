#include "lab2/series/series.hpp"

#include <cmath>
#include <new>

namespace lab2::series
{

bool create(MeasurementSeries& s, std::size_t n)
{
    if (n == 0)
    {
        s.data = nullptr;
        s.size = 0;
        return true;
    }

    double* p = new (std::nothrow) double[n]();
    if (p == nullptr)
    {
        return false;
    }

    s.data = p;
    s.size = n;
    return true;
}

void destroy(MeasurementSeries& s)
{
    delete[] s.data;
    s.data = nullptr;
    s.size = 0;
}

bool min_value(const MeasurementSeries& s, double& out)
{
    if (s.size == 0 || s.data == nullptr)
    {
        return false;
    }

    double result = s.data[0];
    for (std::size_t i = 1; i < s.size; ++i)
    {
        if (s.data[i] < result)
        {
            result = s.data[i];
        }
    }

    out = result;
    return true;
}

bool max_value(const MeasurementSeries& s, double& out)
{
    if (s.size == 0 || s.data == nullptr)
    {
        return false;
    }

    double result = s.data[0];
    for (std::size_t i = 1; i < s.size; ++i)
    {
        if (s.data[i] > result)
        {
            result = s.data[i];
        }
    }

    out = result;
    return true;
}

bool mean(const MeasurementSeries& s, double& out)
{
    if (s.size == 0)
    {
        return false;
    }

    double sum = 0.0;
    for (std::size_t i = 0; i < s.size; ++i)
    {
        sum += s.data[i];
    }

    out = sum / static_cast<double>(s.size);
    return true;
}

std::size_t count_greater(const MeasurementSeries& s, double threshold)
{
    std::size_t count = 0;
    for (std::size_t i = 0; i < s.size; ++i)
    {
        if (s.data[i] > threshold)
        {
            ++count;
        }
    }
    return count;
}
bool stddev(const MeasurementSeries& s, double& out)
{
    double m = 0.0;
    if (!mean(s, m))
    {
        return false;
    }

    double sum = 0.0;
    for (std::size_t i = 0; i < s.size; ++i)
    {
        const double d = s.data[i] - m;
        sum += d * d;
    }

    out = std::sqrt(sum / static_cast<double>(s.size));
    return true;
}

void shift(MeasurementSeries& s, double delta)
{
    for (std::size_t i = 0; i < s.size; ++i)
    {
        s.data[i] += delta;
    }
}

bool normalize(MeasurementSeries& s)
{
    double m = 0.0;
    double sd = 0.0;
    if (!mean(s, m) || !stddev(s, sd))
    {
        return false;
    }
    if (sd <= 0.0)
    {
        return false;
    }

    for (std::size_t i = 0; i < s.size; ++i)
    {
        s.data[i] = (s.data[i] - m) / sd;
    }
    return true;
}

bool filter_greater(const MeasurementSeries& s, double threshold, MeasurementSeries& out)
{
    destroy(out);

    const std::size_t count = count_greater(s, threshold);
    if (!create(out, count))
    {
        return false;
    }

    std::size_t j = 0;
    for (std::size_t i = 0; i < s.size; ++i)
    {
        if (s.data[i] > threshold)
        {
            out.data[j] = s.data[i];
            ++j;
        }
    }
    return true;
}

} // namespace lab2::series
