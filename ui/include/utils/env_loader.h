#pragma once
#ifndef ENV_LOADER_H
#define ENV_LOADER_H

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <unordered_map>
#include <filesystem>
#include <algorithm>

namespace fs = std::filesystem;

class EnvLoader {
public:
    static std::unordered_map<std::string, std::string> load() {
        std::unordered_map<std::string, std::string> env_map;
        fs::path searchPath = fs::current_path();
        fs::path envFile;
        bool found = false;

        int maxDepth = 10;
        while (maxDepth-- > 0) {
            fs::path potential = searchPath / ".env";
            if (fs::exists(potential)) {
                envFile = potential;
                found = true;
                break;
            }

            if (searchPath == searchPath.root_path() || !searchPath.has_parent_path()) {
                break;
            }
            searchPath = searchPath.parent_path();
        }

        if (!found) {
            throw std::runtime_error(std::string("Could not find env file"));
        }

        std::ifstream file(envFile);
        std::string line;

        while (std::getline(file, line)) {
            auto trim = [](std::string& s) {
                s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](unsigned char ch) {
                    return !std::isspace(ch);
                }));
                s.erase(std::find_if(s.rbegin(), s.rend(), [](unsigned char ch) {
                    return !std::isspace(ch);
                }).base(), s.end());
            };

            trim(line);

            if (line.empty() || line[0] == '#') continue;

            size_t delimiterPos = line.find('=');
            if (delimiterPos != std::string::npos) {
                std::string key = line.substr(0, delimiterPos);
                std::string value = line.substr(delimiterPos + 1);

                trim(key);
                trim(value);

                if (value.size() >= 2 && value.front() == '"' && value.back() == '"') {
                    value = value.substr(1, value.size() - 2);
                }

                env_map[key] = value;
            }
        }

        return env_map;
    }
};

#endif // ENV_LOADER_H
