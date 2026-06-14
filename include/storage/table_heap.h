#pragma once

#include "buffer_pool_manager.h"
#include "tuple.h"
#include "common/config.h"
#include "common/rid.h"

namespace maye_sql {

class TableHeap {
 public:
  TableHeap(BufferPoolManager* bpm, page_id_t first_page_id = INVALID_PAGE_ID);
  bool InsertTuple(const Tuple& tuple, RID* rid);
  bool GetTuple(const RID& rid, Tuple* tuple);
  bool DeleteTuple(const RID& rid);
  page_id_t GetFirstPageId() const {
    return first_page_id;
  }

 private:
  BufferPoolManager* bpm;
  page_id_t first_page_id;
  page_id_t last_page_id;
  mutex latch;
};

}  // namespace maye_sql