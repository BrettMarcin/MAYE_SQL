#include <gtest/gtest.h>
#include <cstring>
#include "storage/disk_manager.h"
#include "common/config.h"

using namespace std;

TEST(DiskManagerTest, ReadWriteTest) {
    maye_sql::DiskManager dm("test.db");
    char data[4096];
    fill(data, data + 4096, 'B');

    dm.WritePage(0, data);

    char read_buffer[4096];
    dm.ReadPage(0, read_buffer);

    EXPECT_EQ(memcmp(data, read_buffer, 4096), 0);
    
    remove("test.db");
}

TEST(DiskManagerTest, ContentIntegrityTest) {
    std::string filename = "test_content.db";
    
    std::remove(filename.c_str());

    {
        maye_sql::DiskManager dm(filename);

        char page0_data[PAGE_SIZE];
        char page1_data[PAGE_SIZE];
        
        std::memset(page0_data, 'A', PAGE_SIZE);
        std::memset(page1_data, 'B', PAGE_SIZE);

        // 2. Write them to disk
        dm.WritePage(0, page0_data);
        dm.WritePage(1, page1_data);
    }

    {
        // 3. Re-open to verify persistence
        maye_sql::DiskManager dm(filename);
        char read_buffer[PAGE_SIZE];

        // 4. Verify Page 0
        dm.ReadPage(0, read_buffer);
        EXPECT_EQ(std::memcmp(read_buffer, "AAAAA", 5), 0);
        
        char expected0[PAGE_SIZE];
        std::memset(expected0, 'A', PAGE_SIZE);
        EXPECT_EQ(std::memcmp(read_buffer, expected0, PAGE_SIZE), 0);

        dm.ReadPage(1, read_buffer);
        char expected1[PAGE_SIZE];
        std::memset(expected1, 'B', PAGE_SIZE);
        EXPECT_EQ(std::memcmp(read_buffer, expected1, PAGE_SIZE), 0);
    }

    std::remove(filename.c_str());
}