#include <gtest/gtest.h>

#include <thread>
#include <vector>
#include <mutex>
#include <unordered_set>

#include "storage/buffer_pool_manager.h"
#include "storage/disk_manager.h"
#include "storage/lruk_replacer.h"

#include "storage/table_heap.h"

using namespace std;
using namespace maye_sql;

class TableHeapTest : public ::testing::Test {
 protected:
  void TearDown() override {
    std::remove("test.db");
  }
};

TEST_F(TableHeapTest, BasicInsertGetTest) {
  DiskManager* dm = new DiskManager("test.db");
  BufferPoolManager* bpm = new BufferPoolManager(10, dm);
  TableHeap* heap = new TableHeap(bpm, INVALID_PAGE_ID);

  const char* raw_data = "HelloMayeSQL";
  uint32_t data_len = static_cast<uint32_t>(std::strlen(raw_data));
  Tuple tuple(raw_data, data_len);

  RID rid;

  bool insert_success = heap->InsertTuple(tuple, &rid);
  ASSERT_TRUE(insert_success);

  Tuple retrieved_tuple;
  bool get_success = heap->GetTuple(rid, &retrieved_tuple);

  ASSERT_TRUE(get_success);
  EXPECT_EQ(retrieved_tuple.GetLength(), data_len);
  EXPECT_EQ(std::memcmp(retrieved_tuple.GetData(), raw_data, data_len), 0);
}

TEST_F(TableHeapTest, MultipleInsertGetTest) {
  DiskManager* dm = new DiskManager("test.db");
  BufferPoolManager* bpm = new BufferPoolManager(10, dm);
  TableHeap* heap = new TableHeap(bpm, INVALID_PAGE_ID);

  std::vector<std::string> values;
  std::vector<RID> rids;

  for (int i = 0; i < 10; ++i) {
    std::string value = "tuple_" + std::to_string(i);
    Tuple tuple(value.c_str(), static_cast<uint32_t>(value.size()));

    RID rid;
    bool insert_success = heap->InsertTuple(tuple, &rid);
    ASSERT_TRUE(insert_success) << "Insert failed for tuple " << i;

    values.push_back(value);
    rids.push_back(rid);
  }

  for (int i = 0; i < 10; ++i) {
    Tuple retrieved_tuple;
    bool get_success = heap->GetTuple(rids[i], &retrieved_tuple);

    ASSERT_TRUE(get_success) << "Get failed for tuple " << i;
    EXPECT_EQ(retrieved_tuple.GetLength(), values[i].size());
    EXPECT_EQ(std::memcmp(retrieved_tuple.GetData(), values[i].c_str(), values[i].size()), 0)
        << "Data mismatch for tuple " << i;
  }
}

TEST_F(TableHeapTest, MultiplePageSpillTest) {
  DiskManager* dm = new DiskManager("test.db");
  BufferPoolManager* bpm = new BufferPoolManager(9, dm);
  TableHeap* heap = new TableHeap(bpm, INVALID_PAGE_ID);

  const uint32_t kTupleSize = 2000;
  const int kNumTuples = 10;

  std::vector<std::string> values;
  std::vector<RID> rids;

  for (int i = 0; i < kNumTuples; ++i) {
    std::string value(kTupleSize, static_cast<char>('A' + i));
    Tuple tuple(value.c_str(), kTupleSize);

    RID rid;
    bool insert_success = heap->InsertTuple(tuple, &rid);
    ASSERT_TRUE(insert_success) << "Insert failed for tuple " << i;

    values.push_back(value);
    rids.push_back(rid);
  }

  std::set<page_id_t> page_ids;
  for (const auto& rid : rids) {
    page_ids.insert(rid.GetPageId());
  }
  ASSERT_GT(page_ids.size(), 1u) << "Expected tuples to spill across multiple pages";

  for (int i = 0; i < kNumTuples; ++i) {
    Tuple retrieved_tuple;
    bool get_success = heap->GetTuple(rids[i], &retrieved_tuple);

    ASSERT_TRUE(get_success) << "Get failed for tuple " << i;
    EXPECT_EQ(retrieved_tuple.GetLength(), values[i].size());
    EXPECT_EQ(std::memcmp(retrieved_tuple.GetData(), values[i].c_str(), values[i].size()), 0)
        << "Data mismatch for tuple " << i;
  }
}

TEST_F(TableHeapTest, ConcurrentInsertGetTest) {
  DiskManager* dm = new DiskManager("test.db");
  BufferPoolManager* bpm = new BufferPoolManager(9, dm);
  TableHeap* heap = new TableHeap(bpm, INVALID_PAGE_ID);

  const uint32_t kTupleSize = 100;
  const int kNumTuples = 10;
  std::vector<std::thread> threads;
  std::unordered_set<RID> rids;
  std::mutex rids_mutex;

  for (int t = 0; t < 10; ++t) {
    threads.emplace_back([&heap, &rids, &rids_mutex]() {
      for (int i = 0; i < kNumTuples; ++i) {
        std::string value(kTupleSize, static_cast<char>('A' + i));
        Tuple tuple(value.c_str(), kTupleSize);

        RID rid;
        bool insert_success = heap->InsertTuple(tuple, &rid);
        ASSERT_TRUE(insert_success) << "Insert failed for tuple " << i;
        {
          std::lock_guard<std::mutex> guard(rids_mutex);
          EXPECT_TRUE(rids.find(rid) == rids.end()) << "duplicate RID";
          rids.insert(rid);
        }
      }
    });
  }

  for (auto& t : threads) t.join();
  ASSERT_EQ(rids.size(), kNumTuples * 10);
}

TEST_F(TableHeapTest, ReopenExistingTableTest) {
  DiskManager* dm = new DiskManager("test.db");
  BufferPoolManager* bpm = new BufferPoolManager(10, dm);
  TableHeap* heap = new TableHeap(bpm, INVALID_PAGE_ID);

  std::vector<std::string> values;
  std::vector<RID> rids;

  for (int i = 0; i < 10; ++i) {
    std::string value = "tuple_" + std::to_string(i);
    Tuple tuple(value.c_str(), static_cast<uint32_t>(value.size()));

    RID rid;
    bool insert_success = heap->InsertTuple(tuple, &rid);
    ASSERT_TRUE(insert_success) << "Insert failed for tuple " << i;

    values.push_back(value);
    rids.push_back(rid);
  }

  TableHeap* heap2 = new TableHeap(bpm, heap->GetFirstPageId());
  EXPECT_EQ(heap2->GetFirstPageId(), heap->GetFirstPageId());

  for (int i = 10; i < 20; ++i) {
    std::string value = "tuple_" + std::to_string(i);
    Tuple tuple(value.c_str(), static_cast<uint32_t>(value.size()));

    RID rid;
    bool insert_success = heap2->InsertTuple(tuple, &rid);
    ASSERT_TRUE(insert_success) << "Insert failed for tuple " << i;

    values.push_back(value);
    rids.push_back(rid);
  }

  for (int i = 0; i < 20; ++i) {
    Tuple retrieved_tuple;
    bool get_success = heap2->GetTuple(rids[i], &retrieved_tuple);

    ASSERT_TRUE(get_success) << "Get failed for tuple " << i;
    EXPECT_EQ(retrieved_tuple.GetLength(), values[i].size());
    EXPECT_EQ(std::memcmp(retrieved_tuple.GetData(), values[i].c_str(), values[i].size()), 0)
        << "Data mismatch for tuple " << i;
  }
}