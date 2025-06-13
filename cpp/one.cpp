#include <iostream>
#include <fstream>
#include <cstdint>
#include <string>
#include <map>
#include <unordered_map>

struct Data
{
    double min;
    double max;
    double average;
    std::uint64_t total;
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

    for (const auto &pair : city_data)
    {
        const auto &city = pair.first;
        const auto &data = pair.second;

        std::cout << "City: " << city
                  << ", Min: " << data.min
                  << ", Max: " << data.max
                  << ", Average: " << data.average
                  << ", Total Measurements: " << data.total
                  << '\n';
    }

    return 0;
}