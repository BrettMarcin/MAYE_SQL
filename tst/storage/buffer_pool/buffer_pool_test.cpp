#include <gtest/gtest.h>
#include <algorithm>
#include <vector>
#include <thread>
#include "storage/buffer_pool_manager.h"

namespace maye_sql {

TEST(BufferPoolManagerTest, BinaryDataTest) {
  const size_t pool_size = 10;
  DiskManager* disk_manager = new DiskManager("test.db");
  BufferPoolManager* bpm = new BufferPoolManager(pool_size, disk_manager);

  page_id_t page_id_temp;
  Page* page0 = bpm->NewPage(&page_id_temp);

  ASSERT_NE(nullptr, page0);
  EXPECT_EQ(0, page_id_temp);

  char random_binary_data[PAGE_SIZE];
  for (size_t i = 0; i < PAGE_SIZE; i++) random_binary_data[i] = static_cast<char>(i % 256);
  std::memcpy(page0->data, random_binary_data, PAGE_SIZE);

  for (size_t i = 1; i < pool_size; i++) {
    EXPECT_NE(nullptr, bpm->NewPage(&page_id_temp));
  }

  for (size_t i = pool_size; i < pool_size * 2; i++) {
    EXPECT_EQ(nullptr, bpm->NewPage(&page_id_temp));
  }

  for (size_t i = 0; i < 5; i++) {
    EXPECT_EQ(true, bpm->UnpinPage(i, true));
  }
  for (size_t i = 0; i < 5; i++) {
    EXPECT_NE(nullptr, bpm->NewPage(&page_id_temp));
    bpm->UnpinPage(page_id_temp, false);
  }

  page0 = bpm->FetchPage(0);
  EXPECT_EQ(0, std::memcmp(page0->data, random_binary_data, PAGE_SIZE));
  EXPECT_EQ(true, bpm->UnpinPage(0, true));

  EXPECT_EQ(true, bpm->DeletePage(0));

  delete bpm;
  delete disk_manager;
  std::remove("test.db");
}

TEST(BufferPoolManagerTest, ConcurrencyTest) {
  const size_t pool_size = 10;
  DiskManager* disk_manager = new DiskManager("test.db");
  BufferPoolManager* bpm = new BufferPoolManager(pool_size, disk_manager);

  const size_t num_threads = 8;
  const size_t num_pages = 20;
  std::vector<std::thread> threads;

  for (size_t i = 0; i < num_threads; i++) {
    threads.emplace_back([&bpm]() {
      for (size_t j = 0; j < num_pages; j++) {
        page_id_t page_id;
        auto* page = bpm->NewPage(&page_id);
        if (page != nullptr) {
          std::this_thread::sleep_for(std::chrono::milliseconds(1));
          bpm->UnpinPage(page_id, true);
        }
      }
    });
  }

  for (auto& t : threads) t.join();

  delete bpm;
  delete disk_manager;
  std::remove("test.db");
}

}  // namespace maye_sql