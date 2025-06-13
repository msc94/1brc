#include <iostream>
#include <fstream>
#include <cstdint>
#include <string>
#include <algorithm>
#include <map>
#include <unordered_map>
#include <limits>

struct Data
{
    double min{std::numeric_limits<double>::max()};
    double max{std::numeric_limits<double>::min()};
    double average{0.0};
    std::uint64_t total{0};
};

int main()
{
    auto fstream = std::ifstream("measurements.txt");
    if (!fstream)
    {
        std::cerr << "Error opening file." << std::endl;
        return 1;
    }

    std::unordered_map<std::string, Data> city_data;

    auto line = std::string{};
    while (std::getline(fstream, line))
    {
        auto city = line.substr(0, line.find(';'));

        auto temperature = line.substr(line.find(';') + 1);
        auto parsed_temperature = std::stod(temperature);

        auto &data = city_data[city];
        data.max = std::max(data.max, parsed_temperature);
        data.min = std::min(data.min, parsed_temperature);
        data.total += 1;
        data.average = (data.average * (data.total - 1) + parsed_temperature) / data.total;
    }

    std::cout << "City;Min;Max;Average;Total Measurements\n";
    for (const auto &pair : city_data)
    {
        const auto &city = pair.first;
        const auto &data = pair.second;

        std::cout << city << ";" << data.min << ";"
                  << data.max << ";" << data.average << ";"
                  << data.total << '\n';
    }

    return 0;
}