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
};

void PrintData(const std::vector<CityData> &city_data)
{
    std::cout << "City;Min;Max;Average;Total Measurements\n";
    for (const auto &data : city_data)
    {
        std::cout << data.name << ";" << data.min << ";"
                  << data.max << ";" << data.average << "\n";
    }
}