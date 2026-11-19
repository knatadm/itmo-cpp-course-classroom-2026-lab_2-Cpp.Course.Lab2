#pragma once

#include <cstddef>

namespace lab2::series
{

struct MeasurementSeries
{
    double* data = nullptr;
    std::size_t size = 0;
};

bool create(MeasurementSeries& s, std::size_t n);

void destroy(MeasurementSeries& s);

bool min_value(const MeasurementSeries& s, double& out);
bool max_value(const MeasurementSeries& s, double& out);
bool mean(const MeasurementSeries& s, double& out);
std::size_t count_greater(const MeasurementSeries& s, double threshold);
bool stddev(const MeasurementSeries& s, double& out);
void shift(MeasurementSeries& s, double delta);
bool normalize(MeasurementSeries& s);
// out должна быть пустой
bool filter_greater(const MeasurementSeries& s, double threshold, MeasurementSeries& out);

} // namespace lab2::series