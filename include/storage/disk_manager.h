#pragma once
#include <string>
#include <fstream>
#include "common/config.h"

using namespace std;

namespace maye_sql {

struct DiskHeader {
  page_id_t next_page_id;
  page_id_t first_free_page;
};

class DiskManager {
 public:
  explicit DiskManager(const string& db_file);
  ~DiskManager();

  void WritePage(page_id_t page_id, const char* data);
  void ReadPage(page_id_t page_id, char* data);

  page_id_t AllocatePage();
  void DeallocatePage(page_id_t page_id);

  void ShutDown();

 private:
  void UpdateHeader();
  mutex latch;
  fstream db_io_;
  string file_name_;
  page_id_t first_free_page;
  atomic<page_id_t> next_page_id;
};

}  // namespace maye_sql