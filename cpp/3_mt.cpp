#include <iostream>
#include <fstream>
#include <cstdint>
#include <string>
#include <algorithm>
#include <map>
#include <string_view>
#include <unordered_map>
#include <limits>
#include <sys/mman.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#include <thread>

#include "common.h"

struct MyCityData
{
    std::int64_t min{std::numeric_limits<std::int64_t>::max()};
    std::int64_t max{std::numeric_limits<std::int64_t>::min()};
    std::int64_t sum{0};
    std::int64_t total{0};
};

int main()
{
    // mmap file
    const std::string filename = "measurements.txt";

    auto fd = open(filename.c_str(), O_RDONLY);
    if (fd < 0)
    {
        perror("Error opening file");
        return 1;
    }

    // Get the size of the file
    struct stat file_stat;
    if (fstat(fd, &file_stat) < 0)
    {
        perror("Error getting file size");
        return 1;
    }

    const auto *const start = static_cast<char *>(mmap(nullptr, file_stat.st_size, PROT_READ, MAP_PRIVATE, fd, 0));
    if (start == MAP_FAILED)
    {
        perror("Error mapping file");
        return 1;
    }

    const auto *const end = start + file_stat.st_size;

    // Divide the file into chunks for each thread
    const auto num_threads = std::thread::hardware_concurrency();
    const auto chunk_size = file_stat.st_size / num_threads;
    auto thread_work = std::vector<std::pair<const char *, const char *>>{};
    for (std::size_t i = 0; i < num_threads; ++i)
    {
        // First thread starts at the beginning, all others start at the end of the previous chunk
        const auto *chunk_start = (i == 0) ? start : thread_work[i - 1].second;

        // Last thread goes to the end, all others go to the end of their chunk
        const auto *chunk_end = [&]()
        {
            if (i == num_threads - 1)
            {
                return end;
            }

            auto chunk_end = chunk_start + chunk_size;
            // Ensure we don't split a line
            while (*chunk_end != '\n')
            {
                ++chunk_end;
            }

            return chunk_end + 1;
        }();

        thread_work.emplace_back(chunk_start, chunk_end);
    }

    // Start a thread for each chunk
    auto threads = std::vector<std::thread>{};

    auto thread_results = std::vector<std::unordered_map<std::string_view, MyCityData>>{};
    thread_results.resize(num_threads);

    for (auto i = std::size_t{0}; i < num_threads; ++i)
    {
        threads.emplace_back([&, i]()
                             {
            const auto [chunk_start, chunk_end] = thread_work[i];
            auto *current = chunk_start;
            while (current < chunk_end)
            {
                // Find the first ';', delimiting city
                const auto *const city_start = current;
                auto city_size = std::size_t{0};
                while (city_start[city_size] != ';')
                {
                    ++city_size;
                }

                // Find the next '\n', delimiting temperature
                const auto *const temperature_start = city_start + city_size + 1;
                auto temperature_size = std::size_t{0};
                auto parsed_temperature = std::int64_t{0};
                auto is_negative = false;

                if (temperature_start[temperature_size] == '-')
                {
                    is_negative = true;
                    ++temperature_size;
                }

                while (temperature_start[temperature_size] != '\n')
                {
                    const auto c = temperature_start[temperature_size];
                    if (c == '.')
                    {
                        // Skip decimal point
                    }
                    else if (c < '0' || c > '9')
                    {
                        std::cerr << "Invalid character in temperature: " << c << '\n';
                        exit(1);
                    }
                    else
                    {
                        parsed_temperature = parsed_temperature * 10 + (c - '0');
                    }
                    ++temperature_size;
                }

                if (is_negative)
                {
                    parsed_temperature = -parsed_temperature;
                }

                auto &thread_data = thread_results[i];
                const auto city_name_view = std::string_view{city_start, city_size};
                auto it = thread_data.find(city_name_view);
                if (it == thread_data.end())
                {
                    auto [new_it, _] = thread_data.emplace(city_name_view, MyCityData{});
                    it = new_it;
                }

                auto &data = it->second;
                data.max = std::max(data.max, parsed_temperature);
                data.min = std::min(data.min, parsed_temperature);
                data.total += 1;
                data.sum += parsed_temperature;

                // Move to the next line
                current = temperature_start + temperature_size;
                ++current;
            } });
    }

    // Wait for all threads to finish
    for (auto &thread : threads)
    {
        thread.join();
    }

    // Combine results from all threads
    auto city_data = std::unordered_map<std::string_view, MyCityData>{};
    for (const auto &thread_data : thread_results)
    {
        for (const auto &[city_name, data] : thread_data)
        {
            auto &combined_data = city_data[city_name];
            combined_data.min = std::min(combined_data.min, data.min);
            combined_data.max = std::max(combined_data.max, data.max);
            combined_data.sum += data.sum;
            combined_data.total += data.total;
        }
    }

    // Convert to vector and print
    auto vector_data = std::vector<CityData>{};
    vector_data.reserve(city_data.size());
    for (const auto &[city_name, data] : city_data)
    {
        vector_data.push_back(CityData{
            .name = std::string(city_name),
            .min = static_cast<double>(data.min) / 10.0,
            .max = static_cast<double>(data.max) / 10.0,
            .average = static_cast<double>(data.sum / data.total) / 10.0,
        });
    }
    std::sort(vector_data.begin(), vector_data.end(),
              [](const CityData &a, const CityData &b)
              { return a.name < b.name; });
    PrintData(vector_data);

    return 0;
}