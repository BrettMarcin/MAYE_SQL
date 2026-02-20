#pragma once
#include <string>
#include <fstream>
#include "common/config.h"

namespace maye_sql {

class DiskManager {
public:

    explicit DiskManager(const std::string &db_file);
    ~DiskManager();


    void WritePage(int page_id, const char *data);
    void ReadPage(int page_id, char *data);

    void ShutDown();

private:
    std::fstream db_io_;
    std::string file_name_;
};

}