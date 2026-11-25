#include <cerrno>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iomanip>
#include <iostream>

#include "lab2/series/series.hpp"

namespace ls = lab2::series;

namespace
{

int fail(const char* message)
{
    std::cerr << "error: " << message << '\n';
    return 1;
}

constexpr const char* kStateFile = "series_state.txt";

// Превращает строку в положительное целое число. false, если не получилось.
bool parse_size(const char* text, std::size_t& out)
{
    if (text[0] == '\0' || text[0] == '-')
    {
        return false;
    }

    char* end = nullptr;
    errno = 0;
    const unsigned long long value = std::strtoull(text, &end, 10);
    if (end == text || *end != '\0' || errno != 0 || value == 0)
    {
        return false;
    }

    out = static_cast<std::size_t>(value);
    return true;
}

// Сохраняет серию в файл: сначала размер, потом числа по одному в строке.
bool save_state(const ls::MeasurementSeries& s)
{
    std::ofstream file(kStateFile);
    if (!file)
    {
        return false;
    }

    file << s.size << '\n';
    file << std::setprecision(17);
    for (std::size_t i = 0; i < s.size; ++i)
    {
        file << s.data[i] << '\n';
    }
    return static_cast<bool>(file);
}

bool load_state(ls::MeasurementSeries& s)
{
    std::ifstream file(kStateFile);
    if (!file)
    {
        return false;
    }

    std::size_t n = 0;
    if (!(file >> n))
    {
        return false;
    }

    if (!ls::create(s, n))
    {
        return false;
    }

    for (std::size_t i = 0; i < n; ++i)
    {
        if (!(file >> s.data[i]))
        {
            ls::destroy(s);
            return false;
        }
    }

    return true;
}

int cmd_load(int argc, char* argv[])
{
    if (argc != 3)
    {
        return fail("usage: series_cli load N");
    }

    std::size_t n = 0;
    if (!parse_size(argv[2], n))
    {
        return fail("N must be a positive integer");
    }

    ls::MeasurementSeries s;
    if (!ls::create(s, n))
    {
        return fail("not enough memory");
    }

    for (std::size_t i = 0; i < n; ++i)
    {
        if (!(std::cin >> s.data[i]))
        {
            ls::destroy(s);
            return fail("failed to read a number");
        }
    }

    const bool saved = save_state(s);
    ls::destroy(s);
    if (!saved)
    {
        return fail("cannot save the series");
    }
    return 0;
}

int cmd_print(int argc)
{
    if (argc != 2)
    {
        return fail("usage: series_cli print");
    }

    ls::MeasurementSeries s;
    if (!load_state(s))
    {
        return fail("no series loaded");
    }

    for (std::size_t i = 0; i < s.size; ++i)
    {
        if (i > 0)
        {
            std::cout << ' ';
        }
        std::cout << s.data[i];
    }
    std::cout << '\n';

    ls::destroy(s);
    return 0;
}

int cmd_stats(int argc)
{
    if (argc != 2)
    {
        return fail("usage: series_cli stats");
    }

    ls::MeasurementSeries s;
    if (!load_state(s))
    {
        return fail("no series loaded");
    }

    double min_v = 0.0;
    double max_v = 0.0;
    double mean_v = 0.0;

    if (!ls::min_value(s, min_v) || !ls::max_value(s, max_v) || !ls::mean(s, mean_v))
    {
        ls::destroy(s);
        return fail("cannot compute stats for empty series");
    }

    std::cout << "min=" << min_v << " max=" << max_v << " mean=" << mean_v << '\n';

    ls::destroy(s);
    return 0;
}

bool parse_double(const char* text, double& out)
{
    if (text[0] == '\0')
    {
        return false;
    }

    char* end = nullptr;
    errno = 0;
    const double value = std::strtod(text, &end);
    if (end == text || *end != '\0' || errno != 0)
    {
        return false;
    }

    out = value;
    return true;
}

int cmd_greater(int argc, char* argv[])
{
    if (argc != 3)
    {
        return fail("usage: series_cli greater THRESHOLD");
    }

    double threshold = 0.0;
    if (!parse_double(argv[2], threshold))
    {
        return fail("THRESHOLD must be a number");
    }

    ls::MeasurementSeries s;
    if (!load_state(s))
    {
        return fail("no series loaded");
    }

    std::cout << ls::count_greater(s, threshold) << '\n';

    ls::destroy(s);
    return 0;
}

int cmd_shift(int argc, char* argv[])
{
    if (argc != 3)
    {
        return fail("usage: series_cli shift DELTA");
    }

    double delta = 0.0;
    if (!parse_double(argv[2], delta))
    {
        return fail("DELTA must be a number");
    }

    ls::MeasurementSeries s;
    if (!load_state(s))
    {
        return fail("no series loaded");
    }

    ls::shift(s, delta);
    const bool saved = save_state(s);

    ls::destroy(s);
    if (!saved)
    {
        return fail("cannot save the series");
    }
    return 0;
}

int cmd_normalize(int argc)
{
    if (argc != 2)
    {
        return fail("usage: series_cli normalize");
    }

    ls::MeasurementSeries s;
    if (!load_state(s))
    {
        return fail("no series loaded");
    }

    if (!ls::normalize(s))
    {
        ls::destroy(s);
        return fail("cannot normalize series");
    }

    const bool saved = save_state(s);
    ls::destroy(s);
    if (!saved)
    {
        return fail("cannot save the series");
    }
    return 0;
}

int cmd_filter(int argc, char* argv[])
{
    if (argc != 3)
    {
        return fail("usage: series_cli filter THRESHOLD");
    }

    double threshold = 0.0;
    if (!parse_double(argv[2], threshold))
    {
        return fail("THRESHOLD must be a number");
    }

    ls::MeasurementSeries s;
    if (!load_state(s))
    {
        return fail("no series loaded");
    }

    ls::MeasurementSeries filtered;
    if (!ls::filter_greater(s, threshold, filtered))
    {
        ls::destroy(s);
        return fail("failed to filter series");
    }

    for (std::size_t i = 0; i < filtered.size; ++i)
    {
        if (i > 0)
        {
            std::cout << ' ';
        }
        std::cout << filtered.data[i];
    }
    std::cout << '\n';

    ls::destroy(filtered);
    ls::destroy(s);
    return 0;
}

} // namespace

int main(int argc, char* argv[])
{
    if (argc < 2)
    {
        return fail("no command given");
    }

    const char* cmd = argv[1];

    if (std::strcmp(cmd, "load") == 0)
    {
        return cmd_load(argc, argv);
    }
    if (std::strcmp(cmd, "print") == 0)
    {
        return cmd_print(argc);
    }
    if (std::strcmp(cmd, "stats") == 0)
    {
        return cmd_stats(argc);
    }
    if (std::strcmp(cmd, "greater") == 0)
    {
        return cmd_greater(argc, argv);
    }
    if (std::strcmp(cmd, "shift") == 0)
    {
        return cmd_shift(argc, argv);
    }
    if (std::strcmp(cmd, "normalize") == 0)
    {
        return cmd_normalize(argc);
    }
    if (std::strcmp(cmd, "filter") == 0)
    {
        return cmd_filter(argc, argv);
    }

    return fail("unknown command");
}