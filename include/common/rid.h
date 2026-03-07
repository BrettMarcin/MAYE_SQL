#pragma once

#include <cstdint>
#include <string>
#include "common/config.h"

namespace maye_sql {

class RID {
 public:
  RID() : page_id(INVALID_PAGE_ID), slot_num(0) {}

  RID(page_id_t page_id, uint32_t slot_num) : page_id(page_id), slot_num(slot_num) {}

  inline auto GetPageId() const -> page_id_t {
    return page_id;
  }
  inline uint32_t GetSlotNum() const {
    return slot_num;
  }

  bool operator==(const RID& other) const {
    return page_id == other.page_id && slot_num == other.slot_num;
  }
  std::string ToString() const {
    return "Page: " + std::to_string(page_id) + ", Slot: " + std::to_string(slot_num);
  }

 private:
  page_id_t page_id;
  uint32_t slot_num;
};

}  // namespace maye_sql