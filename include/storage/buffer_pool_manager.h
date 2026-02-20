#include <vector>
#include <unordered_map>
#include <mutex>
#include "storage/disk_scheduler.h"
#include "storage/page.h"
#include "storage/lruk_replacer.h"

using namespace std;
using namespace maye_sql;

namespace maye_sql {

class BufferPoolManager {
public:
    BufferPoolManager(size_t pool_size, DiskManager *disk_manager, size_t replacer_k = 2);
    ~BufferPoolManager();

    Page* FetchPage(page_id_t page_id);
    bool UnpinPage(page_id_t page_id, bool is_dirty);
    Page* NewPage(page_id_t *page_id);
    bool FlushPage(page_id_t page_id);
    bool DeletePage(page_id_t page_id);

private:
    size_t pool_size;
    Page *pages;
    std::unique_ptr<DiskScheduler> disk_scheduler;
    unordered_map<page_id_t, frame_id_t> page_table;
    unique_ptr<LRUK_Replacer> replacer;
    list<frame_id_t> free_list;
    
    mutex latch;
};

}