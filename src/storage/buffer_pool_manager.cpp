#include "storage/buffer_pool_manager.h"
#include "storage/page.h"

using namespace maye_sql;
using namespace std;

maye_sql::Page* BufferPoolManager::FetchPage(page_id_t page_id) {
    if (page_table.find(page_id) != page_table.end()) {
        replacer->RecordAccess(page_id);
        page_table[page_id];
    }

    return nullptr;
}

maye_sql::Page* BufferPoolManager::NewPage(page_id_t page_id) {
    if (page_table.find(page_id) != page_table.end()) {
        replacer->RecordAccess(page_id);
        page_table[page_id];
    }

    return nullptr;
}