#include "common/table_page.h"
#include "common/config.h"

using namespace maye_sql;

void TablePage::Init(page_id_t page_id, page_id_t prev_page_id) {
  std::memset(this->data, 0, PAGE_SIZE);

  *reinterpret_cast<page_id_t*>(this->data + OFFSET_PAGE_ID) = page_id;
  *reinterpret_cast<page_id_t*>(this->data + OFFSET_PREV_PAGE_ID) = prev_page_id;
  *reinterpret_cast<page_id_t*>(this->data + OFFSET_NEXT_PAGE_ID) = INVALID_PAGE_ID;

  *reinterpret_cast<uint32_t*>(this->data + OFFSET_FREE_SPACE) = PAGE_SIZE;

  *reinterpret_cast<uint32_t*>(this->data + OFFSET_SLOT_COUNT) = 0;
  SetFreeSpacePointer(4096);
  SetSlotCount(0);
}

bool TablePage::InsertTuple(const Tuple& tuple, RID* rid) {
  // Use Getters that read from the byte array, NOT member variables
  uint32_t slot_count = GetSlotCount();
  uint32_t free_ptr = GetFreeSpacePointer();
  uint32_t current_top = TABLE_PAGE_HEADER_SIZE + (slot_count * sizeof(Slot));

  if (free_ptr - current_top < tuple.GetLength() + sizeof(Slot)) {
    return false;
  }

  uint32_t new_offset = free_ptr - tuple.GetLength();

  // Make sure 'data' is the start of the 4096 bytes
  std::memcpy(data + new_offset, tuple.GetData(), tuple.GetLength());

  Slot* new_slot_ptr = reinterpret_cast<Slot*>(data + current_top);
  new_slot_ptr->offset = static_cast<uint16_t>(new_offset);
  new_slot_ptr->length = static_cast<uint16_t>(tuple.GetLength());

  SetFreeSpacePointer(new_offset);
  SetSlotCount(slot_count + 1);

  // Use GetPageId() to read from the header bytes
  rid->Set(GetPageId(), slot_count);

  return true;
}

bool TablePage::GetTuple(const RID& rid, Tuple* tuple) {
  uint32_t slot_num = rid.GetSlotNum();
  if (slot_num >= GetSlotCount()) {
    return false;
  }

  uint32_t slot_offset = TABLE_PAGE_HEADER_SIZE + (slot_num * sizeof(Slot));
  Slot* slot = reinterpret_cast<Slot*>(data + slot_offset);

  if (slot->length == 0) {
    return false;
  }

  *tuple = Tuple(data + slot->offset, slot->length, rid);

  return true;
}

bool TablePage::MarkDelete(const RID& rid) {
  uint32_t slot_num = rid.GetSlotNum();

  if (slot_num >= GetSlotCount()) {
    return false;
  }

  uint32_t slot_offset = TABLE_PAGE_HEADER_SIZE + (slot_num * sizeof(Slot));
  Slot* slot = reinterpret_cast<Slot*>(data + slot_offset);
  if (slot->length == 0) {
    return false;
  }

  slot->length = 0;

  return true;
}

// TODO: Include a background thread that cleans up these pages
void TablePage::Compact() {
  uint32_t slot_count = GetSlotCount();
  uint32_t next_free_ptr = PAGE_SIZE;

  for (uint32_t i = 0; i < slot_count; i++) {
    Slot* slot = reinterpret_cast<Slot*>(data + TABLE_PAGE_HEADER_SIZE + (i * sizeof(Slot)));

    if (slot->length == 0) {
      continue;
    }

    uint32_t new_offset = next_free_ptr - slot->length;
    std::memmove(data + new_offset, data + slot->offset, slot->length);
    slot->offset = static_cast<uint16_t>(new_offset);

    next_free_ptr = new_offset;
  }

  SetFreeSpacePointer(next_free_ptr);
}