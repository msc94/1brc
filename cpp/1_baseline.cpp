#include <iostream>
#include <fstream>
#include <cstdint>
#include <string>
#include <algorithm>
#include <map>
#include <unordered_map>
#include <limits>

#include "common.h"

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

int main()
{
    auto fstream = std::ifstream("measurements.txt");
    if (!fstream)
    {
        std::cerr << "Error opening file." << std::endl;
        return 1;
    }

    std::unordered_map<std::string, CityData> city_data;

    auto line = std::string{};
    while (std::getline(fstream, line))
    {
        auto city = line.substr(0, line.find(';'));

        auto temperature = line.substr(line.find(';') + 1);
        auto parsed_temperature = std::stod(temperature);

        auto it = city_data.find(city);
        if (it == city_data.end())
        {
            auto [new_it, _] = city_data.emplace(city, CityData{.name = city});
            it = new_it;
        }

        auto &data = it->second;
        data.max = std::max(data.max, parsed_temperature);
        data.min = std::min(data.min, parsed_temperature);
        data.total += 1;
        data.average = (data.average * (data.total - 1) + parsed_temperature) / data.total;
    }

    auto vector_data = ConvertToVector(city_data);
    PrintData(vector_data);

    return 0;
}