#include <iostream>
#include <nlohmann/json.hpp>
#include <common/config.h>
#include <filesystem>

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
    cout << "creating" << endl;
}