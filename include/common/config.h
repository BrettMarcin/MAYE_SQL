#include <cstdint>
#include <string>

static constexpr std::size_t PAGE_SIZE = 4096;
static constexpr const char* DEFAULT_PATH = "/var/lib/";
static constexpr const char* SETTINGS_FILE_NAME = "maye_db_config.json";

using page_id_t = std::int32_t;
using frame_id_t = std::int32_t;

static constexpr page_id_t INVALID_PAGE_ID = -1;

struct Config {
    std::string database_path;
};