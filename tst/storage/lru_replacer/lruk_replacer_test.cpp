#include <gtest/gtest.h>
#include <thread>
#include <vector>
#include "storage/lruk_replacer.h"

using namespace maye_sql;
using namespace std;

TEST(LRUKReplacerTest, TestCorrectionist1) {
    LRUK_Replacer replacer(10, 3);
    replacer.RecordAccess(1);
    EXPECT_EQ(replacer.Size(), 0);
    replacer.SetEvictable(1,true);
    EXPECT_EQ(replacer.Size(), 1);
    replacer.RecordAccess(2);
    replacer.RecordAccess(2);
    replacer.RecordAccess(3);
    replacer.RecordAccess(1);
    replacer.RecordAccess(1);
    replacer.RecordAccess(1);
    replacer.SetEvictable(2,true);
    int* frame = new int;
    
    bool result = replacer.Evict(frame);
    EXPECT_EQ(result, true);
    EXPECT_EQ(*frame, 2);

    result = replacer.Evict(frame);
    EXPECT_EQ(result, true);
    EXPECT_EQ(*frame, 1);

    result = replacer.Evict(frame);
    EXPECT_EQ(result, false);
}

// If there are no victims in the "k" range, choose the last
TEST(LRUKReplacerTest, FIFOTest) {
    LRUK_Replacer replacer(10, 3);
    replacer.RecordAccess(1);
    replacer.RecordAccess(2); 
    
    replacer.SetEvictable(1, true);
    replacer.SetEvictable(2, true);
    
    int victim;
    replacer.Evict(&victim);
    EXPECT_EQ(victim, 1);
}

TEST(LRUKReplacerTest, ConcurrencyTest) {
    LRUK_Replacer replacer(1000, 2);
    std::vector<std::thread> threads;

    for (int i = 0; i < 10; ++i) {
        threads.emplace_back([&replacer, i]() {
            for (int j = 0; j < 100; ++j) {
                int frame = (i * 100) + j;
                replacer.RecordAccess(frame);
                replacer.SetEvictable(frame, true);
            }
        });
    }

    for (auto &t : threads) t.join();
    EXPECT_EQ(replacer.Size(), 1000);
}

TEST(LRUKReplacerTest, RemoveTest) {
    LRUK_Replacer replacer(10, 2);
    replacer.RecordAccess(1);
    replacer.SetEvictable(1, true);
    EXPECT_EQ(replacer.Size(), 1);
    
    replacer.Remove(1);
    EXPECT_EQ(replacer.Size(), 0);
    
    int victim;
    EXPECT_FALSE(replacer.Evict(&victim));
}