#pragma once

#include <limits>
#include <cstdint>
#include <iostream>
#include <map>
#include <vector>
#include <string>

struct CityData
{
    std::string name{};
    double min{std::numeric_limits<double>::max()};
    double max{std::numeric_limits<double>::min()};
    double average{0.0};
    std::uint64_t total{0};
};

std::vector<CityData> ConvertToVector(const std::unordered_map<std::string, CityData> &city_data)
{
    std::vector<CityData> result;
    result.reserve(city_data.size());
    for (const auto &kv : city_data)
    {
        result.push_back(kv.second);
    }

    std::sort(result.begin(), result.end(),
              [](const CityData &a, const CityData &b)
              { return a.name < b.name; });

    return result;
}

void PrintData(const std::vector<CityData> &city_data)
{
    std::cout << "City;Min;Max;Average;Total Measurements\n";
    for (const auto &data : city_data)
    {
        std::cout << data.name << ";" << data.min << ";"
                  << data.max << ";" << data.average << ";"
                  << data.total << '\n';
    }
}