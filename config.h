#ifndef CONFIG_H
#define CONFIG_H

#include <vector>
#include <string>
#include <fstream>
#include <iostream>

class Config {
public:
    // Configuration parameters
    size_t population_size = 100;
    std::vector<size_t> network_architecture = {4, 16, 16, 4}; // Input, hidden layers, output
    double mutation_rate = 0.1;
    double crossover_rate = 0.7;
    size_t elite_count = 5;
    size_t generations = 100;
    size_t save_interval = 10;
    std::string output_dir = "output";

    // Load configuration from file
    bool load_from_file(const std::string& filename) {
        std::ifstream file(filename);
        if (!file.is_open()) {
            std::cerr << "Failed to open config file: " << filename << std::endl;
            return false;
        }

        std::string line;
        while (std::getline(file, line)) {
            // Skip comments and empty lines
            if (line.empty() || line[0] == '#') {
                continue;
            }

            size_t pos = line.find('=');
            if (pos == std::string::npos) {
                continue;
            }

            std::string key = line.substr(0, pos);
            std::string value = line.substr(pos + 1);

            // Remove whitespace
            key.erase(0, key.find_first_not_of(" \t"));
            key.erase(key.find_last_not_of(" \t") + 1);
            value.erase(0, value.find_first_not_of(" \t"));
            value.erase(value.find_last_not_of(" \t") + 1);

            if (key == "population_size") {
                population_size = std::stoul(value);
            } else if (key == "mutation_rate") {
                mutation_rate = std::stod(value);
            } else if (key == "crossover_rate") {
                crossover_rate = std::stod(value);
            } else if (key == "elite_count") {
                elite_count = std::stoul(value);
            } else if (key == "generations") {
                generations = std::stoul(value);
            } else if (key == "save_interval") {
                save_interval = std::stoul(value);
            } else if (key == "output_dir") {
                output_dir = value;
            } else if (key == "network_architecture") {
                network_architecture.clear();
                size_t start = 0;
                size_t end = value.find(',');
                while (end != std::string::npos) {
                    network_architecture.push_back(std::stoul(value.substr(start, end - start)));
                    start = end + 1;
                    end = value.find(',', start);
                }
                network_architecture.push_back(std::stoul(value.substr(start)));
            }
        }

        return true;
    }
};

#endif // CONFIG_H
