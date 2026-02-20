#include "storage/disk_manager.h"
#include "common/config.h"
#include <fstream>

using namespace maye_sql;

namespace maye_sql {

DiskManager::DiskManager(const std::string &db_file) : file_name_(db_file) {
    
    db_io_.open(db_file, std::ios::binary | std::ios::in | std::ios::out);

    if (!db_io_.is_open()) {
        db_io_.open(db_file, std::ios::binary | std::ios::trunc | std::ios::out);
        db_io_.close();
        db_io_.open(db_file, std::ios::binary | std::ios::in | std::ios::out);
    }

    if (!db_io_.is_open()) {
        throw std::runtime_error("Could not open database file: " + db_file);
    }
}

void DiskManager::WritePage(int page_id, const char *data) {
    std::streamsize offset = static_cast<std::streamsize>(page_id) * PAGE_SIZE;
    db_io_.seekp(offset);
    db_io_.write(data, PAGE_SIZE);
    db_io_.flush();
}

void DiskManager::ReadPage(int page_id, char *data) {
    std::streamsize offset = static_cast<std::streamsize>(page_id) * PAGE_SIZE;
    db_io_.seekg(offset);
    db_io_.read(data, PAGE_SIZE);
    if (db_io_.gcount() < static_cast<std::streamsize>(PAGE_SIZE)) {
        std::fill(data, data + PAGE_SIZE, 0);
    }
}

DiskManager::~DiskManager() {
    ShutDown();
}

void DiskManager::ShutDown() {
    if (db_io_.is_open()) {
        db_io_.flush();
        db_io_.close();
    }
}

}