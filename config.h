#ifndef CONFIG_H
#define CONFIG_H

#include <iostream>
#include <vector>
#include <string.h>
#include <fstream>

struct tuple {
    std::string name;
    std::string value;
};

class Config {
    static std::vector<tuple> configValues;
    static std::vector<std::string> Splitter(std::string del, std::string value);

public:
    Config(std::string configPath);
    Config();
    static std::string LookupString(std::string search);
    static int LookupInt(std::string search);
    static float LookupFloat(std::string search);
    static void LoadConfig(std::string configPath);
};

std::vector<tuple> Config::configValues;

Config::Config() {
}

Config::Config(std::string configPath) {
    LoadConfig(configPath);
}

void Config::LoadConfig(std::string configPath) {
    std::string line;
    std::ifstream myfile(configPath.c_str());
    if (myfile.is_open()) {
        while (getline(myfile,line)) {
//            std::cout << "Parsing line: " << line << "\n";
            std::vector<std::string> tempVector;
            tempVector = Config::Splitter("=", line);
            if(tempVector.size() > 0) {
                tuple tempTuple;
                tempTuple.name = tempVector[0];
                tempTuple.value = tempVector[1];
                Config::configValues.push_back(tempTuple);
//                std::cout << "Adding tuple to setting. Name: " << tempVector[0] << " Value: " << tempVector[1] << "\n";
            }
        }
        myfile.close();
    } else {
        std::cout << "Unable to open config file at: \"" << configPath << "\"\n";
    }
  return;
}

std::vector<std::string> Config::Splitter(std::string del, std::string str) {
    std::vector<std::string> returnString;
    int pos = str.find(del);
    if(pos == -1) {
//        std::cout << "Did not find delimiter in string \"" << str << "\"\n";
        return returnString;
    }
//    std::cout << "Looking for \"" << del << "\" in \"" << str << "\" Position: " << pos << "\n";
//    std::cout << "Substring block 1: \"" << str.substr(0, pos) << "\"\n";
    returnString.push_back(str.substr(0, pos));
    returnString.push_back(str.substr(pos + 1));
    return returnString;
}

std::string Config::LookupString(std::string search) {
    for(int i = 0; i < Config::configValues.size(); i++) {
        if(Config::configValues[i].name == search) {
            return Config::configValues[i].value;
        }
    }
    std::cout << "Config value not found: \"" << search << "\"\n";
    return "";
}

int Config::LookupInt(std::string search) {
    for(int i = 0; i < Config::configValues.size(); i++) {
        if(Config::configValues[i].name == search) {
            int Result;
            std::stringstream convert(Config::configValues[i].value);
            if (!(convert >> Result)) {
                std::cout << "Conversion to int failed for config value: \"" << search << "\"\n";
                Result = 0;
            }
            return Result;
        }
    }
    std::cout << "Config value not found: \"" << search << "\"\n";
    return 0;
}

float Config::LookupFloat(std::string search) {
    for(int i = 0; i < Config::configValues.size(); i++) {
        if(Config::configValues[i].name == search) {
            float Result;
            std::stringstream convert(Config::configValues[i].value);
            if (!(convert >> Result)) {
                std::cout << "Conversion to float failed for config value: \"" << search << "\"\n";
                Result = 0;
            }
            return Result;
        }
    }
    std::cout << "Config value not found: \"" << search << "\"\n";
    return 0;
}



#endif
