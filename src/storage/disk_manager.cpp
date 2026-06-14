#include "storage/disk_manager.h"
#include <cstring>

namespace maye_sql {

DiskManager::DiskManager(const std::string& db_file) : file_name_(db_file) {
  db_io_.open(db_file, std::ios::binary | std::ios::in | std::ios::out);

  if (!db_io_.is_open()) {
    db_io_.open(db_file, std::ios::binary | std::ios::trunc | std::ios::out);
    db_io_.close();
    db_io_.open(db_file, std::ios::binary | std::ios::in | std::ios::out);

    next_page_id = 0;
    first_free_page = INVALID_PAGE_ID;

    char header_buffer[PAGE_SIZE] = {0};
    DiskHeader header{next_page_id.load(), first_free_page};
    std::memcpy(header_buffer, &header, sizeof(DiskHeader));

    db_io_.clear();
    db_io_.seekp(0);
    db_io_.write(header_buffer, PAGE_SIZE);
    db_io_.flush();
  } else {
    char header_buffer[PAGE_SIZE];
    db_io_.seekg(0);
    db_io_.read(header_buffer, PAGE_SIZE);

    DiskHeader header;
    std::memcpy(&header, header_buffer, sizeof(DiskHeader));
    next_page_id = header.next_page_id;
    first_free_page = header.first_free_page;
  }
}

inline std::streamsize GetOffset(page_id_t page_id) {
  return static_cast<std::streamsize>(page_id + 1) * PAGE_SIZE;
}

void DiskManager::WritePage(page_id_t page_id, const char* data) {
  std::lock_guard<std::mutex> lock(latch);
  db_io_.clear();
  db_io_.seekp(GetOffset(page_id));
  db_io_.write(data, PAGE_SIZE);
  db_io_.flush();
}

void DiskManager::ReadPage(page_id_t page_id, char* data) {
  std::lock_guard<std::mutex> lock(latch);
  db_io_.clear();
  db_io_.seekg(GetOffset(page_id));
  db_io_.read(data, PAGE_SIZE);

  if (db_io_.gcount() < static_cast<std::streamsize>(PAGE_SIZE)) {
    std::fill(data, data + PAGE_SIZE, 0);
  }
}

page_id_t DiskManager::AllocatePage() {
  std::lock_guard<std::mutex> lock(latch);
  page_id_t result_id = -1;

  if (first_free_page != INVALID_PAGE_ID) {
    result_id = first_free_page;
    char buffer[PAGE_SIZE];
    db_io_.seekg(GetOffset(result_id));
    db_io_.read(buffer, PAGE_SIZE);
    std::memcpy(&first_free_page, buffer, sizeof(page_id_t));
  } else {
    result_id = next_page_id++;
  }

  char header_buffer[PAGE_SIZE] = {0};
  DiskHeader header{next_page_id.load(), first_free_page};
  std::memcpy(header_buffer, &header, sizeof(DiskHeader));

  db_io_.clear();
  db_io_.seekp(0);
  db_io_.write(header_buffer, PAGE_SIZE);
  db_io_.flush();

  return result_id;
}

void DiskManager::DeallocatePage(page_id_t page_id) {
  if (page_id == INVALID_PAGE_ID) return;

  std::lock_guard<std::mutex> lock(latch);
  char buffer[PAGE_SIZE] = {0};
  std::memcpy(buffer, &first_free_page, sizeof(page_id_t));

  db_io_.clear();
  db_io_.seekp(GetOffset(page_id));
  db_io_.write(buffer, PAGE_SIZE);

  first_free_page = page_id;

  char header_buffer[PAGE_SIZE] = {0};
  DiskHeader header{next_page_id.load(), first_free_page};
  std::memcpy(header_buffer, &header, sizeof(DiskHeader));

  db_io_.clear();
  db_io_.seekp(0);
  db_io_.write(header_buffer, PAGE_SIZE);
  db_io_.flush();
}

void DiskManager::ShutDown() {
  if (db_io_.is_open()) {
    db_io_.flush();
    db_io_.close();
  }
}

DiskManager::~DiskManager() {
  ShutDown();
}

}  // namespace maye_sql