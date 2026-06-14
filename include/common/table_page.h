/**
 * Slotted Page Layout:
 * ---------------------------------------------------------
 * | PAGE_ID (4) | PREV_PAGE_ID (4) | NEXT_PAGE_ID (4)     | <--- Header (Offset 0-11)
 * ---------------------------------------------------------
 * | FREE_SPACE_PTR (4) | SLOT_COUNT (4)                   | <--- Header (Offset 12-19)
 * ---------------------------------------------------------
 * | Slot 0 (Offset: 2, Len: 2) | Slot 1 (Off: 2, Len: 2)  | <--- Slot Array (Starts @ 20)
 * ---------------------------------------------------------
 * | Slot 2 ... |                                          |
 * ------------------- (GROWING DOWN) ----------------------
 * |                                                       |
 * |             FREE SPACE (The "GAP")                    |
 * |                                                       |
 * -------------------- (GROWING UP) -----------------------
 * |                                          |  Tuple 2   |
 * ---------------------------------------------------------
 * |          Tuple 1          |          Tuple 0          | <--- Data Area
 * ---------------------------------------------------------
 * (0)                                                 (4096)
 */

#pragma once

#include "storage/page.h"
#include "storage/tuple.h"

namespace maye_sql {

static constexpr size_t OFFSET_PAGE_ID = 0;
static constexpr size_t OFFSET_PREV_PAGE_ID = 4;
static constexpr size_t OFFSET_NEXT_PAGE_ID = 8;
static constexpr size_t OFFSET_FREE_SPACE = 12;
static constexpr size_t OFFSET_SLOT_COUNT = 16;
static constexpr size_t TABLE_PAGE_HEADER_SIZE = 20;

struct Slot {
  uint16_t offset;
  uint16_t length;
};

class TablePage : public Page {
 public:
  void Init(page_id_t page_id, page_id_t prev_page_id);

  bool InsertTuple(const Tuple& tuple, RID* rid);

  bool GetTuple(const RID& rid, Tuple* tuple);

  bool MarkDelete(const RID& rid);

  void Compact();

  inline page_id_t GetPrevPageId() {
    return *reinterpret_cast<page_id_t*>(this->data + OFFSET_PREV_PAGE_ID);
  }

  inline void SetPrevPageId(page_id_t prev_id) {
    *reinterpret_cast<page_id_t*>(this->data + OFFSET_PREV_PAGE_ID) = prev_id;
  }

  inline page_id_t GetNextPageId() {
    return *reinterpret_cast<page_id_t*>(this->data + OFFSET_NEXT_PAGE_ID);
  }

  inline void SetNextPageId(page_id_t next_id) {
    *reinterpret_cast<page_id_t*>(this->data + OFFSET_NEXT_PAGE_ID) = next_id;
  }

  uint32_t GetSlotCount() {
    return *reinterpret_cast<uint32_t*>(data + 16);
  }

  uint32_t GetFreeSpacePointer() {
    return *reinterpret_cast<uint32_t*>(data + 12);
  }

  void SetSlotCount(uint32_t count) {
    *reinterpret_cast<uint32_t*>(data + 16) = count;
  }

  void SetFreeSpacePointer(uint32_t pointer) {
    *reinterpret_cast<uint32_t*>(data + 12) = pointer;
  }

  inline page_id_t GetPageId() {
    return *reinterpret_cast<page_id_t*>(data + OFFSET_PAGE_ID);
  }

 private:
  // page_id_t page_id;
  // page_id_t prev_page_id;
  // page_id_t next_page_id;
};

}  // namespace maye_sql