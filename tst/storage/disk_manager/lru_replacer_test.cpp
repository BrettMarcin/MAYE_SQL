#include <gtest/gtest.h>
#include "storage/lru_replacer.h"

using namespace std;
using namespace maye_sql;

// 
TEST(LRUReplacerTest, TestCorrectionist1) {
    LRUReplacer lru(5);
    lru.Pin(50);

    EXPECT_EQ(lru.Size(), 0);

    lru.Unpin(50);
    lru.Pin(20);

    int* frame = nullptr;
    bool success = lru.Victim(frame);

    EXPECT_EQ(success, true);
    EXPECT_EQ(50, *frame);


    // char data[4096];
    // fill(data, data + 4096, 'B');

    // dm.WritePage(0, data);

    // char read_buffer[4096];
    // dm.ReadPage(0, read_buffer);

    // EXPECT_EQ(memcmp(data, read_buffer, 4096), 0);
    
    // remove("test.db");
}
