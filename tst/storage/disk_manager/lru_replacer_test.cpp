#include <gtest/gtest.h>
#include <thread>
#include <vector>
#include "storage/lru_replacer.h"

using namespace std;
using namespace maye_sql;

// Just test some basic logic
TEST(LRUReplacerTest, TestCorrectionist1) {
    LRUReplacer lru(5);
    lru.Pin(50);

    EXPECT_EQ(lru.Size(), 0);

    lru.Unpin(50);
    lru.Pin(20);

    int* frame = new int;
    bool success = lru.Victim(frame);

    EXPECT_EQ(success, true);
    EXPECT_EQ(50, *frame);
}

// Test that the size should be 0
TEST(LRUReplacerTest, TestCorrectionist2) {
    LRUReplacer lru(5);
    lru.Pin(1);
    lru.Pin(2);
    lru.Pin(3);
    lru.Pin(4);

    EXPECT_EQ(lru.Size(), 0);
}

// Test that the size should be 4
TEST(LRUReplacerTest, TestCorrectionist3) {
    LRUReplacer lru(5);
    lru.Unpin(1);
    lru.Unpin(2);
    lru.Unpin(3);
    lru.Unpin(4);

    EXPECT_EQ(lru.Size(), 4);
}

TEST(LRUReplacerTest, ConcurrencyTest) {
    const int num_threads = 10;
    const int num_frames_per_thread = 1000;
    LRUReplacer lru(num_threads * num_frames_per_thread);
    vector<thread> threads;

    // Launch 10 threads to unpin frames in parallel
    for (int i = 0; i < num_threads; ++i) {
        threads.emplace_back([&lru, i]() { 
            for (int j = 0; j < num_frames_per_thread; ++j) {
                lru.Unpin(i * num_frames_per_thread + j);
            }
        });
    }

    for (auto &t : threads) t.join();

    // If thread-safe, the size must be exactly 10,000
    EXPECT_EQ(lru.Size(), num_threads * num_frames_per_thread);
}

// High contention test
TEST(LRUReplacerTest, ConcurrencyShuffleTest) {
    const int num_threads = 8;
    const int num_iterations = 1000;
    const int num_frames = 10;
    
    LRUReplacer lru(num_frames);
    std::vector<std::thread> threads;

    for (int i = 0; i < num_threads; ++i) {
        threads.emplace_back([&lru]() {
            for (int j = 0; j < num_iterations; ++j) {
                int frame_id = j % num_frames;
                
                // Randomly choose to Pin, Unpin, or Victim
                if (j % 3 == 0) {
                    lru.Unpin(frame_id);
                } else if (j % 3 == 1) {
                    lru.Pin(frame_id);
                } else {
                    int victim_id;
                    lru.Victim(&victim_id);
                }
            }
        });
    }

    for (auto &t : threads) t.join();
    
    EXPECT_LE(lru.Size(), static_cast<size_t>(num_frames));
}
