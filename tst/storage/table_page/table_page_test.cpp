#include <gtest/gtest.h>
#include <cstring>
#include "common/table_page.h"
#include "common/rid.h"
#include "storage/tuple.h"

namespace maye_sql {

TEST(TablePageTest, BasicInsertGetTest) {
  alignas(TablePage) uint8_t buffer[sizeof(TablePage)];
  std::memset(buffer, 0, sizeof(TablePage));

  auto* page = reinterpret_cast<TablePage*>(buffer);

  page->Init(10, -1);

  const char* raw_data = "HelloMayeSQL";
  uint32_t data_len = static_cast<uint32_t>(std::strlen(raw_data));
  Tuple tuple(raw_data, data_len);

  RID rid;

  bool insert_success = page->InsertTuple(tuple, &rid);

  ASSERT_TRUE(insert_success);
  EXPECT_EQ(rid.GetPageId(), 10);
  EXPECT_EQ(rid.GetSlotNum(), 0);

  Tuple retrieved_tuple;
  bool get_success = page->GetTuple(rid, &retrieved_tuple);

  ASSERT_TRUE(get_success);
  EXPECT_EQ(retrieved_tuple.GetLength(), data_len);
  EXPECT_EQ(std::memcmp(retrieved_tuple.GetData(), raw_data, data_len), 0);
}

TEST(TablePageTest, SlotReuseTest) {
  alignas(TablePage) uint8_t buffer[sizeof(TablePage)];
  auto* page = reinterpret_cast<TablePage*>(buffer);
  page->Init(101, -1);

  RID rid0, rid1, rid2;
  page->InsertTuple(Tuple("A", 1), &rid0);
  page->InsertTuple(Tuple("B", 1), &rid1);
  page->InsertTuple(Tuple("C", 1), &rid2);

  // Delete the middle one (Slot 1)
  page->MarkDelete(rid1);

  // Verify it's gone
  Tuple dummy;
  EXPECT_FALSE(page->GetTuple(rid1, &dummy));

  // Now insert a 4th tuple.
  // Depending on your logic, it will either use a new slot (Slot 3)
  // or reuse Slot 1. Let's see what yours does!
  RID rid3;
  page->InsertTuple(Tuple("D", 1), &rid3);

  // If you didn't implement reuse yet, this will be 3.
  // If you did, it might be 1.
  printf("DEBUG: Tuple 'D' assigned to Slot %u\n", rid3.GetSlotNum());
}

TEST(TablePageTest, FragmentationAndCompactionTest) {
  alignas(TablePage) uint8_t buffer[sizeof(TablePage)];
  auto* page = reinterpret_cast<TablePage*>(buffer);
  page->Init(102, -1);

  // 1. Fill the page with many small tuples
  std::vector<RID> rids;
  for (int i = 0; i < 100; ++i) {
    RID rid;
    page->InsertTuple(Tuple("data", 4), &rid);
    rids.push_back(rid);
  }

  // 2. Delete the first 50 tuples (creates 50 holes)
  for (int i = 0; i < 50; ++i) {
    page->MarkDelete(rids[i]);
  }

  // 3. Try to insert a large tuple (e.g., 100 bytes)
  // This will fail if your Insert doesn't call Compact()
  // because the "Gap" is small, but total free space is large.
  RID big_rid;
  bool success = page->InsertTuple(Tuple(std::string(100, 'X').c_str(), 100), &big_rid);

  if (!success) {
    printf("Page fragmented. Calling Compact manually...\n");
    page->Compact();
    success = page->InsertTuple(Tuple(std::string(100, 'X').c_str(), 100), &big_rid);
  }

  ASSERT_TRUE(success) << "Compaction failed to reclaim space!";
}

TEST(TablePageTest, BoundaryCollisionTest) {
  alignas(TablePage) uint8_t buffer[sizeof(TablePage)];
  auto* page = reinterpret_cast<TablePage*>(buffer);
  page->Init(103, -1);

  // Declare RIDs for both insertions
  RID first_rid;
  RID second_rid;

  // We want to fill the page until there are only a few bytes left.
  // Header(20) + 1 Slot(4) + Data(4000) = 4024 bytes used.
  // Remaining gap = 4096 - 4024 = 72 bytes.
  uint32_t big_size = 4000;
  char* big_data = new char[big_size];
  std::memset(big_data, 'A', big_size);  // Fill with dummy data

  Tuple big_tuple(big_data, big_size);
  ASSERT_TRUE(page->InsertTuple(big_tuple, &first_rid));

  // Create a real 70-byte buffer instead of a 7-byte string literal
  std::string seventy_byte_string(70, 'X');
  Tuple too_big_tuple(seventy_byte_string.c_str(), 70);

  // Now this will correctly fail because of the 4-byte slot overhead
  ASSERT_FALSE(page->InsertTuple(too_big_tuple, &second_rid));

  delete[] big_data;
}

}  // namespace maye_sql