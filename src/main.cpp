#include <iostream>
#include <nlohmann/json.hpp>
#include <common/config.h>
#include <filesystem>
#include <fstream>

using namespace std;
namespace fs = std::filesystem;
void create_config();

int main() {
    cout << "Hello" << endl;

    fs::path config_path = SETTINGS_FILE_NAME;

    if (!fs::exists(config_path)) {
        create_config();
    }
    return 0;
}


void create_config() {
    std::string user_input;
    std::cout << "Enter data directory (default: ./data/): ";
    std::getline(std::cin, user_input);

    if (user_input.empty()) {
        user_input = "./data/"; // Local default is safer for dev
    }

    // 1. Create the directory if it doesn't exist
    fs::path dir_path(user_input);
    if (!fs::exists(dir_path)) {
        fs::create_directories(dir_path); 
    }

    nlohmann::json config;
    config["db_path"] = (dir_path / "maye.db").string();
    config["page_size"] = 4096;
    config["buffer_pool_size"] = 1024;

    std::ofstream file(SETTINGS_FILE_NAME); 
    
    if (file.is_open()) {
        file << config.dump(4);
        file.close();
    }
}