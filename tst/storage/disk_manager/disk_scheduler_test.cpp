#include <gtest/gtest.h>
#include <thread>
#include <vector>
#include "storage/disk_scheduler.h"

using namespace maye_sql;
using namespace std;

namespace maye_sql {

class DiskSchedulerTestPeer {
public:
    static size_t GetQueueSize(DiskScheduler &s) {
        std::lock_guard<std::mutex> lock(s.latch);
        return s.request_queue.size();
    }
};

TEST(DiskSchedulerTest, AsynchronousExecutionTest) {
    unique_ptr<DiskManager> disk_manager = std::make_unique<DiskManager>("test.db");
    DiskScheduler scheduler(disk_manager.get());

    char data[PAGE_SIZE] = "Test Data";
    
    DiskRequest req;
    req.is_write = true;
    req.data = data;
    req.page_id = 1;

    future<bool> fut = req.callback.get_future();
    chrono::time_point<std::chrono::steady_clock> start = std::chrono::steady_clock::now();
    scheduler.Schedule(std::move(req));

    std::chrono::time_point<std::chrono::steady_clock> end = std::chrono::steady_clock::now();
    
    int64_t duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
    
    EXPECT_LT(duration, 10) << "Schedule() took too long"; 

    std::future_status status = fut.wait_for(std::chrono::milliseconds(500));

    EXPECT_EQ(status, std::future_status::ready) << "Worker thread timed out";
    
    bool success = fut.get();
    EXPECT_TRUE(success);
}

TEST(DiskSchedulerTest, RoundTripTest) {
    const char* db_name = "test.db";
    auto disk_manager = std::make_unique<DiskManager>(db_name);
    DiskScheduler scheduler(disk_manager.get());

    page_id_t page_id = 1;
    char write_content[PAGE_SIZE];
    char read_content[PAGE_SIZE];
    
    // Fill buffer with a specific pattern
    std::memset(write_content, 'A', PAGE_SIZE);
    std::memset(read_content, 0, PAGE_SIZE);

    // 1. Schedule a Write
    DiskRequest write_req{true, write_content, page_id, std::promise<bool>()};
    std::future<bool> write_fut = write_req.callback.get_future();
    scheduler.Schedule(std::move(write_req));
    ASSERT_TRUE(write_fut.get()); // Wait for write to finish

    // 2. Schedule a Read
    DiskRequest read_req{false, read_content, page_id, std::promise<bool>()};
    std::future<bool> read_fut = read_req.callback.get_future();
    scheduler.Schedule(std::move(read_req));
    ASSERT_TRUE(read_fut.get()); // Wait for read to finish

    // 3. Compare
    EXPECT_EQ(std::memcmp(write_content, read_content, PAGE_SIZE), 0);
    
    std::remove(db_name);
}

TEST(DiskSchedulerTest, LargeVolumeTest) {
    const char* db_name = "test_volume.db";
    auto disk_manager = std::make_unique<DiskManager>(db_name);
    DiskScheduler scheduler(disk_manager.get());

    const int num_pages = 100;
    std::vector<std::future<bool>> futures;
    
    // We need to keep these buffers alive until the scheduler is done
    // In a real DB, these would be in the Buffer Pool.
    std::vector<std::unique_ptr<char[]>> buffers;

    for (int i = 0; i < num_pages; ++i) {
        auto buf = std::make_unique<char[]>(PAGE_SIZE);
        std::memset(buf.get(), i % 256, PAGE_SIZE);
        
        DiskRequest req{true, buf.get(), static_cast<page_id_t>(i), std::promise<bool>()};
        futures.push_back(req.callback.get_future());
        buffers.push_back(std::move(buf));
        
        scheduler.Schedule(std::move(req));
    }

    for (auto &f : futures) {
        EXPECT_TRUE(f.get());
    }

    std::remove(db_name);
}

TEST(DiskSchedulerTest, InterleavedTest) {
    const char* db_name = "test_interleaved.db";
    auto disk_manager = std::make_unique<DiskManager>(db_name);
    DiskScheduler scheduler(disk_manager.get());

    // Write Page 0
    char data0[PAGE_SIZE] = "Page 0 Data";
    DiskRequest w0{true, data0, 0, std::promise<bool>()};
    auto f1 = w0.callback.get_future();
    scheduler.Schedule(std::move(w0));

    // Write Page 1
    char data1[PAGE_SIZE] = "Page 1 Data";
    DiskRequest w1{true, data1, 1, std::promise<bool>()};
    auto f2 = w1.callback.get_future();
    scheduler.Schedule(std::move(w1));

    f1.get(); f2.get();

    // Now Read them back in reverse order
    char r_buf[PAGE_SIZE];
    DiskRequest r1{false, r_buf, 1, std::promise<bool>()};
    auto f3 = r1.callback.get_future();
    scheduler.Schedule(std::move(r1));
    
    f3.get();
    EXPECT_STREQ(r_buf, "Page 1 Data");

    std::remove(db_name);
}

}