#include "common/config.h"

namespace maye_sql {

class Page {
    char data_[PAGE_SIZE];
    page_id_t page_id = INVALID_PAGE_ID;
    int pin_count = 0;
    bool is_dirty = false;
};

}